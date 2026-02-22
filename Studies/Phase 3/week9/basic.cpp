// Week 9: Schur Complement — BA의 효율적 풀이법
//
// 이 파일은 BA의 Hessian 구조와 Schur Complement를 직접 구현하여
// "왜 BA가 빠르게 풀리는가"의 수학적 원리를 시연한다.
//
// BA의 계산 병목:
//   BA는 비선형 최적화 → 매 반복마다 선형 시스템 H·Δx = b를 풀어야 함.
//   변수 수: m개 카메라(6DoF) + n개 점(3DoF) → 총 (6m+3n)차원
//   직접 풀기: O((6m+3n)³) — 점이 10000개면 30000³ ≈ 불가능
//
// Schur Complement의 핵심 아이디어:
//   "3D 점 변수(많음)를 먼저 소거하고, 카메라 변수(적음)만 풀자!"
//
//   원래 시스템:
//     [Hcc  Hcp] [Δc]   [bc]
//     [Hpc  Hpp] [Δp] = [bp]
//
//   Schur Complement (Hpp 소거):
//     (Hcc - Hcp · Hpp⁻¹ · Hpc) · Δc = bc - Hcp · Hpp⁻¹ · bp
//     → (6m×6m) 시스템만 풀면 됨!
//
//   역대입(Back-substitution):
//     Δp = Hpp⁻¹ · (bp - Hpc · Δc)
//     → 카메라 해(Δc)를 알면 점 해(Δp)는 간단하게 복구
//
// ★ Hpp가 블록 대각인 이유:
//   각 관측은 1개 카메라와 1개 점만 연결.
//   점 i와 점 j는 직접 연결되지 않음 → Hpp의 (i,j) 블록 = 0 (i≠j)
//   → Hpp⁻¹ = 각 3×3 블록의 역행렬만 구하면 됨 → O(n)
//
// 계산량 비교:
//   직접: O((6m+3n)³) ← 점 수에 지배당함
//   Schur: O((6m)³) + O(n) ← 카메라 수에만 의존
//   예: m=100, n=10000
//     직접: (600+30000)³ ≈ 2.9 × 10¹³
//     Schur: 600³ ≈ 2.2 × 10⁸ → ~130000배 빠름!
//
// 실제 시스템에서의 적용:
//   g2o: setMarginalized(true) → 내부적으로 Schur 적용
//   Ceres: DENSE_SCHUR / SPARSE_SCHUR → Solver가 Schur 적용

#include "schur_basic.h"
#include <iostream>
#include <random>
#include <chrono>

// ═══════════════════════════════════════════════════════════════
// SchurBasic 구현
// ═══════════════════════════════════════════════════════════════

