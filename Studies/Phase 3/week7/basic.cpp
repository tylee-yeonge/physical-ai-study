// Week 7: Bundle Adjustment — g2o 기초
//
// 이 파일은 g2o 라이브러리를 사용하여 Bundle Adjustment(BA)의 핵심 개념을 시연한다.
//
// Bundle Adjustment(BA)란?
//   카메라 포즈(R, t)와 3D 점을 "동시에" 최적화하여 재투영 오차를 최소화.
//   SLAM에서 가장 중요한 백엔드 최적화 기법.
//
//   목적 함수:
//     min  Σ  ||z_ij - π(T_i, P_j)||²
//     T,P  i,j
//
//     z_ij = 카메라 i에서 3D 점 j의 2D 관측 (픽셀 좌표)
//     T_i  = 카메라 i의 포즈 (SE(3): 6 DoF)
//     P_j  = 3D 점 j의 좌표 (3 DoF)
//     π()  = 투영 함수: P_camera = R·P_world + t → (fx·X/Z + cx, fy·Y/Z + cy)
//
//   "Bundle" = 3D 점에서 카메라로 향하는 광선 다발(ray bundle)을
//   모든 관측에 일관되게 "조정(Adjustment)"한다는 의미.
//
// g2o (General Graph Optimization):
//   BA를 그래프 최적화로 모델링하는 C++ 라이브러리.
//   ORB-SLAM, LSD-SLAM 등 주요 SLAM 시스템에서 사용.
//
//   그래프 구조:
//     Vertex(노드) = 최적화 변수 (포즈, 3D 점)
//     Edge(엣지)   = 관측/제약 조건 (재투영 오차)
//
// ★ Week 5(VO)와의 관계:
//   VO는 프레임 단위로 독립적 추정 → 오차 누적(드리프트)
//   BA는 여러 프레임을 동시에 최적화 → 드리프트 감소

#include "ba_basic.h"
#include <iostream>
#include <iomanip>
#include <random>

// ═══════════════════════════════════════════════════════════════
// 구현부
// ═══════════════════════════════════════════════════════════════

// 카메라 포즈 Vertex 생성 — g2o 그래프에 SE(3) 노드 추가
//
// VertexSE3Expmap:
//   SE(3)의 지수 사상(exponential map) 표현.
//   내부적으로 SE3Quat(쿼터니언 + 이동) 사용 → 회전의 특이점 없음.
//   6 DoF: 3(회전) + 3(이동)
//
// setFixed(true):
//   이 Vertex를 최적화에서 제외 (고정).
//   ★ 반드시 최소 1개의 포즈를 고정해야 함!
//     → Gauge Freedom: BA 해는 7자유도(3회전+3이동+1스케일)만큼 불확정
//     → 첫 포즈를 고정하면 6자유도(회전+이동)가 결정됨
//     → 스케일은 3D 점 간 거리로 암시적으로 결정
void BABasic::create_vertex_pose(
    g2o::SparseOptimizer& optimizer,
    int id,
    const Eigen::Matrix3d& R,
    const Eigen::Vector3d& t,
    bool fixed)
{
    auto* v = new g2o::VertexSE3Expmap();
    v->setId(id);
    // SE3Quat(R, t): 회전 행렬 R + 이동 벡터 t → SE(3) 변환
    v->setEstimate(g2o::SE3Quat(R, t));
    v->setFixed(fixed);
    optimizer.addVertex(v);
}

