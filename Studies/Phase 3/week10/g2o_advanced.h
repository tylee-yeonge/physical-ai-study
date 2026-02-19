#ifndef PHASE3_WEEK10_G2O_ADVANCED_H_
#define PHASE3_WEEK10_G2O_ADVANCED_H_

#include <g2o/core/sparse_optimizer.h>
#include <g2o/core/block_solver.h>
#include <g2o/core/optimization_algorithm_levenberg.h>
#include <g2o/core/optimization_algorithm_gauss_newton.h>
#include <g2o/solvers/eigen/linear_solver_eigen.h>
#include <g2o/solvers/dense/linear_solver_dense.h>
#include <g2o/types/sba/types_six_dof_expmap.h>

#include <Eigen/Dense>
#include <vector>
#include <string>
#include <chrono>

/**
 * @brief g2o 심화 클래스
 *
 * Solver 조합 비교, 희소성 분석, Marginalization 효과,
 * 성능 프로파일링, ORB-SLAM 패턴 이해.
 *
 * 파이프라인:
 *   compareSolvers → analyzeSparsity → testMarginalization
 *   → profileOptimization → orbSlamPattern
 */
class G2OAdvanced
{
public:
    /// 성능 측정 결과
    struct ProfileResult
    {
        std::string solver_name;
        double time_ms;
        double initial_error;
        double final_error;
        int iterations;
    };

    /**
     * @brief 다양한 Solver 조합으로 BA 실행 및 비교
     * @param num_poses 카메라 포즈 수
     * @param num_points 3D 점 수
     * @return Solver별 성능 결과
     */
    static std::vector<ProfileResult> compare_solvers(
        int num_poses, int num_points);

    /**
     * @brief Hessian 희소 패턴 분석
     * @param num_poses 포즈 수
     * @param num_points 점 수
     * @param sparsity_ratio 희소율 (출력, %)
     */
    static void analyze_sparsity(
        int num_poses, int num_points,
        double& sparsity_ratio);

    /**
     * @brief setMarginalized 효과 비교
     * @param num_poses 포즈 수
     * @param num_points 점 수
     * @return {marginalized 결과, non-marginalized 결과}
     */
    static std::pair<ProfileResult, ProfileResult> test_marginalization(
        int num_poses, int num_points);

    /**
     * @brief 문제 크기별 성능 프로파일링
     * @param sizes {(포즈수, 점수)} 목록
     * @return 크기별 결과
     */
    static std::vector<ProfileResult> profile_optimization(
        const std::vector<std::pair<int, int>>& sizes);

    /**
     * @brief ORB-SLAM 스타일 BA 패턴 시연
     * @param num_keyframes 키프레임 수
     * @param num_points 맵 포인트 수
     */
    static void orb_slam_pattern(int num_keyframes, int num_points);

    /**
     * @brief 전체 데모
     */
    static void demo();
};

#endif  // PHASE3_WEEK10_G2O_ADVANCED_H_