// BA Hessian 구성 — 관측으로부터 H = Σ J^T·J, b = Σ J^T·r 계산
//
// 정규 방정식(Normal Equation):
//   Gauss-Newton에서 매 반복 풀어야 하는 선형 시스템:
//   H · Δx = b
//   H = Σ J_i^T · Ω_i · J_i  (Hessian 근사, Ω = 정보 행렬)
//   b = Σ J_i^T · Ω_i · r_i  (음의 그래디언트)
//
// Jacobian J의 희소 구조:
//   각 관측(카메라 i, 점 j)에 대한 Jacobian은 2×(6m+3n) 행렬이지만,
//   실제로 0이 아닌 부분은 카메라 i 블록(2×6)과 점 j 블록(2×3)뿐.
//   나머지는 모두 0!
//
//   J_ij = [0 ... J_cam_i ... 0 ... J_pt_j ... 0]
//            ↑ 카메라 i 위치    ↑ 점 j 위치
//
//   ★ 이 희소성이 Schur Complement를 가능하게 하는 핵심!
//
// 💡 이 구현에서는 Jacobian을 랜덤 시뮬레이션. 실제 BA에서는
//    투영 함수 π()를 미분하여 정확한 Jacobian을 계산.
HessianBlocks SchurBasic::build_hessian(int num_cameras, int num_points,
                                        const std::vector<std::pair<int, int>>& observations)
{
    // 변수 차원: 카메라 6DoF × 개수, 점 3DoF × 개수
    int cam_dim = num_cameras * 6;
    int pt_dim = num_points * 3;
    int total = cam_dim + pt_dim;

    // H와 b를 0으로 초기화
    Eigen::MatrixXd H = Eigen::MatrixXd::Zero(total, total);
    Eigen::VectorXd b = Eigen::VectorXd::Zero(total);

    std::mt19937 gen(42);
    std::normal_distribution<> noise(0, 1);

    // 각 관측에 대해 Jacobian J를 구성하고 H += J^T·J, b += J^T·r
    for (const auto& [cam_idx, pt_idx] : observations)
    {
        // 2×(total) Jacobian — 대부분 0, 카메라/점 블록만 비영
        Eigen::MatrixXd J = Eigen::MatrixXd::Zero(2, total);

        int cam_start = cam_idx * 6;           // 카메라 블록 시작 인덱스
        int pt_start = cam_dim + pt_idx * 3;   // 점 블록 시작 인덱스

        // 카메라 부분 Jacobian: 2×6 (재투영 오차의 카메라 파라미터에 대한 미분)
        //   실제: ∂π/∂(R,t) — 투영 함수를 SE(3) 파라미터로 미분
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                J(i, cam_start + j) = noise(gen);
            }
        }
        // 점 부분 Jacobian: 2×3 (재투영 오차의 3D 점 좌표에 대한 미분)
        //   실제: ∂π/∂P = [fx/Z  0  -fx·X/Z²; 0  fy/Z  -fy·Y/Z²] · R
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                J(i, pt_start + j) = noise(gen);
            }
        }

        // 잔차(residual) 시뮬레이션: 재투영 오차 (2D)
        Eigen::Vector2d r(noise(gen) * 0.1, noise(gen) * 0.1);

        // 정규 방정식 누적:
        //   H += J^T · J  (Hessian 근사, GN에서 이차 도함수 생략)
        //   b += J^T · r  (음의 그래디언트)
        H += J.transpose() * J;
        b += J.transpose() * r;
    }

    // LM damping: 대각에 작은 값 추가 → 정칙화(regularization)
    //   Hessian이 ill-conditioned일 때 안정적으로 풀 수 있게 함
    //   λ = 1e-3: 작은 damping (Gauss-Newton에 가까움)
    for (int i = 0; i < total; i++)
    {
        H(i, i) += 1e-3;
    }

    return extract_blocks(H, b, cam_dim, pt_dim);
}

// Hessian에서 4개 블록 추출
//
// 전체 Hessian의 구조:
//   H = [Hcc  Hcp]   b = [bc]
//       [Hpc  Hpp]       [bp]
//
//   Hcc (cam_dim × cam_dim): 카메라-카메라 블록 — 밀집(dense), 작은 크기
//   Hcp (cam_dim × pt_dim):  카메라-점 블록 — 희소(sparse)
//   Hpp (pt_dim × pt_dim):   점-점 블록 — 블록 대각(block-diagonal)!
//     → 점 i와 점 j가 직접 연결되지 않으므로 (i,j) 블록은 0
//   bc (cam_dim × 1): 카메라 잔차
//   bp (pt_dim × 1): 점 잔차
HessianBlocks SchurBasic::extract_blocks(const Eigen::MatrixXd& H, const Eigen::VectorXd& b,
                                         int cam_dim, int pt_dim)
{
    HessianBlocks blocks;
    blocks.Hcc = H.block(0, 0, cam_dim, cam_dim);              // 좌상
    blocks.Hcp = H.block(0, cam_dim, cam_dim, pt_dim);          // 우상
    blocks.Hpp = H.block(cam_dim, cam_dim, pt_dim, pt_dim);     // 우하
    blocks.bc = b.segment(0, cam_dim);                           // 상
    blocks.bp = b.segment(cam_dim, pt_dim);                      // 하
    return blocks;
}

