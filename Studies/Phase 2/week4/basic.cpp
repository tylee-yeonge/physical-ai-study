#include "basic.h"
#include <iostream>
#include <chrono>
#include <iomanip>


double FeatureMatchingBasic::matchBruteForce(const cv::Mat& descriptors1,
                                             const cv::Mat& descriptors2,
                                             std::vector<cv::DMatch>& matches, int normType)
{
    // Brute-Force 매처 생성
    cv::BFMatcher matcher(normType, false);  // crossCheck=false

    auto start = std::chrono::high_resolution_clock::now();

    // 매칭 수행
    matcher.match(descriptors1, descriptors2, matches);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return duration.count() / 1000.0;  // ms
}

double FeatureMatchingBasic::matchFLANN(const cv::Mat& descriptors1, const cv::Mat& descriptors2,
                                        std::vector<cv::DMatch>& matches)
{
    // FLANN은 float 타입 필요
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

    // FLANN 매처 (LSH for binary descriptors)
    cv::FlannBasedMatcher matcher;

    auto start = std::chrono::high_resolution_clock::now();

    matcher.match(desc1_float, desc2_float, matches);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return duration.count() / 1000.0;
}

int FeatureMatchingBasic::ratioTest(const cv::Mat& descriptors1, const cv::Mat& descriptors2,
                                    std::vector<cv::DMatch>& good_matches, float ratio_thresh)
{
    // kNN 매칭 (k=2, 최근접 2개)
    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<std::vector<cv::DMatch>> knn_matches;

    matcher.knnMatch(descriptors1, descriptors2, knn_matches, 2);

    // Lowe's Ratio Test
    good_matches.clear();
    for (const auto& match_pair : knn_matches)
    {
        if (match_pair.size() < 2)
            continue;

        // 최근접 거리 / 차근접 거리 < ratio_thresh
        if (match_pair[0].distance < ratio_thresh * match_pair[1].distance)
        {
            good_matches.push_back(match_pair[0]);
        }
    }

    return good_matches.size();
}

double FeatureMatchingBasic::filterRANSAC(const std::vector<cv::KeyPoint>& keypoints1,
                                          const std::vector<cv::KeyPoint>& keypoints2,
                                          const std::vector<cv::DMatch>& matches,
                                          std::vector<cv::DMatch>& inlier_matches,
                                          cv::Mat& homography, double ransac_thresh)
{
    if (matches.size() < 4)
    {
        // 호모그래피 계산에는 최소 4개 점 필요
        homography = cv::Mat::eye(3, 3, CV_64F);
        return 0.0;
    }

    // 매칭된 점들 추출
    std::vector<cv::Point2f> points1, points2;
    for (const auto& match : matches)
    {
        points1.push_back(keypoints1[match.queryIdx].pt);
        points2.push_back(keypoints2[match.trainIdx].pt);
    }

    // RANSAC으로 호모그래피 계산
    std::vector<uchar> inlier_mask;
    homography = cv::findHomography(points1, points2, cv::RANSAC, ransac_thresh, inlier_mask);

    // Inlier 매칭만 선택
    inlier_matches.clear();
    for (size_t i = 0; i < matches.size(); i++)
    {
        if (inlier_mask[i])
        {
            inlier_matches.push_back(matches[i]);
        }
    }

    // Inlier 비율 계산
    double inlier_ratio = (double)inlier_matches.size() / matches.size();

    return inlier_ratio;
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

void FeatureMatchingBasic::demoPipeline(const cv::Mat& img1, const cv::Mat& img2)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "전체 매칭 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Step 1: 특징점 검출
    std::cout << "1️⃣  특징점 검출 (ORB)..." << std::endl;
    cv::Ptr<cv::ORB> orb = cv::ORB::create(1000);

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
    cv::Mat img1 = cv::Mat::zeros(400, 600, CV_8UC1);
    cv::Mat img2 = cv::Mat::zeros(400, 600, CV_8UC1);

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
