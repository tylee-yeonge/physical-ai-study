#include "basic.h"
#include <iostream>
#include <chrono>
#include <iomanip>

// FAST 코너 검출 — 가장 빠른 코너 검출기 중 하나
//
// 원리: 중심 픽셀 p 주위 16개 픽셀(반지름 3의 Bresenham 원)을 검사하여,
//   연속 N개(FAST-9이면 9개, FAST-12이면 12개)가 모두
//   p보다 threshold 이상 밝거나, threshold 이상 어두우면 코너로 판정.
//
//        16 1  2
//     15         3
//   14             4
//   13    center   5    ← 16개 원형 픽셀
//   12             6
//     11         7
//        10 9  8
//
// 고속화 비결:
//   1) 1, 5, 9, 13번 픽셀만 먼저 검사 (4개 중 3개 이상 통과해야 진행)
//   2) 세그먼트 테스트로 대부분의 비코너를 즉시 제거 → O(1) 수준
//
// 파라미터:
//   threshold: 밝기 차이 임계값 (↑ 강한 코너만, ↓ 약한 코너도 포함)
//   nonmaxSuppression: NMS 적용 여부 (인접 코너 중 최대 응답만 유지)
//
// 반환: 검출 시간 (ms)
double FeatureDetectionBasic::detectFAST(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints,
                                         int threshold, bool nonmaxSuppression)
{
    // 그레이스케일 변환 — FAST는 밝기 값만 사용
    cv::Mat gray;
    if (image.channels() == 3)
    {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        gray = image.clone();
    }

    auto start = std::chrono::high_resolution_clock::now();

    // OpenCV의 FAST 구현은 기본적으로 FAST-9 사용
    cv::FAST(gray, keypoints, threshold, nonmaxSuppression);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return duration.count() / 1000.0;
}

// ORB 검출 — FAST 검출 + BRIEF 디스크립터 + 회전 불변성
//
// ORB = Oriented FAST and Rotated BRIEF
//
// 동작 3단계:
//   ① FAST로 코너 검출 (고속)
//   ② Harris 코너 응답으로 상위 nfeatures개 선별 (품질 보장)
//   ③ Intensity Centroid로 방향 계산 → 회전 불변 BRIEF 디스크립터 생성
//
// 디스크립터:
//   256비트 이진 벡터 (32바이트) — 주변 픽셀 쌍 비교 (밝으면 1, 어두우면 0)
//   매칭: 해밍 거리 (XOR → popcount) → L2 거리보다 수십 배 빠름
//
// SLAM에서 ORB를 사용하는 이유:
//   - FAST 수준으로 빠른 검출
//   - 이진 디스크립터로 빠른 매칭
//   - 회전/스케일 불변 → 카메라 움직임에 강건
//   - ORB-SLAM2/3가 이 검출기 사용
//
// 반환: 검출+디스크립터 계산 시간 (ms)
double FeatureDetectionBasic::detectORB(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints,
                                        cv::Mat& descriptors, int nfeatures)
{
    // nfeatures: 검출할 최대 특징점 수 (Harris 점수 상위 N개)
    cv::Ptr<cv::ORB> orb = cv::ORB::create(nfeatures);

    auto start = std::chrono::high_resolution_clock::now();

    // detectAndCompute: 검출과 디스크립터 계산을 한 번에 수행
    //   별도로 detect() → compute() 호출하는 것보다 효율적
    //   (내부적으로 피라미드를 한 번만 구성)
    orb->detectAndCompute(image, cv::noArray(), keypoints, descriptors);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return duration.count() / 1000.0;
}

