#include "ceres_practice.h"
#include <ceres/rotation.h>
#include <iostream>
#include <iomanip>
#include <random>

// ═══════════════════════════════════════════════════════════════
// 구현부
// ═══════════════════════════════════════════════════════════════

// Snavely 재투영 오차 — BAL 데이터셋에서 사용하는 9파라미터 카메라 모델
//
// Week 8의 6파라미터(SE3)와 달리, focal length + 왜곡까지 포함하여
// 카메라 내부/외부 파라미터를 동시에 최적화하는 모델.
//
// 카메라 파라미터 9개:
//   [0-2] angle-axis 회전 벡터 (3) — Rodrigues 표현
//   [3-5] 평행이동 벡터 (3) — 카메라→월드
//   [6]   focal length (1) — 초점 거리 (pixel)
//   [7-8] radial distortion k1, k2 (2) — 방사 왜곡 계수
//
// 투영 과정 (6단계):
//   ① angle-axis로 3D 점 회전: P' = R(camera[0:3]) · P
//   ② 평행이동 적용: P' += camera[3:6]
//   ③ 정규화 좌표 (BAL 관례: -z 방향): xp = -P'x/P'z, yp = -P'y/P'z
//      → BAL은 카메라가 -z 방향을 보는 관례 사용 (OpenCV와 다름)
//   ④ 방사 왜곡 적용: distortion = 1 + k1·r² + k2·r⁴
//      → r² = xp² + yp², 중심에서 멀수록 왜곡 증가
//   ⑤ focal length 적용: u = f · distortion · xp
//   ⑥ 잔차 계산: residual = predicted - observed (2차원)
//
// ★ template <typename T>의 의미:
//   T = double → 일반적인 순방향 계산
//   T = ceres::Jet<double, N> → 자동 미분 (chain rule 자동 적용)
//   operator()를 템플릿으로 정의하면 Ceres가 Jet 타입으로 호출하여
//   자코비안을 정확하게 자동 계산함 (수치 미분이 아닌 정확한 미분!)
template <typename T>
bool CeresPractice::SnavelyError::operator()(
    const T* const camera, const T* const point, T* residual) const
{
    // ① angle-axis 회전 — 3차원 회전 벡터 camera[0:3]로 점을 회전
    //    AngleAxisRotatePoint 내부: Rodrigues 공식 적용
    //      회전축 = camera[0:3] 방향, 회전각 = ||camera[0:3]||
    T p[3];
    ceres::AngleAxisRotatePoint(camera, point, p);

    // ② 평행이동 — 회전된 점에 camera[3:6] 더하기
    //    P_camera = R · P_world + t
    p[0] += camera[3];
    p[1] += camera[4];
    p[2] += camera[5];

    // ③ 정규화 좌표 — 3D 점을 z=1 평면으로 투영 (BAL: -z 방향)
    //    일반적인 핀홀: xp = Px/Pz, yp = Py/Pz
    //    BAL 관례: 부호 반전 (카메라가 -z 방향을 향함)
    T xp = -p[0] / p[2];
    T yp = -p[1] / p[2];

    // ④ 방사 왜곡 — Week 2에서 배운 렌즈 왜곡 모델의 간략 버전
    //    distortion = 1 + k1·r² + k2·r⁴
    //    BAL은 k1, k2만 사용 (접선 왜곡 p1, p2 생략)
    //    k1 < 0 → 배럴 왜곡 (가장자리가 바깥으로)
    //    k1 > 0 → 핀쿠션 왜곡 (가장자리가 안쪽으로)
    const T& k1 = camera[7];
    const T& k2 = camera[8];
    T r2 = xp * xp + yp * yp;
    T distortion = T(1.0) + r2 * (k1 + k2 * r2);

    // ⑤ focal length 적용 — 정규화 좌표를 픽셀 좌표로 변환
    //    일반적 핀홀: u = fx·xp + cx, v = fy·yp + cy
    //    BAL 단순화: u = f·xp, v = f·yp (주점 cx=cy=0 가정)
    const T& focal = camera[6];
    T predicted_x = focal * distortion * xp;
    T predicted_y = focal * distortion * yp;

    // ⑥ 잔차 — 예측된 2D 좌표와 실제 관측의 차이
    //    residual[0] = u_predicted - u_observed
    //    residual[1] = v_predicted - v_observed
    //    → Ceres가 Σ(residual²)를 최소화하도록 파라미터를 조정
    residual[0] = predicted_x - T(observed_x);
    residual[1] = predicted_y - T(observed_y);
    return true;
}

