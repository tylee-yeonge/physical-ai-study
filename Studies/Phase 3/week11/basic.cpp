#include "ceres_practice.h"
#include <ceres/rotation.h>
#include <iostream>
#include <iomanip>
#include <random>

// ═══════════════════════════════════════════════════════════════
// 구현부
// ═══════════════════════════════════════════════════════════════

template <typename T>
bool CeresPractice::SnavelyError::operator()(
    const T* const camera, const T* const point, T* residual) const
{
    // 1. angle-axis 회전
    T p[3];
    ceres::AngleAxisRotatePoint(camera, point, p);

    // 2. 평행이동
    p[0] += camera[3];
    p[1] += camera[4];
    p[2] += camera[5];

    // 3. 정규화 (BAL 관례: -z 방향)
    T xp = -p[0] / p[2];
    T yp = -p[1] / p[2];

    // 4. 방사 왜곡
    const T& k1 = camera[7];
    const T& k2 = camera[8];
    T r2 = xp * xp + yp * yp;
    T distortion = T(1.0) + r2 * (k1 + k2 * r2);

    // 5. focal length 적용
    const T& focal = camera[6];
    T predicted_x = focal * distortion * xp;
    T predicted_y = focal * distortion * yp;

    // 6. 잔차
    residual[0] = predicted_x - T(observed_x);
    residual[1] = predicted_y - T(observed_y);
    return true;
}

ceres::CostFunction* CeresPractice::SnavelyError::create(double x, double y)
{
    return new ceres::AutoDiffCostFunction<SnavelyError, 2, 9, 3>(
        new SnavelyError(x, y));
}

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

    // GT 카메라: 일직선으로 배치
    data.cameras.resize(num_cameras);
    for (int i = 0; i < num_cameras; i++)
    {
        // GT: angle-axis=0, t=(0,0,i*0.5), f=500, k1=k2=0
        data.cameras[i] = {0, 0, 0, 0, 0, i * 0.5, focal_gt, 0, 0};
        // 노이즈 추가 (첫 카메라 제외)
        if (i > 0)
        {
            for (int k = 0; k < 3; k++)
                data.cameras[i][k] += noise_cam(rng);    // 회전
            for (int k = 3; k < 6; k++)
                data.cameras[i][k] += noise_cam(rng) * 3; // 이동
            data.cameras[i][6] += noise_cam(rng) * 50;    // focal
        }
    }

    // GT 3D 점
    data.points.resize(num_points);
    std::vector<Eigen::Vector3d> gt_pts(num_points);
    for (int j = 0; j < num_points; j++)
    {
        gt_pts[j] = Eigen::Vector3d(dist_x(rng), dist_y(rng), dist_z(rng));
        data.points[j] = {gt_pts[j](0) + noise_pt(rng),
                           gt_pts[j](1) + noise_pt(rng),
                           gt_pts[j](2) + noise_pt(rng)};
    }

    // 관측 생성 (BAL 스타일: -z 투영)
    for (int i = 0; i < num_cameras; i++)
    {
        Eigen::Vector3d t_gt(0, 0, i * 0.5);
        for (int j = 0; j < num_points; j++)
        {
            if (coin(rng) > obs_ratio)
                continue;

            // GT 투영: R=I → Pc = P + t, BAL은 -z/pz
            Eigen::Vector3d Pc = gt_pts[j] + t_gt;  // BAL: R*P + t
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

void CeresPractice::build_problem(
    ceres::Problem& problem,
    BALData& data,
    bool use_robust)
{
    for (const auto& obs : data.observations)
    {
        ceres::CostFunction* cost = SnavelyError::create(obs.x, obs.y);

        ceres::LossFunction* loss = nullptr;
        if (use_robust)
            loss = new ceres::HuberLoss(1.0);

        problem.AddResidualBlock(
            cost, loss,
            data.cameras[obs.camera_idx].data(),
            data.points[obs.point_idx].data());
    }

    // 첫 카메라 고정 (Gauge freedom)
    problem.SetParameterBlockConstant(data.cameras[0].data());
}

void CeresPractice::configure_solver(
    ceres::Solver::Options& options,
    const std::string& solver_type,
    bool log_progress)
{
    if (solver_type == "sparse_schur")
        options.linear_solver_type = ceres::SPARSE_SCHUR;
    else
        options.linear_solver_type = ceres::DENSE_SCHUR;

    options.max_num_iterations = 50;
    options.function_tolerance = 1e-6;
    options.minimizer_progress_to_stdout = log_progress;
}

CeresPractice::SolveResult CeresPractice::solve_and_report(
    ceres::Problem& problem,
    const ceres::Solver::Options& options)
{
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    SolveResult result;
    result.initial_cost = summary.initial_cost;
    result.final_cost = summary.final_cost;
    result.iterations = static_cast<int>(summary.iterations.size());
    result.solve_time_ms = summary.total_time_in_seconds * 1000;

    // iteration별 cost 기록
    for (const auto& iter : summary.iterations)
    {
        result.cost_per_iteration.push_back(iter.cost);
    }

    return result;
}

void CeresPractice::analyze_convergence(const SolveResult& result)
{
    std::cout << std::fixed;
    std::cout << "  수렴 분석:" << std::endl;
    std::cout << "  Iter |     Cost     | Change    | Reduction(%)" << std::endl;
    std::cout << "  -----|--------------|-----------|-------------" << std::endl;

    for (size_t i = 0; i < result.cost_per_iteration.size(); i++)
    {
        double cost = result.cost_per_iteration[i];
        double change = (i == 0) ? 0.0
            : result.cost_per_iteration[i - 1] - cost;
        double reduction = (i == 0) ? 0.0
            : (change / result.cost_per_iteration[i - 1]) * 100.0;

        std::cout << std::setw(5) << i << " | "
                  << std::setprecision(3) << std::scientific << std::setw(12) << cost << " | "
                  << std::setw(9) << change << " | "
                  << std::fixed << std::setprecision(2) << std::setw(8) << reduction << "%"
                  << std::endl;
    }

    double total = (result.initial_cost > 0)
        ? (1.0 - result.final_cost / result.initial_cost) * 100 : 0;
    std::cout << "\n  총 cost 감소: " << std::fixed << std::setprecision(2)
              << total << "%" << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// 교육 블록
// ═══════════════════════════════════════════════════════════════

// ── 블록 1: BAL 데이터 형식 ──
static void demoBALFormat()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 1] BAL 데이터 형식 + Snavely 모델" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_easy Q2 (AutoDiffCostFunction<...,2,9,3>), Q1 (Ceres vs g2o) 연결
    std::cout << "Snavely 카메라 = 9 파라미터 (vs Week 8의 6):" << std::endl;
    std::cout << "  [0-2] angle-axis 회전" << std::endl;
    std::cout << "  [3-5] 평행이동" << std::endl;
    std::cout << "  [6]   focal length  ← 추가!" << std::endl;
    std::cout << "  [7-8] k1, k2 왜곡   ← 추가!\n" << std::endl;

    std::cout << "투영 과정:" << std::endl;
    std::cout << "  P' = R*P + t" << std::endl;
    std::cout << "  p = (-P'x/P'z, -P'y/P'z)   ← BAL: -z 방향" << std::endl;
    std::cout << "  distortion = 1 + k1*r² + k2*r⁴" << std::endl;
    std::cout << "  u = f * distortion * px" << std::endl;
    std::cout << "  v = f * distortion * py" << std::endl;
}

// ── 블록 2: BA 실행 + 수렴 관찰 ──
static void demoBAConvergence()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 2] Ceres BA + 수렴 과정 관찰" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_easy Q3 (DENSE_SCHUR), Q4 (HuberLoss) 연결
    // 💡 quiz_medium Q1 (잔차 차원), Q3 (수렴 분석) 연결
    auto data = CeresPractice::generate_bal_data(6, 50, 0.7);
    std::cout << "BA 문제: " << data.num_cameras << " 카메라, "
              << data.num_points << " 점, "
              << data.num_observations << " 관측\n" << std::endl;

    ceres::Problem problem;
    CeresPractice::build_problem(problem, data, true);

    ceres::Solver::Options options;
    CeresPractice::configure_solver(options, "dense_schur", false);

    auto result = CeresPractice::solve_and_report(problem, options);
    CeresPractice::analyze_convergence(result);

    std::cout << "\n  시간: " << std::fixed << std::setprecision(1)
              << result.solve_time_ms << " ms" << std::endl;
}