// 특징점 시각화 — 검출된 특징점의 위치, 크기, 방향을 이미지 위에 표시
//
// DRAW_RICH_KEYPOINTS 플래그:
//   - 원의 크기 = keypoint.size (특징점의 스케일)
//   - 원 안의 선 = keypoint.angle (특징점의 방향)
//   → 단순 점 대신 원+방향을 그려 스케일/회전 정보를 시각적으로 확인
void FeatureDetectionBasic::visualizeKeypoints(const cv::Mat& image,
                                               const std::vector<cv::KeyPoint>& keypoints,
                                               cv::Mat& output, const std::string& title)
{
    cv::drawKeypoints(image, keypoints, output, cv::Scalar(0, 255, 0),
                      cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    std::string info = "Keypoints: " + std::to_string(keypoints.size());
    cv::putText(output, info, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0,
                cv::Scalar(0, 255, 0), 2);
}

// 특징점 공간 분포 분석 — 이미지를 gridSize×gridSize 격자로 나누어 각 셀의 특징점 수 카운트
//
// 균등 분포가 SLAM에서 중요한 이유:
//   - 한 곳에 특징점이 몰리면 → 그 영역의 깊이/움직임만 추정 가능
//   - 넓게 분포하면 → 이미지 전체의 3D 정보 활용 → 더 정확한 포즈 추정
//   - ORB-SLAM은 quadtree로 균등 분포 강제
//   - VINS-Fusion은 setMask()로 인접 특징점 간 최소 거리 보장
//
// 반환: gridSize×gridSize 행렬 (각 셀의 특징점 개수)
cv::Mat FeatureDetectionBasic::analyzeDistribution(const std::vector<cv::KeyPoint>& keypoints,
                                                   cv::Size imageSize, int gridSize)
{
    cv::Mat distribution = cv::Mat::zeros(gridSize, gridSize, CV_32S);

    int cellWidth = imageSize.width / gridSize;
    int cellHeight = imageSize.height / gridSize;

    for (const auto& kp : keypoints)
    {
        int gridX = static_cast<int>(kp.pt.x / cellWidth);
        int gridY = static_cast<int>(kp.pt.y / cellHeight);

        if (gridX >= 0 && gridX < gridSize && gridY >= 0 && gridY < gridSize)
        {
            distribution.at<int>(gridY, gridX)++;
        }
    }

    return distribution;
}

void FeatureDetectionBasic::compareDetectors(const cv::Mat& image)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "특징점 검출기 성능 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;

    // 1. FAST
    double time_fast = detectFAST(image, keypoints, 20, true);
    int count_fast = keypoints.size();

    std::cout << "📍 FAST:" << std::endl;
    std::cout << "   - 검출 시간: " << std::fixed << std::setprecision(2) << time_fast << " ms"
              << std::endl;
    std::cout << "   - 특징점 개수: " << count_fast << std::endl;
    std::cout << "   - 특징: 매우 빠름, 코너 검출, 디스크립터 없음\n" << std::endl;

    // 2. ORB
    keypoints.clear();
    double time_orb = detectORB(image, keypoints, descriptors, 500);
    int count_orb = keypoints.size();

    std::cout << "📍 ORB:" << std::endl;
    std::cout << "   - 검출 시간: " << time_orb << " ms" << std::endl;
    std::cout << "   - 특징점 개수: " << count_orb << std::endl;
    std::cout << "   - 디스크립터 크기: " << descriptors.size() << std::endl;
    std::cout << "   - 특징: FAST + BRIEF, 회전 불변성\n" << std::endl;

    // 3. SIFT (참고 - 특허 만료로 OpenCV 4.x에서 사용 가능)
    try
    {
        cv::Ptr<cv::SIFT> sift = cv::SIFT::create(500);
        keypoints.clear();

        auto start = std::chrono::high_resolution_clock::now();
        sift->detectAndCompute(image, cv::noArray(), keypoints, descriptors);
        auto end = std::chrono::high_resolution_clock::now();
        double time_sift =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;

        std::cout << "📍 SIFT:" << std::endl;
        std::cout << "   - 검출 시간: " << time_sift << " ms" << std::endl;
        std::cout << "   - 특징점 개수: " << keypoints.size() << std::endl;
        std::cout << "   - 특징: 스케일/회전 불변, 느림, 고품질\n" << std::endl;
    }
    catch (...)
    {
        std::cout << "⚠️  SIFT: 사용 불가 (OpenCV contrib 필요)\n" << std::endl;
    }

    std::cout << "💡 SLAM에서의 선택:" << std::endl;
    std::cout << "   - 실시간 중요: FAST + KLT (VINS-Fusion)" << std::endl;
    std::cout << "   - Loop Closure: ORB (ORB-SLAM)" << std::endl;
    std::cout << "   - 고정밀: SIFT (오프라인 처리)" << std::endl;
}