// AutoDiffCostFunction 생성 팩토리
//
// AutoDiffCostFunction<SnavelyError, 2, 9, 3>:
//   SnavelyError: 잔차를 계산하는 functor
//   2: 잔차 차원 (재투영 오차 x, y)
//   9: 첫 번째 파라미터 블록 크기 (카메라 9 파라미터)
//   3: 두 번째 파라미터 블록 크기 (3D 점 x, y, z)
//
// ★ Ceres의 자동 미분은 이 숫자들로 자코비안 크기를 결정:
//   ∂residual/∂camera = 2×9 행렬
//   ∂residual/∂point = 2×3 행렬
//   → Jet<double, 12> 타입으로 operator()를 호출하여 한 번에 계산
ceres::CostFunction* CeresPractice::SnavelyError::create(double x, double y)
{
    return new ceres::AutoDiffCostFunction<SnavelyError, 2, 9, 3>(
        new SnavelyError(x, y));
}

// 합성 BAL 데이터 생성 — 테스트용 가상 BA 문제 구축
//
// BAL (Bundle Adjustment in the Large):
//   Washington 대학에서 만든 대규모 BA 벤치마크 데이터셋 형식.
//   카메라 9파라미터 + 3D 점 + 2D 관측으로 구성.
//
// 생성 전략:
//   ① 카메라: Z축 방향으로 일직선 배치, 첫 카메라 고정
//   ② 3D 점: 카메라 앞 공간에 랜덤 분포 (z=5~25m)
//   ③ 관측: 각 카메라-점 쌍에 대해 obs_ratio 확률로 관측 생성
//     → obs_ratio=0.7이면 약 70%의 카메라-점 쌍이 관측됨
//     → 실제 SfM에서는 모든 카메라가 모든 점을 보지 못함
//
// 노이즈 설계:
//   카메라 회전: σ=0.02 rad (≈1°) — 초기 포즈 추정 오차
//   카메라 이동: σ=0.06m — 초기 위치 추정 오차
//   focal length: σ=1.0 pixel — 내부 파라미터 추정 오차
//   3D 점: σ=0.3m — 삼각측량 오차
//   2D 관측: σ=0.5 pixel — 특징점 검출 오차
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

    // GT(Ground Truth) focal length = 500 pixel
    double focal_gt = 500.0;

    // ── GT 카메라 생성: Z축 방향 일직선 배치 ──
    //
    // 카메라 파라미터 배열 [9]:
    //   [0-2] angle-axis 회전 (GT: 모두 0 = 항등 회전)
    //   [3-5] 평행이동 (GT: (0, 0, i*0.5) = Z 방향 이동)
    //   [6]   focal length (GT: 500)
    //   [7-8] k1, k2 (GT: 모두 0 = 왜곡 없음)
    //
    // 첫 카메라(i=0)만 GT 그대로 유지 (Gauge freedom 해소)
    // 나머지 카메라는 노이즈를 추가하여 "잘못된 초기값" 시뮬레이션
    data.cameras.resize(num_cameras);
    for (int i = 0; i < num_cameras; i++)
    {
        data.cameras[i] = {0, 0, 0, 0, 0, i * 0.5, focal_gt, 0, 0};
        if (i > 0)
        {
            for (int k = 0; k < 3; k++)
                data.cameras[i][k] += noise_cam(rng);      // 회전 노이즈
            for (int k = 3; k < 6; k++)
                data.cameras[i][k] += noise_cam(rng) * 3;  // 이동 노이즈 (더 큼)
            data.cameras[i][6] += noise_cam(rng) * 50;      // focal 노이즈
        }
    }

    // ── GT 3D 점 생성: 카메라 앞 공간에 랜덤 분포 ──
    //   x: -5~5m, y: -3~3m, z: 5~25m (카메라 앞)
    //   초기값에 노이즈를 추가 → 최적화가 보정해야 할 오차
    data.points.resize(num_points);
    std::vector<Eigen::Vector3d> gt_pts(num_points);
    for (int j = 0; j < num_points; j++)
    {
        gt_pts[j] = Eigen::Vector3d(dist_x(rng), dist_y(rng), dist_z(rng));
        data.points[j] = {gt_pts[j](0) + noise_pt(rng),
                           gt_pts[j](1) + noise_pt(rng),
                           gt_pts[j](2) + noise_pt(rng)};
    }

    // ── 관측 생성 (BAL 스타일: -z 방향 투영) ──
    //
    // 각 카메라-점 쌍에 대해:
    //   1. coin(rng) > obs_ratio이면 관측하지 않음
    //      (실제 SfM에서 모든 점이 모든 카메라에서 보이지 않음)
    //   2. GT 투영: R=I → Pc = P + t, BAL 관례로 -z 방향 투영
    //      xp = -Pc_x / Pc_z, yp = -Pc_y / Pc_z
    //   3. focal 적용: u = focal · xp + 노이즈
    //   4. Pc_z < 1.0이면 카메라 뒤/너무 가까우므로 제외
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

