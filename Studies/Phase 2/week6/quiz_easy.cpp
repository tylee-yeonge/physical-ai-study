/**
 * Phase 2 Week 6 - 삼각측량 기초 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

void problem1_depth_from_disparity()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Stereo Depth 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double baseline = 0.12;  // 12cm
    double focal = 600.0;    // 픽셀

    std::cout << "Stereo 카메라 설정:" << std::endl;
    std::cout << "   베이스라인: " << baseline << " m" << std::endl;
    std::cout << "   초점거리: " << focal << " 픽셀\n" << std::endl;

    // TODO: 시차에서 깊이 계산
    // Depth = (baseline * focal) / disparity

    double disparity1 = 60.0;  // 픽셀
    double depth1 = 0.0;       // TODO: 계산하기

    double disparity2 = 30.0;
    double depth2 = 0.0;  // TODO: 계산하기

    std::cout << "시차 " << disparity1 << " 픽셀 → 깊이 ??? m" << std::endl;
    std::cout << "시차 " << disparity2 << " 픽셀 → 깊이 ??? m\n" << std::endl;

    std::cout << "💡 관찰:" << std::endl;
    std::cout << "   - 시차 ↑ → 깊이 ↓ (가까움)" << std::endl;
    std::cout << "   - 시차 ↓ → 깊이 ↑ (멀리)" << std::endl;
    std::cout << "   - 시차 = 0 → 무한대 (매칭 불가)" << std::endl;
}

void problem2_triangulation_geometry()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 삼각측량 기하학" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "두 카메라에서 같은 3D 점을 관측:" << std::endl;
    std::cout << "   카메라 1 방향: ray1" << std::endl;
    std::cout << "   카메라 2 방향: ray2" << std::endl;
    std::cout << "   3D 점 = ray1과 ray2의 교점\n" << std::endl;

    std::cout << "질문: 왜 정확히 교차하지 않나요?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   - 픽셀 양자화 오차" << std::endl;
    std::cout << "   - 특징점 검출 오차" << std::endl;
    std::cout << "   - 카메라 캘리브레이션 오차" << std::endl;
    std::cout << "   → 두 ray가 skew lines (꼬인 위치)\n" << std::endl;

    std::cout << "해결: DLT (Direct Linear Transform)" << std::endl;
    std::cout << "   - 최소제곱법으로 최적 3D 점 찾기" << std::endl;
}

void problem3_reprojection_error()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 재투영 오차" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 3D 점
    cv::Point3f pt3d(1.0, 0.5, 3.0);

    // 카메라 행렬
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);

    // TODO: 투영
    // p = K * [X, Y, Z]^T
    // u = p_x / p_z, v = p_y / p_z

    std::cout << "3D 점: (" << pt3d.x << ", " << pt3d.y << ", " << pt3d.z << ")" << std::endl;
    std::cout << "투영된 2D 점: (???, ???)\n" << std::endl;

    std::cout << "💡 재투영 오차 = ||관측 - 투영|| (픽셀)" << std::endl;
    std::cout << "   - 1 픽셀 이하: 매우 좋음" << std::endl;
    std::cout << "   - 5 픽셀 이상: 문제 있음 (outlier)" << std::endl;
}

void problem4_baseline_vs_accuracy()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Baseline과 정확도" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: Baseline이 크면 좋은가요?\n" << std::endl;

    std::cout << "Baseline ↑ (카메라 간격 넓음):" << std::endl;
    std::cout << "   장점: 깊이 정확도 ↑ (시차 커짐)" << std::endl;
    std::cout << "   단점: 매칭 어려움, 시야 겹침 ↓\n" << std::endl;

    std::cout << "Baseline ↓ (카메라 간격 좁음):" << std::endl;
    std::cout << "   장점: 매칭 쉬움, 시야 겹침 ↑" << std::endl;
    std::cout << "   단점: 깊이 정확도 ↓ (시차 작음)\n" << std::endl;

    std::cout << "💡 실제 응용:" << std::endl;
    std::cout << "   - 실내 (가까움): 짧은 baseline (5-10cm)" << std::endl;
    std::cout << "   - 자율주행 (멀리): 긴 baseline (30-60cm)" << std::endl;
    std::cout << "   - VINS: 연속 프레임 (작은 baseline)" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 6 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_depth_from_disparity() problem2_triangulation_geometry() problem3_reprojection_error()
            problem4_baseline_vs_accuracy()

                std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
