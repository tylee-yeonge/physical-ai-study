/**
 * Phase 2 Week 3 - 특징점 검출 직접 구현
 *
 * basic.h의 FeatureDetectionBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make my_basic
 * 실행: ./my_basic (각 Step 구현 후 실행하여 확인)
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │                    구현 순서 가이드                            │
 * ├──────┬──────────────────────┬────────┬───────────────────────┤
 * │ Step │ 함수                  │ 난이도 │ 검증 방법              │
 * ├──────┼──────────────────────┼────────┼───────────────────────┤
 * │  1   │ detectFAST           │ 쉬움   │ ./my_basic            │
 * │  2   │ detectORB            │ 쉬움   │ ./my_basic            │
 * │  3   │ visualizeKeypoints   │ 쉬움   │ ./my_basic            │
 * │  4   │ analyzeDistribution  │ 보통   │ ./my_basic            │
 * │  5   │ compareDetectors     │ 보통   │ ./my_basic            │
 * │  6   │ demoNMS              │ 보통   │ ./my_basic            │
 * └──────┴──────────────────────┴────────┴───────────────────────┘
 *
 * 💡 각 Step 구현 후 ./my_basic을 실행하면 해당 단계의 ✅/❌를 확인할 수 있습니다.
 */
#include "basic.h"
#include <iostream>
#include <chrono>
#include <iomanip>

double FeatureDetectionBasic::detectFAST(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints,
                                         int threshold, bool nonmaxSuppression)
{
    // [Step 1] FAST 코너 검출 + 시간 측정
    // 1) 그레이스케일 변환 (image.channels() == 3이면)
    // 2) auto start = std::chrono::high_resolution_clock::now()
    // 3) cv::FAST(gray, keypoints, threshold, nonmaxSuppression)
    // 4) 종료 시간 측정, ms로 변환하여 반환
    // 참고: basic.cpp의 detectFAST()
    // 기대값: 체커보드 이미지에서 keypoints.size() > 0
    return 0.0;
}

double FeatureDetectionBasic::detectORB(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints,
                                        cv::Mat& descriptors, int nfeatures)
{
    // [Step 2] ORB 검출 + 디스크립터 계산 + 시간 측정
    // 1) cv::Ptr<cv::ORB> orb = cv::ORB::create(nfeatures)
    // 2) 시간 측정 시작
    // 3) orb->detectAndCompute(image, cv::noArray(), keypoints, descriptors)
    // 4) ms로 변환하여 반환
    // 참고: basic.cpp의 detectORB()
    // 기대값: descriptors.cols == 32 (256비트 이진 디스크립터)
    return 0.0;
}

void FeatureDetectionBasic::visualizeKeypoints(const cv::Mat& image,
                                               const std::vector<cv::KeyPoint>& keypoints,
                                               cv::Mat& output, const std::string& title)
{
    // [Step 3] 특징점 시각화
    // 1) cv::drawKeypoints(image, keypoints, output, 색상, 플래그)
    //    색상: cv::Scalar(0, 255, 0) (녹색)
    //    플래그: cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS (방향+크기 표시)
    // 참고: basic.cpp의 visualizeKeypoints()
    // 기대값: output.empty() == false
}

cv::Mat FeatureDetectionBasic::analyzeDistribution(const std::vector<cv::KeyPoint>& keypoints,
                                                   cv::Size imageSize, int gridSize)
{
    // [Step 4] 그리드별 특징점 분포 분석
    // 1) cv::Mat distribution = cv::Mat::zeros(gridSize, gridSize, CV_32S)
    // 2) cellWidth = imageSize.width / gridSize
    // 3) 각 keypoint에 대해 해당 셀의 카운트 증가
    //    gridX = kp.pt.x / cellWidth, gridY = kp.pt.y / cellHeight
    // 4) 범위 체크 후 distribution.at<int>(gridY, gridX)++
    // 참고: basic.cpp의 analyzeDistribution()
    // 기대값: 전체 합 == keypoints.size()
    return cv::Mat::zeros(gridSize, gridSize, CV_32S);
}

void FeatureDetectionBasic::compareDetectors(const cv::Mat& image)
{
    // [Step 5] FAST vs ORB 비교 출력
    // 1) detectFAST() 호출 → 시간, 개수 출력
    // 2) detectORB() 호출 → 시간, 개수, 디스크립터 크기 출력
    // 참고: basic.cpp의 compareDetectors()
}

