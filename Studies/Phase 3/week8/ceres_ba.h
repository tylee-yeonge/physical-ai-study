#ifndef PHASE3_WEEK8_CERES_BA_H_
#define PHASE3_WEEK8_CERES_BA_H_

#include <ceres/ceres.h>
#include <Eigen/Dense>
#include <vector>
#include <string>

/**
 * @brief Ceres 기반 Bundle Adjustment 기초 클래스
 *
 * Ceres 자동 미분으로 BA 문제를 풀고, g2o와 비교한다.
 *
 * 파이프라인:
 *   defineCostFunction → addResidualBlock → configureSolver
 *   → solve → compareWithG2o
 */
class CeresBA
{
public:
    /// BA 결과 구조체
    struct BAResult
    {
        double initial_cost;
        double final_cost;
        int iterations;
        int num_residuals;
        double solve_time_ms;
    };

    /**
     * @brief 재투영 오차 CostFunction (자동 미분용)
     *
     * operator()(camera, point, residual)
     * camera[0-2]: angle-axis, camera[3-5]: translation
     * point[0-2]: 3D 점 좌표
     * residual[0-1]: 재투영 오차 (u, v)
     */
    struct ReprojectionError
    {
        double observed_x, observed_y;
        double fx, fy, cx, cy;

        ReprojectionError(double x, double y, double fx, double fy, double cx, double cy)
            : observed_x(x), observed_y(y), fx(fx), fy(fy), cx(cx), cy(cy) {}

        template <typename T>
        bool operator()(const T* const camera, const T* const point, T* residual) const;

        static ceres::CostFunction* create(
            double x, double y, double fx, double fy, double cx, double cy);
    };

    /**
     * @brief Problem에 관측 추가
     * @param problem Ceres Problem
     * @param camera 카메라 파라미터 (6: angle-axis + translation)
     * @param point 3D 점 (3)
     * @param observed_x, observed_y 관측된 2D 좌표
     * @param K 카메라 내부 파라미터 (fx, fy, cx, cy)
     * @param use_robust Huber Loss 사용 여부
     */
    static void add_residual_block(
        ceres::Problem& problem,
        double* camera, double* point,
        double observed_x, double observed_y,
        const Eigen::Vector4d& K,
        bool use_robust = false);

    /**
     * @brief Solver 옵션 설정
     * @param options Ceres Solver 옵션
     * @param use_schur DENSE_SCHUR 사용 여부
     */
    static void configure_solver(
        ceres::Solver::Options& options,
        bool use_schur = true);

    /**
     * @brief 최적화 실행
     * @param problem Ceres Problem
     * @param options Solver 옵션
     * @return BA 결과
     */
    static BAResult solve(
        ceres::Problem& problem,
        const ceres::Solver::Options& options);

    /**
     * @brief g2o 결과와 비교 출력
     * @param ceres_result Ceres 결과
     * @param g2o_final_cost g2o 최종 오차 (비교용)
     */
    static void compare_with_g2o(
        const BAResult& ceres_result,
        double g2o_final_cost);

    /**
     * @brief 전체 데모
     */
    static void demo();
};

#endif  // PHASE3_WEEK8_CERES_BA_H_