// 3D 점 Vertex 생성 — g2o 그래프에 XYZ 노드 추가
//
// VertexPointXYZ:
//   3D 공간의 점 좌표 (X, Y, Z). 3 DoF.
//
// setMarginalized(true):
//   Schur Complement 최적화 적용 대상으로 지정.
//   ★ BA에서 가장 중요한 최적화 기법!
//     전체 Hessian: [Hcc Hcp; Hpc Hpp] → 점 변수(Hpp)를 먼저 소거
//     → 카메라 변수만으로 축소된 시스템 풀기 → 역대입으로 점 복구
//     → 계산량: O((m+n)³) → O(m³)  (m=카메라 수, n=점 수, n≫m)
//
//   💡 이 한 줄의 설정으로 g2o가 내부적으로 Schur Complement를 적용!
//     Week 9에서 Schur의 수학적 원리를 직접 구현함.
void BABasic::create_vertex_point(
    g2o::SparseOptimizer& optimizer,
    int id,
    const Eigen::Vector3d& point,
    bool marginalized)
{
    auto* v = new g2o::VertexPointXYZ();
    v->setId(id);
    v->setEstimate(point);
    v->setMarginalized(marginalized);   // Schur Complement 적용
    optimizer.addVertex(v);
}

// 재투영 오차 Edge 생성 — 관측(2D)과 모델 예측의 차이
//
// EdgeProjectXYZ2UV:
//   하나의 관측을 나타내는 이항(binary) 엣지.
//   연결: VertexPointXYZ (3D 점) ←→ VertexSE3Expmap (카메라 포즈)
//   관측값: 2D 픽셀 좌표 (u, v)
//   오차: ||관측 - π(T, P)||² = 재투영 오차
//
// Information Matrix (Σ⁻¹):
//   관측의 신뢰도를 나타내는 역공분산 행렬.
//   Identity = 모든 관측이 동일한 신뢰도 (σ = 1 pixel)
//   실제로는 특징점 검출 품질에 따라 가중치를 다르게 설정 가능.
//
// Robust Kernel (Huber Loss):
//   outlier(잘못된 매칭)가 최적화를 망가뜨리는 것을 방지.
//   Huber Loss 함수:
//     e < δ → L(e) = 0.5 · e²      (inlier: 일반 L2 손실)
//     e ≥ δ → L(e) = δ · (|e| - 0.5δ)  (outlier: L1로 감쇠)
//   → 큰 오차의 영향을 선형으로 제한하여 outlier 강건성 확보
//   ★ δ = 1.0: 1px 이상의 오차를 outlier로 간주하여 영향 감쇠
void BABasic::create_edge(
    g2o::SparseOptimizer& optimizer,
    int pose_id, int point_id,
    const Eigen::Vector2d& observation,
    const Eigen::Vector4d& K,
    bool use_robust)
{
    auto* e = new g2o::EdgeProjectXYZ2UV();
    e->setVertex(0, optimizer.vertex(point_id));    // 0번 = 3D 점
    e->setVertex(1, optimizer.vertex(pose_id));     // 1번 = 카메라 포즈
    e->setMeasurement(observation);                 // 관측된 2D 좌표
    e->setInformation(Eigen::Matrix2d::Identity()); // 정보 행렬 (σ=1px)

    // 카메라 내부 파라미터 — 투영 함수 π()에 필요
    //   CameraParameters(fx, principal_point, baseline)
    //   K(0)=fx, K(2)=cx, K(3)=cy, baseline=0 (단안)
    //   addParameter가 false를 반환하면 이미 등록된 것 → 해제
    auto* cam = new g2o::CameraParameters(K(0), Eigen::Vector2d(K(2), K(3)), 0);
    cam->setId(0);
    if (!optimizer.addParameter(cam))
    {
        delete cam;
    }
    e->setParameterId(0, 0);

    // Robust Kernel — outlier 방어
    if (use_robust)
    {
        auto* rk = new g2o::RobustKernelHuber();
        rk->setDelta(1.0);     // δ = 1px — 이 이상의 오차는 L1으로 감쇠
        e->setRobustKernel(rk);
    }

    optimizer.addEdge(e);
}

