#include "ceres_ba.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <ceres/rotation.h>

// ═══════════════════════════════════════════════════════════════
// 구현부
// ═══════════════════════════════════════════════════════════════

// ReprojectionError: 자동 미분 템플릿
template <typename T>
bool CeresBA::ReprojectionError::operator()(
    const T* const camera, const T* const point, T* residual) const
{
    // camera[0-2]: angle-axis, camera[3-5]: translation
    T p[3];
    // 회전 적용: Pc = R * (Pw - t) → AngleAxis 회전 후 translation 더하기
    T pw[3] = {point[0] - camera[3], point[1] - camera[4], point[2] - camera[5]};
    ceres::AngleAxisRotatePoint(camera, pw, p);

    // 투영
    T u = T(fx) * p[0] / p[2] + T(cx);
    T v = T(fy) * p[1] / p[2] + T(cy);

    residual[0] = u - T(observed_x);
    residual[1] = v - T(observed_y);
    return true;
}

ceres::CostFunction* CeresBA::ReprojectionError::create(
    double x, double y, double fx, double fy, double cx, double cy)
{
    return new ceres::AutoDiffCostFunction<ReprojectionError, 2, 6, 3>(
        new ReprojectionError(x, y, fx, fy, cx, cy));
}

void CeresBA::add_residual_block(
    ceres::Problem& problem,
    double* camera, double* point,
    double observed_x, double observed_y,
    const Eigen::Vector4d& K,
    bool use_robust)
{
    ceres::CostFunction* cost = ReprojectionError::create(
        observed_x, observed_y, K(0), K(1), K(2), K(3));

    ceres::LossFunction* loss = nullptr;
    if (use_robust)
    {
        loss = new ceres::HuberLoss(1.0);
    }

    problem.AddResidualBlock(cost, loss, camera, point);
}

void CeresBA::configure_solver(
    ceres::Solver::Options& options,
    bool use_schur)
{
    options.linear_solver_type = use_schur
        ? ceres::DENSE_SCHUR
        : ceres::DENSE_QR;
    options.minimizer_progress_to_stdout = false;
    options.max_num_iterations = 50;
    options.function_tolerance = 1e-6;
}

CeresBA::BAResult CeresBA::solve(
    ceres::Problem& problem,
    const ceres::Solver::Options& options)
{
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    BAResult result;
    result.initial_cost = summary.initial_cost;
    result.final_cost = summary.final_cost;
    result.iterations = static_cast<int>(summary.iterations.size());
    result.num_residuals = summary.num_residuals;
    result.solve_time_ms = summary.total_time_in_seconds * 1000;
    return result;
}

void CeresBA::compare_with_g2o(
    const BAResult& ceres_result,
    double g2o_final_cost)
{
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  g2o vs Ceres 비교:" << std::endl;
    std::cout << "    g2o  최종 오차: " << g2o_final_cost << std::endl;
    std::cout << "    Ceres 최종 오차: " << ceres_result.final_cost << std::endl;
    std::cout << "    Ceres 시간: " << ceres_result.solve_time_ms << " ms" << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// 교육 블록
// ═══════════════════════════════════════════════════════════════

// ── 블록 1: Ceres 자동 미분 ──
static void demoAutoDiff()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 1] Ceres 자동 미분 — template T" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_easy Q1 (자동 미분), Q2 (AutoDiffCostFunction), Q3 (template T) 연결
    std::cout << "자동 미분 원리:" << std::endl;
    std::cout << "  1. CostFunction을 template<T>로 정의" << std::endl;
    std::cout << "  2. T=double → 값 계산" << std::endl;
    std::cout << "  3. T=Jet → 값 + Jacobian 동시 계산\n" << std::endl;

    std::cout << "AutoDiffCostFunction<Error, 2, 6, 3>:" << std::endl;
    std::cout << "  Error: 오차 Functor" << std::endl;
    std::cout << "  2: residual 차원 (u, v)" << std::endl;
    std::cout << "  6: camera 파라미터 (angle-axis 3 + translation 3)" << std::endl;
    std::cout << "  3: point 파라미터 (x, y, z)\n" << std::endl;

    // 간단한 예시: 1D 최적화
    std::cout << "예시: f(x) = (x-3)² 최소화" << std::endl;

    struct SimpleError
    {
        template <typename T>
        bool operator()(const T* const x, T* residual) const
        {
            residual[0] = x[0] - T(3.0);
            return true;
        }
    };

    double x = 0.0;  // 초기값
    ceres::Problem problem;
    problem.AddResidualBlock(
        new ceres::AutoDiffCostFunction<SimpleError, 1, 1>(new SimpleError()),
        nullptr, &x);

    ceres::Solver::Options options;
    options.minimizer_progress_to_stdout = false;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    std::cout << "  초기 x=0.0 → 최적화 후 x=" << x << " (기대: 3.0)" << std::endl;
}

