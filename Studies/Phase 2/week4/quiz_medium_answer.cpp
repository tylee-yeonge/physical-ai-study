/**
 * @file quiz_medium_answer.cpp
 * @brief Week 4 구현 퀴즈 (Medium) - 답안
 *
 * 주제:
 *   - DLT 삼각측량 직접 구현
 *   - RANSAC 반복 수 계산
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <vector>

// 문제 1: DLT 삼각측량 직접 구현
void problem1_dlt_triangulation()
{
    std::cout << "\n------------------------------------" << std::endl;
    std::cout << "문제 1: DLT 삼각측량 직접 구현" << std::endl;
    std::cout << "------------------------------------\n" << std::endl;

    double fx = 500, fy = 500, cx = 320, cy = 240;
    cv::Mat K = (cv::Mat_<double>(3, 3) << fx, 0, cx, 0, fy, cy, 0, 0, 1);

    // 실제 3D 점
    cv::Point3d ground_truth(2.0, 1.0, 8.0);

    // 카메라 1: 원점
    cv::Mat Rt1 = (cv::Mat_<double>(3, 4) << 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0);
    cv::Mat P1 = K * Rt1;

    // 카메라 2: x+0.5m
    cv::Mat Rt2 = (cv::Mat_<double>(3, 4) << 1, 0, 0, 0.5,  0, 1, 0, 0,  0, 0, 1, 0);
    cv::Mat P2 = K * Rt2;

    // 관측 (투영)
    double u1 = fx * ground_truth.x / ground_truth.z + cx;
    double v1 = fy * ground_truth.y / ground_truth.z + cy;
    double u2 = fx * (ground_truth.x - (-0.5)) / ground_truth.z + cx;
    double v2 = fy * ground_truth.y / ground_truth.z + cy;

    std::cout << "관측: 카메라1=(" << u1 << ", " << v1 << "), 카메라2=(" << u2 << ", " << v2 << ")" << std::endl;

    // [답] A 행렬 구성 (4x4)
    // Row 0: u1 * P1.row(2) - P1.row(0)
    // Row 1: v1 * P1.row(2) - P1.row(1)
    // Row 2: u2 * P2.row(2) - P2.row(0)
    // Row 3: v2 * P2.row(2) - P2.row(1)
    cv::Mat A(4, 4, CV_64F);
    cv::Mat row0 = u1 * P1.row(2) - P1.row(0);
    cv::Mat row1 = v1 * P1.row(2) - P1.row(1);
    cv::Mat row2 = u2 * P2.row(2) - P2.row(0);
    cv::Mat row3 = v2 * P2.row(2) - P2.row(1);
    row0.copyTo(A.row(0));
    row1.copyTo(A.row(1));
    row2.copyTo(A.row(2));
    row3.copyTo(A.row(3));

    // [답] SVD 로 풀기
    cv::SVD svd(A, cv::SVD::FULL_UV);
    // V^T 의 마지막 행 = 최소 특이값에 대응하는 특이벡터
    cv::Mat last_row = svd.vt.row(3);
    double W = last_row.at<double>(3);
    cv::Point3d recovered(
        last_row.at<double>(0) / W,
        last_row.at<double>(1) / W,
        last_row.at<double>(2) / W
    );

    std::cout << "\n실제 3D: (" << ground_truth.x << ", " << ground_truth.y << ", " << ground_truth.z << ")" << std::endl;
    std::cout << "복원 3D: (" << recovered.x << ", " << recovered.y << ", " << recovered.z << ")" << std::endl;

    double err = cv::norm(cv::Vec3d(recovered.x - ground_truth.x,
                                     recovered.y - ground_truth.y,
                                     recovered.z - ground_truth.z));
    std::cout << "오차: " << err << " m" << std::endl;

    std::cout << "\n[NOTE] cv::triangulatePoints 가 내부적으로 이 DLT 를 사용합니다" << std::endl;
}

// 문제 2: RANSAC 반복 수 계산
void problem2_ransac_iterations()
{
    std::cout << "\n------------------------------------" << std::endl;
    std::cout << "문제 2: RANSAC 반복 수 계산" << std::endl;
    std::cout << "------------------------------------\n" << std::endl;

    double p = 0.99;  // 성공 확률
    int n = 4;        // PnP 최소 점 수 (EPnP)

    std::vector<double> inlier_ratios = {0.9, 0.7, 0.5, 0.3, 0.1};

    std::cout << "PnP + RANSAC (n=" << n << ", p=" << p << ")\n" << std::endl;

    for (double w : inlier_ratios)
    {
        // [답] N = log(1-p) / log(1-w^n)
        double w_n = std::pow(w, n);
        int N = static_cast<int>(std::ceil(std::log(1.0 - p) / std::log(1.0 - w_n)));

        std::cout << "   inlier 비율 " << (w * 100) << "%: " << N << " 회 반복 필요" << std::endl;
    }

    std::cout << "\n[KEY]" << std::endl;
    std::cout << "   inlier 비율이 낮을수록 RANSAC 반복 수가 급격히 증가" << std::endl;
    std::cout << "   실전에서는 매칭 품질 향상 (outlier 제거) 이 RANSAC 효율을 크게 좌우" << std::endl;
}

int main()
{
    std::cout << "-------------------------------------" << std::endl;
    std::cout << "Phase 2 Week 4 Quiz - Medium [ANSWER]" << std::endl;
    std::cout << "-------------------------------------" << std::endl;

    problem1_dlt_triangulation();
    problem2_ransac_iterations();

    return 0;
}
