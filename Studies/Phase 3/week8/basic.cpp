// Week 8: Bundle Adjustment — Ceres Solver 기초
//
// 이 파일은 Ceres Solver로 BA 문제를 풀고, Week 7의 g2o와 비교한다.
//
// Ceres Solver란?
//   Google이 개발한 비선형 최소제곱법(Nonlinear Least Squares) 라이브러리.
//   자동 미분(Automatic Differentiation)이 핵심 특징.
//
// ★ g2o vs Ceres:
//   g2o: 그래프 기반 → Vertex/Edge 모델 → SLAM 특화 → ORB-SLAM에서 사용
//   Ceres: 일반 비선형 최적화 → CostFunction + Problem → 범용 → Google Maps에서 사용
//
//   개발 편의성: Ceres 우세 (자동 미분 → Jacobian 수동 유도 불필요)
//   SLAM 특화: g2o 우세 (SE3, Schur 등이 내장)
//   실무 선택: 프로토타입/연구 → Ceres, SLAM 시스템 → g2o
//
// 자동 미분(Automatic Differentiation, AD):
//   사용자가 비용 함수 f(x)를 template<T>로 정의하면,
//   Ceres가 Jacobian ∂f/∂x를 자동으로 계산.
//
//   원리: Dual Number (Jet)
//     T = double: f(x)만 계산 (값)
//     T = Jet:    f(x)와 f'(x)를 동시에 계산 (값 + Jacobian)
//     Jet = a + bε (ε² = 0) → 체인 룰이 자동 적용됨
//
//   ★ 수치 미분(유한 차분)과의 차이:
//     수치 미분: f'(x) ≈ (f(x+h) - f(x))/h — 근사, 오차 존재
//     자동 미분: 정확한 Jacobian — 수학적으로 동일한 결과

#include "ceres_ba.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <ceres/rotation.h>

// ═══════════════════════════════════════════════════════════════
// 구현부
// ═══════════════════════════════════════════════════════════════

// 재투영 오차 CostFunction — 자동 미분 템플릿
//
// operator():
//   camera[0-2]: angle-axis 회전 (3 DoF)
//     → Rodrigues 공식으로 3×3 회전 행렬에 해당
//     → 벡터 방향 = 회전 축, 벡터 크기 = 회전 각도(라디안)
//   camera[3-5]: translation 이동 (3 DoF)
//   point[0-2]: 3D 점 좌표 (X, Y, Z)
//   residual[0-1]: 재투영 오차 (u 방향, v 방향)
//
// 투영 과정:
//   ① 카메라 좌표계 변환: Pc = R · (Pw - t)
//      → ceres::AngleAxisRotatePoint로 회전 적용
//   ② 정규화 좌표 → 픽셀 좌표: u = fx · X/Z + cx
//   ③ 잔차 계산: residual = predicted - observed
//
// ★ template <typename T>의 의미:
//   T=double일 때: 일반 값 계산
//   T=Jet<double, N>일 때: 값 + N개 파라미터에 대한 편미분 동시 계산
//   → 사용자는 같은 코드를 작성하지만, Ceres가 타입을 바꿔가며 호출
template <typename T>
bool CeresBA::ReprojectionError::operator()(
    const T* const camera, const T* const point, T* residual) const
{
    // ① 카메라 좌표계 변환
    //    Pw - t → 세계 좌표에서 카메라 원점 기준으로 이동
    //    AngleAxisRotatePoint: angle-axis로 회전 적용 (Rodrigues)
    T p[3];
    T pw[3] = {point[0] - camera[3], point[1] - camera[4], point[2] - camera[5]};
    ceres::AngleAxisRotatePoint(camera, pw, p);

    // ② 투영: 카메라 좌표 → 픽셀 좌표
    //    u = fx · X/Z + cx,  v = fy · Y/Z + cy
    T u = T(fx) * p[0] / p[2] + T(cx);
    T v = T(fy) * p[1] / p[2] + T(cy);

    // ③ 잔차 = 예측 - 관측
    residual[0] = u - T(observed_x);
    residual[1] = v - T(observed_y);
    return true;
}