// 옵티마이저 설정 — BlockSolver_6_3 + Levenberg-Marquardt
//
// BlockSolver_6_3:
//   BA 전용 블록 구조. "6" = 포즈 차원(SE3), "3" = 점 차원(XYZ).
//   이 구조를 알려주면 g2o가 Schur Complement를 효율적으로 적용 가능.
//
// LinearSolverEigen:
//   Hessian 선형 시스템(H·Δx = b)을 Eigen 라이브러리로 풀이.
//   소규모 BA에 적합. 대규모에서는 Cholmod, CSparse 등 사용.
//
// Levenberg-Marquardt (LM):
//   비선형 최적화 알고리즘. Gauss-Newton + 정규화(damping) 결합.
//   GN: 빠르지만 발산 위험 → LM: 안정적이지만 약간 느림
//   λ가 크면 → Gradient Descent처럼 (안전하지만 느림)
//   λ가 작으면 → Gauss-Newton처럼 (빠르지만 발산 가능)
//   ★ BA의 표준 알고리즘 — 거의 모든 SLAM 시스템이 LM 사용
void BABasic::setup_optimizer(g2o::SparseOptimizer& optimizer)
{
    using BlockSolverType = g2o::BlockSolver_6_3;
    using LinearSolverType = g2o::LinearSolverEigen<BlockSolverType::PoseMatrixType>;

    auto solver = new g2o::OptimizationAlgorithmLevenberg(
        std::make_unique<BlockSolverType>(
            std::make_unique<LinearSolverType>()));

    optimizer.setAlgorithm(solver);
    optimizer.setVerbose(false);    // true로 바꾸면 매 반복 오차 출력
}

// 최적화 실행 — 반복적으로 오차를 줄여나감
//
// 내부 동작 (매 반복):
//   1. 현재 추정값으로 재투영 오차 계산
//   2. Jacobian 행렬 계산 (각 Edge가 자동으로)
//   3. Hessian 구축: H = Σ J_i^T · Ω_i · J_i,  b = Σ J_i^T · Ω_i · e_i
//   4. Schur Complement로 축소 (marginalized 변수 소거)
//   5. 선형 시스템 H·Δx = b 풀기
//   6. 변수 업데이트: x ← x ⊕ Δx (SE3에서는 exp map으로 업데이트)
//   7. 수렴 체크: |ΔΔ| < ε이면 종료
//
// activeChi2(): 현재 총 오차 (χ² = Σ e_i^T · Ω_i · e_i)
//   ★ chi2가 감소하면 최적화가 진행되고 있다는 의미
BABasic::BAResult BABasic::optimize(g2o::SparseOptimizer& optimizer, int iterations)
{
    BAResult result;
    result.num_poses = 0;
    result.num_points = 0;
    result.num_edges = static_cast<int>(optimizer.edges().size());

    // Vertex 종류별 개수 세기
    for (auto* v : optimizer.vertices())
    {
        if (dynamic_cast<g2o::VertexSE3Expmap*>(v.second))
            result.num_poses++;
        else if (dynamic_cast<g2o::VertexPointXYZ*>(v.second))
            result.num_points++;
    }

    optimizer.initializeOptimization();
    result.initial_error = optimizer.activeChi2();      // 최적화 전 총 오차
    result.iterations = optimizer.optimize(iterations);  // LM 반복 실행
    result.final_error = optimizer.activeChi2();         // 최적화 후 총 오차

    return result;
}

