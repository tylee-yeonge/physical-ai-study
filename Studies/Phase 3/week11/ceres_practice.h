#ifndef PHASE3_WEEK11_CERES_PRACTICE_H_
#define PHASE3_WEEK11_CERES_PRACTICE_H_

#include <Eigen/Dense>
#include <ceres/ceres.h>
#include <vector>
#include <string>

/**
 * @brief Ceres 실습 클래스
 *
 * BAL 스타일 BA: Snavely 카메라 모델 (9 파라미터),
 * 수렴 과정 관찰, DENSE_SCHUR vs SPARSE_SCHUR 비교.
 *
 * 파이프라인:
 *   parseBALData → defineCostFunction → buildProblem
 *   → solveAndReport → analyzeConvergence
 */
class CeresPractice
{
public:
    /// BAL 관측 데이터
    struct Observation
    {
        int camera_idx;
        int point_idx;
        double x, y;
    };

    /// BAL 형식 데이터
    struct BALData
    {
        int num_cameras;
        int num_points;
        int num_observations;
        std::vector<Observation> observations;
        std::vector<std::array<double, 9>> cameras;  // angle-axis(3) + t(3) + f + k1 + k2
        std::vector<std::array<double, 3>> points;
    };

    /// 최적화 결과
    struct SolveResult
    {
        double initial_cost;
        double final_cost;
        int iterations;
        double solve_time_ms;
        std::vector<double> cost_per_iteration;
    };

    /**
     * @brief Snavely 재투영 오차 (9 파라미터 카메라)
     *
     * camera[0-2]: angle-axis 회전
     * camera[3-5]: 평행이동
     * camera[6]: focal length
     * camera[7-8]: radial distortion k1, k2
     */
    struct SnavelyError
    {
        double observed_x, observed_y;

        SnavelyError(double x, double y) : observed_x(x), observed_y(y) {}

        template <typename T>
        bool operator()(const T* const camera, const T* const point, T* residual) const;

        /**
         * @brief AutoDiffCostFunction 생성 팩토리
         * @param x 관측 x 좌표
         * @param y 관측 y 좌표
         * @return CostFunction 포인터
         */
        static ceres::CostFunction* create(double x, double y);
    };

    /**
     * @brief 합성 BAL 데이터 생성
     * @param num_cameras 카메라 수
     * @param num_points 점 수
     * @param obs_ratio 관측 비율 (0~1)
     * @return BAL 형식 데이터
     */
    static BALData generate_bal_data(
        int num_cameras, int num_points, double obs_ratio = 0.7);

    /**
     * @brief Problem 구성 — ResidualBlock 추가
     * @param problem Ceres Problem
     * @param data BAL 데이터 (카메라/점 배열이 직접 수정됨)
     * @param use_robust HuberLoss 사용 여부
     */
    static void build_problem(
        ceres::Problem& problem,
        BALData& data,
        bool use_robust);

    /**
     * @brief Solver 옵션 설정
     * @param options Solver 옵션 (출력)
     * @param solver_type "dense_schur" 또는 "sparse_schur"
     * @param log_progress iteration별 cost 기록 여부
     */
    static void configure_solver(
        ceres::Solver::Options& options,
        const std::string& solver_type,
        bool log_progress = true);

    /**
     * @brief 최적화 실행 및 결과 반환
     * @param problem Ceres Problem
     * @param options Solver 옵션
     * @return 최적화 결과
     */
    static SolveResult solve_and_report(
        ceres::Problem& problem,
        const ceres::Solver::Options& options);

    /**
     * @brief 수렴 과정 분석 출력
     * @param result 최적화 결과
     */
    static void analyze_convergence(const SolveResult& result);

    /**
     * @brief 전체 데모
     */
    static void demo();
};

#endif  // PHASE3_WEEK11_CERES_PRACTICE_H_
