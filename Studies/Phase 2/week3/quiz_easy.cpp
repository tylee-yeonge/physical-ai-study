/**
 * @file quiz_easy.cpp
 * @brief Week 3 개념 퀴즈 (Easy)
 *
 * 주제:
 *   - 에피폴라 제약의 의미
 *   - Rectification 의 목적
 *   - Disparity-Depth 공식 (Z = fB/d)
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

// 문제 1: 에피폴라 제약의 의미
//
// 에피폴라 제약 (Epipolar Constraint):
//   p2ᵀ · F · p1 = 0
//
// 이 식의 의미:
//   왼쪽 이미지의 점 p1 에 대응하는 오른쪽 이미지의 점 p2 는
//   "에피폴라 선" 위에 있어야 한다.
//   즉, 2D 이미지 전체를 탐색할 필요 없이 1D 선만 탐색하면 된다.
//
// TODO: 주어진 F, p1 에 대해 에피폴라 선 l = F · p1 을 계산하고
//       l = [a, b, c] 일 때 ax + by + c = 0 의 기울기를 구하세요
void problem1_epipolar_constraint()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 에피폴라 제약" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 샘플 Fundamental Matrix (정규화된)
    cv::Mat F = (cv::Mat_<double>(3, 3) <<
         0.0,    0.0,   -0.002,
         0.0,    0.0,    0.01,
         0.003, -0.008,  1.0);

    // 왼쪽 이미지의 점 (동차 좌표)
    cv::Mat p1 = (cv::Mat_<double>(3, 1) << 320.0, 240.0, 1.0);

    // TODO: 에피폴라 선 계산 l = F * p1
    cv::Mat l;  // TODO

    std::cout << "왼쪽 점 p1 = (320, 240)" << std::endl;
    std::cout << "에피폴라 선 l = F * p1 = ?" << std::endl;
    std::cout << std::endl;
    std::cout << "💡 힌트:" << std::endl;
    std::cout << "   l = [a, b, c] 일 때, 에피폴라 선의 방정식은 ax + by + c = 0" << std::endl;
    std::cout << "   기울기 = -a/b" << std::endl;
    std::cout << "   Rectification 후에는 기울기가 0 (수평선) 이 되어야 함!" << std::endl;
}

// 문제 2: Rectification 의 목적
//
// Rectification 전:
//   - 에피폴라 선이 기울어져 있음 → 2D 탐색 필요
//   - Stereo 매칭이 복잡하고 느림
//
// Rectification 후:
//   - 에피폴라 선이 수평 → 같은 행(row)만 탐색
//   - Disparity = u_left - u_right (같은 y좌표에서 x 차이)
//   - Stereo Depth 모델(HITNet 등)은 이 rectified 쌍을 전제로 설계됨
//
// TODO: 두 점이 Rectification 전후에 어떻게 변하는지 확인하세요
void problem2_rectification_purpose()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Rectification 의 목적" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Rectification 전: 대응점의 y좌표가 다름
    cv::Point2f before_left(200, 150);
    cv::Point2f before_right(180, 155);  // y 가 5 픽셀 차이

    // Rectification 후: y좌표가 같아짐 (이상적)
    cv::Point2f after_left(200, 150);
    cv::Point2f after_right(180, 150);   // y 동일!

    double disparity = after_left.x - after_right.x;

    std::cout << "Rectification 전:" << std::endl;
    std::cout << "   왼쪽: " << before_left << ", 오른쪽: " << before_right << std::endl;
    std::cout << "   y 차이: " << std::abs(before_left.y - before_right.y) << " px" << std::endl;
    std::cout << std::endl;
    std::cout << "Rectification 후:" << std::endl;
    std::cout << "   왼쪽: " << after_left << ", 오른쪽: " << after_right << std::endl;
    std::cout << "   y 차이: " << std::abs(after_left.y - after_right.y) << " px" << std::endl;
    std::cout << "   Disparity: " << disparity << " px" << std::endl;
    std::cout << std::endl;
    std::cout << "💡 핵심:" << std::endl;
    std::cout << "   Rectification = 두 이미지를 같은 평면에 정렬하여" << std::endl;
    std::cout << "   대응점의 y좌표를 동일하게 만드는 변환." << std::endl;
    std::cout << "   → Stereo Depth network (HITNet 등) 는 이 전처리를 전제로 작동" << std::endl;
}

// 문제 3: Disparity → Depth 공식
//
// 핵심 공식: Z = fB / d
//   Z: 깊이 (미터)
//   f: 초점 거리 (픽셀 단위)
//   B: baseline (미터 단위)
//   d: disparity (픽셀 단위)
//
// 반비례 관계:
//   - d ↑ (가까운 물체) → Z ↓
//   - d ↓ (먼 물체) → Z ↑
//   - d = 0 → Z = ∞ (무한 원점)
//
// TODO: 주어진 f, B 에서 disparity 값에 따른 depth 를 계산하세요
void problem3_disparity_depth()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Disparity → Depth (Z = fB/d)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double f = 500.0;    // 초점 거리 (px)
    double B = 0.12;     // baseline (m)

    std::cout << "카메라 설정: f=" << f << " px, B=" << B << " m" << std::endl;
    std::cout << "f×B = " << f * B << " px·m\n" << std::endl;

    std::vector<double> disparities = {60.0, 30.0, 15.0, 6.0, 1.0};

    for (double d : disparities)
    {
        // TODO: Z = fB / d 계산
        double Z = 0.0;  // TODO

        std::cout << "   d = " << d << " px → Z = " << Z << " m" << std::endl;
    }

    std::cout << "\n💡 핵심 관찰:" << std::endl;
    std::cout << "   disparity 가 절반이 되면 depth 는 2배가 됩니다 (반비례)" << std::endl;
    std::cout << "   Stereo Depth 모델은 이 원리로 disparity → 미터 단위 depth 를 변환합니다" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 3 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_epipolar_constraint();
    problem2_rectification_purpose();
    problem3_disparity_depth();

    return 0;
}
