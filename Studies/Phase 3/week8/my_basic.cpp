/**
 * Phase 3 Week 8 - Ceres BA 직접 구현
 *
 * ceres_ba.h의 함수들을 직접 구현해보세요.
 *
 * ┌──────┬─────────────────────────┬────────┬──────────────┐
 * │ Step │ 함수                    │ 난이도  │ 검증 방법     │
 * ├──────┼─────────────────────────┼────────┼──────────────┤
 * │  1   │ ReprojectionError::op() │ 핵심   │ ./my_basic   │
 * │  2   │ add_residual_block      │ 쉬움   │ ./my_basic   │
 * │  3   │ configure_solver        │ 쉬움   │ ./my_basic   │
 * │  4   │ solve                   │ 쉬움   │ ./my_basic   │
 * │  5   │ compare_with_g2o        │ 쉬움   │ ./my_basic   │
 * └──────┴─────────────────────────┴────────┴──────────────┘
 */
#include "ceres_ba.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <ceres/rotation.h>

// [Step 1] ReprojectionError::operator() — 재투영 오차 계산 (핵심!)
// 힌트:
//   1. pw[i] = point[i] - camera[i+3]  (월드→카메라 이동)
//   2. ceres::AngleAxisRotatePoint(camera, pw, p) (회전)
//   3. u = fx * p[0]/p[2] + cx,  v = fy * p[1]/p[2] + cy
//   4. residual[0] = u - observed_x,  residual[1] = v - observed_y
// 주의: template<T>이므로 T 타입 사용 (T(3.0) 등)
// 참고: basic.cpp ReprojectionError::operator()
template <typename T>
bool CeresBA::ReprojectionError::operator()(
    const T* const camera, const T* const point, T* residual) const
{
    // TODO: 재투영 오차 계산
    residual[0] = T(0);
    residual[1] = T(0);
    return true;
}

ceres::CostFunction* CeresBA::ReprojectionError::create(
    double x, double y, double fx, double fy, double cx, double cy)
{
    return new ceres::AutoDiffCostFunction<ReprojectionError, 2, 6, 3>(
        new ReprojectionError(x, y, fx, fy, cx, cy));
}

// [Step 2] add_residual_block — Problem에 관측 추가
// 힌트:
//   1. ReprojectionError::create(x, y, K(0), K(1), K(2), K(3))
//   2. use_robust이면 new ceres::HuberLoss(1.0)
//   3. problem.AddResidualBlock(cost, loss, camera, point)
// 참고: basic.cpp add_residual_block()
void CeresBA::add_residual_block(
    ceres::Problem& problem,
    double* camera, double* point,
    double observed_x, double observed_y,
    const Eigen::Vector4d& K,
    bool use_robust)
{
    // TODO: ResidualBlock 추가
}

// [Step 3] configure_solver — Solver 옵션 설정
// 힌트:
//   use_schur → DENSE_SCHUR, 아니면 DENSE_QR
//   max_num_iterations = 50
//   minimizer_progress_to_stdout = false
// 참고: basic.cpp configure_solver()
void CeresBA::configure_solver(
    ceres::Solver::Options& options,
    bool use_schur)
{
    // TODO: Solver 옵션 설정
}

// [Step 4] solve — 최적화 실행
// 힌트:
//   ceres::Solver::Summary summary;
//   ceres::Solve(options, &problem, &summary);
//   summary.initial_cost, final_cost, iterations, total_time 사용
// 참고: basic.cpp solve()
CeresBA::BAResult CeresBA::solve(
    ceres::Problem& problem,
    const ceres::Solver::Options& options)
{
    BAResult result;
    // TODO: 최적화 실행 + 결과 저장
    result.initial_cost = -1;
    result.final_cost = -1;
    result.iterations = 0;
    result.solve_time_ms = 0;
    return result;
}

