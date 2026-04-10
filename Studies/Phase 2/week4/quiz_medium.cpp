/**
 * @file quiz_medium.cpp
 * @brief Week 4 구현 퀴즈 (Medium)
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
//
// DLT (Direct Linear Transform) 삼각측량:
//   두 관측 (u₁,v₁), (u₂,v₂) 와 투영 행렬 P₁, P₂ 로부터
//   AX = 0 형태의 선형 시스템을 만들고 SVD 로 풀기
//
// A 행렬 구성 (4×4):
//   Row 0: u₁·P₁[2] - P₁[0]
//   Row 1: v₁·P₁[2] - P₁[1]
//   Row 2: u₂·P₂[2] - P₂[0]
//   Row 3: v₂·P₂[2] - P₂[1]
//
// SVD(A) 의 마지막 특이벡터(V 의 마지막 열) 가 X (동차 좌표)
//
// TODO: DLT 삼각측량을 직접 구현하세요
void problem1_dlt_triangulation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: DLT 삼각측량 직접 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

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

    // TODO: A 행렬 구성 (4×4)
    // 힌트:
    //   cv::Mat A(4, 4, CV_64F);
    //   P1.row(0), P1.row(1), P1.row(2) 를 사용
    //   Row i: u*P[2] - P[0]  또는  v*P[2] - P[1]

    // TODO: SVD 로 풀기
    //   cv::SVD svd(A, cv::SVD::FULL_UV);
    //   마지막 열 = svd.vt.row(3) → 동차 좌표 [X, Y, Z, W]
    //   3D 좌표 = (X/W, Y/W, Z/W)

    cv::Point3d recovered(0, 0, 0);  // TODO: SVD 결과로 채우기

    std::cout << "\n실제 3D: (" << ground_truth.x << ", " << ground_truth.y << ", " << ground_truth.z << ")" << std::endl;
    std::cout << "복원 3D: (" << recovered.x << ", " << recovered.y << ", " << recovered.z << ")" << std::endl;

    double err = cv::norm(cv::Vec3d(recovered.x - ground_truth.x,
                                     recovered.y - ground_truth.y,
                                     recovered.z - ground_truth.z));
    std::cout << "오차: " << err << " m" << std::endl;

    std::cout << "\n💡 참고: cv::triangulatePoints 가 내부적으로 이 DLT 를 사용합니다" << std::endl;
}

// 문제 2: RANSAC 반복 수 계산
//
// RANSAC 반복 수 공식:
//   N = log(1 - p) / log(1 - wⁿ)
//
//   p: 원하는 성공 확률 (보통 0.99)
//   w: inlier 비율 (0~1)
//   n: 모델 추정에 필요한 최소 점 수
//
// PnP + RANSAC 에서:
//   n = 4 (EPnP 기준)
//   w = inlier 비율 (예: 0.5 → 매칭의 절반이 outlier)
//
// TODO: 다양한 inlier 비율에서 필요한 RANSAC 반복 수를 계산하세요
void problem2_ransac_iterations()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: RANSAC 반복 수 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double p = 0.99;  // 성공 확률
    int n = 4;        // PnP 최소 점 수 (EPnP)

    std::vector<double> inlier_ratios = {0.9, 0.7, 0.5, 0.3, 0.1};

    std::cout << "PnP + RANSAC (n=" << n << ", p=" << p << ")\n" << std::endl;

    for (double w : inlier_ratios)
    {
        // TODO: N = log(1-p) / log(1-w^n) 계산
        int N = 0;  // TODO

        std::cout << "   inlier 비율 " << (w * 100) << "%: " << N << " 회 반복 필요" << std::endl;
    }

    std::cout << "\n💡 핵심:" << std::endl;
    std::cout << "   inlier 비율이 낮을수록 RANSAC 반복 수가 급격히 증가" << std::endl;
    std::cout << "   실전에서는 매칭 품질 향상 (outlier 제거) 이 RANSAC 효율을 크게 좌우" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 4 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_dlt_triangulation();
    problem2_ransac_iterations();

    return 0;
}
