#include "ba_basic.h"
#include <iostream>
#include <iomanip>
#include <random>

// ═══════════════════════════════════════════════════════════════
// 구현부
// ═══════════════════════════════════════════════════════════════

void BABasic::create_vertex_pose(
    g2o::SparseOptimizer& optimizer,
    int id,
    const Eigen::Matrix3d& R,
    const Eigen::Vector3d& t,
    bool fixed)
{
    auto* v = new g2o::VertexSE3Expmap();
    v->setId(id);
    v->setEstimate(g2o::SE3Quat(R, t));
    v->setFixed(fixed);
    optimizer.addVertex(v);
}

void BABasic::create_vertex_point(
    g2o::SparseOptimizer& optimizer,
    int id,
    const Eigen::Vector3d& point,
    bool marginalized)
{
    auto* v = new g2o::VertexPointXYZ();
    v->setId(id);
    v->setEstimate(point);
    v->setMarginalized(marginalized);
    optimizer.addVertex(v);
}

void BABasic::create_edge(
    g2o::SparseOptimizer& optimizer,
    int pose_id, int point_id,
    const Eigen::Vector2d& observation,
    const Eigen::Vector4d& K,
    bool use_robust)
{
    auto* e = new g2o::EdgeProjectXYZ2UV();
    e->setVertex(0, optimizer.vertex(point_id));
    e->setVertex(1, optimizer.vertex(pose_id));
    e->setMeasurement(observation);
    e->setInformation(Eigen::Matrix2d::Identity());

    // 카메라 내부 파라미터 설정
    auto* cam = new g2o::CameraParameters(K(0), Eigen::Vector2d(K(2), K(3)), 0);
    cam->setId(0);
    // addParameter는 이미 존재하면 false — 무시
    if (!optimizer.addParameter(cam))
    {
        delete cam;
    }
    e->setParameterId(0, 0);

    if (use_robust)
    {
        auto* rk = new g2o::RobustKernelHuber();
        rk->setDelta(1.0);
        e->setRobustKernel(rk);
    }

    optimizer.addEdge(e);
}

void BABasic::setup_optimizer(g2o::SparseOptimizer& optimizer)
{
    // BlockSolver_6_3: 포즈 6자유도 + 3D점 3자유도
    using BlockSolverType = g2o::BlockSolver_6_3;
    using LinearSolverType = g2o::LinearSolverEigen<BlockSolverType::PoseMatrixType>;

    auto solver = new g2o::OptimizationAlgorithmLevenberg(
        std::make_unique<BlockSolverType>(
            std::make_unique<LinearSolverType>()));

    optimizer.setAlgorithm(solver);
    optimizer.setVerbose(false);
}

BABasic::BAResult BABasic::optimize(g2o::SparseOptimizer& optimizer, int iterations)
{
    BAResult result;
    result.num_poses = 0;
    result.num_points = 0;
    result.num_edges = static_cast<int>(optimizer.edges().size());

    for (auto* v : optimizer.vertices())
    {
        if (dynamic_cast<g2o::VertexSE3Expmap*>(v.second))
            result.num_poses++;
        else if (dynamic_cast<g2o::VertexPointXYZ*>(v.second))
            result.num_points++;
    }

    optimizer.initializeOptimization();
    result.initial_error = optimizer.activeChi2();
    result.iterations = optimizer.optimize(iterations);
    result.final_error = optimizer.activeChi2();

    return result;
}

