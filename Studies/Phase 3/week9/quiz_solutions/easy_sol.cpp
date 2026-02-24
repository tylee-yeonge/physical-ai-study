/**
 * Quiz Easy (정답) - Week 9: BA 최적화 기법 (Schur Complement)
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. BA 자코비안의 희소성 — 관측 구조에 의한 희소 패턴
 *   2. Schur Complement — 3D 점 소거를 통한 문제 축소
 *   3. Local BA vs Global BA — 최적화 범위의 차이
 *   4. Hll (점-점 블록)의 블록 대각 구조
 *   5. g2o의 setMarginalized — Schur 소거 대상 지정
 *   6. Gauge freedom — 전역 좌표계의 불확정성
 *
 * BA 헤시안의 희소 구조:
 *
 *   H = J^T J = [Hpp  Hpl]     Hpp: 포즈-포즈 (밀집, 작음)
 *               [Hpl' Hll]     Hpl: 포즈-점 (희소)
 *                              Hll: 점-점 (블록 대각, 큼)
 *
 *   Hll이 블록 대각인 이유:
 *     3D 점 j의 관측은 카메라 i에만 의존
 *     → 3D 점끼리는 직접 연결 안 됨
 *     → Hll의 비대각 블록 = 0
 *
 *   Schur Complement:
 *     (Hpp - Hpl·Hll⁻¹·Hpl')·Δp = bp - Hpl·Hll⁻¹·bl
 *     → 큰 시스템(포즈+점)을 작은 시스템(포즈만)으로 축소
 *
 * 난이도: ★☆☆ (기본 개념)
 * 선수 지식: Week 7-8 (BA 기초, g2o/Ceres)
 */

#include <iostream>

// Q1: BA 자코비안의 희소성
//
// 자코비안 J의 각 행 = 하나의 관측 e_ij
//   e_ij = z_ij - π(T_i, P_j)
//
// e_ij는 T_i (카메라 i)와 P_j (3D 점 j)에만 의존
//   → ∂e_ij/∂T_k = 0 (k ≠ i)
//   → ∂e_ij/∂P_l = 0 (l ≠ j)
//   → J의 대부분이 0 (희소!)
//
//     J = [ ∂e_01/∂T_0  0  ...  ∂e_01/∂P_1  0  ... ]
//         [     0    ∂e_12/∂T_1    0     ∂e_12/∂P_2 ]
//         [    ...       ...      ...      ...       ]
//
// ★ 이 희소 구조 덕분에 H = J^T J도 희소 → Schur Complement 가능
void problem1_sparsity()
{
    std::cout << "Q1. BA에서 자코비안 행렬이 희소(sparse)한 이유는?\n";
    std::cout << "   a) 카메라가 모든 점을 관측하기 때문\n";
    std::cout << "   b) 각 관측이 하나의 카메라와 하나의 점에만 연결되기 때문\n";
    std::cout << "   c) 3D 점이 모든 카메라에 보이기 때문\n";
    std::cout << "   d) Hessian이 대칭이기 때문\n";
    // ✅ 정답:
    std::cout << "   답: b) 각 관측이 하나의 카메라와 하나의 점에만 연결되기 때문\n" << std::endl;
}

// Q2: Schur Complement에서 먼저 소거하는 변수
//
// Schur Complement 과정:
//   1. 3D 점(l)을 소거 → 포즈(p)만의 reduced system
//   2. Δp를 풀고 → Δl = Hll⁻¹(bl - Hpl'·Δp)로 역대입
//
// 왜 3D 점을 소거하는가?
//   - 3D 점 수(N_l) >> 포즈 수(N_p): 수만 vs 수백
//   - Hll이 블록 대각 → Hll⁻¹ 계산이 O(N_l)로 효율적
//   - reduced system 크기 = 6·N_p × 6·N_p (훨씬 작음)
//
// ★ 포즈를 소거하면? Hpp가 밀집이라 비효율적
void problem2_schur_variable()
{
    std::cout << "Q2. Schur Complement에서 먼저 소거하는 변수는?\n";
    std::cout << "   a) 카메라 포즈\n";
    std::cout << "   b) 3D 점\n";
    std::cout << "   c) 관측값\n";
    std::cout << "   d) 잔차\n";
    // ✅ 정답:
    std::cout << "   답: b) 3D 점\n" << std::endl;
}

