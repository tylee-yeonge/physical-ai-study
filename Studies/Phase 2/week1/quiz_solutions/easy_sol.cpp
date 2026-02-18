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

/**
 * @brief 문제 5 정답: 3D 점의 가시성 판별
 *
 * 3가지 조건으로 3D 점이 카메라에 보이는지 판별합니다.
 * (1) Zc > 0, (2) 이미지 경계 내, (3) FOV 범위 이내
 */
void problem5_solution()
{
    std::cout << "\n━━━ 문제 5 정답 ━━━\n" << std::endl;

    double fx = 500.0, fy = 500.0;
    double cx = 320.0, cy = 240.0;
    int image_width = 640, image_height = 480;

    double half_fov_x_rad = std::atan2(image_width, 2.0 * fx);
    double half_fov_y_rad = std::atan2(image_height, 2.0 * fy);

    struct TestPoint
    {
        double x, y, z;
        std::string description;
    };

    std::vector<TestPoint> test_points = {
        {0, 0, 5, "정면 중앙"},
        {3, 0, 5, "오른쪽"},
        {10, 0, 5, "멀리 오른쪽"},
        {0, 0, -5, "카메라 뒤"},
        {0, 5, 5, "위쪽 멀리"},
    };

    for (const auto& pt : test_points)
    {
        std::cout << "점 (" << pt.x << ", " << pt.y << ", " << pt.z
                  << ") - " << pt.description << ":" << std::endl;

        // 조건 1: Zc > 0
        bool cond1 = pt.z > 0;

        // 조건 2: 이미지 경계 내
        double u = 0.0, v = 0.0;
        bool cond2 = false;
        if (cond1)
        {
            u = fx * pt.x / pt.z + cx;
            v = fy * pt.y / pt.z + cy;
            cond2 = (u >= 0 && u < image_width && v >= 0 && v < image_height);
        }

        // 조건 3: FOV 범위 이내
        bool cond3 = false;
        if (cond1)
        {
            double angle_x = std::atan2(std::abs(pt.x), pt.z);
            double angle_y = std::atan2(std::abs(pt.y), pt.z);
            cond3 = (angle_x < half_fov_x_rad && angle_y < half_fov_y_rad);
        }

        std::cout << "   조건1 (Zc>0):     " << (cond1 ? "PASS" : "FAIL") << std::endl;
        std::cout << "   조건2 (경계 내):   " << (cond2 ? "PASS" : "FAIL");
        if (cond1)
        {
            std::cout << "  (u=" << u << ", v=" << v << ")";
        }
        std::cout << std::endl;
        std::cout << "   조건3 (FOV 이내): " << (cond3 ? "PASS" : "FAIL") << std::endl;

        bool visible = cond1 && cond2 && cond3;
        std::cout << "   → 결과: " << (visible ? "보임" : "보이지 않음") << "\n" << std::endl;
    }
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
    problem5_solution();

    return 0;
}
