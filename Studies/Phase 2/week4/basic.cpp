#include "basic.h"
#include <iostream>
#include <chrono>
#include <iomanip>


// Brute-Force 매칭 — KNN(최근접 이웃 탐색)을 전수 비교로 구현
//
// KNN은 "가장 가까운 k개를 찾아라"라는 개념이고,
// BF(Brute-Force)는 모든 쌍을 비교하여 정확하게 찾는 구현 방법이다.
// (빠른 근사 탐색이 필요하면 FLANN을 사용)
//
// 동작: desc1의 각 디스크립터에 대해 desc2의 모든 디스크립터와 거리를 계산,
//   가장 거리가 작은 것을 매칭 결과로 반환.
//   시간 복잡도: O(N × M × D)  (N,M: 각 이미지 특징점 수, D: 디스크립터 차원)
//
// normType에 따른 거리 계산:
//   NORM_HAMMING: XOR 후 1의 개수 (이진 디스크립터용 — ORB, BRIEF)
//     → 해밍 거리. CPU에서 popcount 명령어로 매우 빠름
//   NORM_L2: 유클리드 거리 (실수 디스크립터용 — SIFT, SURF)
//
// crossCheck=false: desc1→desc2 단방향 매칭
//   true로 하면 desc2→desc1도 같은 결과인 경우만 유지 (더 정확하지만 2배 느림)
double FeatureMatchingBasic::matchBruteForce(const cv::Mat& descriptors1,
                                             const cv::Mat& descriptors2,
                                             std::vector<cv::DMatch>& matches, int normType)
{
    // [OpenCV] cv::BFMatcher matcher(normType, false);
    // [OpenCV] matcher.match(descriptors1, descriptors2, matches);
    // 아래는 동일한 동작의 직접 구현
    matches.clear();

    auto start = std::chrono::high_resolution_clock::now();

    // BF 매칭을 직접 구현: desc1의 각 디스크립터에 대해 desc2 전체를 순회하며
    // 거리가 가장 작은 것을 찾는다 (KNN k=1의 전수 비교 버전)
    for (int i = 0; i < descriptors1.rows; i++)
    {
        int best_j = 0;
        float best_dist = std::numeric_limits<float>::max();

        for (int j = 0; j < descriptors2.rows; j++)
        {
            float dist = 0.0f;

            if (normType == cv::NORM_HAMMING)
            {
                // 해밍 거리: 바이트별 XOR → 1인 비트 개수 합산
                int hamming = 0;
                for (int k = 0; k < descriptors1.cols; k++)
                {
                    uint8_t xor_val = descriptors1.at<uint8_t>(i, k)
                                    ^ descriptors2.at<uint8_t>(j, k);
                    while (xor_val)
                    {
                        hamming += xor_val & 1;
                        xor_val >>= 1;
                    }
                }
                dist = (float)hamming;
            }
            else
            {
                // 유클리드(L2) 거리: sqrt(Σ(a_i - b_i)²)
                double sum_sq = 0.0;
                for (int k = 0; k < descriptors1.cols; k++)
                {
                    double diff = descriptors1.at<float>(i, k)
                                - descriptors2.at<float>(j, k);
                    sum_sq += diff * diff;
                }
                dist = (float)std::sqrt(sum_sq);
            }

            if (dist < best_dist)
            {
                best_dist = dist;
                best_j = j;
            }
        }

        // DMatch(queryIdx, trainIdx, distance)
        matches.push_back(cv::DMatch(i, best_j, best_dist));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return duration.count() / 1000.0;
}

// 매칭 품질 평가 — 평균 디스크립터 거리 계산
//   거리가 작을수록 디스크립터가 유사 → 좋은 매칭
//   DMatch.distance: 매칭된 두 디스크립터 간의 거리
//     ORB(HAMMING): 0~256 범위 (비트 차이 수)
//     SIFT(L2): 연속 값
double FeatureMatchingBasic::evaluateMatchQuality(const std::vector<cv::DMatch>& matches)
{
    if (matches.empty())
        return 0.0;

    double total_distance = 0.0;
    for (const auto& match : matches)
    {
        total_distance += match.distance;
    }

    return total_distance / matches.size();
}

// FLANN 매칭 — KNN을 근사 탐색으로 구현 (BF의 대안)
//
// FLANN = Fast Library for Approximate Nearest Neighbors
//   BF와 FLANN은 둘 다 KNN(가장 가까운 k개 찾기)의 구현 방법이다.
//   BF가 모든 쌍을 전수 비교하는 반면, FLANN은 KD-tree/LSH 등의
//   자료구조로 비슷한 것끼리 미리 묶어두고 해당 영역만 탐색한다.
//   결과는 동일한 형태(DMatch)로 반환되며, 내부 탐색 방식만 다르다.
//
// 자료구조와 디스크립터 유형은 고정 조합:
//   이진 디스크립터 (ORB, BRIEF) → LSH (해시 기반, 비트열에 최적화)
//   실수 디스크립터 (SIFT, SURF) → KD-Tree (공간 분할, 연속 값에 최적화)
//
// 주의: FLANN의 기본 인덱스는 float 타입만 지원하므로,
//   ORB 같은 이진 디스크립터(CV_8U)는 float으로 변환 필요.
//   이진 디스크립터에는 LSH(Locality-Sensitive Hashing) 인덱스가 더 적합.
//
// BF vs FLANN 선택:
//   - 특징점 < 1000: BF가 충분히 빠름 (오버헤드 적음)
//   - 특징점 > 1000: FLANN이 유리 (인덱스 구축 비용을 상쇄)
double FeatureMatchingBasic::matchFLANN(const cv::Mat& descriptors1, const cv::Mat& descriptors2,
                                        std::vector<cv::DMatch>& matches)
{
    cv::Mat desc1_float, desc2_float;

    if (descriptors1.type() != CV_32F)
    {
        descriptors1.convertTo(desc1_float, CV_32F);
        descriptors2.convertTo(desc2_float, CV_32F);
    }
    else
    {
        desc1_float = descriptors1;
        desc2_float = descriptors2;
    }

    cv::FlannBasedMatcher matcher;

    auto start = std::chrono::high_resolution_clock::now();
    matcher.match(desc1_float, desc2_float, matches);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return duration.count() / 1000.0;
}

// Lowe's Ratio Test — 모호한 매칭을 제거하는 핵심 필터
//
// 왜 KNN(k=2)이 필요한가?
//   일반 매칭(k=1)은 가장 가까운 1개만 반환하므로
//   "이 매칭이 진짜 좋은 건지" 판단할 기준이 없다.
//   k=2로 1등과 2등을 함께 받으면, 1등이 얼마나 확실한지 비교할 수 있다.
//
// knn_matches의 자료형이 std::vector<std::vector<cv::DMatch>>인 이유:
//   바깥 벡터: 모든 query 디스크립터 (N개)
//   안쪽 벡터: 각 query에 대한 k개의 후보 매칭
//   knn_matches[i][0] = i번째 특징점의 1등 매칭 (best)
//   knn_matches[i][1] = i번째 특징점의 2등 매칭 (second-best)
//
// 원리 (David Lowe, 2004):
//   best_distance / second_distance < ratio_thresh → 좋은 매칭
//
//   확실한 매칭: 1등=15, 2등=82 → ratio=0.18 → 1등이 압도적 → 수락
//   애매한 매칭: 1등=45, 2등=48 → ratio=0.94 → 구분 안됨 → 거부
//
// ratio_thresh 선택:
//   0.7: Lowe의 권장값. 잘못된 매칭의 90%를 제거하면서 좋은 매칭의 95% 유지
//   0.6: 더 엄격 (precision↑, recall↓)
//   0.8: 더 느슨 (precision↓, recall↑)
int FeatureMatchingBasic::ratioTest(const cv::Mat& descriptors1, const cv::Mat& descriptors2,
                                    std::vector<cv::DMatch>& good_matches, float ratio_thresh)
{
    // [OpenCV] cv::BFMatcher matcher(cv::NORM_HAMMING);
    // [OpenCV] matcher.knnMatch(descriptors1, descriptors2, knn_matches, 2);
    // kNN(k=2) 매칭을 직접 구현 — 각 디스크립터에 대해 가장 가까운 2개를 찾음
    // 1등(best)과 2등(second)의 거리 비율로 매칭 품질을 판단하기 위해 k=2 필요
    std::vector<std::vector<cv::DMatch>> knn_matches;
    for (int i = 0; i < descriptors1.rows; i++)
    {
        // 1등, 2등 후보를 추적
        int best_j = -1, second_j = -1;
        float best_dist = std::numeric_limits<float>::max();
        float second_dist = std::numeric_limits<float>::max();

        for (int j = 0; j < descriptors2.rows; j++)
        {
            // 해밍 거리 계산: XOR → popcount
            int hamming = 0;
            for (int k = 0; k < descriptors1.cols; k++)
            {
                uint8_t xor_val = descriptors1.at<uint8_t>(i, k)
                                ^ descriptors2.at<uint8_t>(j, k);
                while (xor_val)
                {
                    hamming += xor_val & 1;
                    xor_val >>= 1;
                }
            }
            float dist = (float)hamming;

            // 1등, 2등 갱신
            if (dist < best_dist)
            {
                second_dist = best_dist;
                second_j = best_j;
                best_dist = dist;
                best_j = j;
            }
            else if (dist < second_dist)
            {
                second_dist = dist;
                second_j = j;
            }
        }

        std::vector<cv::DMatch> pair;
        if (best_j >= 0)
            pair.push_back(cv::DMatch(i, best_j, best_dist));
        if (second_j >= 0)
            pair.push_back(cv::DMatch(i, second_j, second_dist));
        knn_matches.push_back(pair);
    }

    good_matches.clear();
    for (const auto& match_pair : knn_matches)
    {
        // 디스크립터가 하나뿐이면 2등이 없으므로 비교 불가 → 건너뜀
        if (match_pair.size() < 2)
            continue;

        // 1등 거리가 2등 거리 대비 충분히 작으면 → 확실한 매칭 → 유지
        if (match_pair[0].distance < ratio_thresh * match_pair[1].distance)
        {
            good_matches.push_back(match_pair[0]);
        }
    }

    return good_matches.size();
}

void FeatureMatchingBasic::visualizeMatches(const cv::Mat& img1,
                                            const std::vector<cv::KeyPoint>& kp1,
                                            const cv::Mat& img2,
                                            const std::vector<cv::KeyPoint>& kp2,
                                            const std::vector<cv::DMatch>& matches, cv::Mat& output)
{
    // 매칭 선 그리기
    cv::drawMatches(img1, kp1, img2, kp2, matches, output, cv::Scalar(0, 255, 0),  // 매칭 선: 녹색
                    cv::Scalar(255, 0, 0),                                         // 특징점: 파란색
                    std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    // 정보 텍스트
    std::string info = "Matches: " + std::to_string(matches.size());
    cv::putText(output, info, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0,
                cv::Scalar(0, 255, 0), 2);
}

// RANSAC으로 기하학적 outlier 제거 — 호모그래피 모델에 맞지 않는 매칭 제거
//
// Ratio Test와 RANSAC의 관계:
//   Ratio Test → "이 매칭, 확신 있어?" (디스크립터 유사도, 개별 판단)
//   RANSAC     → "이 매칭, 전체 그림에 맞아?" (기하학적 일관성, 전체 판단)
//   둘은 보는 관점이 다르므로 순서대로 둘 다 적용한다.
//
// RANSAC (Random Sample Consensus):
//   Consensus(합의) = 가장 많은 매칭이 동의하는 모델을 정답으로 삼는다
//   ① 매칭 중 4개를 랜덤 선택 → 호모그래피 H 계산 (4점 × 2방정식 = 8 DOF 결정)
//   ② 나머지 매칭에 H 적용 → 재투영 오차 < ransac_thresh면 inlier
//   ③ N번 반복 → inlier가 가장 많은(합의가 가장 큰) H를 최종 모델로 선택
//   ④ 최종 H의 inlier만 남기고 outlier 제거
//
// 호모그래피(H)란?
//   이미지1의 점을 이미지2의 점으로 변환하는 3x3 행렬: p2 = H · p1
//   이동, 회전, 크기변환, 원근변형을 하나의 행렬로 표현한다.
//   평면 가정이 성립하면 (벽면, 바닥, 포스터 등) 효과적.
//   비평면 장면에서는 Essential Matrix(E) 기반 RANSAC이 더 적합.
//
// ransac_thresh: 재투영 오차 허용치 (픽셀)
//   이미지1 점에 H를 적용한 예측 좌표와, 실제 매칭된 이미지2 좌표의 차이.
//   이 오차가 thresh 이내면 inlier, 초과하면 outlier.
//   3.0: 일반적인 값. ↓ 엄격, ↑ 느슨
//
// 반환: inlier 비율 (0.0~1.0). 0.5 이상이면 대체로 양호한 매칭.
// 파라미터의 const 유무로 입력/출력을 구분:
//   const &matches       → 읽기만 하는 입력 (이미 데이터가 채워져 있음)
//   &inlier_matches      → 함수가 채울 출력 (빈 벡터로 넘기면 됨)
//   &homography          → 함수가 채울 출력 (추정된 호모그래피 행렬)
double FeatureMatchingBasic::filterRANSAC(const std::vector<cv::KeyPoint>& keypoints1,
                                          const std::vector<cv::KeyPoint>& keypoints2,
                                          const std::vector<cv::DMatch>& matches,
                                          std::vector<cv::DMatch>& inlier_matches,
                                          cv::Mat& homography, double ransac_thresh)
{
    // 호모그래피 계산에 최소 4점이 필요 (4점 × 2방정식 = 8 DOF)
    // 부족하면 단위 행렬(변환 없음)을 반환 — 안전한 기본값
    if (matches.size() < 4)
    {
        homography = cv::Mat::eye(3, 3, CV_64F);
        return 0.0;
    }

    // DMatch에서 2D 점 좌표(.pt)만 추출
    //   queryIdx: desc1(이미지1)의 인덱스 — "매칭 상대를 찾아줘"라고 질문하는 쪽
    //   trainIdx: desc2(이미지2)의 인덱스 — 검색 대상 데이터베이스 쪽
    //   .pt: KeyPoint에서 (x, y) 좌표만 꺼냄 (findHomography가 Point2f를 요구)
    std::vector<cv::Point2f> points1, points2;
    for (const auto& match : matches)
    {
        points1.push_back(keypoints1[match.queryIdx].pt);
        points2.push_back(keypoints2[match.trainIdx].pt);
    }
    // points1[i]와 points2[i]는 같은 인덱스끼리 매칭 쌍을 이룸

    // RANSAC으로 호모그래피 추정 + inlier/outlier 분류
    // inlier_mask: 빈 벡터로 넘기면 findHomography가 결과를 채워주는 출력 파라미터
    //   inlier_mask[i] = 1이면 inlier(H에 부합), 0이면 outlier
    std::vector<uchar> inlier_mask;
    homography = cv::findHomography(points1, points2, cv::RANSAC, ransac_thresh, inlier_mask);

    // inlier만 추출
    // inlier_mask와 matches는 같은 순서로 만들어졌으므로 인덱스가 1:1 대응
    inlier_matches.clear();
    for (size_t i = 0; i < matches.size(); i++)
    {
        // uchar 타입에서 0이면 false(outlier), 0이 아니면 true(inlier)
        if (inlier_mask[i])
        {
            inlier_matches.push_back(matches[i]);
        }
    }

    double inlier_ratio = (double)inlier_matches.size() / matches.size();

    return inlier_ratio;
}

void FeatureMatchingBasic::demoPipeline(const cv::Mat& img1, const cv::Mat& img2)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "전체 매칭 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Step 1: 특징점 검출
    std::cout << "1️⃣  특징점 검출 (ORB)..." << std::endl;
    const int kMaxFeatures = 1000;  // ORB 최대 특징점 수
    cv::Ptr<cv::ORB> orb = cv::ORB::create(kMaxFeatures);

    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat desc1, desc2;

    orb->detectAndCompute(img1, cv::noArray(), kp1, desc1);
    orb->detectAndCompute(img2, cv::noArray(), kp2, desc2);

    std::cout << "   이미지1: " << kp1.size() << "개" << std::endl;
    std::cout << "   이미지2: " << kp2.size() << "개\n" << std::endl;

    // Step 2: 초기 매칭
    std::cout << "2️⃣  Brute-Force 매칭..." << std::endl;
    std::vector<cv::DMatch> matches;
    double time_match = matchBruteForce(desc1, desc2, matches, cv::NORM_HAMMING);

    std::cout << "   매칭 개수: " << matches.size() << std::endl;
    std::cout << "   매칭 시간: " << time_match << " ms" << std::endl;
    std::cout << "   평균 거리: " << evaluateMatchQuality(matches) << "\n" << std::endl;

    // Step 3: Ratio Test
    std::cout << "3️⃣  Lowe's Ratio Test (0.7)..." << std::endl;
    std::vector<cv::DMatch> good_matches;
    int count_ratio = ratioTest(desc1, desc2, good_matches, kLoweRatioThreshold);

    std::cout << "   필터링 전: " << matches.size() << "개" << std::endl;
    std::cout << "   필터링 후: " << count_ratio << "개" << std::endl;
    double ratio_kept = 100.0 * count_ratio / matches.size();
    std::cout << "   유지율: " << std::fixed << std::setprecision(1) << ratio_kept << "%\n"
              << std::endl;

    // Step 4: RANSAC
    std::cout << "4️⃣  RANSAC outlier 제거..." << std::endl;
    std::vector<cv::DMatch> inlier_matches;
    cv::Mat H;
    double inlier_ratio = filterRANSAC(kp1, kp2, good_matches, inlier_matches, H, 3.0);

    std::cout << "   Ratio Test 후: " << good_matches.size() << "개" << std::endl;
    std::cout << "   RANSAC 후: " << inlier_matches.size() << "개" << std::endl;
    std::cout << "   Inlier 비율: " << std::setprecision(1) << inlier_ratio * 100 << "%\n"
              << std::endl;

    // 결과 저장
    cv::Mat output_before, output_after;
    visualizeMatches(img1, kp1, img2, kp2, good_matches, output_before);
    visualizeMatches(img1, kp1, img2, kp2, inlier_matches, output_after);

    cv::imwrite("matches_before_ransac.png", output_before);
    cv::imwrite("matches_after_ransac.png", output_after);

    std::cout << "💾 시각화 저장:" << std::endl;
    std::cout << "   - matches_before_ransac.png" << std::endl;
    std::cout << "   - matches_after_ransac.png\n" << std::endl;

    std::cout << "💡 매칭 파이프라인 요약:" << std::endl;
    std::cout << "   1. 검출: " << kp1.size() << " × " << kp2.size() << " 특징점" << std::endl;
    std::cout << "   2. BF 매칭: " << matches.size() << "개" << std::endl;
    std::cout << "   3. Ratio Test: " << good_matches.size() << "개" << std::endl;
    std::cout << "   4. RANSAC: " << inlier_matches.size() << "개 (최종)" << std::endl;
}

// 메인 함수
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  특징점 매칭 기본 데모 (Week 4)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 생성
    const int kImageHeight = 400;
    const int kImageWidth = 600;
    cv::Mat img1 = cv::Mat::zeros(kImageHeight, kImageWidth, CV_8UC1);
    cv::Mat img2 = cv::Mat::zeros(kImageHeight, kImageWidth, CV_8UC1);

    // 이미지 1: 체커보드
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 12; j++)
        {
            if ((i + j) % 2 == 0)
            {
                cv::rectangle(img1, cv::Point(j * 50, i * 50),
                              cv::Point((j + 1) * 50, (i + 1) * 50), cv::Scalar(255), -1);
            }
        }
    }

    // 이미지 2: 약간 변형된 체커보드 (회전 + 이동)
    cv::Mat M = cv::getRotationMatrix2D(cv::Point(300, 200), 15, 1.0);  // 15도 회전
    M.at<double>(0, 2) += 30;                                           // X 이동
    M.at<double>(1, 2) += 20;                                           // Y 이동
    cv::warpAffine(img1, img2, M, img2.size());

    // 노이즈 추가
    cv::Mat noise = cv::Mat(img2.size(), CV_8UC1);
    cv::randn(noise, 0, 10);
    img2 += noise;

    std::cout << "📸 테스트 이미지 생성 완료" << std::endl;
    std::cout << "   이미지1: 원본 체커보드 (600x400)" << std::endl;
    std::cout << "   이미지2: 회전(15°) + 이동 + 노이즈\n" << std::endl;

    // 💡 매칭이란?
    std::cout << "💡 [교육] 특징점 매칭이란?" << std::endl;
    std::cout << "   두 이미지에서 '같은 3D 점'에 해당하는 특징점 쌍 찾기" << std::endl;
    std::cout << "   디스크립터 벡터의 거리가 작으면 → 같은 점!" << std::endl;
    std::cout << "   - ORB (이진): 해밍 거리 (XOR 비트 수)" << std::endl;
    std::cout << "   - SIFT (실수): 유클리드 거리 (L2 노름)\n" << std::endl;

    // 전체 파이프라인 실행
    FeatureMatchingBasic::demoPipeline(img1, img2);

    // 💡 Ratio Test 원리 (quiz 문제 2에서 사용!)
    std::cout << "\n💡 [교육] Lowe's Ratio Test 원리 (quiz 문제 2에서 사용!):" << std::endl;
    std::cout << "   kNN(k=2)로 최근접 2개 매칭을 찾은 뒤:" << std::endl;
    std::cout << "   best_dist / second_dist < 0.7 이면 좋은 매칭" << std::endl;
    std::cout << "   → 두 번째 후보와 차이가 클수록 확실한 매칭!" << std::endl;
    std::cout << "   임계값↑(0.8): 더 많이 통과 (recall↑, precision↓)" << std::endl;
    std::cout << "   임계값↓(0.6): 더 적게 통과 (recall↓, precision↑)\n" << std::endl;

    // 💡 RANSAC 원리 (quiz 문제 3에서 사용!)
    std::cout << "💡 [교육] RANSAC 원리 (quiz 문제 3에서 사용!):" << std::endl;
    std::cout << "   1) 랜덤으로 4개 점 선택 → 호모그래피 계산" << std::endl;
    std::cout << "   2) 나머지 점들이 이 모델에 맞는지 확인 (inlier)" << std::endl;
    std::cout << "   3) N회 반복 → 가장 많은 inlier를 가진 모델 선택" << std::endl;
    std::cout << "   → outlier(잘못된 매칭)에 강건한 추정!\n" << std::endl;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ Week 4 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md 이론 읽기 (BF/FLANN, Ratio Test, RANSAC)" << std::endl;
    std::cout << "   2. quiz_easy.cpp — 해밍 거리, Ratio Test, RANSAC" << std::endl;
    std::cout << "   3. my_basic.cpp — Step 1~7 순서대로 직접 구현" << std::endl;
    std::cout << "   4. quiz_medium.cpp — Essential Matrix, Homography DLT" << std::endl;
    std::cout << "   5. PRACTICE.md — 실제 스테레오 매칭 실습\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