// Q3: Local BA vs Global BA
//
// Global BA:
//   모든 KF + 모든 맵 포인트를 동시에 최적화
//   + 전체 맵의 일관성 보장
//   - 계산량 O(n³) → KF 수백 개이면 수 초~수십 초
//   - 실시간 불가 → Loop Closure 후에만 수행
//
// Local BA:
//   현재 KF + covisible KF(~20개)만 최적화
//   + 실시간 가능 (수십 ms)
//   - 전역 일관성 보장 못 함 → 드리프트 누적
//   - ORB-SLAM2: Local Mapping 스레드에서 매 KF 삽입 시 수행
//
// ★ 핵심 차이: 최적화하는 "변수의 범위"
//   알고리즘(LM, GN)은 동일, 대상만 다름
void problem3_local_vs_global()
{
    std::cout << "Q3. Local BA와 Global BA의 가장 큰 차이점은?\n";
    std::cout << "   a) 사용하는 알고리즘\n";
    std::cout << "   b) 최적화하는 변수의 범위\n";
    std::cout << "   c) 수렴 속도\n";
    std::cout << "   d) 정확도\n";
    // ✅ 정답:
    std::cout << "   답: b) 최적화하는 변수의 범위\n" << std::endl;
}

// Q4: Hll (점-점 블록)의 특징
//
// 주의: 문제에서 "Hpp"라고 쓰여있지만 실제로는 Hll(점-점 블록)의 특징을 묻고 있음
//
// Hll의 구조:
//   Hll = diag(H_l1, H_l2, ..., H_ln)  ← 블록 대각
//   각 H_lj = 3×3 행렬 (3D 점 j의 헤시안)
//
// 왜 블록 대각인가?
//   3D 점 j와 k가 직접 연결되려면 같은 관측에 포함되어야 함
//   하지만 하나의 관측은 "하나의 카메라 + 하나의 점"만 관련
//   → 점끼리는 카메라를 통해서만 간접 연결
//   → Hll의 비대각 블록 = 0
//
// ★ 이 블록 대각 구조가 Schur Complement의 핵심
//   Hll⁻¹ = diag(H_l1⁻¹, H_l2⁻¹, ...) → O(n)에 계산
void problem4_hll_structure()
{
    std::cout << "Q4. Hessian 행렬의 블록 구조에서 Hpp (점-점 블록)의 특징은?\n";
    std::cout << "   a) 밀집(dense) 행렬\n";
    std::cout << "   b) 블록 대각 행렬\n";
    std::cout << "   c) 영행렬\n";
    std::cout << "   d) 단위행렬\n";
    // ✅ 정답:
    std::cout << "   답: b) 블록 대각 행렬\n" << std::endl;
}

// Q5: g2o에서 Schur Complement 적용 방법
//
// g2o API:
//   vertex_point->setMarginalized(true);
//
// "marginalize" = "주변화" = "소거"
//   Schur Complement에서 3D 점을 소거하여 포즈만의 시스템으로 축소
//
// setFixed(true) → 변수 고정 (최적화 제외)
// setMarginalized(true) → Schur 소거 대상 (최적화는 하되 소거로 처리)
//
// ★ 주의: setMarginalized는 3D 점(VertexPointXYZ)에만 설정!
//   포즈(VertexSE3)에 설정하면 Schur 구조가 깨짐
void problem5_set_marginalized()
{
    std::cout << "Q5. g2o에서 Schur Complement를 적용하려면 어떤 함수를 호출?\n";
    std::cout << "   a) setFixed(true)\n";
    std::cout << "   b) setMarginalized(true)\n";
    std::cout << "   c) setOptimized(true)\n";
    std::cout << "   d) setSchur(true)\n";
    // ✅ 정답:
    std::cout << "   답: b) setMarginalized(true)\n" << std::endl;
}

// Q6: Gauge Freedom
//
// BA 비용 함수: Σ ||z_ij - π(T_i, P_j)||²
//
// 문제: 모든 T_i, P_j에 같은 변환 S를 적용해도 비용이 동일
//   T_i → S·T_i,  P_j → S·P_j  일 때
//   π(S·T_i, S·P_j) = π(T_i, P_j)  (투영은 상대적)
//
// Gauge freedom = 이 불확정 자유도
//   SE(3): 6 DoF (3 회전 + 3 이동)
//   단안: +1 DoF (스케일) = 7 DoF
//
// 해결: 첫 포즈를 고정 (setFixed/SetParameterBlockConstant)
//   → 좌표계 원점 확정 → 유일한 해
//
// ★ 고정하지 않으면 헤시안이 특이(singular) → 최적화 발산
void problem6_gauge_freedom()
{
    std::cout << "Q6. BA에서 Gauge freedom이란?\n";
    std::cout << "   a) 카메라 해상도 자유도\n";
    std::cout << "   b) 전역 좌표계를 고정할 자유도\n";
    std::cout << "   c) 점의 색상 자유도\n";
    std::cout << "   d) 최적화 반복 횟수\n";
    // ✅ 정답:
    std::cout << "   답: b) 전역 좌표계를 고정할 자유도\n" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 9 Quiz - Easy (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_sparsity();
    problem2_schur_variable();
    problem3_local_vs_global();
    problem4_hll_structure();
    problem5_set_marginalized();
    problem6_gauge_freedom();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
