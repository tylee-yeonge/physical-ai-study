/**
 * Phase 2 Week 1 - 기초 퀴즈 정답
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

void problem1_solution()
{
    std::cout << "\n━━━ 문제 1 정답 ━━━\n" << std::endl;

    cv::Mat K = (cv::Mat_<double>(3, 3) << 500.0, 0.0, 320.0, 0.0, 500.0, 240.0, 0.0, 0.0, 1.0);

    double fx = K.at<double>(0, 0);  // 500.0
    double fy = K.at<double>(1, 1);  // 500.0
    double cx = K.at<double>(0, 2);  // 320.0
    double cy = K.at<double>(1, 2);  // 240.0

    std::cout << "fx = " << fx << ", fy = " << fy << std::endl;
    std::cout << "cx = " << cx << ", cy = " << cy << std::endl;

    // 이미지 크기 추정: 주점이 중심이므로
    int estimated_width = (int)(cx * 2);   // 640
    int estimated_height = (int)(cy * 2);  // 480

    std::cout << "추정 이미지 크기: " << estimated_width << " x " << estimated_height << std::endl;
}

void problem2_solution()
{
    std::cout << "\n━━━ 문제 2 정답 ━━━\n" << std::endl;

    double Xw = 2.0, Yw = 1.0, Zw = 5.0;
    double fx = 600.0, fy = 600.0, cx = 400.0, cy = 300.0;

    // Step 1: R=I, t=0이므로 Pc = Pw
    double Xc = Xw;  // 2.0
    double Yc = Yw;  // 1.0
    double Zc = Zw;  // 5.0

    std::cout << "Step 1: Pc = (" << Xc << ", " << Yc << ", " << Zc << ")" << std::endl;

    // Step 2: 정규화 좌표
    double x_norm = Xc / Zc;  // 2/5 = 0.4
    double y_norm = Yc / Zc;  // 1/5 = 0.2

    std::cout << "Step 2: x'=" << x_norm << ", y'=" << y_norm << std::endl;

    // Step 3: 픽셀 좌표
    double u = fx * x_norm + cx;  // 600*0.4 + 400 = 640
    double v = fy * y_norm + cy;  // 600*0.2 + 300 = 420

    std::cout << "Step 3: pixel = (" << u << ", " << v << ")" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n━━━ 문제 3 정답 ━━━\n" << std::endl;

    int image_width = 640;

    double fx_A = 300.0;
    double fov_A = 2.0 * std::atan2(image_width, 2.0 * fx_A) * 180.0 / CV_PI;
    // fov_A ≈ 93.7°

    double fx_B = 1200.0;
    double fov_B = 2.0 * std::atan2(image_width, 2.0 * fx_B) * 180.0 / CV_PI;
    // fov_B ≈ 29.9°

    std::cout << "카메라 A (fx=300): FOV = " << fov_A << "° → 광각" << std::endl;
    std::cout << "카메라 B (fx=1200): FOV = " << fov_B << "° → 망원" << std::endl;
}

void problem4_solution()
{
    std::cout << "\n━━━ 문제 4 정답 ━━━\n" << std::endl;

    double fx = 600.0, fy = 600.0;
    double cx = 400.0, cy = 300.0;

    // 이미지 중심
    double u1 = 400.0, v1 = 300.0;
    double x1_norm = (u1 - cx) / fx;  // 0.0
    double y1_norm = (v1 - cy) / fy;  // 0.0

    std::cout << "중심 (400, 300): ray = [" << x1_norm << ", " << y1_norm << ", 1]"
              << " → 카메라 정면 (Z축)" << std::endl;

    // 좌상단
    double u2 = 0.0, v2 = 0.0;
    double x2_norm = (u2 - cx) / fx;  // -0.667
    double y2_norm = (v2 - cy) / fy;  // -0.5

    std::cout << "좌상단 (0, 0): ray = [" << x2_norm << ", " << y2_norm << ", 1]"
              << " → 왼쪽 위 방향" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 1 Quiz Easy - 정답" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    return 0;
}
