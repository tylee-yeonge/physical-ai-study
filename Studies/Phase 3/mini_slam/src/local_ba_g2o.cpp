#include "local_ba_g2o.h"

#include <iostream>

namespace slam {

LocalBAG2O::LocalBAG2O(const cv::Mat& K,
                       int max_iterations,
                       bool use_schur)
    : K_(K.clone()), max_iterations_(max_iterations), use_schur_(use_schur)
{
}

// TODO (W9-W10): g2o BA 구현
//
// 구현 순서:
//  1. g2o::SparseOptimizer 생성
//  2. Solver 설정:
//     - use_schur_ == true:
//       BlockSolver<BlockSolverTraits<6, 3>> + LinearSolverSchur
//     - use_schur_ == false:
//       BlockSolver<BlockSolverTraits<6, 3>> + LinearSolverDense
//  3. 정점 추가:
//     - g2o::VertexSE3Expmap (카메라 포즈) — 첫 키프레임 setFixed(true)
//     - g2o::VertexPointXYZ  (맵 포인트)   — setMarginalized(true)
//  4. 간선 추가:
//     - g2o::EdgeProjectXYZ2UV (재투영 오차)
//     - 정보 행렬: Ω = I₂ (1 픽셀 노이즈 가정)
//     - 로버스트 커널: Huber (δ = √5.99)
//  5. optimizer.optimize(max_iterations_)
//  6. 최적화된 포즈와 포인트를 Map에 반영
//
// 참고: g2o/core, g2o/types/sba 필요
double LocalBAG2O::optimize(Map& map) const
{
    std::cout << "[LocalBAG2O] TODO: g2o BA 미구현" << std::endl;
    std::cout << "  키프레임 수: " << map.numKeyframes() << std::endl;
    std::cout << "  맵 포인트 수: " << map.numMapPoints() << std::endl;
    std::cout << "  최대 반복: " << max_iterations_ << std::endl;
    std::cout << "  Schur: " << (use_schur_ ? "ON" : "OFF") << std::endl;
    (void)K_;  // 미사용 경고 방지
    return -1.0;
}

}  // namespace slam