// AutoDiffCostFunction 팩토리 — Ceres에 등록할 형태로 래핑
//
// AutoDiffCostFunction<ReprojectionError, 2, 6, 3>:
//   ReprojectionError = 오차 Functor (위의 operator() 정의)
//   2 = residual 차원 (u 오차, v 오차)
//   6 = 첫 번째 파라미터 블록 차원 (camera: angle-axis 3 + translation 3)
//   3 = 두 번째 파라미터 블록 차원 (point: X, Y, Z)
//
// ★ Ceres는 이 템플릿 정보로 Jacobian 크기를 결정:
//   J_camera: 2×6 행렬 (residual 2차원 / camera 6파라미터)
//   J_point:  2×3 행렬 (residual 2차원 / point 3파라미터)
ceres::CostFunction* CeresBA::ReprojectionError::create(
    double x, double y, double fx, double fy, double cx, double cy)
{
    return new ceres::AutoDiffCostFunction<ReprojectionError, 2, 6, 3>(
        new ReprojectionError(x, y, fx, fy, cx, cy));
}

// Problem에 관측(residual block) 추가
//
// g2o의 addEdge()에 해당하는 Ceres 패턴.
//
// AddResidualBlock(cost, loss, param1, param2):
//   cost: 비용 함수 (재투영 오차 계산)
//   loss: 손실 함수 (nullptr = L2, HuberLoss = robust)
//   param1, param2: 최적화할 파라미터 블록의 포인터
//     → camera[6]: angle-axis(3) + translation(3)
//     → point[3]: X, Y, Z
//
// ★ g2o와의 핵심 차이:
//   g2o: Vertex ID로 연결 → 정수 기반
//   Ceres: 실제 메모리 주소(double*)로 연결 → 포인터 기반
//   → Ceres가 더 직관적이고 유연하지만, 메모리 관리에 주의 필요
void CeresBA::add_residual_block(
    ceres::Problem& problem,
    double* camera, double* point,
    double observed_x, double observed_y,
    const Eigen::Vector4d& K,
    bool use_robust)
{
    ceres::CostFunction* cost = ReprojectionError::create(
        observed_x, observed_y, K(0), K(1), K(2), K(3));

    // HuberLoss(1.0): δ=1.0px — g2o의 RobustKernelHuber과 동일한 역할
    ceres::LossFunction* loss = nullptr;
    if (use_robust)
    {
        loss = new ceres::HuberLoss(1.0);
    }

    // camera, point: 실제 배열의 포인터
    //   → Ceres가 이 배열을 직접 수정하여 최적화 결과를 반영
    problem.AddResidualBlock(cost, loss, camera, point);
}

// Solver 옵션 설정 — Linear Solver 선택
//
// DENSE_SCHUR:
//   BA 전용 Solver. Schur Complement로 3D 점을 소거한 후
//   카메라 변수만으로 축소된 정규 방정식을 풀음.
//   g2o에서 setMarginalized(true) + BlockSolver_6_3에 해당.
//
// DENSE_QR:
//   일반 QR 분해. BA 구조를 활용하지 않아 느림.
//   BA가 아닌 일반 최적화 문제에 적합.
//
// 💡 대규모 BA에서의 선택:
//   SPARSE_SCHUR: 희소 Cholesky 분해 → 중규모 BA (카메라 ~100)
//   ITERATIVE_SCHUR + SCHUR_JACOBI: 반복적 풀이 → 대규모 BA (카메라 1000+)
void CeresBA::configure_solver(
    ceres::Solver::Options& options,
    bool use_schur)
{
    options.linear_solver_type = use_schur
        ? ceres::DENSE_SCHUR       // BA 전용: Schur Complement 적용
        : ceres::DENSE_QR;         // 일반: 구조 활용 안 함
    options.minimizer_progress_to_stdout = false;   // true면 매 반복 출력
    options.max_num_iterations = 50;
    options.function_tolerance = 1e-6;  // 비용 변화가 이보다 작으면 수렴 판정
}

// 최적화 실행 — Solver::Summary에서 결과 추출
//
// ceres::Solve():
//   problem과 options를 받아 LM 알고리즘으로 최적화 수행.
//   결과는 Summary에 기록:
//     initial_cost: 최적화 전 총 비용 (Σ residual²/2)
//     final_cost: 최적화 후 총 비용
//     iterations: 실행된 반복 횟수
//     total_time_in_seconds: 총 소요 시간
//
// ★ g2o의 activeChi2()와 Ceres의 cost 비교:
//   g2o: chi2 = Σ e^T · Ω · e  (information matrix 포함)
//   Ceres: cost = Σ residual² / 2  (1/2 factor 차이)
//   → 절대값이 다를 수 있지만 경향(감소 여부)은 동일
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

