#include "local_ba_ceres.h"

#include <iostream>

namespace slam {

LocalBACeres::LocalBACeres(const cv::Mat& K, int max_iterations)
    : K_(K.clone()), max_iterations_(max_iterations)
{
}

// TODO (W8): Ceres BA 구현
//
// 구현 순서:
//  1. 각 키프레임 포즈를 Rodrigues 벡터(3) + 이동(3)으로 변환
//  2. 각 관측(키프레임-맵포인트 쌍)에 대해 재투영 오차 비용 함수 추가
//     - CostFunction: ceres::SizedCostFunction<2, 6, 3> 또는 AutoDiffCostFunction
//     - 잔차 = π(K * exp(ξ) * X) - x_obs  (2차원)
//  3. 첫 번째 키프레임은 SetParameterBlockConstant()로 고정
//  4. Solver::Options 설정 (linear_solver_type = DENSE_SCHUR)
//  5. Solve 후 결과를 Map에 반영
//
// 참고: ceres/ceres.h, ceres/rotation.h 필요
double LocalBACeres::optimize(Map& map) const
{
    std::cout << "[LocalBACeres] TODO: Ceres BA 미구현" << std::endl;
    std::cout << "  키프레임 수: " << map.numKeyframes() << std::endl;
    std::cout << "  맵 포인트 수: " << map.numMapPoints() << std::endl;
    std::cout << "  최대 반복: " << max_iterations_ << std::endl;
    (void)K_;  // 미사용 경고 방지
    return -1.0;
}

}  // namespace slam