// Ceres Problem 구성 — 각 관측에 대해 ResidualBlock 추가
//
// g2o와의 대응:
//   g2o: Vertex + Edge + setRobustKernel()
//   Ceres: Parameter Block + ResidualBlock + LossFunction
//
// ResidualBlock 구조:
//   CostFunction: SnavelyError (잔차 계산 functor)
//   LossFunction: HuberLoss (outlier 억제) 또는 nullptr (제곱 오차)
//   파라미터 블록: cameras[i] (9개), points[j] (3개)
//
// ★ Ceres에서 파라미터 블록은 raw 배열의 포인터
//   → data.cameras[i].data()가 직접 최적화 변수로 사용됨
//   → Solve() 후 data.cameras, data.points의 값이 직접 업데이트됨
//
// HuberLoss(1.0):
//   잔차 < 1.0: 일반 제곱 오차 (inlier)
//   잔차 ≥ 1.0: 선형 증가로 전환 (outlier 영향 제한)
//   → g2o의 RobustKernelHuber와 동일한 역할
//   → 특징점 매칭 오류(outlier)가 전체 최적화를 망치는 것을 방지
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

    // 첫 카메라 고정 — Gauge freedom 해소
    //   BA에서 절대 좌표계를 결정하려면 최소 하나의 카메라를 고정해야 함
    //   g2o의 setFixed(true)에 해당
    //   SetParameterBlockConstant: 이 블록의 값을 최적화에서 변경하지 않음
    problem.SetParameterBlockConstant(data.cameras[0].data());
}

// Solver 옵션 설정 — 선형 solver 타입과 반복 조건 구성
//
// DENSE_SCHUR vs SPARSE_SCHUR:
//   ① DENSE_SCHUR: Schur Complement 후 S 행렬을 dense로 풀기
//      → 카메라 수가 적을 때 오버헤드 적음 (S 크기: 9N × 9N)
//      → 소규모(카메라 < ~50)에서 유리
//
//   ② SPARSE_SCHUR: Schur Complement 후 S 행렬을 sparse로 풀기
//      → S 행렬 자체도 희소하면 효율적
//      → 대규모(카메라 > ~50)에서 유리
//
// ★ 두 방법 모두 Schur Complement를 사용 — 점을 먼저 소거!
//   차이는 소거 후 남은 S·Δc = b'를 푸는 방법.
//   g2o의 LinearSolverEigen ≈ SPARSE_SCHUR,
//   g2o의 LinearSolverDense ≈ DENSE_SCHUR
//
// function_tolerance = 1e-6: cost 변화가 10⁻⁶ 이하면 수렴으로 판정
//   → 너무 작으면 불필요한 iteration, 너무 크면 조기 종료
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

// 최적화 실행 및 결과 반환
//
// ceres::Solve()가 내부에서 수행하는 과정 (매 iteration):
//   ① 잔차 계산: 모든 ResidualBlock의 오차 벡터 계산
//   ② 자코비안 계산: AutoDiff(Jet 타입)로 정확한 미분 자동 생성
//   ③ 정규 방정식 구성: H = JᵀΩJ, b = -JᵀΩe
//   ④ Schur Complement: 점 소거 → S·Δc = b'
//   ⑤ 선형 시스템 풀기: DENSE_SCHUR 또는 SPARSE_SCHUR
//   ⑥ 파라미터 업데이트: cameras += Δc, points += Δp (Back-sub)
//   ⑦ 수렴 판정: function_tolerance, gradient_tolerance 확인
//
// Summary에서 얻을 수 있는 정보:
//   initial_cost: 최적화 전 총 cost = Σ(잔차²)
//   final_cost: 최적화 후 총 cost
//   iterations: 각 iteration의 상세 정보 (cost, 시간 등)
//   total_time_in_seconds: 전체 소요 시간
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

    // iteration별 cost 기록 — 수렴 과정 분석에 사용
    for (const auto& iter : summary.iterations)
    {
        result.cost_per_iteration.push_back(iter.cost);
    }

    return result;
}

