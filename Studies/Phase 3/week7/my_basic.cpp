/**
 * Phase 3 Week 7 - g2o BA 직접 구현
 *
 * ba_basic.h의 함수들을 직접 구현해보세요.
 *
 * ┌──────┬──────────────────────┬────────┬──────────────┐
 * │ Step │ 함수                 │ 난이도  │ 검증 방법     │
 * ├──────┼──────────────────────┼────────┼──────────────┤
 * │  1   │ create_vertex_pose   │ 쉬움   │ ./my_basic   │
 * │  2   │ create_vertex_point  │ 쉬움   │ ./my_basic   │
 * │  3   │ create_edge          │ 핵심   │ ./my_basic   │
 * │  4   │ setup_optimizer      │ 핵심   │ ./my_basic   │
 * │  5   │ optimize             │ 쉬움   │ ./my_basic   │
 * │  6   │ evaluate_result      │ 쉬움   │ ./my_basic   │
 * └──────┴──────────────────────┴────────┴──────────────┘
 */
#include "ba_basic.h"
#include <iostream>
#include <iomanip>
#include <random>

// [Step 1] create_vertex_pose — SE3 포즈 Vertex 생성
// 힌트:
//   1. new g2o::VertexSE3Expmap()
//   2. setId(id), setEstimate(g2o::SE3Quat(R, t)), setFixed(fixed)
//   3. optimizer.addVertex(v)
// 참고: basic.cpp create_vertex_pose()
// 기대값: optimizer.vertices()에 추가됨
void BABasic::create_vertex_pose(
    g2o::SparseOptimizer& optimizer,
    int id,
    const Eigen::Matrix3d& R,
    const Eigen::Vector3d& t,
    bool fixed)
{
    // TODO: SE3 Vertex 생성 + 추가
}

// [Step 2] create_vertex_point — 3D 점 Vertex 생성
// 힌트:
//   1. new g2o::VertexPointXYZ()
//   2. setId(id), setEstimate(point), setMarginalized(marginalized)
//   3. optimizer.addVertex(v)
// 참고: basic.cpp create_vertex_point()
// 기대값: marginalized=true이면 Schur Complement 대상
void BABasic::create_vertex_point(
    g2o::SparseOptimizer& optimizer,
    int id,
    const Eigen::Vector3d& point,
    bool marginalized)
{
    // TODO: PointXYZ Vertex 생성 + 추가
}

// [Step 3] create_edge — 재투영 오차 Edge 생성 (핵심!)
// 힌트:
//   1. new g2o::EdgeProjectXYZ2UV()
//   2. setVertex(0, point), setVertex(1, pose)
//   3. setMeasurement(observation)
//   4. setInformation(Matrix2d::Identity())
//   5. CameraParameters(fx, (cx,cy), 0) → setParameterId(0, 0)
//   6. use_robust이면 RobustKernelHuber(delta=1.0) 설정
// 참고: basic.cpp create_edge()
// 기대값: optimizer.edges()에 추가됨
void BABasic::create_edge(
    g2o::SparseOptimizer& optimizer,
    int pose_id, int point_id,
    const Eigen::Vector2d& observation,
    const Eigen::Vector4d& K,
    bool use_robust)
{
    // TODO: 재투영 Edge 생성 + 추가
}

// [Step 4] setup_optimizer — BlockSolver_6_3 + LM 설정 (핵심!)
// 힌트:
//   using BlockSolverType = g2o::BlockSolver_6_3;
//   using LinearSolverType = g2o::LinearSolverEigen<BlockSolverType::PoseMatrixType>;
//   new g2o::OptimizationAlgorithmLevenberg(
//       make_unique<BlockSolverType>(make_unique<LinearSolverType>()))
//   optimizer.setAlgorithm(solver)
// 참고: basic.cpp setup_optimizer()
void BABasic::setup_optimizer(g2o::SparseOptimizer& optimizer)
{
    // TODO: BlockSolver + LM 설정
}

// [Step 5] optimize — 최적화 실행
// 힌트:
//   1. optimizer.initializeOptimization()
//   2. initial = optimizer.activeChi2()
//   3. optimizer.optimize(iterations)
//   4. final = optimizer.activeChi2()
// 기대값: final_error < initial_error
BABasic::BAResult BABasic::optimize(g2o::SparseOptimizer& optimizer, int iterations)
{
    BAResult result;
    // TODO: 초기화 + 최적화 실행 + 결과 저장
    result.initial_error = -1;
    result.final_error = -1;
    result.iterations = 0;
    return result;
}