// 결과 평가 및 출력 — 최적화 전후 오차 비교
//
// 개선율 = (1 - final/initial) × 100%
//   > 90%: 매우 효과적인 최적화
//   50~90%: 양호
//   < 50%: 초기값이 이미 좋거나, 최적화가 제대로 안 됨
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
//
// 테스트용 3D 장면 + 카메라 + 관측 생성:
//   1. GT 포즈: 전진 궤적 (0.5m 간격)
//   2. GT 3D 점: 카메라 전방에 랜덤 분포
//   3. 관측: GT 투영 + 가우시안 노이즈
//
// noise_px: 관측 노이즈 (σ=1.0px이 전형적인 서브픽셀 검출 오차)
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

    // GT 포즈 — +Z 방향 전진 (0.5m 간격)
    //   모든 카메라가 같은 방향을 보고 있음 (R = I)
    for (int i = 0; i < num_poses; i++)
    {
        poses_R.push_back(Eigen::Matrix3d::Identity());
        poses_t.push_back(Eigen::Vector3d(0, 0, i * 0.5));
    }

    // GT 3D 점 — 카메라 전방에 랜덤 분포
    //   X: [-3, 3], Y: [-2, 2], Z: [3, 15] — 전방 3~15m
    std::uniform_real_distribution<double> dist_x(-3, 3);
    std::uniform_real_distribution<double> dist_y(-2, 2);
    std::uniform_real_distribution<double> dist_z(3, 15);
    for (int j = 0; j < num_points; j++)
    {
        points.push_back(Eigen::Vector3d(dist_x(rng), dist_y(rng), dist_z(rng)));
    }

    // 관측 생성 — 모든 (카메라, 점) 쌍에 대해 투영
    //   Pc = R · (Pw - t) → u = fx·X/Z + cx + noise
    //   이미지 범위 내에 있는 관측만 저장
    for (int i = 0; i < num_poses; i++)
    {
        for (int j = 0; j < num_points; j++)
        {
            Eigen::Vector3d Pc = poses_R[i] * (points[j] - poses_t[i]);
            if (Pc(2) < 0.1)   // 카메라 뒤에 있는 점은 무시
                continue;

            double u = fx * Pc(0) / Pc(2) + cx + noise(rng);
            double v = fy * Pc(1) / Pc(2) + cy + noise(rng);

            // 이미지 범위 체크 (0 < u < 640, 0 < v < 480)
            if (u > 0 && u < 2 * cx && v > 0 && v < 2 * cy)
            {
                observations.push_back({i, j, Eigen::Vector2d(u, v)});
            }
        }
    }
}

// ── 블록 1: BA 기본 구조 — Vertex + Edge 그래프 최적화 ──
//
// g2o의 핵심 패턴:
//   1. Optimizer 설정 (BlockSolver + 알고리즘)
//   2. Vertex 추가 (포즈 + 3D 점)
//   3. Edge 추가 (관측 = 재투영 오차)
//   4. optimize() 실행
//   5. 결과 추출
//
// 💡 quiz_easy Q1: 재투영 오차 공식
// 💡 quiz_easy Q2: Vertex의 종류와 DoF
// 💡 quiz_easy Q3: Edge의 의미
static void demoBAStructure()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 1] BA = Vertex + Edge 그래프 최적화" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // BA 목적 함수: 재투영 오차의 총합 최소화
    //   min Σ ||z_ij - π(T_i, P_j)||²
    //
    //   이것을 g2o 그래프로 표현:
    //     VertexSE3Expmap = T_i (카메라 포즈, 6 DoF)
    //     VertexPointXYZ  = P_j (3D 점, 3 DoF)
    //     EdgeProjectXYZ2UV = z_ij - π(T_i, P_j) (재투영 오차)
    std::cout << "BA 목적 함수:" << std::endl;
    std::cout << "  min Σ ||z_ij - π(T_i, P_j)||²" << std::endl;
    std::cout << "  z_ij: 관측 (2D), T_i: 포즈, P_j: 3D 점\n" << std::endl;

    std::cout << "g2o 구조:" << std::endl;
    std::cout << "  Vertex = 최적화 변수" << std::endl;
    std::cout << "    VertexSE3Expmap: 카메라 포즈 (6 DoF)" << std::endl;
    std::cout << "    VertexPointXYZ:  3D 점 (3 DoF)" << std::endl;
    std::cout << "  Edge = 관측/제약" << std::endl;
    std::cout << "    EdgeProjectXYZ2UV: 재투영 오차\n" << std::endl;

    // ── 수치 예시: 4 포즈 + 20 점으로 간단한 BA ──
    Eigen::Vector4d K(500, 500, 320, 240);  // (fx, fy, cx, cy)

    std::vector<Eigen::Matrix3d> gt_R;
    std::vector<Eigen::Vector3d> gt_t, gt_pts;
    std::vector<std::tuple<int, int, Eigen::Vector2d>> obs;
    generate_ba_data(4, 20, 1.0, gt_R, gt_t, gt_pts, obs, K);

    // 노이즈가 추가된 초기값 — BA가 이 노이즈를 줄이는 것이 목표
    //   포즈: σ_t = 0.1m (이동에 노이즈)
    //   3D 점: σ_pt = 0.3m (위치에 노이즈)
    //   ★ 초기값이 GT에서 너무 멀면 BA가 수렴하지 않을 수 있음 (local minimum)
    std::mt19937 rng(123);
    std::normal_distribution<double> noise_pt(0.0, 0.3);
    std::normal_distribution<double> noise_t(0.0, 0.1);

    g2o::SparseOptimizer optimizer;
    BABasic::setup_optimizer(optimizer);

    // 포즈 Vertex — 첫 포즈(i=0)는 고정 (Gauge Freedom 해결)
    for (int i = 0; i < 4; i++)
    {
        Eigen::Vector3d t_noisy = gt_t[i];
        if (i > 0)  // 첫 포즈는 정확한 값 (기준점)
        {
            t_noisy += Eigen::Vector3d(noise_t(rng), noise_t(rng), noise_t(rng));
        }
        BABasic::create_vertex_pose(optimizer, i, gt_R[i], t_noisy, i == 0);
    }

    // 3D 점 Vertex — 노이즈 추가 + marginalized(Schur 적용)
    const int kPointIdOffset = 100;  // 포즈 ID(0~3)와 겹치지 않도록 오프셋
    for (int j = 0; j < 20; j++)
    {
        Eigen::Vector3d pt_noisy = gt_pts[j] +
            Eigen::Vector3d(noise_pt(rng), noise_pt(rng), noise_pt(rng));
        BABasic::create_vertex_point(optimizer, kPointIdOffset + j, pt_noisy);
    }

    // Edge — 각 관측을 그래프에 추가
    for (const auto& [pose_id, point_id, pixel] : obs)
    {
        BABasic::create_edge(optimizer, pose_id, kPointIdOffset + point_id, pixel, K);
    }

    // 최적화 실행 (10회 반복)
    //   관찰: initial_error → final_error 로 오차가 감소하는지 확인
    auto result = BABasic::optimize(optimizer, 10);
    BABasic::evaluate_result(optimizer, result, "4포즈+20점");
}

