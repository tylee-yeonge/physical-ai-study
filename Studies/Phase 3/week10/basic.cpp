#include "g2o_advanced.h"
#include <g2o/core/robust_kernel_impl.h>
#include <iostream>
#include <iomanip>
#include <random>

// ═══════════════════════════════════════════════════════════════
// 유틸리티: 합성 BA 그래프 빌드
// ═══════════════════════════════════════════════════════════════

// BA(Bundle Adjustment) 테스트를 위한 합성 그래프 생성
//
// 실제 SLAM에서는 카메라로 특징점을 관측하여 그래프를 구성하지만,
// 여기서는 가상의 카메라 포즈와 3D 점을 만들어 g2o 그래프에 넣는다.
//
// 그래프 구조:
//   Vertex: 카메라 포즈 (SE3, 6 DoF) + 3D 점 (XYZ, 3 DoF)
//   Edge: 카메라→점 관측 (2D 재투영, EdgeProjectXYZ2UV)
//
//   포즈 0 ──── 점 0
//   포즈 0 ──── 점 1
//   포즈 1 ──── 점 0      ← 같은 점을 여러 포즈에서 관측
//   포즈 1 ──── 점 1
//   ...
//
// marginalize_points:
//   true → 점 Vertex에 setMarginalized(true) 설정
//     → BlockSolver가 Schur Complement로 점을 소거하여 효율적으로 풀기
//   false → 전체 H·Δx = b를 그대로 풀기 (행렬 크기: (6N+3M)²)
static void build_ba_graph(
    g2o::SparseOptimizer& optimizer,
    int num_poses, int num_points, bool marginalize_points = true)
{
    // 노이즈 생성기 — 합성 데이터에 현실적인 오차를 부여
    //   noise_obs: 2D 관측 노이즈 (σ=1.0 픽셀, 실제 특징점 검출 오차 수준)
    //   noise_pt: 3D 점 초기값 노이즈 (σ=0.3m, 삼각측량 오차 시뮬레이션)
    //   noise_t: 포즈 이동 노이즈 (σ=0.1m, 초기 포즈 추정 오차)
    std::mt19937 rng(42);
    std::normal_distribution<double> noise_obs(0.0, 1.0);
    std::normal_distribution<double> noise_pt(0.0, 0.3);
    std::normal_distribution<double> noise_t(0.0, 0.1);
    std::uniform_real_distribution<double> dist_x(-5, 5);
    std::uniform_real_distribution<double> dist_y(-3, 3);
    std::uniform_real_distribution<double> dist_z(3, 20);

    // 가상 카메라 내부 파라미터
    //   fx=500: 초점거리 (정사각 픽셀 가정이므로 fy=fx)
    //   cx=320, cy=240: 주점 (640×480 이미지의 중심)
    double fx = 500, cx = 320, cy = 240;

    // 점 ID와 포즈 ID가 충돌하지 않도록 오프셋 설정
    //   포즈 ID: 0, 1, 2, ... (num_poses-1)
    //   점 ID: 1000, 1001, ... (kPointIdOffset + num_points-1)
    const int kPointIdOffset = 1000;

    // ── 포즈 Vertex 생성 ──
    //
    // VertexSE3Expmap: 카메라 포즈를 SE(3) (회전+이동)으로 표현
    //   내부적으로 quaternion + translation으로 저장
    //   optimize() 시 6차원 업데이트 벡터 [δω, δt]를 exp map으로 적용
    //
    // 배치: 카메라들을 Z축 방향으로 0.5m 간격으로 일직선 배치
    //   포즈 0: (0, 0, 0) — 고정 (Gauge freedom 해소)
    //   포즈 1: (0, 0, 0.5) + 노이즈
    //   포즈 2: (0, 0, 1.0) + 노이즈
    //   ...
    //   ★ 첫 포즈를 고정하지 않으면 전체 좌표계가 자유로워 해가 무한히 많아짐
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

    // ── 3D 점 Vertex + 관측 Edge 생성 ──
    //
    // VertexPointXYZ: 3D 점을 (x, y, z)로 표현 — 3 DoF
    //   setMarginalized(true) → Schur Complement에서 소거 대상으로 지정
    //     → Hessian의 점-점 블록(Hpp)이 블록 대각이므로 역행렬 계산이 효율적
    //     → BlockSolver가 S = Hcc - Hcp·Hpp⁻¹·Hpc 로 포즈만의 시스템을 만듦
    //
    // EdgeProjectXYZ2UV: 3D 점 → 2D 픽셀 투영 오차
    //   잔차 = 관측된 (u,v) - 투영된 (u,v)   ← 2차원
    //   Vertex 0: 점 (3D, marginalized)
    //   Vertex 1: 포즈 (SE3)
    //   CameraParameters: 내부 파라미터 (fx, cx, cy)를 Edge에 전달
    for (int j = 0; j < num_points; j++)
    {
        auto* v = new g2o::VertexPointXYZ();
        v->setId(kPointIdOffset + j);
        Eigen::Vector3d pt(dist_x(rng), dist_y(rng), dist_z(rng));
        Eigen::Vector3d pt_noisy = pt + Eigen::Vector3d(noise_pt(rng), noise_pt(rng), noise_pt(rng));
        v->setEstimate(pt_noisy);
        v->setMarginalized(marginalize_points);
        optimizer.addVertex(v);

        // CameraParameters: g2o의 투영 모델에 내부 파라미터 전달
        //   (fx, (cx, cy), baseline) — baseline=0은 단안 카메라
        auto* cam_param = new g2o::CameraParameters(fx, Eigen::Vector2d(cx, cy), 0);
        cam_param->setId(0);
        optimizer.addParameter(cam_param);

        // 각 점을 모든 포즈에서 관측 (카메라 뒤에 있는 점은 제외)
        //
        // 투영: Pc = Pw - t_gt (항등 회전 가정으로 간략화)
        //   u = fx · Pc_x / Pc_z + cx + 노이즈
        //   v = fx · Pc_y / Pc_z + cy + 노이즈
        //   Pc_z < 0.5 이면 카메라 뒤/너무 가까우므로 제외
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

// Solver 조합 비교 — 같은 BA 문제를 다른 Solver로 풀어 성능 차이 관찰
//
// g2o의 3계층 구조:
//   ┌─ SparseOptimizer ──── 반복 관리, 수렴 판정
//   ├─ BlockSolver_6_3 ──── Hessian 블록 구성, Schur Complement
//   └─ LinearSolver ─────── 선형 시스템 S·Δc = b' 풀기
//
// 테스트 조합:
//   ① LM + Eigen: Levenberg-Marquardt + Eigen 내장 solver
//      → BA의 표준 조합. LM의 damping(λI)이 수렴 안정성 보장
//      → (H + λI)·Δx = b, λ가 자동 조절됨
//
//   ② GN + Dense: Gauss-Newton + Dense solver
//      → damping 없이 H·Δx = b를 직접 풀기
//      → 초기값이 좋으면 빠르지만, 나쁘면 발산 가능
//      → Dense solver는 소규모에서만 효율적 (O(n³) 비용)
std::vector<G2OAdvanced::ProfileResult> G2OAdvanced::compare_solvers(
    int num_poses, int num_points)
{
    std::vector<ProfileResult> results;

    // ── 조합 1: LM + Eigen ──
    //
    // LinearSolverEigen: Eigen의 SimplicialLLT(Cholesky) 사용
    //   → 추가 라이브러리 설치 불필요, 중소규모에 적합
    // OptimizationAlgorithmLevenberg: LM 방법
    //   → 매 iteration마다 λ를 조절하여 GN과 Gradient Descent 사이 전환
    //   → λ 큼 → Gradient Descent (안전하지만 느림)
    //   → λ 작음 → Gauss-Newton (빠르지만 불안정)
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

    // ── 조합 2: GN + Dense ──
    //
    // LinearSolverDense: QR/LU 분해로 직접 풀기
    //   → 희소성을 활용하지 않으므로 대규모에서 매우 느림
    //   → 소규모(포즈 <10, 점 <100)에서는 오버헤드가 적어 오히려 빠를 수 있음
    // OptimizationAlgorithmGaussNewton: GN 방법
    //   → H·Δx = b를 damping 없이 풀기
    //   → 초기값이 정답 근처면 2차 수렴 (매우 빠름)
    //   → 초기값이 나쁘면 발산 위험 → BA에는 LM이 더 안전
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

// Hessian 희소성 분석 — BA의 Hessian이 왜 희소한지 수치로 확인
//
// BA의 Hessian 구조:
//   H = J^T · Ω · J 의 크기: (6·N_pose + 3·N_point)²
//
//   [Hcc  Hcp]   Hcc: 포즈-포즈 블록 (dense, 크기 6N×6N)
//   [Hpc  Hpp]   Hpp: 점-점 블록 (블록 대각, 크기 3M×3M)
//                Hcp: 포즈-점 교차 블록 (sparse)
//
// ★ 핵심: Hpp가 블록 대각인 이유
//   점 i와 점 j가 직접 연결되는 Edge는 없음 (점↔포즈 Edge만 존재)
//   → Hpp에서 점 i, j 사이의 블록이 0
//   → 점별로 독립적인 3×3 블록만 존재
//   → Hpp⁻¹ 계산이 O(M)으로 매우 효율적!
//   → 이것이 Schur Complement가 BA에서 빠른 핵심 이유
//
// 반환: sparsity_ratio (%) — 전체 원소 중 0인 비율
//   값이 높을수록 희소 → 희소 solver(Cholmod 등)가 유리
void G2OAdvanced::analyze_sparsity(
    int num_poses, int num_points, double& sparsity_ratio)
{
    // 전체 Hessian 크기: (6·포즈 + 3·점)²
    int dim = 6 * num_poses + 3 * num_points;
    int total_elements = dim * dim;

    // 비영(non-zero) 블록 수 추정:
    //   pose_block: 포즈-포즈 (Hcc) — 같은 점을 관측하면 연결 → dense
    //   point_block: 점-점 (Hpp) — 점끼리 직접 연결 없으므로 블록 대각
    //   cross_block: 포즈-점 (Hcp, Hpc) — 관측 관계에 따라 sparse
    //     각 점이 평균 70% 포즈에서 관측된다고 가정 (×2는 Hcp+Hpc 대칭)
    int pose_block = 6 * num_poses * 6 * num_poses;
    int point_block = 3 * num_points * 3 * num_points;
    int cross_block = 2 * static_cast<int>(6 * num_poses * 3 * num_points * 0.7);

    int nonzero = pose_block + point_block + cross_block;
    sparsity_ratio = (1.0 - static_cast<double>(nonzero) / total_elements) * 100;
}

// setMarginalized 효과 비교 — Schur Complement 적용 유무에 따른 성능 차이
//
// Marginalization이란?
//   "특정 변수를 소거하여 더 작은 시스템을 만드는 것"
//
// BA에서의 동작:
//   ① setMarginalized(true) 적용 시:
//      H를 [Hcc, Hcp; Hpc, Hpp] 블록으로 분할
//      → S = Hcc - Hcp·Hpp⁻¹·Hpc (Schur Complement)
//      → S·Δc = b' 풀기 (크기: 6N×6N)
//      → Δp = Hpp⁻¹·(bp - Hpc·Δc) (Back-substitution)
//      ★ 풀어야 할 행렬 크기: 6N×6N (포즈 수에만 비례!)
//
//   ② setMarginalized(false) 적용 시:
//      H·Δx = b를 통째로 풀기
//      ★ 풀어야 할 행렬 크기: (6N+3M)×(6N+3M)
//      → 점이 많을수록 급격히 느려짐
//
// ★ 실제 BA에서 점 수(M)가 포즈 수(N)보다 훨씬 많으므로
//   Marginalization의 효과가 매우 크다.
//   예: N=10 포즈, M=1000 점 → 60×60 vs 3060×3060
std::pair<G2OAdvanced::ProfileResult, G2OAdvanced::ProfileResult>
G2OAdvanced::test_marginalization(int num_poses, int num_points)
{
    ProfileResult with_marg, without_marg;

    // ── Marginalized = true (Schur Complement 적용) ──
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

    // ── Marginalized = false (전체 시스템 직접 풀기) ──
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

// 문제 크기별 성능 프로파일링 — 포즈/점 수를 늘려가며 최적화 시간 측정
//
// BA 연산 복잡도 (Schur Complement 적용 시):
//   S 계산: O(N²·M) (N: 포즈, M: 점)
//   S·Δc = b' 풀기: O(N³) (직접법) 또는 O(N²·k) (반복법)
//   Back-substitution: O(M)
//   → 포즈 수에 3차, 점 수에 선형으로 증가
//
// 실제 SLAM에서는:
//   Local BA: N=10~20, M=100~500 → 수 ms
//   Global BA: N=100~1000, M=10,000~100,000 → 수 초~분
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

// ORB-SLAM 스타일 BA 패턴 시연
//
// ORB-SLAM2/3에서 g2o를 사용하는 3가지 패턴:
//
//   ① Motion-only BA (Tracking, 매 프레임):
//      → 카메라 포즈만 최적화, 맵 포인트는 고정(setFixed)
//      → 가장 빠름 (하나의 포즈, 6 DoF)
//
//   ② Local BA (Local Mapping, 새 키프레임마다):
//      → 현재 키프레임 + covisible 키프레임(~10-20개) + 관련 맵 포인트
//      → 나머지 키프레임은 setFixed(true)로 고정
//      → setMarginalized(true)로 점에 Schur Complement 적용
//      → 실시간으로 수행 (~수십 ms)
//
//   ③ Global BA (Loop Closing 후):
//      → 전체 키프레임 + 전체 맵 포인트 최적화
//      → 별도 스레드에서 비동기 실행 (실시간 Tracking 방해 안 함)
//
// ★ ORB-SLAM의 BlockSolver 설정:
//   Local BA: BlockSolver_6_3 (포즈 6DoF + 점 3DoF)
//   Pose Graph: BlockSolver_7_3 (Sim(3) 7DoF + 점 3DoF)
//     → 7은 스케일 자유도가 추가된 Sim(3)! (Week 12에서 학습)
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

    // 시연: Local BA (소규모) — ORB-SLAM의 가장 빈번한 BA
    //   optimize(5): Local BA는 실시간 요구 때문에 반복 횟수를 적게 설정
    //   ORB-SLAM2 실제 코드: optimize(5) for Local BA, optimize(20) for Global BA
    g2o::SparseOptimizer opt;
    auto solver = new g2o::OptimizationAlgorithmLevenberg(
        std::make_unique<g2o::BlockSolver_6_3>(
            std::make_unique<g2o::LinearSolverEigen<g2o::BlockSolver_6_3::PoseMatrixType>>()));
    opt.setAlgorithm(solver);
    opt.setVerbose(false);
    build_ba_graph(opt, num_keyframes, num_points);

    opt.initializeOptimization();
    double init = opt.activeChi2();
    opt.optimize(5);
    double final_err = opt.activeChi2();

    std::cout << "  Local BA 시연 (" << num_keyframes << " KF + " << num_points << " pt):" << std::endl;
    std::cout << "    오차: " << std::fixed << std::setprecision(2) << init
              << " → " << final_err << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// 교육 블록 + 데모
// ═══════════════════════════════════════════════════════════════

// 전체 데모 — g2o 내부 동작을 4가지 관점에서 시연
//
// 학습 목표:
//   블록 1: Solver 조합에 따른 성능 차이 관찰
//   블록 2: BA Hessian의 희소 구조 이해
//   블록 3: setMarginalized가 Schur Complement를 트리거하는 효과 체험
//   블록 4: ORB-SLAM이 g2o를 어떻게 활용하는지 패턴 파악
//
// g2o optimize() 내부 파이프라인 (매 iteration):
//   ① computeActiveErrors() — 모든 Edge의 잔차 e 계산
//   ② linearizeOplus() — 모든 Edge의 자코비안 J 계산
//   ③ buildSystem() — H = JᵀΩJ, b = -JᵀΩe 조립
//   ④ solver->solve() — Schur + 선형 시스템 풀기
//   ⑤ update() — x_new = x_old ⊕ Δx (manifold 업데이트)
//   ⑥ 수렴 판정 — cost 감소량이 임계값 이하면 종료
void G2OAdvanced::demo()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Week 10: g2o 심화 — Solver 내부 동작" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    // ── 블록 1: Solver 비교 ──
    //
    // BlockSolver_6_3: 포즈 차원(6) + 점 차원(3)을 컴파일 타임에 고정
    //   → Hessian을 6×6, 3×3 블록으로 분할하여 효율적으로 처리
    //   → 블록 크기가 고정이므로 루프 언롤링, SIMD 최적화 가능
    //   → BlockSolverX(-1, -1)는 동적이지만 오버헤드 있음
    //
    // 💡 quiz에서 "BlockSolver_6_3에서 6과 3의 의미" 문제 출제!
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
    //
    // BA Hessian이 희소한 이유:
    //   점↔포즈 Edge만 존재하고, 점↔점 직접 연결이 없음
    //   → Hpp (점-점 블록)가 블록 대각
    //   → Hcp (포즈-점 블록)도 관측 관계만큼만 비영
    //   → 전체 H에서 대부분의 원소가 0
    //
    // 규모가 커질수록 희소율 증가:
    //   4P+20pt: H=84×84 → 희소율 낮음
    //   50P+500pt: H=1800×1800 → 희소율 높음
    //   → 대규모에서 Cholmod 같은 희소 solver가 필수인 이유!
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
    //
    // setMarginalized(true)가 내부적으로 트리거하는 과정:
    //   1. BlockSolver가 Vertex를 두 그룹으로 분류
    //      Group 0: Non-marginalized (카메라) → Schur 후에도 남음
    //      Group 1: Marginalized (점) → Schur로 소거
    //   2. buildSystem() 시 H를 [Hcc, Hcp; Hpc, Hpp]로 분할
    //   3. solve() 시:
    //      Hpp⁻¹ 계산 → S 계산 → S·Δc = b' 풀기 → Back-sub
    //
    // 결과: 같은 정확도, 더 빠른 속도 (점이 많을수록 차이 극대화)
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
