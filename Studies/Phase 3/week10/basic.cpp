#include "g2o_advanced.h"
#include <g2o/core/robust_kernel_impl.h>
#include <iostream>
#include <iomanip>
#include <random>

// ═══════════════════════════════════════════════════════════════
// 유틸리티: 합성 BA 그래프 빌드
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

std::vector<G2OAdvanced::ProfileResult> G2OAdvanced::compare_solvers(
    int num_poses, int num_points)
{
    std::vector<ProfileResult> results;

    // LM + Eigen
    {
        g2o::SparseOptimizer opt;
        auto solver = new g2o::OptimizationAlgorithmLevenberg(
            std::make_unique<g2o::BlockSolver_6_3>(
                std::make_unique<g2o::LinearSolverEigen<g2o::BlockSolver_6_3::PoseMatrixType>>()));
        opt.setAlgorithm(solver);
        opt.setVerbose(false);
        build_ba_graph(opt, num_poses, num_points);

        opt.initializeOptimization();
        double init_err = opt.activeChi2();
        auto start = std::chrono::high_resolution_clock::now();
        int iters = opt.optimize(10);
        auto end = std::chrono::high_resolution_clock::now();

        results.push_back({"LM+Eigen",
            std::chrono::duration<double, std::milli>(end - start).count(),
            init_err, opt.activeChi2(), iters});
    }

    // GN + Dense
    {
        g2o::SparseOptimizer opt;
        auto solver = new g2o::OptimizationAlgorithmGaussNewton(
            std::make_unique<g2o::BlockSolver_6_3>(
                std::make_unique<g2o::LinearSolverDense<g2o::BlockSolver_6_3::PoseMatrixType>>()));
        opt.setAlgorithm(solver);
        opt.setVerbose(false);
        build_ba_graph(opt, num_poses, num_points);

        opt.initializeOptimization();
        double init_err = opt.activeChi2();
        auto start = std::chrono::high_resolution_clock::now();
        int iters = opt.optimize(10);
        auto end = std::chrono::high_resolution_clock::now();

        results.push_back({"GN+Dense",
            std::chrono::duration<double, std::milli>(end - start).count(),
            init_err, opt.activeChi2(), iters});
    }

    return results;
}

void G2OAdvanced::analyze_sparsity(
    int num_poses, int num_points, double& sparsity_ratio)
{
    // Hessian 크기: (6*num_poses + 3*num_points)²
    int dim = 6 * num_poses + 3 * num_points;
    int total_elements = dim * dim;

    // 비영 블록 추정: 포즈-포즈 (dense), 포즈-점 (sparse)
    int pose_block = 6 * num_poses * 6 * num_poses;
    int point_block = 3 * num_points * 3 * num_points;
    // 포즈-점 상호작용: 각 점은 평균 ~70% 포즈에서 관측
    int cross_block = 2 * static_cast<int>(6 * num_poses * 3 * num_points * 0.7);

    int nonzero = pose_block + point_block + cross_block;
    sparsity_ratio = (1.0 - static_cast<double>(nonzero) / total_elements) * 100;
}

std::pair<G2OAdvanced::ProfileResult, G2OAdvanced::ProfileResult>
G2OAdvanced::test_marginalization(int num_poses, int num_points)
{
    ProfileResult with_marg, without_marg;

    // Marginalized = true
    {
        g2o::SparseOptimizer opt;
        auto solver = new g2o::OptimizationAlgorithmLevenberg(
            std::make_unique<g2o::BlockSolver_6_3>(
                std::make_unique<g2o::LinearSolverEigen<g2o::BlockSolver_6_3::PoseMatrixType>>()));
        opt.setAlgorithm(solver);
        opt.setVerbose(false);
        build_ba_graph(opt, num_poses, num_points, true);

        opt.initializeOptimization();
        with_marg.initial_error = opt.activeChi2();
        auto start = std::chrono::high_resolution_clock::now();
        with_marg.iterations = opt.optimize(10);
        auto end = std::chrono::high_resolution_clock::now();
        with_marg.final_error = opt.activeChi2();
        with_marg.time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        with_marg.solver_name = "Marginalized";
    }

    // Marginalized = false
    {
        g2o::SparseOptimizer opt;
        auto solver = new g2o::OptimizationAlgorithmLevenberg(
            std::make_unique<g2o::BlockSolver_6_3>(
                std::make_unique<g2o::LinearSolverEigen<g2o::BlockSolver_6_3::PoseMatrixType>>()));
        opt.setAlgorithm(solver);
        opt.setVerbose(false);
        build_ba_graph(opt, num_poses, num_points, false);

        opt.initializeOptimization();
        without_marg.initial_error = opt.activeChi2();
        auto start = std::chrono::high_resolution_clock::now();
        without_marg.iterations = opt.optimize(10);
        auto end = std::chrono::high_resolution_clock::now();
        without_marg.final_error = opt.activeChi2();
        without_marg.time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        without_marg.solver_name = "Non-Marginalized";
    }

    return {with_marg, without_marg};
}