void FeatureDetectionBasic::demoNMS(const cv::Mat& image)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Non-Maximum Suppression 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::vector<cv::KeyPoint> kp_without_nms, kp_with_nms;

    // NMS 없이
    double time1 = detectFAST(image, kp_without_nms, 20, false);

    // NMS 적용
    double time2 = detectFAST(image, kp_with_nms, 20, true);

    std::cout << "📊 결과 비교:" << std::endl;
    std::cout << "   NMS 없음: " << kp_without_nms.size() << "개 특징점, " << time1 << " ms"
              << std::endl;
    std::cout << "   NMS 적용: " << kp_with_nms.size() << "개 특징점, " << time2 << " ms"
              << std::endl;

    double reduction = 0.0;
    if (!kp_without_nms.empty())
    {
        reduction = 100.0 * (1.0 - (double)kp_with_nms.size() / kp_without_nms.size());
    }
    std::cout << "   → " << std::fixed << std::setprecision(1) << reduction << "% 감소\n"
              << std::endl;

    std::cout << "💡 NMS의 역할:" << std::endl;
    std::cout << "   - 인접한 여러 응답 중 최대값만 유지" << std::endl;
    std::cout << "   - 중복 특징점 제거 → 더 분산된 분포" << std::endl;
    std::cout << "   - SLAM에서 필수! (맵 점의 품질 향상)" << std::endl;
}