// g2o 결과와 비교 출력
//
// 같은 BA 문제를 g2o와 Ceres로 풀어 결과를 비교.
// 동일한 최적화 문제이므로 최종 오차는 유사해야 함.
// 속도와 사용 편의성에서 차이가 남.
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

// ── 블록 1: Ceres 자동 미분 — template T의 마법 ──
//
// Ceres의 핵심 아이디어:
//   "비용 함수를 template<T>로 작성하면, Jacobian을 자동으로 계산해준다"
//
// 원리: Dual Number (Jet)
//   Jet<double, N> = value + partial[N]
//   f(Jet(x, 1)) = Jet(f(x), f'(x))  ← 자동으로 미분!
//
//   예: f(x) = x²  →  f(Jet(3, 1)) = Jet(9, 6)
//   → f(3) = 9, f'(3) = 2·3 = 6
//
// ★ 수치 미분(유한 차분)과의 비교:
//   수치: f'(x) ≈ (f(x+h) - f(x))/h → 근사, h 선택 문제
//   자동: 정확한 도함수, h 선택 불필요
//   해석: 사람이 직접 Jacobian 유도 → 실수 가능, 복잡
//   → 자동 미분이 정확성 + 편의성에서 최적
//
// 💡 quiz_easy Q1: 자동 미분의 원리 (Jet)
// 💡 quiz_easy Q2: AutoDiffCostFunction 템플릿 파라미터 의미
// 💡 quiz_easy Q3: template T가 Jet이 되는 시점
static void demoAutoDiff()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 1] Ceres 자동 미분 — template T" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    std::cout << "자동 미분 원리:" << std::endl;
    std::cout << "  1. CostFunction을 template<T>로 정의" << std::endl;
    std::cout << "  2. T=double → 값 계산" << std::endl;
    std::cout << "  3. T=Jet → 값 + Jacobian 동시 계산\n" << std::endl;

    // AutoDiffCostFunction 템플릿 파라미터 설명:
    //   <Error, 2, 6, 3>
    //   Error: 오차 계산 Functor (operator() 가진 구조체)
    //   2: residual 차원 → Jacobian의 행 수
    //   6: 첫 번째 파라미터 블록 → J1은 2×6
    //   3: 두 번째 파라미터 블록 → J2는 2×3
    std::cout << "AutoDiffCostFunction<Error, 2, 6, 3>:" << std::endl;
    std::cout << "  Error: 오차 Functor" << std::endl;
    std::cout << "  2: residual 차원 (u, v)" << std::endl;
    std::cout << "  6: camera 파라미터 (angle-axis 3 + translation 3)" << std::endl;
    std::cout << "  3: point 파라미터 (x, y, z)\n" << std::endl;

    // ── 간단한 예시: f(x) = (x-3)² 최소화 ──
    //    residual = x - 3 → 최소화하면 x = 3
    //    AutoDiffCostFunction<SimpleError, 1, 1>: residual 1개, 파라미터 1개
    std::cout << "예시: f(x) = (x-3)² 최소화" << std::endl;

    struct SimpleError
    {
        template <typename T>
        bool operator()(const T* const x, T* residual) const
        {
            residual[0] = x[0] - T(3.0);   // r = x - 3 → r² = (x-3)²
            return true;
        }
    };

    double x = 0.0;  // 초기값 x=0
    ceres::Problem problem;
    // AutoDiffCostFunction<SimpleError, 1, 1>:
    //   residual 1개, 파라미터 블록 1개(차원 1)
    problem.AddResidualBlock(
        new ceres::AutoDiffCostFunction<SimpleError, 1, 1>(new SimpleError()),
        nullptr,    // 손실 함수 없음 (L2)
        &x);        // 최적화할 변수의 포인터

    ceres::Solver::Options options;
    options.minimizer_progress_to_stdout = false;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);

    // 초기값 0.0 → 최적화 후 3.0으로 수렴해야 함
    std::cout << "  초기 x=0.0 → 최적화 후 x=" << x << " (기대: 3.0)" << std::endl;
}