std::vector<G2OAdvanced::ProfileResult> G2OAdvanced::profile_optimization(
    const std::vector<std::pair<int, int>>& sizes)
{
    std::vector<ProfileResult> results;
    for (const auto& [np, npt] : sizes)
    {
        g2o::SparseOptimizer opt;
        auto solver = new g2o::OptimizationAlgorithmLevenberg(
            std::make_unique<g2o::BlockSolver_6_3>(
                std::make_unique<g2o::LinearSolverEigen<g2o::BlockSolver_6_3::PoseMatrixType>>()));
        opt.setAlgorithm(solver);
        opt.setVerbose(false);
        build_ba_graph(opt, np, npt);

        opt.initializeOptimization();
        double init = opt.activeChi2();
        auto start = std::chrono::high_resolution_clock::now();
        int iters = opt.optimize(10);
        auto end = std::chrono::high_resolution_clock::now();

        results.push_back({
            std::to_string(np) + "P+" + std::to_string(npt) + "pt",
            std::chrono::duration<double, std::milli>(end - start).count(),
            init, opt.activeChi2(), iters});
    }
    return results;
}

void G2OAdvanced::orb_slam_pattern(int num_keyframes, int num_points)
{
    std::cout << "  ORB-SLAM BA 패턴:\n" << std::endl;
    std::cout << "  Local BA (매 키프레임):" << std::endl;
    std::cout << "    포즈: 현재 + covisible KFs (~10-20개)" << std::endl;
    std::cout << "    점: 관찰되는 맵 포인트" << std::endl;
    std::cout << "    고정: 나머지 KFs\n" << std::endl;

    std::cout << "  Global BA (Loop Closure 후):" << std::endl;
    std::cout << "    포즈: 전체 키프레임" << std::endl;
    std::cout << "    점: 전체 맵 포인트" << std::endl;
    std::cout << "    별도 스레드에서 실행\n" << std::endl;

    // 시연: Local BA (소규모)
    g2o::SparseOptimizer opt;
    auto solver = new g2o::OptimizationAlgorithmLevenberg(
        std::make_unique<g2o::BlockSolver_6_3>(
            std::make_unique<g2o::LinearSolverEigen<g2o::BlockSolver_6_3::PoseMatrixType>>()));
    opt.setAlgorithm(solver);
    opt.setVerbose(false);
    build_ba_graph(opt, num_keyframes, num_points);

    opt.initializeOptimization();
    double init = opt.activeChi2();
    opt.optimize(5);  // Local BA: 적은 반복
    double final_err = opt.activeChi2();

    std::cout << "  Local BA 시연 (" << num_keyframes << " KF + " << num_points << " pt):" << std::endl;
    std::cout << "    오차: " << std::fixed << std::setprecision(2) << init
              << " → " << final_err << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// 교육 블록 + 데모
// ═══════════════════════════════════════════════════════════════

void G2OAdvanced::demo()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Week 10: g2o 심화 — Solver 내부 동작" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    // ── 블록 1: Solver 비교 ──
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 1] Solver 조합 비교" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    std::cout << "BlockSolver_6_3: 포즈(6 DoF) + 점(3 DoF)" << std::endl;
    std::cout << "  LinearSolver: Eigen (일반), Dense (소규모)\n" << std::endl;

    auto results = compare_solvers(4, 20);
    std::cout << std::fixed << std::setprecision(2);
    for (const auto& r : results)
    {
        std::cout << "  " << r.solver_name
                  << ": " << r.time_ms << "ms"
                  << ", 오차 " << r.initial_error << " → " << r.final_error
                  << " (" << r.iterations << "회)" << std::endl;
    }

    // ── 블록 2: 희소성 분석 ──
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 2] Hessian 희소성" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    for (auto [np, npt] : std::vector<std::pair<int,int>>{{4,20}, {10,100}, {50,500}})
    {
        double sparsity;
        analyze_sparsity(np, npt, sparsity);
        int dim = 6 * np + 3 * npt;
        std::cout << "  " << np << "P+" << npt << "pt: H=" << dim << "x" << dim
                  << ", 희소율=" << sparsity << "%" << std::endl;
    }

    // ── 블록 3: Marginalization 효과 ──
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 3] setMarginalized 효과" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    auto [with_m, without_m] = test_marginalization(4, 30);
    std::cout << "  " << with_m.solver_name << ": " << with_m.time_ms << "ms"
              << ", 오차 " << with_m.final_error << std::endl;
    std::cout << "  " << without_m.solver_name << ": " << without_m.time_ms << "ms"
              << ", 오차 " << without_m.final_error << std::endl;

    // ── 블록 4: ORB-SLAM 패턴 ──
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 4] ORB-SLAM BA 패턴" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    orb_slam_pattern(5, 30);

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  다음 단계:" << std::endl;
    std::cout << "  1. ./my_basic 으로 직접 구현 연습" << std::endl;
    std::cout << "  2. Week 11: Ceres 실습으로 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
}

int main()
{
    G2OAdvanced::demo();
    return 0;
}