// Schur Complement 계산 — 점 변수를 소거하여 축소된 시스템 생성
//
// 수학적 유도:
//   원래 시스템: [Hcc Hcp; Hpc Hpp] [Δc; Δp] = [bc; bp]
//
//   2번째 행에서: Hpp·Δp = bp - Hpc·Δc
//     → Δp = Hpp⁻¹·(bp - Hpc·Δc)  ... (역대입)
//
//   1번째 행에 대입:
//     Hcc·Δc + Hcp·Hpp⁻¹·(bp - Hpc·Δc) = bc
//     (Hcc - Hcp·Hpp⁻¹·Hpc)·Δc = bc - Hcp·Hpp⁻¹·bp
//
//   정리:
//     H_reduced = Hcc - Hcp·Hpp⁻¹·Hpc  (Schur Complement)
//     b_reduced = bc - Hcp·Hpp⁻¹·bp
//
// ★ 핵심: Hpp⁻¹ 계산이 쉬움!
//   Hpp는 블록 대각 → 각 3×3 블록만 역행렬 → O(n) 시간
//   (전체 Hpp를 역행렬하면 O(n³)이지만, 블록 대각 구조 덕분에 O(n))
//
// 💡 이 교육용 구현에서는 Hpp.inverse()를 직접 계산 (밀집 역행렬).
//   실제 구현에서는 블록 대각 구조를 활용하여 3×3씩 역행렬을 계산.
void SchurBasic::schur_complement(const HessianBlocks& blocks,
                                  Eigen::MatrixXd& H_reduced, Eigen::VectorXd& b_reduced)
{
    // Hpp의 역행렬 — 실제로는 블록 대각이므로 3×3씩 역행렬
    Eigen::MatrixXd Hpp_inv = blocks.Hpp.inverse();

    // Schur Complement:
    //   H_reduced = Hcc - Hcp · Hpp⁻¹ · Hcp^T
    //   b_reduced = bc  - Hcp · Hpp⁻¹ · bp
    //
    //   H_reduced 크기: (6m × 6m) — 카메라 변수만!
    //   원래 (6m+3n × 6m+3n)에서 (6m × 6m)으로 축소됨
    H_reduced = blocks.Hcc - blocks.Hcp * Hpp_inv * blocks.Hcp.transpose();
    b_reduced = blocks.bc - blocks.Hcp * Hpp_inv * blocks.bp;
}

// 축소된 시스템 풀기 — 카메라 변수만으로 구성된 선형 시스템
//
// H_reduced · Δc = b_reduced
//
// LDLT 분해: 대칭 양정치 행렬에 적합한 분해
//   H = L·D·L^T → 안정적이고 효율적
//   Cholesky(LL^T)보다 수치적으로 안정 (D가 음수일 수 있으므로)
Eigen::VectorXd SchurBasic::solve_reduced(const Eigen::MatrixXd& H_reduced,
                                          const Eigen::VectorXd& b_reduced)
{
    return H_reduced.ldlt().solve(b_reduced);
}

// 역대입(Back-substitution) — 카메라 해로부터 점 해 복구
//
// Δp = Hpp⁻¹ · (bp - Hpc · Δc)
//
// 카메라 해(Δc)가 구해지면, 점 해(Δp)는 단순 행렬 연산으로 복구.
// Hpp⁻¹는 블록 대각이므로 O(n) 시간.
//
// ★ 이것이 Schur의 2단계:
//   1단계: 축소 시스템 → Δc 구하기 (비쌈: O(m³))
//   2단계: 역대입 → Δp 구하기 (싼: O(n))
Eigen::VectorXd SchurBasic::back_substitute(const HessianBlocks& blocks,
                                            const Eigen::VectorXd& delta_c)
{
    Eigen::MatrixXd Hpp_inv = blocks.Hpp.inverse();
    return Hpp_inv * (blocks.bp - blocks.Hcp.transpose() * delta_c);
}

// ═══════════════════════════════════════════════════════════════
// 교육 블록
// ═══════════════════════════════════════════════════════════════

