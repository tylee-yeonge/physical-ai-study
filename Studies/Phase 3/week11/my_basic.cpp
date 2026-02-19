/**
 * Phase 3 Week 11 - Ceres 실습 직접 구현
 *
 * ceres_practice.h의 함수들을 직접 구현해보세요.
 *
 * ┌──────┬────────────────────┬────────┬──────────────┐
 * │ Step │ 함수               │ 난이도  │ 검증 방법     │
 * ├──────┼────────────────────┼────────┼──────────────┤
 * │  1   │ SnavelyError::op() │ 핵심   │ ./my_basic   │
 * │  2   │ build_problem      │ 쉬움   │ ./my_basic   │
 * │  3   │ configure_solver   │ 쉬움   │ ./my_basic   │
 * │  4   │ solve_and_report   │ 쉬움   │ ./my_basic   │
 * │  5   │ analyze_convergence│ 쉬움   │ ./my_basic   │
 * └──────┴────────────────────┴────────┴──────────────┘
 */
#include "ceres_practice.h"
#include <ceres/rotation.h>
#include <iostream>
#include <iomanip>
#include <random>

// ═══════════════════════════════════════════════════════════════
// generate_bal_data — 제공됨 (수정 불필요)
// ═══════════════════════════════════════════════════════════════

CeresPractice::BALData CeresPractice::generate_bal_data(
    int num_cameras, int num_points, double obs_ratio)
{
    std::mt19937 rng(42);
    std::normal_distribution<double> noise_obs(0.0, 0.5);
    std::normal_distribution<double> noise_pt(0.0, 0.3);
    std::normal_distribution<double> noise_cam(0.0, 0.02);
    std::uniform_real_distribution<double> dist_x(-5, 5);
    std::uniform_real_distribution<double> dist_y(-3, 3);
    std::uniform_real_distribution<double> dist_z(5, 25);
    std::uniform_real_distribution<double> coin(0, 1);

    BALData data;
    data.num_cameras = num_cameras;
    data.num_points = num_points;

    double focal_gt = 500.0;

    data.cameras.resize(num_cameras);
    for (int i = 0; i < num_cameras; i++)
    {
        data.cameras[i] = {0, 0, 0, 0, 0, i * 0.5, focal_gt, 0, 0};
        if (i > 0)
        {
            for (int k = 0; k < 3; k++)
                data.cameras[i][k] += noise_cam(rng);
            for (int k = 3; k < 6; k++)
                data.cameras[i][k] += noise_cam(rng) * 3;
            data.cameras[i][6] += noise_cam(rng) * 50;
        }
    }

    data.points.resize(num_points);
    std::vector<Eigen::Vector3d> gt_pts(num_points);
    for (int j = 0; j < num_points; j++)
    {
        gt_pts[j] = Eigen::Vector3d(dist_x(rng), dist_y(rng), dist_z(rng));
        data.points[j] = {gt_pts[j](0) + noise_pt(rng),
                           gt_pts[j](1) + noise_pt(rng),
                           gt_pts[j](2) + noise_pt(rng)};
    }

    for (int i = 0; i < num_cameras; i++)
    {
        Eigen::Vector3d t_gt(0, 0, i * 0.5);
        for (int j = 0; j < num_points; j++)
        {
            if (coin(rng) > obs_ratio)
                continue;
            Eigen::Vector3d Pc = gt_pts[j] + t_gt;
            if (Pc(2) < 1.0)
                continue;
            double xp = -Pc(0) / Pc(2);
            double yp = -Pc(1) / Pc(2);
            double u = focal_gt * xp + noise_obs(rng);
            double v = focal_gt * yp + noise_obs(rng);
            data.observations.push_back({i, j, u, v});
        }
    }
    data.num_observations = static_cast<int>(data.observations.size());
    return data;
}

// ═══════════════════════════════════════════════════════════════
// 구현부
// ═══════════════════════════════════════════════════════════════

// [Step 1] SnavelyError::operator() — 9 파라미터 재투영 오차 (핵심!)
// 힌트:
//   1. ceres::AngleAxisRotatePoint(camera, point, p) — 회전
//   2. p[k] += camera[k+3] — 평행이동
//   3. xp = -p[0]/p[2], yp = -p[1]/p[2] — BAL 정규화 (-z 방향)
//   4. r2 = xp² + yp²
//   5. distortion = 1 + k1*r2 + k2*r2*r2
//   6. predicted = focal * distortion * (xp, yp)
//   7. residual = predicted - observed
// 주의: template<T>이므로 T 타입 사용
// 참고: basic.cpp SnavelyError::operator(), README §2.2
template <typename T>
bool CeresPractice::SnavelyError::operator()(
    const T* const camera, const T* const point, T* residual) const
{
    // TODO: Snavely 재투영 오차 계산
    residual[0] = T(0);
    residual[1] = T(0);
    return true;
}