// 수렴 과정 분석 — iteration별 cost 변화를 표로 출력
//
// 전형적인 수렴 패턴:
//   Iter 0: cost = 10⁷  (초기, 큰 오차)
//   Iter 1: cost = 10⁵  (급감, LM이 큰 스텝)
//   Iter 2: cost = 10⁴  (급감 계속)
//   Iter 3: cost = 8000  (감소 둔화, 정답 근처)
//   ...
//   Iter 10: cost = 7500  (수렴, 변화 미미)
//
// 관찰 포인트:
//   - 처음 1~3회에서 cost가 급감하면 초기값이 양호
//   - cost가 감소하지 않으면 초기값이 너무 나쁘거나 outlier 존재
//   - Reduction(%)이 0.01% 미만이면 실질적으로 수렴 완료
//   - 💡 quiz_medium Q3에서 이 수렴 패턴을 분석하는 문제 출제!
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
//
// BAL (Bundle Adjustment in the Large):
//   Washington 대학에서 만든 SfM 벤치마크 데이터셋.
//   Ceres 공식 예제에서 사용하며, 다양한 규모의 BA 문제를 제공.
//
// Snavely 모델이 Week 8의 카메라 모델과 다른 점:
//   Week 8: camera = [rvec(3), tvec(3)] = 6 파라미터
//     → 내부 파라미터(K)는 고정으로 가정
//   BAL: camera = [rvec(3), tvec(3), f(1), k1(1), k2(1)] = 9 파라미터
//     → focal length와 왜곡을 함께 최적화!
//     → SfM에서는 K를 모르는 경우가 많아 함께 추정
//
// 💡 quiz_easy Q2: AutoDiffCostFunction<..., 2, 9, 3>의 숫자 의미
// 💡 quiz_easy Q1: Ceres vs g2o 비교
static void demoBALFormat()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 1] BAL 데이터 형식 + Snavely 모델" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

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
//
// 합성 BAL 데이터로 BA를 실행하고 iteration별 cost 변화를 관찰.
//
// BA 문제 규모:
//   6 카메라 × 50 점 × 70% 관측 → ~210개 관측
//   총 파라미터: 6×9 + 50×3 = 204개
//   총 잔차: ~420개 (관측×2차원)
//
// 💡 quiz_easy Q3: DENSE_SCHUR의 의미
// 💡 quiz_easy Q4: HuberLoss가 필요한 이유
// 💡 quiz_medium Q1: 잔차 차원이 2인 이유
// 💡 quiz_medium Q3: 수렴 분석 방법
static void demoBAConvergence()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 2] Ceres BA + 수렴 과정 관찰" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

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
//
// 동일한 BA 문제를 DENSE_SCHUR과 SPARSE_SCHUR로 풀어 비교.
//
// 둘 다 Schur Complement를 적용하지만, S·Δc = b' 풀기 방법이 다름:
//   DENSE_SCHUR: S를 dense 행렬로 다루고 Cholesky 분해
//     → 오버헤드 작음, 소규모 유리
//   SPARSE_SCHUR: S의 희소 구조를 활용하여 sparse Cholesky
//     → 희소 구조 분석 오버헤드, 대규모 유리
//
// 소규모(8 카메라)에서는 DENSE_SCHUR이 빠른 것이 일반적.
// 카메라 수 ~50 이상에서 SPARSE_SCHUR이 유리해지기 시작.
//
// 💡 quiz_medium Q2: Schur Complement의 역할
static void demoSolverTypes()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 3] DENSE_SCHUR vs SPARSE_SCHUR" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

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

// 전체 데모 — Ceres BA의 핵심 개념을 3가지 블록으로 시연
//
// 학습 흐름:
//   블록 1: BAL 데이터 형식과 Snavely 9파라미터 카메라 모델 이해
//   블록 2: BA 실행 후 iteration별 수렴 과정 관찰
//   블록 3: DENSE_SCHUR vs SPARSE_SCHUR 성능 비교
//
// g2o(Week 10)와 Ceres의 핵심 차이:
//   g2o:   Vertex + Edge + 수동 자코비안 (또는 내장 Edge 사용)
//   Ceres: CostFunction + Problem + 자동 미분 (AutoDiff)
//   → Ceres는 operator()만 정의하면 자코비안을 자동 계산
//   → g2o는 더 빠르지만, Ceres는 새 오차 함수 추가가 훨씬 쉬움
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
