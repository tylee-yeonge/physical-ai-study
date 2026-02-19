/**
 * Phase 3 Week 10 - g2o 심화: Solver 내부 동작 직접 구현
 *
 * g2o_advanced.h의 함수들을 직접 구현해보세요.
 *
 * ┌──────┬──────────────────────┬────────┬──────────────┐
 * │ Step │ 함수                 │ 난이도  │ 검증 방법     │
 * ├──────┼──────────────────────┼────────┼──────────────┤
 * │  1   │ compare_solvers      │ 핵심   │ ./my_basic   │
 * │  2   │ analyze_sparsity     │ 쉬움   │ ./my_basic   │
 * │  3   │ test_marginalization │ 핵심   │ ./my_basic   │
 * │  4   │ profile_optimization │ 쉬움   │ ./my_basic   │
 * │  5   │ orb_slam_pattern     │ 쉬움   │ ./my_basic   │
 * └──────┴──────────────────────┴────────┴──────────────┘
 */
#include "g2o_advanced.h"
#include <g2o/core/robust_kernel_impl.h>
#include <iostream>
#include <iomanip>
#include <random>

// ═══════════════════════════════════════════════════════════════
// 유틸리티: 합성 BA 그래프 빌드 (제공됨 — 수정 불필요)
// ═══════════════════════════════════════════════════════════════

static void build_ba_graph(
    g2o::SparseOptimizer& optimizer,
    int num_poses, int num_points, bool marginalize_points = true)
{
    std::mt19937 rng(42);
    std::normal_distribution<double> noise_obs(0.0, 1.0);
    std::normal_distribution<double> noise_pt(0.0, 0.3);
    std::normal_distribution<double> noise_t(0.0, 0.1);
    std::uniform_real_distribution<double> dist_x(-5, 5);
    std::uniform_real_distribution<double> dist_y(-3, 3);
    std::uniform_real_distribution<double> dist_z(3, 20);

    double fx = 500, cx = 320, cy = 240;
    const int kPointIdOffset = 1000;

    // 포즈 Vertex
    for (int i = 0; i < num_poses; i++)
    {
        auto* v = new g2o::VertexSE3Expmap();
        v->setId(i);
        Eigen::Vector3d t_gt(0, 0, i * 0.5);
        Eigen::Vector3d t_noisy = t_gt;
        if (i > 0)
            t_noisy += Eigen::Vector3d(noise_t(rng), noise_t(rng), noise_t(rng));
        v->setEstimate(g2o::SE3Quat(Eigen::Matrix3d::Identity(), t_noisy));
        v->setFixed(i == 0);
        optimizer.addVertex(v);
    }

    // 3D 점 Vertex
    for (int j = 0; j < num_points; j++)
    {
        auto* v = new g2o::VertexPointXYZ();
        v->setId(kPointIdOffset + j);
        Eigen::Vector3d pt(dist_x(rng), dist_y(rng), dist_z(rng));
        Eigen::Vector3d pt_noisy = pt + Eigen::Vector3d(noise_pt(rng), noise_pt(rng), noise_pt(rng));
        v->setEstimate(pt_noisy);
        v->setMarginalized(marginalize_points);
        optimizer.addVertex(v);

        // 관측 Edge
        auto* cam_param = new g2o::CameraParameters(fx, Eigen::Vector2d(cx, cy), 0);
        cam_param->setId(0);
        optimizer.addParameter(cam_param);

        for (int i = 0; i < num_poses; i++)
        {
            Eigen::Vector3d t_gt(0, 0, i * 0.5);
            Eigen::Vector3d Pc = pt - t_gt;
            if (Pc(2) < 0.5)
                continue;
            double u = fx * Pc(0) / Pc(2) + cx + noise_obs(rng);
            double v_px = fx * Pc(1) / Pc(2) + cy + noise_obs(rng);

            auto* e = new g2o::EdgeProjectXYZ2UV();
            e->setVertex(0, optimizer.vertex(kPointIdOffset + j));
            e->setVertex(1, optimizer.vertex(i));
            e->setMeasurement(Eigen::Vector2d(u, v_px));
            e->setInformation(Eigen::Matrix2d::Identity());
            e->setParameterId(0, 0);
            optimizer.addEdge(e);
        }
    }
}

// ═══════════════════════════════════════════════════════════════
// 구현부
// ═══════════════════════════════════════════════════════════════

// [Step 1] compare_solvers — Solver 조합 비교 (핵심!)
// 힌트:
//   1. LM + Eigen:
//      OptimizationAlgorithmLevenberg + BlockSolver_6_3 + LinearSolverEigen
//   2. GN + Dense:
//      OptimizationAlgorithmGaussNewton + BlockSolver_6_3 + LinearSolverDense
//   3. build_ba_graph(opt, num_poses, num_points) 로 그래프 구성
//   4. opt.initializeOptimization(), opt.optimize(10)
//   5. chrono로 시간 측정, activeChi2()로 오차
// 참고: basic.cpp compare_solvers()
std::vector<G2OAdvanced::ProfileResult> G2OAdvanced::compare_solvers(
    int num_poses, int num_points)
{
    std::vector<ProfileResult> results;
    // TODO: LM+Eigen, GN+Dense 두 조합을 각각 실행하고 결과 비교
    return results;
}