ceres::CostFunction* CeresPractice::SnavelyError::create(double x, double y)
{
    return new ceres::AutoDiffCostFunction<SnavelyError, 2, 9, 3>(
        new SnavelyError(x, y));
}

// [Step 2] build_problem — Problem에 관측 추가
// 힌트:
//   1. data.observations를 순회
//   2. SnavelyError::create(obs.x, obs.y) → CostFunction
//   3. use_robust이면 new ceres::HuberLoss(1.0)
//   4. problem.AddResidualBlock(cost, loss, camera_data, point_data)
//   5. problem.SetParameterBlockConstant(data.cameras[0].data())
// 참고: basic.cpp build_problem()
void CeresPractice::build_problem(
    ceres::Problem& problem,
    BALData& data,
    bool use_robust)
{
    // TODO: ResidualBlock 추가 + 첫 카메라 고정
}

// [Step 3] configure_solver — Solver 옵션 설정
// 힌트:
//   solver_type == "sparse_schur" → SPARSE_SCHUR, 아니면 DENSE_SCHUR
//   max_num_iterations = 50
//   function_tolerance = 1e-6
//   minimizer_progress_to_stdout = log_progress
// 참고: basic.cpp configure_solver()
void CeresPractice::configure_solver(
    ceres::Solver::Options& options,
    const std::string& solver_type,
    bool log_progress)
{
    // TODO: Solver 옵션 설정
}

// [Step 4] solve_and_report — 최적화 실행 + 결과 반환
// 힌트:
//   1. ceres::Solver::Summary summary;
//   2. ceres::Solve(options, &problem, &summary);
//   3. summary.initial_cost, final_cost, total_time_in_seconds
//   4. summary.iterations[i].cost → cost_per_iteration
// 참고: basic.cpp solve_and_report()
CeresPractice::SolveResult CeresPractice::solve_and_report(
    ceres::Problem& problem,
    const ceres::Solver::Options& options)
{
    SolveResult result;
    // TODO: 최적화 실행 + 결과 저장
    result.initial_cost = -1;
    result.final_cost = -1;
    result.iterations = 0;
    result.solve_time_ms = 0;
    return result;
}

// [Step 5] analyze_convergence — 수렴 과정 분석
// 힌트:
//   1. cost_per_iteration 순회
//   2. 각 iteration의 cost, change, reduction(%) 출력
//   3. 총 감소율 출력
// 참고: basic.cpp analyze_convergence()
void CeresPractice::analyze_convergence(const SolveResult& result)
{
    // TODO: 수렴 과정 분석 출력
    std::cout << "  TODO: 수렴 분석 출력" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] Ceres 실습 — BAL 모델" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 데이터 생성
    auto data = CeresPractice::generate_bal_data(4, 20, 0.8);
    std::cout << "BAL 데이터: " << data.num_cameras << " 카메라, "
              << data.num_points << " 점, "
              << data.num_observations << " 관측\n" << std::endl;

    // ── Step 2 테스트: build_problem ──
    std::cout << "[Step 2] build_problem" << std::endl;
    ceres::Problem problem;
    CeresPractice::build_problem(problem, data, true);
    bool step2 = (problem.NumResidualBlocks() > 0);
    std::cout << "  Residual Blocks: " << problem.NumResidualBlocks()
              << (step2 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 3 테스트: configure_solver ──
    std::cout << "[Step 3] configure_solver" << std::endl;
    ceres::Solver::Options options;
    CeresPractice::configure_solver(options, "dense_schur", false);
    bool step3 = (options.max_num_iterations > 0);
    std::cout << "  Solver: "
              << (options.linear_solver_type == ceres::DENSE_SCHUR ? "DENSE_SCHUR" : "other")
              << ", max_iter=" << options.max_num_iterations
              << (step3 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 4 테스트: solve_and_report ──
    std::cout << "[Step 4] solve_and_report" << std::endl;
    auto result = CeresPractice::solve_and_report(problem, options);
    bool step4 = (result.final_cost >= 0 && result.final_cost <= result.initial_cost);
    std::cout << "  초기: " << std::scientific << std::setprecision(3)
              << result.initial_cost
              << " → 최종: " << result.final_cost
              << (step4 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 5 테스트: analyze_convergence ──
    std::cout << "[Step 5] analyze_convergence" << std::endl;
    CeresPractice::analyze_convergence(result);

    // ── 종합 결과 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  종합: Step2 " << (step2 ? "✅" : "❌")
              << " Step3 " << (step3 ? "✅" : "❌")
              << " Step4 " << (step4 ? "✅" : "❌") << std::endl;
    std::cout << "  (Step 1은 Step 4 성공 시 자동 검증)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
