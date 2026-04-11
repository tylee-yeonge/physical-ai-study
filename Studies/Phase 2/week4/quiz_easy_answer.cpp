/**
 * @file quiz_easy_answer.cpp
 * @brief Week 4 개념 퀴즈 (Easy) - 답안
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
void problem1_triangulation_principle()
{
    std::cout << "\n------------------------------------" << std::endl;
    std::cout << "문제 1: 삼각측량 원리" << std::endl;
    std::cout << "------------------------------------\n" << std::endl;

    std::cout << "Q1: 삼각측량에 카메라가 최소 몇 대 필요한가?" << std::endl;
    std::cout << "A: 2대. 하나의 카메라로는 광선 방향만 알 수 있고,"
              << " 두 광선의 교차점으로 3D 위치를 결정한다.\n" << std::endl;

    std::cout << "Q2: baseline (두 카메라 사이 거리) 이 길어지면 depth 정확도는 어떻게 되나?" << std::endl;
    std::cout << "A: 정확도가 높아진다. baseline 이 길수록 두 광선의 교차각이 커져서"
              << " depth 추정의 불확실성이 줄어든다."
              << " 단, 너무 길면 매칭이 어려워지는 trade-off 가 있다.\n" << std::endl;

    std::cout << "Q3: 두 카메라가 같은 위치에 있으면 (baseline = 0) 삼각측량이 가능한가?" << std::endl;
    std::cout << "A: 불가능하다. baseline = 0 이면 두 광선이 동일하여"
              << " 교차점이 정의되지 않는다. (depth 를 결정할 수 없음)\n" << std::endl;

    std::cout << "[TIP]" << std::endl;
    std::cout << "   baseline 이 길수록 삼각측량 정확도가 높아지지만," << std::endl;
    std::cout << "   너무 길면 매칭이 어려워짐 (trade-off)" << std::endl;
}

// 문제 2: PnP 최소 점 수
void problem2_pnp_minimum_points()
{
    std::cout << "\n------------------------------------" << std::endl;
    std::cout << "문제 2: PnP 최소 점 수" << std::endl;
    std::cout << "------------------------------------\n" << std::endl;

    std::cout << "Q1: P3P 알고리즘에 최소 몇 개의 3D-2D 대응이 필요한가?" << std::endl;
    int p3p_min = 4;  // [답] 3개로 최대 4개 해를 구하고, 1개 추가로 모호성 해소
    std::cout << "A: " << p3p_min << " 개 (3개로 최대 4개 해를 구하고, 1개 추가로 모호성 해소)\n" << std::endl;

    std::cout << "Q2: cv::solvePnPRansac 에서 RANSAC 이 필요한 이유는?" << std::endl;
    std::cout << "A: 특징점 매칭에는 outlier (잘못된 대응) 가 섞여 있기 때문."
              << " RANSAC 은 무작위로 최소 점 집합을 선택하여 모델을 추정하고,"
              << " inlier 가 가장 많은 모델을 채택하여 outlier 에 강건한 추정을 한다.\n" << std::endl;

    std::cout << "Q3: Perception 에서 PnP 가 사용되는 예시를 하나 들어보세요" << std::endl;
    std::cout << "A: 3D Detection 모델이 예측한 3D 박스의 8개 코너와"
              << " 2D 이미지의 박스 코너를 매칭하여 카메라 포즈를 검증하거나,"
              << " AR 마커의 알려진 3D 좌표와 검출된 2D 좌표로 카메라 위치를 추정하는 경우.\n" << std::endl;

    std::cout << "[TIP]" << std::endl;
    std::cout << "   3D Detection 모델이 예측한 3D 박스의 8 코너와" << std::endl;
    std::cout << "   2D 이미지의 박스 코너를 매칭하면 PnP 로 카메라 포즈 검증 가능" << std::endl;
}

// 문제 3: 재투영 오차 계산
void problem3_reprojection_error()
{
    std::cout << "\n------------------------------------" << std::endl;
    std::cout << "문제 3: 재투영 오차 계산" << std::endl;
    std::cout << "------------------------------------\n" << std::endl;

    struct Pair
    {
        double obs_x, obs_y;
        double reproj_x, reproj_y;
    };

    std::vector<Pair> pairs = {
        {320.0, 240.0, 320.5, 240.3},
        {100.0, 200.0, 101.2, 199.5},
        {500.0, 300.0, 503.0, 298.0},
        {250.0, 100.0, 250.1, 100.1},
    };

    std::vector<double> errors;
    double sum_error = 0.0;
    for (size_t i = 0; i < pairs.size(); i++)
    {
        // [답] 유클리드 거리 계산
        double dx = pairs[i].obs_x - pairs[i].reproj_x;
        double dy = pairs[i].obs_y - pairs[i].reproj_y;
        double error = std::sqrt(dx * dx + dy * dy);
        errors.push_back(error);
        sum_error += error;

        std::cout << "   점 " << i << ": 관측(" << pairs[i].obs_x << ", " << pairs[i].obs_y
                  << ") 재투영(" << pairs[i].reproj_x << ", " << pairs[i].reproj_y
                  << ") -> 오차 = " << error << " px" << std::endl;
    }

    // [답] 평균 오차 계산
    double mean_error = sum_error / errors.size();

    std::cout << "\n   평균 재투영 오차: " << mean_error << " px" << std::endl;
    std::cout << "\n[KEY]" << std::endl;
    std::cout << "   < 1.0 px: 우수 (3D Detection 결과 신뢰 가능)" << std::endl;
    std::cout << "   1.0 ~ 3.0 px: 양호" << std::endl;
    std::cout << "   > 5.0 px: 3D 예측 또는 캘리브레이션 문제" << std::endl;
}

int main()
{
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Phase 2 Week 4 Quiz - Easy [ANSWER]" << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    problem1_triangulation_principle();
    problem2_pnp_minimum_points();
    problem3_reprojection_error();

    return 0;
}