void BABasic::evaluate_result(
    const g2o::SparseOptimizer& optimizer,
    const BAResult& result,
    const std::string& label)
{
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  [" << label << "] 결과:" << std::endl;
    std::cout << "    포즈: " << result.num_poses
              << ", 3D점: " << result.num_points
              << ", Edge: " << result.num_edges << std::endl;
    std::cout << "    초기 오차: " << result.initial_error << std::endl;
    std::cout << "    최종 오차: " << result.final_error << std::endl;
    std::cout << "    반복: " << result.iterations << "회" << std::endl;

    double ratio = (result.initial_error > 0)
        ? (1.0 - result.final_error / result.initial_error) * 100
        : 0.0;
    std::cout << "    개선율: " << ratio << "%" << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// 교육 블록
// ═══════════════════════════════════════════════════════════════

// 합성 BA 데이터 생성 유틸리티
static void generate_ba_data(
    int num_poses, int num_points, double noise_px,
    std::vector<Eigen::Matrix3d>& poses_R,
    std::vector<Eigen::Vector3d>& poses_t,
    std::vector<Eigen::Vector3d>& points,
    std::vector<std::tuple<int, int, Eigen::Vector2d>>& observations,
    const Eigen::Vector4d& K)
{
    std::mt19937 rng(42);
    std::normal_distribution<double> noise(0.0, noise_px);

    double fx = K(0), fy = K(1), cx = K(2), cy = K(3);

    // GT 포즈 (전진 궤적)
    for (int i = 0; i < num_poses; i++)
    {
        poses_R.push_back(Eigen::Matrix3d::Identity());
        poses_t.push_back(Eigen::Vector3d(0, 0, i * 0.5));
    }

    // GT 3D 점 (전방에 분포)
    std::uniform_real_distribution<double> dist_x(-3, 3);
    std::uniform_real_distribution<double> dist_y(-2, 2);
    std::uniform_real_distribution<double> dist_z(3, 15);
    for (int j = 0; j < num_points; j++)
    {
        points.push_back(Eigen::Vector3d(dist_x(rng), dist_y(rng), dist_z(rng)));
    }

    // 관측 생성 (투영 + 노이즈)
    for (int i = 0; i < num_poses; i++)
    {
        for (int j = 0; j < num_points; j++)
        {
            Eigen::Vector3d Pc = poses_R[i] * (points[j] - poses_t[i]);
            if (Pc(2) < 0.1)
                continue;

            double u = fx * Pc(0) / Pc(2) + cx + noise(rng);
            double v = fy * Pc(1) / Pc(2) + cy + noise(rng);

            if (u > 0 && u < 2 * cx && v > 0 && v < 2 * cy)
            {
                observations.push_back({i, j, Eigen::Vector2d(u, v)});
            }
        }
    }
}

// ── 블록 1: BA 기본 구조 ──
static void demoBAStructure()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 1] BA = Vertex + Edge 그래프 최적화" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_easy Q1 (재투영 오차), Q2 (Vertex), Q3 (Edge) 연결
    std::cout << "BA 목적 함수:" << std::endl;
    std::cout << "  min Σ ||z_ij - π(T_i, P_j)||²" << std::endl;
    std::cout << "  z_ij: 관측 (2D), T_i: 포즈, P_j: 3D 점\n" << std::endl;

    std::cout << "g2o 구조:" << std::endl;
    std::cout << "  Vertex = 최적화 변수" << std::endl;
    std::cout << "    VertexSE3Expmap: 카메라 포즈 (6 DoF)" << std::endl;
    std::cout << "    VertexPointXYZ:  3D 점 (3 DoF)" << std::endl;
    std::cout << "  Edge = 관측/제약" << std::endl;
    std::cout << "    EdgeProjectXYZ2UV: 재투영 오차\n" << std::endl;

    // 수치 예시: 간단한 BA (4 포즈 + 20 점)
    Eigen::Vector4d K(500, 500, 320, 240);

    std::vector<Eigen::Matrix3d> gt_R;
    std::vector<Eigen::Vector3d> gt_t, gt_pts;
    std::vector<std::tuple<int, int, Eigen::Vector2d>> obs;
    generate_ba_data(4, 20, 1.0, gt_R, gt_t, gt_pts, obs, K);

    // 노이즈 추가된 초기값으로 시작
    std::mt19937 rng(123);
    std::normal_distribution<double> noise_pt(0.0, 0.3);
    std::normal_distribution<double> noise_t(0.0, 0.1);

    g2o::SparseOptimizer optimizer;
    BABasic::setup_optimizer(optimizer);

    // 포즈 Vertex
    for (int i = 0; i < 4; i++)
    {
        Eigen::Vector3d t_noisy = gt_t[i];
        if (i > 0)  // 첫 포즈는 정확
        {
            t_noisy += Eigen::Vector3d(noise_t(rng), noise_t(rng), noise_t(rng));
        }
        BABasic::create_vertex_pose(optimizer, i, gt_R[i], t_noisy, i == 0);
    }

    // 3D 점 Vertex (노이즈 추가)
    const int kPointIdOffset = 100;
    for (int j = 0; j < 20; j++)
    {
        Eigen::Vector3d pt_noisy = gt_pts[j] +
            Eigen::Vector3d(noise_pt(rng), noise_pt(rng), noise_pt(rng));
        BABasic::create_vertex_point(optimizer, kPointIdOffset + j, pt_noisy);
    }

    // Edge (관측)
    for (const auto& [pose_id, point_id, pixel] : obs)
    {
        BABasic::create_edge(optimizer, pose_id, kPointIdOffset + point_id, pixel, K);
    }

    auto result = BABasic::optimize(optimizer, 10);
    BABasic::evaluate_result(optimizer, result, "4포즈+20점");
}

