/**
 * @file quiz_medium.cpp
 * @brief Week 3 구현 퀴즈 (Medium)
 *
 * 주제:
 *   - Rectified 쌍에서 disparity → depth 계산 (실제 좌표 기반)
 *   - 재투영 오차 확인 (rectification 품질 검증)
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <vector>

// 문제 1: 수동 Disparity → Depth + 3D 점 복원
//
// rectified 이미지 쌍에서 수동으로 매칭한 대응점이 주어짐.
// disparity = u_left - u_right (같은 y 행)
// depth = fB / d
// 3D 좌표: X = (u - cx) * Z / fx, Y = (v - cy) * Z / fy
//
// TODO: 각 대응점에 대해 disparity, depth, 3D 좌표를 계산하세요
void problem1_manual_depth_recovery()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 수동 Disparity → Depth + 3D 점 복원" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double fx = 500.0, fy = 500.0;
    double cx = 320.0, cy = 240.0;
    double baseline = 0.12;  // 미터

    // rectified 대응점 (같은 y 좌표)
    struct MatchPair
    {
        cv::Point2f left;
        cv::Point2f right;
    };

    std::vector<MatchPair> pairs = {
        {{400, 200}, {380, 200}},  // disparity = 20
        {{300, 300}, {270, 300}},  // disparity = 30
        {{500, 100}, {490, 100}},  // disparity = 10
        {{100, 400}, {40,  400}},  // disparity = 60
    };

    for (size_t i = 0; i < pairs.size(); i++)
    {
        double u_left = pairs[i].left.x;
        double v = pairs[i].left.y;
        double u_right = pairs[i].right.x;

        // TODO: disparity, depth, 3D 좌표 계산
        double disparity = 0.0;  // TODO: u_left - u_right
        double Z = 0.0;          // TODO: fx * baseline / disparity
        double X = 0.0;          // TODO: (u_left - cx) * Z / fx
        double Y = 0.0;          // TODO: (v - cy) * Z / fy

        std::cout << "   점 " << i << ": left=(" << u_left << ", " << v
                  << "), right=(" << u_right << ", " << v << ")" << std::endl;
        std::cout << "     d=" << disparity << " px, Z=" << Z
                  << " m, 3D=(" << X << ", " << Y << ", " << Z << ")" << std::endl;
    }

    std::cout << "\n💡 힌트:" << std::endl;
    std::cout << "   disparity 가 큰 점 = 가까운 물체" << std::endl;
    std::cout << "   이 계산이 Stereo Depth network 의 출력을 미터 단위로 변환하는 후처리와 동일" << std::endl;
}

// 문제 2: Rectification 품질 검증
//
// 이상적인 rectification 후에는 대응점의 y좌표가 정확히 같아야 한다.
// 실제로는 약간의 오차가 있을 수 있으며, 이를 "y-disparity 오차"로 측정한다.
//
// y-disparity 오차 = |y_left - y_right|
//   < 0.5 px: 우수
//   < 1.0 px: 양호
//   > 2.0 px: Rectification 실패 의심
//
// TODO: 주어진 대응점 쌍의 y-disparity 오차 통계를 계산하세요
void problem2_rectification_quality()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Rectification 품질 검증" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // rectified 후 대응점 (약간의 y 오차 포함)
    std::vector<cv::Point2f> left_pts = {
        {200, 150.2}, {300, 250.1}, {400, 100.5}, {150, 350.0}, {500, 200.3},
        {250, 180.8}, {350, 300.4}, {450, 50.1},  {100, 420.2}, {550, 270.6}
    };
    std::vector<cv::Point2f> right_pts = {
        {180, 150.0}, {270, 250.3}, {390, 100.2}, {90, 350.5},  {490, 200.1},
        {220, 181.0}, {320, 300.0}, {440, 50.0},  {40,  420.8}, {530, 270.2}
    };

    // TODO: 각 쌍의 y-disparity 오차 = |y_left - y_right| 계산
    // TODO: 평균, 최대값, 표준편차 계산
    double sum_error = 0.0;
    double max_error = 0.0;
    std::vector<double> errors;

    for (size_t i = 0; i < left_pts.size(); i++)
    {
        double y_error = 0.0;  // TODO: |left_pts[i].y - right_pts[i].y|
        errors.push_back(y_error);
        sum_error += y_error;
        max_error = std::max(max_error, y_error);
    }

    double mean_error = sum_error / errors.size();

    // TODO: 표준편차 계산
    double std_error = 0.0;  // TODO

    std::cout << "y-disparity 오차 통계:" << std::endl;
    std::cout << "   평균: " << mean_error << " px" << std::endl;
    std::cout << "   최대: " << max_error << " px" << std::endl;
    std::cout << "   표준편차: " << std_error << " px" << std::endl;
    std::cout << std::endl;
    std::cout << "💡 기준:" << std::endl;
    std::cout << "   < 0.5 px: 우수 — Stereo Depth 모델 입력으로 바로 사용 가능" << std::endl;
    std::cout << "   < 1.0 px: 양호" << std::endl;
    std::cout << "   > 2.0 px: Rectification 재수행 필요" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 3 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_manual_depth_recovery();
    problem2_rectification_quality();

    return 0;
}