// ── 블록 2: Robust Kernel — Outlier에 대한 방어 ──
//
// 현실에서는 관측 중 일부가 잘못된 매칭(outlier).
// 일반 L2 손실은 outlier의 큰 오차에 지배당해 전체 최적화가 망가짐.
// → Huber Robust Kernel로 outlier 영향을 제한.
//
// 실험:
//   같은 BA 문제에 20% outlier를 추가하고,
//   Robust Kernel 유무에 따른 최적화 결과를 비교.
//
// 💡 quiz_easy Q4: Huber Loss 함수의 수식과 의미
static void demoRobustKernel()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 2] Huber Robust Kernel — Outlier 제거" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // Huber Loss:
    //   |e| < δ → 0.5 · e²           (inlier: 일반 제곱 손실)
    //   |e| ≥ δ → δ · (|e| - 0.5δ)   (outlier: 선형으로 감쇠)
    //
    // ★ 효과: outlier의 오차가 100px여도 영향은 ~δ·100 (선형)
    //   L2 손실이면 100² = 10000 → 최적화를 완전히 지배
    //   Huber면 ~100 → 영향이 제한됨
    std::cout << "Huber Loss:" << std::endl;
    std::cout << "  e < δ → 0.5 * e²  (inlier: 일반 L2)" << std::endl;
    std::cout << "  e ≥ δ → δ(|e| - 0.5δ)  (outlier: L1로 감쇠)\n" << std::endl;

    Eigen::Vector4d K(500, 500, 320, 240);

    std::vector<Eigen::Matrix3d> gt_R;
    std::vector<Eigen::Vector3d> gt_t, gt_pts;
    std::vector<std::tuple<int, int, Eigen::Vector2d>> obs;
    generate_ba_data(4, 20, 1.0, gt_R, gt_t, gt_pts, obs, K);

    // 20%의 관측을 랜덤 위치로 대체 → outlier 시뮬레이션
    //   실제 SLAM에서 outlier 원인: 잘못된 특징점 매칭, 동적 물체, 반사광
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

    // 실험 1: Robust Kernel 없이 — outlier가 최적화를 망가뜨림
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
            BABasic::create_edge(opt, pid, kOffset + ptid, px, K, false);  // no robust

        auto res = BABasic::optimize(opt, 10);
        BABasic::evaluate_result(opt, res, "Robust 없음 (20% outlier)");
    }

    // 실험 2: Huber Robust Kernel 사용 — outlier 영향 제한
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
            BABasic::create_edge(opt, pid, kOffset + ptid, px, K, true);   // use robust

        auto res = BABasic::optimize(opt, 10);
        BABasic::evaluate_result(opt, res, "Huber Robust (20% outlier)");
    }
}

