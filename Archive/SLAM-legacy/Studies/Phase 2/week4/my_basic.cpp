/**
 * Phase 2 Week 4 - 특징점 매칭 직접 구현
 *
 * basic.h의 FeatureMatchingBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make my_basic
 * 실행: ./my_basic (각 Step 구현 후 실행하여 확인)
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │                    구현 순서 가이드                            │
 * ├──────┬──────────────────────┬────────┬───────────────────────┤
 * │ Step │ 함수                  │ 난이도 │ 검증 방법              │
 * ├──────┼──────────────────────┼────────┼───────────────────────┤
 * │  1   │ matchBruteForce      │ 쉬움   │ ./my_basic            │
 * │  2   │ evaluateMatchQuality │ 쉬움   │ ./my_basic            │
 * │  3   │ matchFLANN           │ 쉬움   │ ./my_basic            │
 * │  4   │ ratioTest            │ 보통   │ ./my_basic            │
 * │  5   │ visualizeMatches     │ 쉬움   │ ./my_basic            │
 * │  6   │ filterRANSAC         │ 어려움 │ ./my_basic            │
 * └──────┴──────────────────────┴────────┴───────────────────────┘
 *
 * 💡 각 Step 구현 후 ./my_basic을 실행하면 해당 단계의 ✅/❌를 확인할 수 있습니다.
 */
#include "basic.h"
#include <iostream>
#include <chrono>
#include <iomanip>

double FeatureMatchingBasic::matchBruteForce(const cv::Mat& descriptors1,
                                             const cv::Mat& descriptors2,
                                             std::vector<cv::DMatch>& matches, int normType)
{
    // [Step 1] Brute-Force 매칭 + 시간 측정
    // 1) normType을 사용하여 BFMatcher 객체 생성 (crossCheck=false)
    // 2) 두 디스크립터 집합 간 매칭 수행
    // 3) 시간 측정하여 ms로 반환
    // 참고: basic.cpp의 matchBruteForce()
    // 기대값: 동일 디스크립터 → 매칭 거리 = 0
    return 0.0;
}

double FeatureMatchingBasic::evaluateMatchQuality(const std::vector<cv::DMatch>& matches)
{
    // [Step 2] 매칭 품질 평가 (평균 거리)
    // 모든 매칭의 distance 합 / 매칭 수
    // 참고: basic.cpp의 evaluateMatchQuality()
    // 기대값: 동일 디스크립터 매칭 → 평균 거리 = 0
    return 0.0;
}

double FeatureMatchingBasic::matchFLANN(const cv::Mat& descriptors1, const cv::Mat& descriptors2,
                                        std::vector<cv::DMatch>& matches)
{
    // [Step 3] FLANN 매칭 (근사 최근접 이웃)
    // 1) float 타입으로 변환 (FLANN은 CV_32F 필요)
    // 2) FlannBasedMatcher 객체 생성
    // 3) 두 디스크립터 간 매칭 수행
    // 참고: basic.cpp의 matchFLANN()
    // 기대값: 매칭 개수 > 0
    return 0.0;
}

int FeatureMatchingBasic::ratioTest(const cv::Mat& descriptors1, const cv::Mat& descriptors2,
                                    std::vector<cv::DMatch>& good_matches, float ratio_thresh)
{
    // [Step 4] Lowe's Ratio Test
    // 1) NORM_HAMMING을 사용하여 BFMatcher 객체 생성
    // 2) 각 특징점마다 k=2 최근접 이웃 매칭 수행 (knnMatch)
    // 3) 각 쌍에서 best.distance < ratio_thresh * second.distance면 통과
    // 참고: basic.cpp의 ratioTest()
    // 기대값: good_matches.size() < 전체 매칭 수
    return 0;
}

void FeatureMatchingBasic::visualizeMatches(const cv::Mat& img1,
                                            const std::vector<cv::KeyPoint>& kp1,
                                            const cv::Mat& img2,
                                            const std::vector<cv::KeyPoint>& kp2,
                                            const std::vector<cv::DMatch>& matches, cv::Mat& output)
{
    // [Step 5] 매칭 시각화
    // drawMatches 함수를 사용하여 두 이미지의 매칭 결과를 시각화
    // 참고: basic.cpp의 visualizeMatches()
    // 기대값: output.empty() == false
}