// ── 교육 블록 1: BA Hessian의 희소 구조 ──
//
// BA의 Hessian이 왜 희소한가?
// → 각 관측이 1개 카메라 + 1개 점에만 연결되기 때문!
//
// 비유:
//   관측 = "카메라 i가 점 j를 봤다"
//   Jacobian J_ij: 카메라 i 블록과 점 j 블록만 비영 (나머지 0)
//   H += J^T·J → 카메라 i-i, 카메라 i-점 j, 점 j-j 블록에만 누적
//   → 점 i와 점 j는 직접 연결 없음 → Hpp의 (i,j) 블록 = 0
//
// ★ Hpp가 블록 대각인 핵심 이유:
//   점 i와 점 j를 동시에 관측하는 edge가 없음
//   (관측은 항상 1개 카메라 + 1개 점)
//   → H에서 점 i-점 j 교차항이 0
//   → Hpp는 블록 대각 → 역행렬이 O(n)
//
// 💡 quiz_easy Q1: Jacobian이 희소한 이유
// 💡 quiz_easy Q4: Hpp가 블록 대각인 이유
void demoSparseStructure()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 BA Hessian의 희소 구조" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "핵심: 각 관측은 1개 카메라 + 1개 점에만 연결!\n" << std::endl;

    // Hessian 블록 구조:
    //   [Hcc  Hcp]   c = 카메라 변수, p = 점 변수
    //   [Hpc  Hpp]
    //
    //   Hcc: 카메라끼리의 관계 → 같은 점을 관측하는 카메라 쌍 → 밀집
    //   Hcp: 카메라-점 관계 → 관측이 있는 쌍만 비영 → 희소
    //   Hpp: 점끼리의 관계 → 직접 연결 없음 → 블록 대각!
    std::cout << "Hessian 구조:" << std::endl;
    std::cout << "  H = [Hcc  Hcp]" << std::endl;
    std::cout << "      [Hpc  Hpp]\n" << std::endl;

    std::cout << "  Hcc: 카메라-카메라 (작고 밀집)" << std::endl;
    std::cout << "  Hcp: 카메라-점 (희소)" << std::endl;
    std::cout << "  Hpp: 점-점 (블록 대각!)\n" << std::endl;

    std::cout << "💡 Hpp가 블록 대각인 이유:" << std::endl;
    std::cout << "   점 i와 점 j는 서로 직접 연결 없음" << std::endl;
    std::cout << "   → Hpp의 (i,j) 블록 = 0 (i≠j)" << std::endl;

    // ── 수치 시연: 3 카메라 + 5 점 + 9 관측 ──
    //
    // 관측 구조:
    //   카메라 0 → 점 0, 1, 2
    //   카메라 1 → 점 1, 2, 3
    //   카메라 2 → 점 2, 3, 4
    //   → 점 0과 점 4는 서로 다른 카메라에서만 관측 → Hpp(0,4) = 0
    int num_cam = 3, num_pt = 5;
    std::vector<std::pair<int, int>> obs = {
        {0, 0}, {0, 1}, {0, 2},
        {1, 1}, {1, 2}, {1, 3},
        {2, 2}, {2, 3}, {2, 4}
    };

    auto blocks = SchurBasic::build_hessian(num_cam, num_pt, obs);

    std::cout << "\n예시: 3 카메라, 5 점, 9 관측" << std::endl;
    std::cout << "  Hcc 크기: " << blocks.Hcc.rows() << "x" << blocks.Hcc.cols()
              << " (18x18)" << std::endl;
    std::cout << "  Hpp 크기: " << blocks.Hpp.rows() << "x" << blocks.Hpp.cols()
              << " (15x15)" << std::endl;

    // Hpp의 비대각 블록 norm 확인 — 0에 가까워야 블록 대각
    //   ★ LM damping으로 대각에만 값이 추가되므로 비대각 블록은 정확히 0은 아닐 수 있음
    //   하지만 Jacobian 희소성에 의해 비대각 블록은 매우 작아야 함
    double off_diag_norm = 0;
    for (int i = 0; i < num_pt; i++)
    {
        for (int j = 0; j < num_pt; j++)
        {
            if (i != j)
            {
                off_diag_norm += blocks.Hpp.block(i * 3, j * 3, 3, 3).norm();
            }
        }
    }
    std::cout << "  Hpp 비대각 블록 norm: " << off_diag_norm
              << (off_diag_norm < 1e-6 ? " ≈ 0 (블록 대각!)" : " > 0") << std::endl;
}