// 메인 함수 - 데모
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  특징점 검출 기본 데모 (Week 3)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 생성 (체커보드 패턴)
    cv::Mat test_image = cv::Mat::zeros(600, 800, CV_8UC3);

    // 체커보드 패턴 그리기
    int square_size = 50;
    for (int i = 0; i < test_image.rows; i += square_size)
    {
        for (int j = 0; j < test_image.cols; j += square_size)
        {
            if ((i / square_size + j / square_size) % 2 == 0)
            {
                cv::rectangle(test_image, cv::Point(j, i),
                              cv::Point(j + square_size, i + square_size),
                              cv::Scalar(255, 255, 255), -1);
            }
        }
    }

    // 원 추가 (다양한 특징)
    cv::circle(test_image, cv::Point(400, 300), 80, cv::Scalar(200, 100, 50), -1);
    cv::circle(test_image, cv::Point(200, 200), 50, cv::Scalar(100, 200, 150), -1);

    std::cout << "📸 테스트 이미지 생성 완료 (800x600)" << std::endl;
    std::cout << "   - 체커보드 패턴 + 원형 객체\n" << std::endl;

    // 💡 이미지 속 특징점 후보 설명
    std::cout << "💡 [교육] 이 이미지의 특징점 후보:" << std::endl;
    std::cout << "   - 체커보드 교차점: 모든 방향으로 밝기 변화 → 코너 ✅" << std::endl;
    std::cout << "   - 체커보드 경계선: 한 방향만 변화 → 에지 (특징점 X)" << std::endl;
    std::cout << "   - 균일한 흰/검 영역: 변화 없음 → 플랫 (특징점 X)\n" << std::endl;

    // 검출기 비교
    FeatureDetectionBasic::compareDetectors(test_image);

    // 💡 FAST 원리 + ORB 디스크립터 설명
    std::cout << "💡 [교육] FAST 코너 검출 원리 (quiz 문제 1에서 사용!):" << std::endl;
    std::cout << "   중심 픽셀 주위 16개 픽셀(Bresenham 원) 검사:" << std::endl;
    std::cout << "   → 연속 9~12개가 모두 밝거나 어두우면 코너!" << std::endl;
    std::cout << "   threshold↑ → 강한 코너만 (적은 수)" << std::endl;
    std::cout << "   threshold↓ → 약한 코너도 포함 (많은 수)\n" << std::endl;

    std::cout << "💡 [교육] ORB 디스크립터 (quiz 문제 2에서 사용!):" << std::endl;
    std::cout << "   = FAST 검출 + BRIEF 이진 디스크립터 + 회전 불변성" << std::endl;
    std::cout << "   디스크립터 크기: 32 바이트 (256비트 / 8)" << std::endl;
    std::cout << "   매칭: 해밍 거리 (XOR 연산) → 매우 빠름\n" << std::endl;

    // NMS 데모
    FeatureDetectionBasic::demoNMS(test_image);

    // FAST 검출 및 시각화
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "FAST 특징점 검출 및 분포 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::vector<cv::KeyPoint> keypoints;
    double time = FeatureDetectionBasic::detectFAST(test_image, keypoints, 10, true);

    std::cout << "✅ 검출 완료:" << std::endl;
    std::cout << "   - 특징점 개수: " << keypoints.size() << std::endl;
    std::cout << "   - 검출 시간: " << time << " ms" << std::endl;

    // 💡 KeyPoint 속성 설명
    if (!keypoints.empty())
    {
        const auto& kp = keypoints[0];
        std::cout << "\n💡 [교육] KeyPoint 속성 (첫 번째 점):" << std::endl;
        std::cout << "   pt: (" << kp.pt.x << ", " << kp.pt.y << ") — 픽셀 위치" << std::endl;
        std::cout << "   response: " << kp.response << " — 코너 강도 (높을수록 강함)" << std::endl;
        std::cout << "   size: " << kp.size << " — 특징점 크기" << std::endl;
        std::cout << "   angle: " << kp.angle << " — 방향 (FAST는 -1 = 방향 없음, ORB는 계산됨)"
                  << std::endl;
    }

    // 분포 분석
    cv::Mat distribution =
        FeatureDetectionBasic::analyzeDistribution(keypoints, test_image.size(), 8);

    std::cout << "\n📊 8x8 그리드 분포:" << std::endl;
    for (int i = 0; i < distribution.rows; i++)
    {
        std::cout << "   ";
        for (int j = 0; j < distribution.cols; j++)
        {
            std::cout << std::setw(4) << distribution.at<int>(i, j);
        }
        std::cout << std::endl;
    }

    // 💡 균등 분포의 중요성 (quiz medium 문제 1에서 사용!)
    std::cout << "\n💡 [교육] 균등한 분포가 SLAM에서 중요한 이유:" << std::endl;
    std::cout << "   - 한 곳에 몰리면 → 나머지 영역 정보 없음" << std::endl;
    std::cout << "   - 넓게 분포 → 더 안정적인 포즈 추정" << std::endl;
    std::cout << "   - VINS-Fusion: setMask()로 최소 간격 보장\n" << std::endl;

    // 시각화 (저장)
    cv::Mat output;
    FeatureDetectionBasic::visualizeKeypoints(test_image, keypoints, output);

    std::cout << "\n💾 시각화 이미지 저장: keypoints_demo.png" << std::endl;
    cv::imwrite("keypoints_demo.png", output);

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ Week 3 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md 이론 읽기 (Harris, FAST, ORB)" << std::endl;
    std::cout << "   2. quiz_easy.cpp — FAST 파라미터, ORB, NMS, Harris" << std::endl;
    std::cout << "   3. my_basic.cpp — Step 1~6 순서대로 직접 구현" << std::endl;
    std::cout << "   4. quiz_medium.cpp — 균등 분포, 멀티스케일, NMS 구현" << std::endl;
    std::cout << "   5. PRACTICE.md — 실제 카메라 영상 처리\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