// ── 블록 2: Ceres BA — 재투영 오차 최소화 ──
//
// Week 7(g2o)과 동일한 BA 문제를 Ceres로 풀어보기.
//   4개 포즈 + 20개 3D 점 + 노이즈 관측 → 최적화
//
// g2o와의 구현 차이:
//   g2o:  VertexSE3Expmap, EdgeProjectXYZ2UV 사용
//   Ceres: double 배열 + template operator() 사용
//   g2o:  Vertex ID로 연결
//   Ceres: 포인터로 연결
//
// ★ Ceres의 카메라 파라미터화:
//   camera[0-2] = angle-axis (3 DoF 회전)
//   camera[3-5] = translation (3 DoF 이동)
//   → 총 6개 double 배열로 6 DoF 표현
//   → g2o는 SE3Quat(쿼터니언+이동)으로 7개지만 내부 6 DoF
//
// 💡 quiz_easy Q4: HuberLoss의 역할
// 💡 quiz_easy Q6: SetParameterBlockConstant의 역할
static void demoCeresBA()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 2] Ceres BA — 재투영 오차 최소화" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    Eigen::Vector4d K(500, 500, 320, 240);  // (fx, fy, cx, cy)

    std::mt19937 rng(42);
    std::normal_distribution<double> noise_obs(0.0, 1.0);  // 관측 노이즈 σ=1px
    std::normal_distribution<double> noise_pt(0.0, 0.3);    // 3D 점 노이즈
    std::normal_distribution<double> noise_cam(0.0, 0.1);   // 포즈 노이즈
    std::uniform_real_distribution<double> dist_x(-3, 3);
    std::uniform_real_distribution<double> dist_y(-2, 2);
    std::uniform_real_distribution<double> dist_z(3, 15);

    const int kNumPoses = 4;
    const int kNumPoints = 20;

    // GT 데이터 생성
    std::vector<Eigen::Vector3d> gt_t, gt_pts;
    for (int i = 0; i < kNumPoses; i++)
        gt_t.push_back(Eigen::Vector3d(0, 0, i * 0.5));
    for (int j = 0; j < kNumPoints; j++)
        gt_pts.push_back(Eigen::Vector3d(dist_x(rng), dist_y(rng), dist_z(rng)));

    // Ceres 파라미터 배열 — 최적화 대상
    //   cameras[i][0-2]: angle-axis 회전 (초기값 = 무회전 = 0)
    //   cameras[i][3-5]: translation (GT + 노이즈)
    //   points[j][0-2]: 3D 점 좌표 (GT + 노이즈)
    //
    // ★ Ceres는 이 배열을 "직접" 수정 → 최적화 후 배열에 결과가 들어있음
    std::vector<std::array<double, 6>> cameras(kNumPoses);
    std::vector<std::array<double, 3>> points(kNumPoints);

    // 초기값 설정 (노이즈 추가)
    for (int i = 0; i < kNumPoses; i++)
    {
        cameras[i] = {0, 0, 0, gt_t[i](0), gt_t[i](1), gt_t[i](2)};
        if (i > 0)  // 첫 포즈는 정확 (고정할 예정)
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

    // Problem 구성 — 모든 관측을 residual block으로 추가
    ceres::Problem problem;
    int obs_count = 0;

    for (int i = 0; i < kNumPoses; i++)
    {
        for (int j = 0; j < kNumPoints; j++)
        {
            // GT로 투영 + 노이즈 → 관측 시뮬레이션
            Eigen::Vector3d Pc = gt_pts[j] - gt_t[i];
            if (Pc(2) < 0.1)
                continue;
            double u = K(0) * Pc(0) / Pc(2) + K(2) + noise_obs(rng);
            double v = K(1) * Pc(1) / Pc(2) + K(3) + noise_obs(rng);

            // add_residual_block: camera[i]와 point[j]를 연결
            //   → 이 관측이 camera[i]와 point[j]에 영향을 줌
            CeresBA::add_residual_block(
                problem, cameras[i].data(), points[j].data(),
                u, v, K, true);     // use_robust = true (Huber)
            obs_count++;
        }
    }

    // 첫 포즈 고정 — Gauge Freedom 해결
    //   g2o의 setFixed(true)에 해당
    //   SetParameterBlockConstant: 이 파라미터 블록을 최적화에서 제외
    problem.SetParameterBlockConstant(cameras[0].data());

    std::cout << "BA 문제: " << kNumPoses << " 포즈, " << kNumPoints
              << " 3D점, " << obs_count << " 관측\n" << std::endl;

    // Solver 설정 + 실행
    ceres::Solver::Options options;
    CeresBA::configure_solver(options, true);   // DENSE_SCHUR 사용

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

// ── 블록 3: Solver 비교 — DENSE_SCHUR vs DENSE_QR ──
//
// BA 문제의 구조를 활용하는 Solver와 그렇지 않은 Solver를 비교.
//
// BA의 구조적 특징:
//   포즈 수 ≪ 3D 점 수 (예: 100 카메라, 10000 점)
//   Hessian이 희소하고 블록 구조를 가짐
//   → Schur Complement로 점 변수를 소거하면 작은 시스템만 풀면 됨
//
// Solver 선택 가이드:
//   소규모(카메라 <20): DENSE_SCHUR 또는 DENSE_QR (둘 다 빠름)
//   중규모(카메라 20~500): SPARSE_SCHUR (희소 Cholesky)
//   대규모(카메라 500+): ITERATIVE_SCHUR + SCHUR_JACOBI (반복적)
//
// 💡 quiz_easy Q5: g2o의 단점 (수동 Jacobian)
// 💡 quiz_medium Q1: Jet의 동작 원리
// 💡 quiz_medium Q2: DENSE_SCHUR의 원리
// 💡 quiz_medium Q3: Ceres vs g2o 비교
static void demoSolverComparison()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 3] DENSE_SCHUR vs DENSE_QR" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    std::cout << "Solver 선택 가이드:" << std::endl;
    std::cout << "  DENSE_SCHUR: BA 전용 (포즈≪점 구조 활용)" << std::endl;
    std::cout << "  DENSE_QR: 일반 문제 (구조 활용 안 함)" << std::endl;
    std::cout << "  SPARSE_SCHUR: 대규모 BA (희소 행렬)" << std::endl;
    std::cout << "  ITERATIVE_SCHUR: 초대규모 BA\n" << std::endl;

    // Ceres vs g2o 종합 비교
    //
    //   항목             Ceres              g2o
    //   ──────────────────────────────────────────
    //   미분             자동 (Jet)          수동/수치
    //   인터페이스       CostFunction       Vertex/Edge
    //   강점             범용, 개발 빠름    SLAM 특화, 빠른 실행
    //   사용 예          Google Maps        ORB-SLAM, LSD-SLAM
    //   Schur            Solver 옵션 선택   setMarginalized
    //
    // ★ 실무에서의 선택:
    //   연구/프로토타입 → Ceres (개발 시간 단축)
    //   SLAM 시스템 → g2o (ORB-SLAM 코드 재활용, SLAM 특화 기능)
    //   둘 다 아는 것이 이상적 (주제에 따라 선택)
    std::cout << "Ceres vs g2o:" << std::endl;
    std::cout << "  Ceres: 자동 미분 → 개발 빠름, 범용" << std::endl;
    std::cout << "  g2o:   그래프 구조 → SLAM 특화, ORB-SLAM 사용" << std::endl;
    std::cout << "  선택:  연구/프로토타입 → Ceres, SLAM 시스템 → g2o" << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// 전체 데모 + main
// ═══════════════════════════════════════════════════════════════

// 전체 Week 8 데모 실행
//   블록 1: 자동 미분 원리 (template T, Jet)
//   블록 2: Ceres BA 구현 (재투영 오차 최적화)
//   블록 3: Solver 비교 (DENSE_SCHUR vs DENSE_QR, Ceres vs g2o)
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

    // Ceres BA의 5단계 핵심 패턴:
    //   1. CostFunction 정의: template<T> operator()로 비용 함수 작성
    //   2. Problem.AddResidualBlock: 관측마다 cost + loss + 파라미터 등록
    //   3. Solver::Options 설정: DENSE_SCHUR이 BA에 최적
    //   4. SetParameterBlockConstant: 첫 포즈 고정 (Gauge Freedom)
    //   5. Solve: LM 실행 → Summary에서 결과 확인
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