// ── 교육 블록 2: Schur Complement 시연 ──
//
// 직접 풀기 vs Schur Complement의 결과가 동일함을 수치로 검증.
//
// 직접 풀기:
//   (6m+3n)×(6m+3n) 전체 시스템을 LDLT로 풀기
//   → 이 예시에서는 33×33 (작으므로 가능)
//
// Schur 풀기:
//   1단계: Schur → 18×18 축소 시스템 풀기
//   2단계: 역대입 → 15개 점 변수 복구
//   → 결과가 직접 풀기와 동일!
//
// 💡 quiz_easy Q2: Schur에서 먼저 소거하는 변수는?
// 💡 quiz_medium Q1: Schur가 계산량을 줄이는 원리
void demoSchurComplement()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Schur Complement 시연" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 전략: 점 변수(많음, 3n)를 먼저 소거!
    //   원래: 33×33 풀기 (18 카메라 + 15 점)
    //   Schur: 18×18 풀기 → 역대입으로 15개 복구
    //   → 시스템 크기 33 → 18로 축소 (45% 감소)
    std::cout << "전략: 점 변수(많음)를 먼저 소거!\n" << std::endl;
    std::cout << "  원래: (18+15)x(18+15) = 33x33 풀기" << std::endl;
    std::cout << "  Schur: 18x18 풀기 → 역대입으로 15개 복구\n" << std::endl;

    int num_cam = 3, num_pt = 5;
    std::vector<std::pair<int, int>> obs = {
        {0, 0}, {0, 1}, {0, 2},
        {1, 1}, {1, 2}, {1, 3},
        {2, 2}, {2, 3}, {2, 4}
    };

    auto blocks = SchurBasic::build_hessian(num_cam, num_pt, obs);

    // ── 방법 1: 전체 시스템 직접 풀기 ──
    int total = num_cam * 6 + num_pt * 3;
    Eigen::MatrixXd H_full(total, total);
    // 4개 블록을 합쳐서 전체 Hessian 재구성
    H_full << blocks.Hcc, blocks.Hcp,
              blocks.Hcp.transpose(), blocks.Hpp;
    Eigen::VectorXd b_full(total);
    b_full << blocks.bc, blocks.bp;

    auto t1 = std::chrono::high_resolution_clock::now();
    Eigen::VectorXd x_direct = H_full.ldlt().solve(b_full);
    auto t2 = std::chrono::high_resolution_clock::now();

    // ── 방법 2: Schur Complement ──
    Eigen::MatrixXd H_reduced;
    Eigen::VectorXd b_reduced;
    auto t3 = std::chrono::high_resolution_clock::now();
    // 1단계: Schur Complement로 축소 시스템 구성
    SchurBasic::schur_complement(blocks, H_reduced, b_reduced);
    // 2단계: 카메라 변수 풀기 (18×18 시스템)
    Eigen::VectorXd delta_c = SchurBasic::solve_reduced(H_reduced, b_reduced);
    // 3단계: 역대입으로 점 변수 복구
    Eigen::VectorXd delta_p = SchurBasic::back_substitute(blocks, delta_c);
    auto t4 = std::chrono::high_resolution_clock::now();

    // 결과 비교 — 두 방법의 해가 동일해야 함
    Eigen::VectorXd x_schur(total);
    x_schur << delta_c, delta_p;

    double diff = (x_direct - x_schur).norm();
    std::cout << "결과 비교:" << std::endl;
    std::cout << "  직접 풀기: " << x_direct.head(6).transpose() << " ..." << std::endl;
    std::cout << "  Schur:     " << delta_c.head(6).transpose() << " ..." << std::endl;
    std::cout << "  차이 norm: " << diff << (diff < 1e-8 ? " ✅ 동일!" : " ❌") << std::endl;

    // ★ 실제 BA에서의 효과 — 대규모 문제에서 극적인 차이
    //   100 카메라 + 10000 점:
    //     직접: (600+30000)² = 30600² ≈ 9.4억 원소 → 메모리/속도 불가
    //     Schur: 600² = 36만 원소 → 약 2600배 감소!
    //     역대입: 10000개 × 3×3 역행렬 → 매우 빠름
    std::cout << "\n💡 실제 BA에서의 효과:" << std::endl;
    std::cout << "  100 카메라 + 10000 점:" << std::endl;
    std::cout << "  직접: (600+30000)² = 30600² ≈ 9.4억 원소" << std::endl;
    std::cout << "  Schur: 600² = 36만 원소 (2600배 감소!)" << std::endl;
}

// ── 교육 블록 3: Local BA vs Global BA ──
//
// BA를 어느 범위에서 수행할 것인가?
//
// Local BA:
//   범위: 현재 Keyframe + Covisible Keyframes (Week 6)
//   장점: 실시간 가능 (10~50ms) → SLAM 추적 중 매 KF마다 실행
//   단점: 드리프트 완전 제거 불가 (먼 KF은 최적화 범위 밖)
//   ★ ORB-SLAM의 Local Mapping 스레드에서 실행
//
// Global BA:
//   범위: 전체 맵의 모든 KF + 모든 3D 점
//   장점: 전역 최적 → 드리프트 최소화
//   단점: 느림 (수 초~수 분) → Loop Closure 후에만 실행
//   ★ ORB-SLAM에서 Loop Closure가 감지된 후 별도 스레드에서 실행
//
// Sliding Window BA (VINS-Mono):
//   범위: 최근 N개 KF만 (보통 N=10~20)
//   오래된 KF은 marginalize(prior로 변환)하여 정보 보존
//   → Local BA의 변형, VIO(Visual-Inertial Odometry)에서 주로 사용
//
// 💡 quiz_easy Q3: Local BA vs Global BA 비교
// 💡 quiz_medium Q2: Local BA의 장단점
void demoLocalVsGlobal()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Local BA vs Global BA" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Local BA:" << std::endl;
    std::cout << "  범위: Covisible Keyframes만" << std::endl;
    std::cout << "  장점: 실시간 가능 (10-50ms)" << std::endl;
    std::cout << "  단점: 드리프트 누적\n" << std::endl;

    std::cout << "Global BA:" << std::endl;
    std::cout << "  범위: 전체 맵" << std::endl;
    std::cout << "  장점: 전역 최적" << std::endl;
    std::cout << "  단점: 느림 (Loop Closure 후만)\n" << std::endl;

    // 실제 SLAM 시스템에서의 BA 전략:
    //   ORB-SLAM: Local BA(추적 중) + Global BA(Loop 후) 조합
    //   VINS: Sliding Window BA (최근 N개 KF만)
    std::cout << "💡 ORB-SLAM: Local BA (실시간) + Global BA (Loop 후)" << std::endl;
    std::cout << "💡 VINS: Sliding Window BA = Local BA의 변형" << std::endl;
}