// ── 블록 2: BA 최적화 ──
static void demoCeresBA()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 2] Ceres BA — 재투영 오차 최소화" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_easy Q4 (HuberLoss), Q6 (SetParameterBlockConstant) 연결
    Eigen::Vector4d K(500, 500, 320, 240);

    std::mt19937 rng(42);
    std::normal_distribution<double> noise_obs(0.0, 1.0);
    std::normal_distribution<double> noise_pt(0.0, 0.3);
    std::normal_distribution<double> noise_cam(0.0, 0.1);
    std::uniform_real_distribution<double> dist_x(-3, 3);
    std::uniform_real_distribution<double> dist_y(-2, 2);
    std::uniform_real_distribution<double> dist_z(3, 15);

    const int kNumPoses = 4;
    const int kNumPoints = 20;

    // GT 데이터
    std::vector<Eigen::Vector3d> gt_t, gt_pts;
    for (int i = 0; i < kNumPoses; i++)
        gt_t.push_back(Eigen::Vector3d(0, 0, i * 0.5));
    for (int j = 0; j < kNumPoints; j++)
        gt_pts.push_back(Eigen::Vector3d(dist_x(rng), dist_y(rng), dist_z(rng)));

    // Ceres 파라미터 배열 (camera: 6 per pose, point: 3 per point)
    std::vector<std::array<double, 6>> cameras(kNumPoses);
    std::vector<std::array<double, 3>> points(kNumPoints);

    // 초기값 (노이즈 추가)
    for (int i = 0; i < kNumPoses; i++)
    {
        cameras[i] = {0, 0, 0, gt_t[i](0), gt_t[i](1), gt_t[i](2)};  // angle-axis=0
        if (i > 0)
        {
            cameras[i][3] += noise_cam(rng);
            cameras[i][4] += noise_cam(rng);
            cameras[i][5] += noise_cam(rng);
        }
    }
    for (int j = 0; j < kNumPoints; j++)
    {
        points[j] = {gt_pts[j](0) + noise_pt(rng),
                      gt_pts[j](1) + noise_pt(rng),
                      gt_pts[j](2) + noise_pt(rng)};
    }

    // Problem 구성
    ceres::Problem problem;
    int obs_count = 0;

    for (int i = 0; i < kNumPoses; i++)
    {
        for (int j = 0; j < kNumPoints; j++)
        {
            Eigen::Vector3d Pc = gt_pts[j] - gt_t[i];
            if (Pc(2) < 0.1)
                continue;
            double u = K(0) * Pc(0) / Pc(2) + K(2) + noise_obs(rng);
            double v = K(1) * Pc(1) / Pc(2) + K(3) + noise_obs(rng);

            CeresBA::add_residual_block(
                problem, cameras[i].data(), points[j].data(),
                u, v, K, true);
            obs_count++;
        }
    }

    // 첫 포즈 고정 (Gauge freedom)
    problem.SetParameterBlockConstant(cameras[0].data());

    std::cout << "BA 문제: " << kNumPoses << " 포즈, " << kNumPoints
              << " 3D점, " << obs_count << " 관측\n" << std::endl;

    ceres::Solver::Options options;
    CeresBA::configure_solver(options, true);

    auto result = CeresBA::solve(problem, options);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  초기 비용: " << result.initial_cost << std::endl;
    std::cout << "  최종 비용: " << result.final_cost << std::endl;
    std::cout << "  반복: " << result.iterations << "회" << std::endl;
    std::cout << "  시간: " << result.solve_time_ms << " ms" << std::endl;

    double ratio = (result.initial_cost > 0)
        ? (1.0 - result.final_cost / result.initial_cost) * 100 : 0;
    std::cout << "  개선율: " << ratio << "%" << std::endl;
}

// ── 블록 3: Solver 비교 ──
static void demoSolverComparison()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 3] DENSE_SCHUR vs DENSE_QR" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_easy Q5 (g2o 단점), quiz_medium Q2 (DENSE_SCHUR) 연결
    std::cout << "Solver 선택 가이드:" << std::endl;
    std::cout << "  DENSE_SCHUR: BA 전용 (포즈≪점 구조 활용)" << std::endl;
    std::cout << "  DENSE_QR: 일반 문제 (구조 활용 안 함)" << std::endl;
    std::cout << "  SPARSE_SCHUR: 대규모 BA (희소 행렬)" << std::endl;
    std::cout << "  ITERATIVE_SCHUR: 초대규모 BA\n" << std::endl;

    // 💡 quiz_medium Q1 (Jet), Q3 (Ceres vs g2o) 연결
    std::cout << "Ceres vs g2o:" << std::endl;
    std::cout << "  Ceres: 자동 미분 → 개발 빠름, 범용" << std::endl;
    std::cout << "  g2o:   그래프 구조 → SLAM 특화, ORB-SLAM 사용" << std::endl;
    std::cout << "  선택:  연구/프로토타입 → Ceres, SLAM 시스템 → g2o" << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// 전체 데모 + main
// ═══════════════════════════════════════════════════════════════

void CeresBA::demo()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Week 8: Bundle Adjustment — Ceres 기초" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    demoAutoDiff();
    demoCeresBA();
    demoSolverComparison();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  정리" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    std::cout << "Ceres BA 핵심 패턴:" << std::endl;
    std::cout << "  1. CostFunction (template T, AutoDiff)" << std::endl;
    std::cout << "  2. Problem.AddResidualBlock(cost, loss, params)" << std::endl;
    std::cout << "  3. Solver::Options (DENSE_SCHUR)" << std::endl;
    std::cout << "  4. SetParameterBlockConstant (첫 포즈)" << std::endl;
    std::cout << "  5. Solve → Summary 확인\n" << std::endl;
    std::cout << "다음 단계:" << std::endl;
    std::cout << "  1. ./my_basic 으로 직접 구현 연습" << std::endl;
    std::cout << "  2. quiz_easy / quiz_medium 풀기" << std::endl;
}

int main()
{
    CeresBA::demo();
    return 0;
}