// ── 블록 2: Robust Kernel ──
static void demoRobustKernel()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 2] Huber Robust Kernel — Outlier 제거" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_easy Q4 (Huber Loss) 연결
    std::cout << "Huber Loss:" << std::endl;
    std::cout << "  e < δ → 0.5 * e²  (inlier: 일반 L2)" << std::endl;
    std::cout << "  e ≥ δ → δ(|e| - 0.5δ)  (outlier: L1로 감쇠)\n" << std::endl;

    Eigen::Vector4d K(500, 500, 320, 240);

    std::vector<Eigen::Matrix3d> gt_R;
    std::vector<Eigen::Vector3d> gt_t, gt_pts;
    std::vector<std::tuple<int, int, Eigen::Vector2d>> obs;
    generate_ba_data(4, 20, 1.0, gt_R, gt_t, gt_pts, obs, K);

    // Outlier 추가 (20%를 랜덤 관측으로)
    std::mt19937 rng(77);
    std::uniform_real_distribution<double> rand_px(0, 640);
    int n_outlier = static_cast<int>(obs.size() * 0.2);
    for (int i = 0; i < n_outlier; i++)
    {
        auto& [pid, ptid, pixel] = obs[i];
        pixel = Eigen::Vector2d(rand_px(rng), rand_px(rng) * 0.75);
    }

    std::normal_distribution<double> noise_pt(0.0, 0.3);
    std::normal_distribution<double> noise_t(0.0, 0.1);

    // Robust 없이
    {
        g2o::SparseOptimizer opt;
        BABasic::setup_optimizer(opt);
        for (int i = 0; i < 4; i++)
        {
            Eigen::Vector3d t_noisy = gt_t[i];
            if (i > 0)
                t_noisy += Eigen::Vector3d(noise_t(rng), noise_t(rng), noise_t(rng));
            BABasic::create_vertex_pose(opt, i, gt_R[i], t_noisy, i == 0);
        }
        const int kOffset = 100;
        for (int j = 0; j < 20; j++)
        {
            Eigen::Vector3d pt_noisy = gt_pts[j] +
                Eigen::Vector3d(noise_pt(rng), noise_pt(rng), noise_pt(rng));
            BABasic::create_vertex_point(opt, kOffset + j, pt_noisy);
        }
        for (const auto& [pid, ptid, px] : obs)
            BABasic::create_edge(opt, pid, kOffset + ptid, px, K, false);

        auto res = BABasic::optimize(opt, 10);
        BABasic::evaluate_result(opt, res, "Robust 없음 (20% outlier)");
    }

    // Robust Kernel 사용
    {
        g2o::SparseOptimizer opt;
        BABasic::setup_optimizer(opt);
        for (int i = 0; i < 4; i++)
        {
            Eigen::Vector3d t_noisy = gt_t[i];
            if (i > 0)
                t_noisy += Eigen::Vector3d(noise_t(rng), noise_t(rng), noise_t(rng));
            BABasic::create_vertex_pose(opt, i, gt_R[i], t_noisy, i == 0);
        }
        const int kOffset = 100;
        for (int j = 0; j < 20; j++)
        {
            Eigen::Vector3d pt_noisy = gt_pts[j] +
                Eigen::Vector3d(noise_pt(rng), noise_pt(rng), noise_pt(rng));
            BABasic::create_vertex_point(opt, kOffset + j, pt_noisy);
        }
        for (const auto& [pid, ptid, px] : obs)
            BABasic::create_edge(opt, pid, kOffset + ptid, px, K, true);

        auto res = BABasic::optimize(opt, 10);
        BABasic::evaluate_result(opt, res, "Huber Robust (20% outlier)");
    }
}

// ── 블록 3: Gauge Freedom ──
static void demoGaugeFreedom()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 3] Gauge Freedom — 기준 프레임 고정" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_medium Q3 (setFixed), quiz_easy Q5 (Schur) 연결
    std::cout << "왜 첫 포즈를 고정하는가?" << std::endl;
    std::cout << "  BA 해는 7자유도만큼 불확정 (Gauge Freedom)" << std::endl;
    std::cout << "  = 3 이동 + 3 회전 + 1 스케일" << std::endl;
    std::cout << "  → 첫 포즈 고정으로 6자유도 제거\n" << std::endl;

    // 💡 quiz_medium Q1 (Schur), Q2 (setMarginalized) 연결
    std::cout << "setMarginalized(true) — 3D 점에 적용:" << std::endl;
    std::cout << "  Schur Complement로 포즈만 먼저 최적화" << std::endl;
    std::cout << "  BlockSolver_6_3: 포즈(6) + 점(3)" << std::endl;
    std::cout << "  결과: 포즈 수에 비례하는 속도 (O(n³) → O(m³))" << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// 전체 데모 + main
// ═══════════════════════════════════════════════════════════════

void BABasic::demo()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Week 7: Bundle Adjustment — g2o 기초" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    demoBAStructure();
    demoRobustKernel();
    demoGaugeFreedom();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  정리" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    std::cout << "g2o BA 핵심 패턴:" << std::endl;
    std::cout << "  1. setup_optimizer (BlockSolver_6_3 + LM)" << std::endl;
    std::cout << "  2. create_vertex_pose (첫 포즈 fixed)" << std::endl;
    std::cout << "  3. create_vertex_point (marginalized)" << std::endl;
    std::cout << "  4. create_edge (재투영 오차 + Huber)" << std::endl;
    std::cout << "  5. optimize → 오차 감소 확인\n" << std::endl;
    std::cout << "다음 단계:" << std::endl;
    std::cout << "  1. ./my_basic 으로 직접 구현 연습" << std::endl;
    std::cout << "  2. quiz_easy / quiz_medium 풀기" << std::endl;
    std::cout << "  3. Week 8: Ceres로 동일 문제 풀어보기" << std::endl;
}

int main()
{
    BABasic::demo();
    return 0;
}