void FeatureDetectionBasic::demoNMS(const cv::Mat& image)
{
    // [Step 6] Non-Maximum Suppression 비교
    // 1) detectFAST(image, kp1, 20, false) — NMS 없이
    // 2) detectFAST(image, kp2, 20, true)  — NMS 적용
    // 3) 개수 차이와 감소율 출력
    // 참고: basic.cpp의 demoNMS()
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 특징점 검출 - 단계별 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 생성 (체커보드 패턴)
    cv::Mat test_img = cv::Mat::zeros(600, 800, CV_8UC3);
    int sq = 50;
    for (int i = 0; i < test_img.rows; i += sq)
    {
        for (int j = 0; j < test_img.cols; j += sq)
        {
            if ((i / sq + j / sq) % 2 == 0)
            {
                cv::rectangle(test_img, cv::Point(j, i), cv::Point(j + sq, i + sq),
                              cv::Scalar(255, 255, 255), -1);
            }
        }
    }
    cv::circle(test_img, cv::Point(400, 300), 80, cv::Scalar(200, 100, 50), -1);

    // ── Step 1: detectFAST ──────────────────────
    std::cout << "Step 1: detectFAST" << std::endl;
    std::vector<cv::KeyPoint> kp_fast;
    double t_fast = FeatureDetectionBasic::detectFAST(test_img, kp_fast, 20, true);
    std::cout << "   특징점: " << kp_fast.size() << "개, 시간: " << std::fixed
              << std::setprecision(2) << t_fast << " ms"
              << (kp_fast.size() > 0 ? " ✅" : " ❌ 기대: > 0개") << std::endl;

    // threshold 높이면 적어지는지 확인
    std::vector<cv::KeyPoint> kp_high;
    FeatureDetectionBasic::detectFAST(test_img, kp_high, 80, true);
    std::cout << "   threshold 80: " << kp_high.size() << "개"
              << (kp_high.size() < kp_fast.size() ? " ✅ (threshold↑ → 적어짐)"
                                                   : " ❌ 기대: FAST(80) < FAST(20)")
              << std::endl;

    // ── Step 2: detectORB ───────────────────────
    std::cout << "\nStep 2: detectORB" << std::endl;
    std::vector<cv::KeyPoint> kp_orb;
    cv::Mat desc;
    double t_orb = FeatureDetectionBasic::detectORB(test_img, kp_orb, desc, 500);
    std::cout << "   특징점: " << kp_orb.size() << "개, 시간: " << t_orb << " ms"
              << (kp_orb.size() > 0 ? " ✅" : " ❌ 기대: > 0개") << std::endl;
    std::cout << "   디스크립터: " << desc.size()
              << (desc.cols == 32 ? " ✅ (32 byte = 256bit)" : " ❌ 기대: cols=32")
              << std::endl;

    // ── Step 3: visualizeKeypoints ──────────────
    std::cout << "\nStep 3: visualizeKeypoints" << std::endl;
    cv::Mat vis;
    FeatureDetectionBasic::visualizeKeypoints(test_img, kp_fast, vis);
    std::cout << "   출력 이미지: "
              << (vis.empty() ? "비어있음 ❌" : std::to_string(vis.cols) + "×"
                                                    + std::to_string(vis.rows) + " ✅")
              << std::endl;

    // ── Step 4: analyzeDistribution ─────────────
    std::cout << "\nStep 4: analyzeDistribution" << std::endl;
    cv::Mat dist = FeatureDetectionBasic::analyzeDistribution(kp_fast, test_img.size(), 4);
    int total = 0;
    for (int i = 0; i < dist.rows; i++)
    {
        for (int j = 0; j < dist.cols; j++)
        {
            total += dist.at<int>(i, j);
        }
    }
    std::cout << "   4×4 그리드 합: " << total
              << (total == static_cast<int>(kp_fast.size()) ? " ✅ (== 전체 특징점)"
                                                            : " ❌ 기대: == " + std::to_string(kp_fast.size()))
              << std::endl;

    // ── Step 5: compareDetectors ────────────────
    std::cout << "\nStep 5: compareDetectors" << std::endl;
    FeatureDetectionBasic::compareDetectors(test_img);
    std::cout << "   (위 출력 확인)" << std::endl;

    // ── Step 6: demoNMS ─────────────────────────
    std::cout << "\nStep 6: demoNMS" << std::endl;
    FeatureDetectionBasic::demoNMS(test_img);
    std::cout << "   (위 출력 확인)" << std::endl;

    // ── 요약 ────────────────────────────────────
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  모든 Step ✅ 확인 후 → quiz_easy.cpp, quiz_medium.cpp" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
