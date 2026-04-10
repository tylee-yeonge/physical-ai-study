/**
 * @file quiz_easy.cpp
 * @brief Week 4 개념 퀴즈 (Easy)
 *
 * 주제:
 *   - 삼각측량 원리
 *   - PnP 최소 점 수
 *   - 재투영 오차의 의미
 */

#include <iostream>
#include <cmath>
#include <vector>

// 문제 1: 삼각측량 원리
//
// 두 카메라에서 같은 3D 점을 관측하면 각각 2D 좌표 (u₁, v₁), (u₂, v₂) 를 얻음.
// 각 관측은 카메라 원점에서 3D 점 방향으로의 **광선(ray)** 을 정의함.
// 이 두 광선의 교차점이 3D 점의 위치.
//
// 실제로는 노이즈 때문에 두 광선이 정확히 만나지 않음
// → 최소 거리 점 또는 DLT(Direct Linear Transform) 로 추정
//
// TODO: 두 카메라 설정에서 삼각측량의 기하학적 의미를 답하세요
void problem1_triangulation_principle()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 삼각측량 원리" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1: 삼각측량에 카메라가 최소 몇 대 필요한가?" << std::endl;
    std::cout << "A: TODO (답을 적으세요)\n" << std::endl;

    std::cout << "Q2: baseline (두 카메라 사이 거리) 이 길어지면 depth 정확도는 어떻게 되나?" << std::endl;
    std::cout << "A: TODO (답을 적으세요)\n" << std::endl;

    std::cout << "Q3: 두 카메라가 같은 위치에 있으면 (baseline = 0) 삼각측량이 가능한가?" << std::endl;
    std::cout << "A: TODO (답을 적으세요)\n" << std::endl;

    std::cout << "💡 힌트:" << std::endl;
    std::cout << "   baseline 이 길수록 삼각측량 정확도가 높아지지만," << std::endl;
    std::cout << "   너무 길면 매칭이 어려워짐 (trade-off)" << std::endl;
}

// 문제 2: PnP 최소 점 수
//
// PnP = Perspective-n-Point: n개의 3D-2D 대응에서 카메라 포즈 추정
//
// 알고리즘별 최소 점 수:
//   P3P: 3개 (+ 1개로 모호성 해소 → 실질 4개)
//   EPnP: 4개
//   DLT: 6개
//   Iterative (Levenberg-Marquardt): 4개 이상
//
// RANSAC 과 결합 시: 최소 점 수 + 충분한 inlier 필요
//
// TODO: 각 질문에 답하세요
void problem2_pnp_minimum_points()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: PnP 최소 점 수" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1: P3P 알고리즘에 최소 몇 개의 3D-2D 대응이 필요한가?" << std::endl;
    int p3p_min = 0;  // TODO
    std::cout << "A: " << p3p_min << " 개\n" << std::endl;

    std::cout << "Q2: cv::solvePnPRansac 에서 RANSAC 이 필요한 이유는?" << std::endl;
    std::cout << "A: TODO (답을 적으세요)\n" << std::endl;

    std::cout << "Q3: Perception 에서 PnP 가 사용되는 예시를 하나 들어보세요" << std::endl;
    std::cout << "A: TODO (답을 적으세요)\n" << std::endl;

    std::cout << "💡 힌트:" << std::endl;
    std::cout << "   3D Detection 모델이 예측한 3D 박스의 8 코너와" << std::endl;
    std::cout << "   2D 이미지의 박스 코너를 매칭하면 PnP 로 카메라 포즈 검증 가능" << std::endl;
}

// 문제 3: 재투영 오차 계산
//
// 재투영 오차 = ||p_obs - π(K, R, t, X₃D)||₂
//
// 의미:
//   3D 점을 카메라에 투영한 결과와 실제 관측된 2D 점의 픽셀 거리
//   작을수록 추정이 정확
//
// TODO: 주어진 관측/재투영 좌표 쌍에서 재투영 오차를 계산하세요
void problem3_reprojection_error()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 재투영 오차 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    struct Pair
    {
        double obs_x, obs_y;      // 관측된 2D 좌표
        double reproj_x, reproj_y; // 3D→2D 재투영 좌표
    };

    std::vector<Pair> pairs = {
        {320.0, 240.0, 320.5, 240.3},
        {100.0, 200.0, 101.2, 199.5},
        {500.0, 300.0, 503.0, 298.0},
        {250.0, 100.0, 250.1, 100.1},
    };

    std::vector<double> errors;
    for (size_t i = 0; i < pairs.size(); i++)
    {
        // TODO: 유클리드 거리 계산 sqrt((obs_x - reproj_x)² + (obs_y - reproj_y)²)
        double error = 0.0;  // TODO
        errors.push_back(error);

        std::cout << "   점 " << i << ": 관측(" << pairs[i].obs_x << ", " << pairs[i].obs_y
                  << ") 재투영(" << pairs[i].reproj_x << ", " << pairs[i].reproj_y
                  << ") → 오차 = " << error << " px" << std::endl;
    }

    // TODO: 평균 오차 계산
    double mean_error = 0.0;  // TODO

    std::cout << "\n   평균 재투영 오차: " << mean_error << " px" << std::endl;
    std::cout << "\n💡 기준:" << std::endl;
    std::cout << "   < 1.0 px: 우수 (3D Detection 결과 신뢰 가능)" << std::endl;
    std::cout << "   1.0 ~ 3.0 px: 양호" << std::endl;
    std::cout << "   > 5.0 px: 3D 예측 또는 캘리브레이션 문제" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 4 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_triangulation_principle();
    problem2_pnp_minimum_points();
    problem3_reprojection_error();

    return 0;
}