// ── 교육 블록 4: Gauge Freedom & Marginalization ──
//
// BA에서 반드시 이해해야 하는 두 가지 개념:
//
// 1. Gauge Freedom (게이지 자유도):
//    BA의 해는 좌표계 변환에 불변 → 7 DoF(회전3+이동3+스케일1)만큼 자유도 과잉
//    → Hessian이 rank deficient → 풀 수 없음!
//    해결: 첫 KF를 setFixed(true) → 6 DoF 제거
//          (스케일은 3D 점 간 거리로 암시적 결정)
//
// 2. Marginalization:
//    g2o: vertex_point->setMarginalized(true) → Schur Complement 자동 적용
//    Ceres: Solver::Options에서 DENSE_SCHUR 선택 → 자동 적용
//    → 사용자는 "이 변수를 marginalize하라"고 지시만 하면 됨
//
// 💡 quiz_easy Q5: g2o setMarginalized의 의미
// 💡 quiz_easy Q6: Gauge freedom이 생기는 이유
void demoGaugeAndMarginalize()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Gauge Freedom & Marginalization" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Gauge Freedom:
    //   전체 맵을 1m 오른쪽으로 이동해도 → 재투영 오차 동일
    //   전체 맵을 2배 확대해도 → 재투영 오차 동일
    //   → 무한히 많은 해가 존재 → 기준점을 고정해야 함
    std::cout << "Gauge Freedom:" << std::endl;
    std::cout << "  BA는 7 DoF 자유도 (3 회전 + 3 이동 + 1 스케일)" << std::endl;
    std::cout << "  → 첫 KF를 setFixed(true)로 고정\n" << std::endl;

    // Marginalization:
    //   g2o에서 vertex->setMarginalized(true)를 호출하면
    //   내부적으로 Schur Complement를 적용하여
    //   해당 변수를 먼저 소거한 후 나머지 변수를 풀음.
    //
    //   ★ 이것이 이 Week에서 배운 Schur Complement의 실제 적용!
    //   우리가 손으로 구현한 schur_complement() →
    //   g2o/Ceres에서는 한 줄 설정으로 자동 적용
    std::cout << "Marginalization (g2o):" << std::endl;
    std::cout << "  vertex_point->setMarginalized(true)" << std::endl;
    std::cout << "  → 내부적으로 Schur Complement 적용!" << std::endl;
    std::cout << "  → 점 변수를 먼저 소거 → 카메라만 풀기" << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// main
// ═══════════════════════════════════════════════════════════════

#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 9: Schur Complement" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 교육 블록
    demoSparseStructure();      // 블록 1: BA Hessian의 희소 구조
    demoSchurComplement();      // 블록 2: Schur Complement 시연 (직접 vs Schur)
    demoLocalVsGlobal();        // 블록 3: Local BA vs Global BA
    demoGaugeAndMarginalize();  // 블록 4: Gauge Freedom & Marginalization

    // 다음 단계 안내
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📌 다음 단계:" << std::endl;
    std::cout << "  1. quiz_easy    → 희소 구조, Schur, Local/Global BA" << std::endl;
    std::cout << "  2. quiz_medium  → Schur 계산량 분석, VINS" << std::endl;
    std::cout << "  3. my_basic.cpp → 직접 구현 (5 Step)" << std::endl;
    std::cout << "  4. Week 10      → g2o 심화" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