// ── 블록 3: Gauge Freedom — 기준 프레임 고정 ──
//
// BA에서 가장 흔한 실수: 어떤 포즈도 고정하지 않아서 해가 불확정.
//
// Gauge Freedom (게이지 자유도):
//   BA의 해는 전체 좌표계를 이동/회전/스케일링해도 변하지 않음.
//   → 무한히 많은 해가 존재 → Hessian이 특이(singular)
//   → 7 DoF(3 이동 + 3 회전 + 1 스케일)만큼 자유도가 과잉
//
// 해결: 첫 포즈를 setFixed(true)로 고정 → 6 DoF 제거
//   스케일은 3D 점 간 거리로 암시적으로 결정됨
//   또는 첫 두 포즈 사이 거리를 1로 고정하여 스케일도 결정
//
// 💡 quiz_medium Q3: setFixed(true)의 역할
// 💡 quiz_easy Q5: Schur Complement 개요
static void demoGaugeFreedom()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 3] Gauge Freedom — 기준 프레임 고정" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    std::cout << "왜 첫 포즈를 고정하는가?" << std::endl;
    std::cout << "  BA 해는 7자유도만큼 불확정 (Gauge Freedom)" << std::endl;
    std::cout << "  = 3 이동 + 3 회전 + 1 스케일" << std::endl;
    std::cout << "  → 첫 포즈 고정으로 6자유도 제거\n" << std::endl;

    // Schur Complement + Marginalization 설명
    //   setMarginalized(true): g2o에게 이 변수는 Schur로 소거하라고 지시
    //   BlockSolver_6_3: 포즈(6 DoF) + 점(3 DoF) 구조를 g2o에게 알림
    //
    // ★ 실제 효과:
    //   BA에 100개 포즈, 10000개 점이 있을 때
    //   전체 시스템: (600 + 30000)² = 30600² 원소
    //   Schur 후: 600² = 360000 원소 → ~2600배 감소!
    std::cout << "setMarginalized(true) — 3D 점에 적용:" << std::endl;
    std::cout << "  Schur Complement로 포즈만 먼저 최적화" << std::endl;
    std::cout << "  BlockSolver_6_3: 포즈(6) + 점(3)" << std::endl;
    std::cout << "  결과: 포즈 수에 비례하는 속도 (O(n³) → O(m³))" << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// 전체 데모 + main
// ═══════════════════════════════════════════════════════════════

// 전체 Week 7 데모 실행
//   블록 1: BA 기본 구조 (Vertex + Edge + Optimize)
//   블록 2: Robust Kernel (Huber Loss로 outlier 방어)
//   블록 3: Gauge Freedom (기준 프레임 고정의 필요성)
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

    // g2o BA의 5단계 핵심 패턴:
    //   1. setup_optimizer: BlockSolver_6_3 + LM 알고리즘 설정
    //   2. create_vertex_pose: 카메라 포즈 추가 (첫 포즈 fixed)
    //   3. create_vertex_point: 3D 점 추가 (marginalized로 Schur 적용)
    //   4. create_edge: 관측 추가 (재투영 오차 + 선택적 Huber)
    //   5. optimize: LM 반복으로 오차 최소화
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