// [Step 5] compare_with_g2o — 비교 출력
// 힌트: ceres_result와 g2o_final_cost 출력
void CeresBA::compare_with_g2o(
    const BAResult& ceres_result,
    double g2o_final_cost)
{
    // TODO: 비교 출력
    std::cout << "  TODO: g2o vs Ceres 비교 출력" << std::endl;
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] Ceres Bundle Adjustment" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Eigen::Vector4d K(500, 500, 320, 240);

    std::mt19937 rng(42);
    std::normal_distribution<double> noise(0.0, 1.0);
    std::normal_distribution<double> noise_pt(0.0, 0.3);
    std::uniform_real_distribution<double> dist_x(-3, 3);
    std::uniform_real_distribution<double> dist_y(-2, 2);
    std::uniform_real_distribution<double> dist_z(3, 12);

    const int kNumPoses = 4;
    const int kNumPoints = 10;

    // GT 데이터
    std::vector<Eigen::Vector3d> gt_t, gt_pts;
    for (int i = 0; i < kNumPoses; i++)
        gt_t.push_back(Eigen::Vector3d(0, 0, i * 0.5));
    for (int j = 0; j < kNumPoints; j++)
        gt_pts.push_back(Eigen::Vector3d(dist_x(rng), dist_y(rng), dist_z(rng)));

    // Ceres 파라미터 배열
    std::vector<std::array<double, 6>> cameras(kNumPoses);
    std::vector<std::array<double, 3>> points(kNumPoints);

    for (int i = 0; i < kNumPoses; i++)
    {
        cameras[i] = {0, 0, 0, gt_t[i](0), gt_t[i](1), gt_t[i](2)};
        if (i > 0)
        {
            cameras[i][3] += noise(rng) * 0.1;
            cameras[i][4] += noise(rng) * 0.1;
            cameras[i][5] += noise(rng) * 0.1;
        }
    }
    for (int j = 0; j < kNumPoints; j++)
    {
        points[j] = {gt_pts[j](0) + noise_pt(rng),
                      gt_pts[j](1) + noise_pt(rng),
                      gt_pts[j](2) + noise_pt(rng)};
    }

    // ── Step 2 테스트: add_residual_block ──
    std::cout << "[Step 2] add_residual_block" << std::endl;
    ceres::Problem problem;
    int obs_count = 0;
    for (int i = 0; i < kNumPoses; i++)
    {
        for (int j = 0; j < kNumPoints; j++)
        {
            Eigen::Vector3d Pc = gt_pts[j] - gt_t[i];
            if (Pc(2) < 0.1)
                continue;
            double u = K(0) * Pc(0) / Pc(2) + K(2) + noise(rng);
            double v = K(1) * Pc(1) / Pc(2) + K(3) + noise(rng);
            CeresBA::add_residual_block(
                problem, cameras[i].data(), points[j].data(),
                u, v, K, true);
            obs_count++;
        }
    }
    bool step2 = (problem.NumResidualBlocks() > 0);
    std::cout << "  관측: " << obs_count
              << ", Residual Blocks: " << problem.NumResidualBlocks()
              << (step2 ? " ✅" : " ❌") << "\n" << std::endl;

    // 첫 포즈 고정
    problem.SetParameterBlockConstant(cameras[0].data());

    // ── Step 3 테스트: configure_solver ──
    std::cout << "[Step 3] configure_solver" << std::endl;
    ceres::Solver::Options options;
    CeresBA::configure_solver(options, true);
    bool step3 = (options.max_num_iterations > 0);
    std::cout << "  Solver: "
              << (options.linear_solver_type == ceres::DENSE_SCHUR ? "DENSE_SCHUR" : "other")
              << (step3 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 4 테스트: solve ──
    std::cout << "[Step 4] solve" << std::endl;
    auto result = CeresBA::solve(problem, options);
    bool step4 = (result.final_cost >= 0 && result.final_cost <= result.initial_cost);
    std::cout << "  초기: " << result.initial_cost
              << " → 최종: " << result.final_cost
              << (step4 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 5 테스트: compare_with_g2o ──
    std::cout << "[Step 5] compare_with_g2o" << std::endl;
    CeresBA::compare_with_g2o(result, 0.5);  // g2o 결과 예시

    // ── 종합 결과 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  종합: Step2 " << (step2 ? "✅" : "❌")
              << " Step3 " << (step3 ? "✅" : "❌")
              << " Step4 " << (step4 ? "✅" : "❌") << std::endl;
    std::cout << "  (Step 1은 Step 4 성공 시 자동 검증)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