double FeatureMatchingBasic::filterRANSAC(const std::vector<cv::KeyPoint>& keypoints1,
                                          const std::vector<cv::KeyPoint>& keypoints2,
                                          const std::vector<cv::DMatch>& matches,
                                          std::vector<cv::DMatch>& inlier_matches,
                                          cv::Mat& homography, double ransac_thresh)
{
    // [Step 6] RANSAC으로 outlier 제거 (가장 어려운 단계!)
    // 1) matches에서 queryIdx, trainIdx를 이용해 Point2f 쌍 추출
    // 2) RANSAC 방식으로 Homography 추정 (임계값: ransac_thresh, inlier 마스크 포함)
    // 3) inlier_mask가 1인 매칭만 inlier_matches에 추가
    // 4) inlier 비율 반환
    // 참고: basic.cpp의 filterRANSAC()
    // 기대값: inlier_ratio > 0
    return 0.0;
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 특징점 매칭 - 단계별 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 데이터 생성: 동일 디스크립터
    cv::Mat desc1 = cv::Mat::zeros(20, 32, CV_8UC1);
    cv::Mat desc2;
    cv::randu(desc1, 0, 255);
    desc2 = desc1.clone();

    // ── Step 1: matchBruteForce ─────────────────
    std::cout << "Step 1: matchBruteForce" << std::endl;
    std::vector<cv::DMatch> matches;
    double t_bf = FeatureMatchingBasic::matchBruteForce(desc1, desc2, matches, cv::NORM_HAMMING);
    std::cout << "   매칭: " << matches.size() << "개, 시간: " << std::fixed << std::setprecision(2)
              << t_bf << " ms"
              << (matches.size() == 20 ? " ✅" : " ❌ 기대: 20개") << std::endl;
    if (!matches.empty())
    {
        std::cout << "   첫 매칭 거리: " << matches[0].distance
                  << (matches[0].distance == 0 ? " ✅ (동일 → 0)" : " ❌ 기대: 0") << std::endl;
    }

    // ── Step 2: evaluateMatchQuality ────────────
    std::cout << "\nStep 2: evaluateMatchQuality" << std::endl;
    double avg_dist = FeatureMatchingBasic::evaluateMatchQuality(matches);
    std::cout << "   평균 거리: " << avg_dist
              << (avg_dist == 0 ? " ✅ (동일 디스크립터)" : " ❌ 기대: 0") << std::endl;

    // ── Step 3: matchFLANN ──────────────────────
    std::cout << "\nStep 3: matchFLANN" << std::endl;
    std::vector<cv::DMatch> flann_matches;
    double t_fl = FeatureMatchingBasic::matchFLANN(desc1, desc2, flann_matches);
    std::cout << "   매칭: " << flann_matches.size() << "개, 시간: " << t_fl << " ms"
              << (flann_matches.size() > 0 ? " ✅" : " ❌ 기대: > 0개") << std::endl;

    // ── Step 4: ratioTest ───────────────────────
    std::cout << "\nStep 4: ratioTest" << std::endl;
    // 실제 이미지로 테스트 (동일 디스크립터는 ratio test 의미 없음)
    const int kImageHeight = 400;
    const int kImageWidth = 600;
    cv::Mat img1 = cv::Mat::zeros(kImageHeight, kImageWidth, CV_8UC1);
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
    cv::Mat img2;
    cv::Mat M = cv::getRotationMatrix2D(cv::Point(300, 200), 15, 1.0);
    M.at<double>(0, 2) += 30;
    M.at<double>(1, 2) += 20;
    cv::warpAffine(img1, img2, M, img1.size());

    const int kMaxFeatures = 500;  // ORB 최대 특징점 수
    cv::Ptr<cv::ORB> orb = cv::ORB::create(kMaxFeatures);
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat d1, d2;
    orb->detectAndCompute(img1, cv::noArray(), kp1, d1);
    orb->detectAndCompute(img2, cv::noArray(), kp2, d2);

    if (d1.empty() || d2.empty())
    {
        std::cout << "   ⚠️  ORB 검출 실패 (이미지 특징 부족)" << std::endl;
    }
    else
    {
        std::vector<cv::DMatch> good;
        int n_good = FeatureMatchingBasic::ratioTest(d1, d2, good, 0.7f);
        std::vector<cv::DMatch> all_matches;
        FeatureMatchingBasic::matchBruteForce(d1, d2, all_matches, cv::NORM_HAMMING);
        std::cout << "   전체: " << all_matches.size() << "개 → Ratio Test 후: " << n_good << "개"
                  << (n_good > 0 && n_good <= static_cast<int>(all_matches.size())
                          ? " ✅"
                          : " ❌ 기대: 0 < good ≤ all")
                  << std::endl;

        // ── Step 5: visualizeMatches ────────────
        std::cout << "\nStep 5: visualizeMatches" << std::endl;
        cv::Mat vis;
        FeatureMatchingBasic::visualizeMatches(img1, kp1, img2, kp2, good, vis);
        std::cout << "   출력 이미지: "
                  << (vis.empty() ? "비어있음 ❌" : std::to_string(vis.cols) + "×"
                                                        + std::to_string(vis.rows) + " ✅")
                  << std::endl;

        // ── Step 6: filterRANSAC ────────────────
        std::cout << "\nStep 6: filterRANSAC" << std::endl;
        if (good.size() >= 4)
        {
            std::vector<cv::DMatch> inliers;
            cv::Mat H;
            double inlier_ratio = FeatureMatchingBasic::filterRANSAC(kp1, kp2, good, inliers, H);
            std::cout << "   입력: " << good.size() << "개 → Inlier: " << inliers.size() << "개"
                      << std::endl;
            std::cout << "   Inlier 비율: " << std::setprecision(1) << inlier_ratio * 100 << "%"
                      << (inlier_ratio > 0 ? " ✅" : " ❌ 기대: > 0%") << std::endl;
        }
        else
        {
            std::cout << "   ⚠️  매칭 < 4개, Step 4 먼저 확인" << std::endl;
        }

    }

    // ── 요약 ────────────────────────────────────
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  모든 Step ✅ 확인 후 → quiz_easy.cpp, quiz_medium.cpp" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ── 실제 이미지 테스트 ─────────────────────
    // 합성 체커보드는 반복 패턴이라 inlier 비율이 낮음.
    // 실제 자연 이미지(고유한 텍스처)에서 매칭 품질 차이를 비교한다.
    // demoPipeline은 위에서 구현한 Step 1~6 함수를 순서대로 호출하는 파이프라인이다.
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  실제 이미지 테스트 (box / box_in_scene)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat real_img1 = cv::imread("../images/box.png", cv::IMREAD_GRAYSCALE);
    cv::Mat real_img2 = cv::imread("../images/box_in_scene.png", cv::IMREAD_GRAYSCALE);

    if (real_img1.empty() || real_img2.empty())
    {
        std::cout << "   ⚠️  이미지 로드 실패 — images/ 폴더에 box.png, box_in_scene.png 필요" << std::endl;
        std::cout << "   build/ 폴더에서 실행했는지 확인하세요.\n" << std::endl;
        return 0;
    }

    std::cout << "   이미지1 (box): " << real_img1.cols << "×" << real_img1.rows << std::endl;
    std::cout << "   이미지2 (scene): " << real_img2.cols << "×" << real_img2.rows << "\n" << std::endl;

    // Step 1~6에서 구현한 함수들을 순서대로 호출하는 파이프라인
    // 1. 특징점 검출
    const int kMaxFeaturesReal = 1000;  // 실제 이미지용 ORB 최대 특징점 수
    cv::Ptr<cv::ORB> orb_real = cv::ORB::create(kMaxFeaturesReal);
    std::vector<cv::KeyPoint> real_kp1, real_kp2;
    cv::Mat real_d1, real_d2;
    orb_real->detectAndCompute(real_img1, cv::noArray(), real_kp1, real_d1);
    orb_real->detectAndCompute(real_img2, cv::noArray(), real_kp2, real_d2);
    std::cout << "1. 특징점 검출: " << real_kp1.size() << "개 × " << real_kp2.size() << "개" << std::endl;

    // 2. BF 매칭 (Step 1 함수 사용)
    std::vector<cv::DMatch> real_matches;
    double real_t_bf = FeatureMatchingBasic::matchBruteForce(real_d1, real_d2, real_matches, cv::NORM_HAMMING);
    std::cout << "2. BF 매칭: " << real_matches.size() << "개 (" << std::fixed << std::setprecision(2) << real_t_bf << " ms)" << std::endl;

    // 매칭 품질 평가 (Step 2 함수 사용)
    std::cout << "   평균 거리: " << std::setprecision(1)
              << FeatureMatchingBasic::evaluateMatchQuality(real_matches) << std::endl;

    // 3. Ratio Test (Step 4 함수 사용)
    std::vector<cv::DMatch> real_good;
    int real_n_good = FeatureMatchingBasic::ratioTest(real_d1, real_d2, real_good, kLoweRatioThreshold);
    std::cout << "3. Ratio Test: " << real_matches.size() << "개 → " << real_n_good << "개" << std::endl;

    // 4. RANSAC (Step 6 함수 사용)
    if (real_good.size() >= 4)
    {
        std::vector<cv::DMatch> real_inliers;
        cv::Mat real_H;
        double real_inlier_ratio = FeatureMatchingBasic::filterRANSAC(
            real_kp1, real_kp2, real_good, real_inliers, real_H);
        std::cout << "4. RANSAC: " << real_good.size() << "개 → " << real_inliers.size() << "개" << std::endl;
        std::cout << "   Inlier 비율: " << std::setprecision(1) << real_inlier_ratio * 100 << "%" << std::endl;

        // 5. 시각화 (Step 5 함수 사용)
        cv::Mat real_vis_before, real_vis_after;
        FeatureMatchingBasic::visualizeMatches(real_img1, real_kp1, real_img2, real_kp2, real_good, real_vis_before);
        FeatureMatchingBasic::visualizeMatches(real_img1, real_kp1, real_img2, real_kp2, real_inliers, real_vis_after);
        cv::imwrite("real_before_ransac.png", real_vis_before);
        cv::imwrite("real_after_ransac.png", real_vis_after);
        std::cout << "\n   저장: real_before_ransac.png (Ratio Test 후)" << std::endl;
        std::cout << "   저장: real_after_ransac.png (RANSAC 후)" << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  매칭 < 4개, Ratio Test 결과 확인 필요" << std::endl;
    }

    std::cout << "\n   체커보드 대비 Inlier 비율이 훨씬 높은 것을 확인!" << std::endl;
    std::cout << "   → 고유한 텍스처가 있는 실제 이미지에서 매칭이 더 잘 동작한다.\n" << std::endl;

    return 0;
}
#endif