// [Step 6] evaluate_result — 결과 출력
// 힌트: result의 필드들 출력 + 개선율 계산
void BABasic::evaluate_result(
    const g2o::SparseOptimizer& optimizer,
    const BAResult& result,
    const std::string& label)
{
    // TODO: 결과 출력
    std::cout << "  [" << label << "] TODO: 결과 출력 구현" << std::endl;
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] g2o Bundle Adjustment" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Eigen::Vector4d K(500, 500, 320, 240);  // fx, fy, cx, cy

    // 합성 데이터: 4 포즈 + 10 점
    std::mt19937 rng(42);
    std::normal_distribution<double> noise(0.0, 1.0);
    std::normal_distribution<double> noise_pt(0.0, 0.3);
    std::uniform_real_distribution<double> dist_x(-3, 3);
    std::uniform_real_distribution<double> dist_y(-2, 2);
    std::uniform_real_distribution<double> dist_z(3, 12);

    const int kNumPoses = 4;
    const int kNumPoints = 10;
    const int kPointIdOffset = 100;

    // GT 데이터
    std::vector<Eigen::Vector3d> gt_t, gt_pts;
    for (int i = 0; i < kNumPoses; i++)
        gt_t.push_back(Eigen::Vector3d(0, 0, i * 0.5));
    for (int j = 0; j < kNumPoints; j++)
        gt_pts.push_back(Eigen::Vector3d(dist_x(rng), dist_y(rng), dist_z(rng)));

    // ── Step 4 테스트: setup_optimizer ──
    std::cout << "[Step 4] setup_optimizer" << std::endl;
    g2o::SparseOptimizer optimizer;
    BABasic::setup_optimizer(optimizer);
    bool step4 = (optimizer.algorithm() != nullptr);
    std::cout << "  알고리즘: " << (step4 ? "설정됨 ✅" : "없음 ❌") << "\n" << std::endl;

    // ── Step 1 테스트: create_vertex_pose ──
    std::cout << "[Step 1] create_vertex_pose" << std::endl;
    for (int i = 0; i < kNumPoses; i++)
    {
        Eigen::Vector3d t_noisy = gt_t[i];
        if (i > 0)
            t_noisy += Eigen::Vector3d(noise(rng) * 0.1, noise(rng) * 0.1, noise(rng) * 0.1);
        BABasic::create_vertex_pose(optimizer, i, Eigen::Matrix3d::Identity(), t_noisy, i == 0);
    }
    bool step1 = (optimizer.vertex(0) != nullptr);
    std::cout << "  Vertex 수: " << optimizer.vertices().size()
              << (step1 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 2 테스트: create_vertex_point ──
    std::cout << "[Step 2] create_vertex_point" << std::endl;
    for (int j = 0; j < kNumPoints; j++)
    {
        Eigen::Vector3d pt_noisy = gt_pts[j] +
            Eigen::Vector3d(noise_pt(rng), noise_pt(rng), noise_pt(rng));
        BABasic::create_vertex_point(optimizer, kPointIdOffset + j, pt_noisy);
    }
    bool step2 = (optimizer.vertex(kPointIdOffset) != nullptr);
    std::cout << "  3D점 Vertex: " << (step2 ? "추가됨 ✅" : "없음 ❌") << "\n" << std::endl;

    // ── Step 3 테스트: create_edge ──
    std::cout << "[Step 3] create_edge" << std::endl;
    int edge_count = 0;
    for (int i = 0; i < kNumPoses; i++)
    {
        for (int j = 0; j < kNumPoints; j++)
        {
            Eigen::Vector3d Pc = gt_pts[j] - gt_t[i];
            if (Pc(2) < 0.1)
                continue;
            double u = K(0) * Pc(0) / Pc(2) + K(2) + noise(rng);
            double v = K(1) * Pc(1) / Pc(2) + K(3) + noise(rng);
            BABasic::create_edge(optimizer, i, kPointIdOffset + j,
                                 Eigen::Vector2d(u, v), K, true);
            edge_count++;
        }
    }
    bool step3 = (optimizer.edges().size() > 0);
    std::cout << "  Edge 수: " << optimizer.edges().size()
              << (step3 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 5 테스트: optimize ──
    std::cout << "[Step 5] optimize" << std::endl;
    auto result = BABasic::optimize(optimizer, 10);
    bool step5 = (result.final_error >= 0 && result.final_error <= result.initial_error);
    std::cout << "  초기: " << result.initial_error
              << " → 최종: " << result.final_error
              << (step5 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 6 테스트: evaluate_result ──
    std::cout << "[Step 6] evaluate_result" << std::endl;
    BABasic::evaluate_result(optimizer, result, "테스트");

    // ── 종합 결과 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  종합: Step1 " << (step1 ? "✅" : "❌")
              << " Step2 " << (step2 ? "✅" : "❌")
              << " Step3 " << (step3 ? "✅" : "❌")
              << " Step4 " << (step4 ? "✅" : "❌")
              << " Step5 " << (step5 ? "✅" : "❌") << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