// ── 블록 3: Solver 타입 비교 ──
static void demoSolverTypes()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 3] DENSE_SCHUR vs SPARSE_SCHUR" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_medium Q2 (Schur Complement) 연결
    std::cout << "Solver 비교 (동일 문제):\n" << std::endl;

    for (const auto& type : {"dense_schur", "sparse_schur"})
    {
        auto data = CeresPractice::generate_bal_data(8, 80, 0.6);

        ceres::Problem problem;
        CeresPractice::build_problem(problem, data, true);

        ceres::Solver::Options options;
        CeresPractice::configure_solver(options, type, false);

        auto result = CeresPractice::solve_and_report(problem, options);

        std::cout << "  " << type << ":"
                  << " cost " << std::scientific << std::setprecision(3)
                  << result.initial_cost << " → " << result.final_cost
                  << ", " << std::fixed << std::setprecision(1)
                  << result.solve_time_ms << "ms"
                  << " (" << result.iterations << "회)" << std::endl;
    }

    std::cout << "\n  소규모: DENSE_SCHUR이 빠름 (오버헤드 적음)" << std::endl;
    std::cout << "  대규모: SPARSE_SCHUR가 빠름 (희소성 활용)" << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// 전체 데모
// ═══════════════════════════════════════════════════════════════

void CeresPractice::demo()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Week 11: Ceres 실습 — BAL 모델 + 수렴 관찰" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    demoBALFormat();
    demoBAConvergence();
    demoSolverTypes();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  다음 단계:" << std::endl;
    std::cout << "  1. ./my_basic 으로 직접 구현 연습" << std::endl;
    std::cout << "  2. quiz_easy / quiz_medium 풀기" << std::endl;
    std::cout << "  3. Week 12: 스케일 모호성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
}

int main()
{
    CeresPractice::demo();
    return 0;
}
