/**
 * Phase 2 Week 1 - 중급 퀴즈 정답
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>

void problem1_solution()
{
    std::cout << "\n━━━ 문제 1 정답: 큐브 투영 ━━━\n" << std::endl;

    double fx = 600.0, fy = 600.0, cx = 400.0, cy = 300.0;

    std::vector<cv::Point3d> cube = {{-1, -1, 4}, {1, -1, 4}, {1, 1, 4}, {-1, 1, 4},
                                     {-1, -1, 6}, {1, -1, 6}, {1, 1, 6}, {-1, 1, 6}};

    // R=I, t=0 투영
    std::vector<cv::Point> pixels;
    for (const auto &pt : cube)
    {
        double u = fx * pt.x / pt.z + cx;
        double v = fy * pt.y / pt.z + cy;
        pixels.push_back(cv::Point((int)u, (int)v));
        std::cout << "(" << pt.x << ", " << pt.y << ", " << pt.z << ") → (" << (int)u << ", "
                  << (int)v << ")" << std::endl;
    }

    // 이미지에 큐브 그리기
    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC3);

    // 앞면 (초록)
    for (int i = 0; i < 4; i++)
    {
        cv::line(image, pixels[i], pixels[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
    }
    // 뒷면 (파랑)
    for (int i = 0; i < 4; i++)
    {
        cv::line(image, pixels[4 + i], pixels[4 + (i + 1) % 4], cv::Scalar(255, 0, 0), 2);
    }
    // 앞뒤 연결 (빨강)
    for (int i = 0; i < 4; i++)
    {
        cv::line(image, pixels[i], pixels[i + 4], cv::Scalar(0, 0, 255), 1);
    }

    // 꼭짓점 표시
    for (size_t i = 0; i < pixels.size(); i++)
    {
        cv::circle(image, pixels[i], 5, cv::Scalar(255, 255, 0), -1);
    }

    std::cout << "\n앞면(Z=4)의 점이 뒷면(Z=6)보다 크게 투영됨 → 원근감!" << std::endl;

    // cv::imshow("Cube Projection", image);
    // cv::waitKey(0);
}

void problem2_solution()
{
    std::cout << "\n━━━ 문제 2 정답: 재투영 오차 ━━━\n" << std::endl;

    double fx = 600.0, fy = 600.0, cx = 400.0, cy = 300.0;

    std::vector<cv::Point3d> points_3d;
    std::vector<cv::Point2d> observed_2d;

    srand(42);
    for (int i = 0; i < 20; i++)
    {
        double x = (rand() % 100 - 50) / 25.0;
        double y = (rand() % 100 - 50) / 25.0;
        double z = 3.0 + (rand() % 70) / 10.0;
        points_3d.push_back(cv::Point3d(x, y, z));

        double u = fx * x / z + cx;
        double v = fy * y / z + cy;

        double noise_u = (rand() % 200 - 100) / 100.0;
        double noise_v = (rand() % 200 - 100) / 100.0;
        observed_2d.push_back(cv::Point2d(u + noise_u, v + noise_v));
    }

    // 재투영 오차 계산
    std::vector<double> errors;
    for (size_t i = 0; i < points_3d.size(); i++)
    {
        double u_proj = fx * points_3d[i].x / points_3d[i].z + cx;
        double v_proj = fy * points_3d[i].y / points_3d[i].z + cy;

        double du = u_proj - observed_2d[i].x;
        double dv = v_proj - observed_2d[i].y;
        double error = std::sqrt(du * du + dv * dv);
        errors.push_back(error);
    }

    // 평균
    double sum = std::accumulate(errors.begin(), errors.end(), 0.0);
    double mean = sum / errors.size();

    // 표준편차
    double sq_sum = 0.0;
    for (double e : errors)
    {
        sq_sum += (e - mean) * (e - mean);
    }
    double std_dev = std::sqrt(sq_sum / errors.size());

    std::cout << "점 개수: " << points_3d.size() << std::endl;
    std::cout << "평균 오차: " << mean << " 픽셀" << std::endl;
    std::cout << "표준편차: " << std_dev << " 픽셀" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n━━━ 문제 3 정답: 외부 파라미터 효과 ━━━\n" << std::endl;

    double fx = 600.0, fy = 600.0, cx = 400.0, cy = 300.0;
    cv::Point3d P(0, 0, 5);

    // 기본: R=I, t=0
    double u0 = fx * P.x / P.z + cx;  // 400
    double v0 = fy * P.y / P.z + cy;  // 300
    std::cout << "기본: (" << u0 << ", " << v0 << ")" << std::endl;

    // 시나리오 1: t = [1, 0, 0]
    // Pc = R*P + t = (0,0,5) + (1,0,0) = (1, 0, 5)
    double Xc1 = P.x + 1.0, Yc1 = P.y, Zc1 = P.z;
    double u1 = fx * Xc1 / Zc1 + cx;  // 600*1/5 + 400 = 520
    double v1 = fy * Yc1 / Zc1 + cy;  // 300
    std::cout << "시나리오 1 (t=[1,0,0]): (" << u1 << ", " << v1 << ")"
              << " → 물체가 오른쪽으로 이동" << std::endl;

    // 시나리오 2: Y축 15° 회전
    double angle = 15.0 * CV_PI / 180.0;
    cv::Mat Ry = (cv::Mat_<double>(3, 3) << std::cos(angle), 0, std::sin(angle), 0, 1, 0,
                  -std::sin(angle), 0, std::cos(angle));

    cv::Mat Pw = (cv::Mat_<double>(3, 1) << P.x, P.y, P.z);
    cv::Mat Pc = Ry * Pw;

    double Xc2 = Pc.at<double>(0);
    double Yc2 = Pc.at<double>(1);
    double Zc2 = Pc.at<double>(2);

    double u2 = fx * Xc2 / Zc2 + cx;
    double v2 = fy * Yc2 / Zc2 + cy;

    std::cout << "시나리오 2 (Y축 15°): (" << u2 << ", " << v2 << ")"
              << " → Pc=(" << Xc2 << ", " << Yc2 << ", " << Zc2 << ")" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 1 Quiz Medium - 정답" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    return 0;
}