// [Step 2] analyze_sparsity — Hessian 희소성 분석
// 힌트:
//   1. dim = 6*num_poses + 3*num_points (Hessian 크기)
//   2. total = dim * dim
//   3. pose_block = (6*num_poses)²
//   4. point_block = (3*num_points)²
//   5. cross_block = 2 * 6*num_poses * 3*num_points * 0.7 (관측 비율)
//   6. sparsity_ratio = (1 - nonzero/total) * 100
// 참고: basic.cpp analyze_sparsity()
void G2OAdvanced::analyze_sparsity(
    int num_poses, int num_points, double& sparsity_ratio)
{
    // TODO: 희소율 계산
    sparsity_ratio = 0.0;
}

// [Step 3] test_marginalization — Marginalize 효과 비교 (핵심!)
// 힌트:
//   1. build_ba_graph(..., true)  → marginalized
//   2. build_ba_graph(..., false) → non-marginalized
//   3. 같은 Solver (LM+Eigen) 으로 각각 실행
//   4. 시간과 오차 비교
// 참고: basic.cpp test_marginalization()
std::pair<G2OAdvanced::ProfileResult, G2OAdvanced::ProfileResult>
G2OAdvanced::test_marginalization(int num_poses, int num_points)
{
    ProfileResult with_marg, without_marg;
    // TODO: marginalized vs non-marginalized 비교
    with_marg.solver_name = "Marginalized";
    without_marg.solver_name = "Non-Marginalized";
    return {with_marg, without_marg};
}

// [Step 4] profile_optimization — 다양한 크기에서 성능 측정
// 힌트:
//   1. sizes 벡터를 순회: for (auto& [np, npt] : sizes)
//   2. 각각 LM+Eigen으로 BA 실행
//   3. solver_name을 "4P+20pt" 형식으로
// 참고: basic.cpp profile_optimization()
std::vector<G2OAdvanced::ProfileResult> G2OAdvanced::profile_optimization(
    const std::vector<std::pair<int, int>>& sizes)
{
    std::vector<ProfileResult> results;
    // TODO: 각 크기별 BA 실행 + 결과 수집
    return results;
}

// [Step 5] orb_slam_pattern — ORB-SLAM BA 패턴 시연
// 힌트:
//   1. Local BA vs Global BA 설명 출력
//   2. build_ba_graph(opt, num_keyframes, num_points)
//   3. opt.optimize(5)  — Local BA는 적은 반복
//   4. 오차 변화 출력
// 참고: basic.cpp orb_slam_pattern()
void G2OAdvanced::orb_slam_pattern(int num_keyframes, int num_points)
{
    // TODO: ORB-SLAM 패턴 설명 + Local BA 시연
    std::cout << "  TODO: ORB-SLAM BA 패턴 시연" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] g2o 심화 — Solver 내부 동작" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ── Step 1 테스트: compare_solvers ──
    std::cout << "[Step 1] compare_solvers" << std::endl;
    auto results = G2OAdvanced::compare_solvers(4, 20);
    bool step1 = (results.size() >= 2);
    if (step1)
    {
        std::cout << std::fixed << std::setprecision(2);
        for (const auto& r : results)
        {
            std::cout << "  " << r.solver_name
                      << ": " << r.time_ms << "ms"
                      << ", 오차 " << r.initial_error << " → " << r.final_error
                      << " (" << r.iterations << "회)" << std::endl;
        }
    }
    std::cout << "  결과: " << results.size() << "개 Solver"
              << (step1 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 2 테스트: analyze_sparsity ──
    std::cout << "[Step 2] analyze_sparsity" << std::endl;
    double sparsity = 0;
    G2OAdvanced::analyze_sparsity(10, 100, sparsity);
    bool step2 = (sparsity > 0);
    std::cout << "  10P+100pt: 희소율=" << std::fixed << std::setprecision(1)
              << sparsity << "%"
              << (step2 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 3 테스트: test_marginalization ──
    std::cout << "[Step 3] test_marginalization" << std::endl;
    auto [with_m, without_m] = G2OAdvanced::test_marginalization(4, 30);
    bool step3 = (with_m.time_ms > 0 && without_m.time_ms > 0);
    if (step3)
    {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  " << with_m.solver_name << ": " << with_m.time_ms << "ms"
                  << ", 오차 " << with_m.final_error << std::endl;
        std::cout << "  " << without_m.solver_name << ": " << without_m.time_ms << "ms"
                  << ", 오차 " << without_m.final_error << std::endl;
    }
    std::cout << "  " << (step3 ? "✅" : "❌") << "\n" << std::endl;

    // ── Step 4 테스트: profile_optimization ──
    std::cout << "[Step 4] profile_optimization" << std::endl;
    auto profiles = G2OAdvanced::profile_optimization({{4, 20}, {8, 50}});
    bool step4 = (profiles.size() == 2);
    if (step4)
    {
        for (const auto& r : profiles)
        {
            std::cout << "  " << r.solver_name
                      << ": " << r.time_ms << "ms" << std::endl;
        }
    }
    std::cout << "  결과: " << profiles.size() << "개"
              << (step4 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 5 테스트: orb_slam_pattern ──
    std::cout << "[Step 5] orb_slam_pattern" << std::endl;
    G2OAdvanced::orb_slam_pattern(5, 30);

    // ── 종합 결과 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  종합: Step1 " << (step1 ? "✅" : "❌")
              << " Step2 " << (step2 ? "✅" : "❌")
              << " Step3 " << (step3 ? "✅" : "❌")
              << " Step4 " << (step4 ? "✅" : "❌") << std::endl;
    std::cout << "  (Step 5는 출력 확인)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
