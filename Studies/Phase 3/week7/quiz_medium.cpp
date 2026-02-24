/**
 * Quiz Medium - Week 7: Bundle Adjustment (g2o)
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. Schur Complement의 소거 순서 — 3D 점 소거 → 포즈 먼저 풀기
 *   2. setMarginalized(true) — Schur 소거 대상 지정
 *   3. setFixed(true) — Gauge freedom 제거
 *
 * BA 헤시안의 구조와 Schur Complement:
 *
 *   [Hpp  Hpl] [Δp]   [bp]      Hpp: 포즈-포즈 블록 (작음)
 *   [Hpl' Hll] [Δl] = [bl]      Hll: 점-점 블록 (크지만 블록 대각)
 *
 *   Schur 소거: Δl을 소거하여 Δp만의 작은 시스템으로 축소
 *   (Hpp - Hpl·Hll⁻¹·Hpl')·Δp = bp - Hpl·Hll⁻¹·bl
 *
 *   풀이 순서: Δp를 먼저 풀고 → Δl = Hll⁻¹(bl - Hpl'·Δp)로 역대입
 *
 * 난이도: ★★☆ (g2o API 이해)
 * 선수 지식: quiz_easy (BA 기본, Vertex/Edge), basic.cpp (g2o 구현)
 */

#include <iostream>

// Q1: Schur Complement에서 먼저 최적화하는 것
//
// Schur Complement의 풀이 순서:
//   1단계: 3D 점(l)을 소거 → 포즈(p)만의 reduced system
//   2단계: reduced system을 풀어 Δp 계산
//   3단계: Δp를 대입하여 Δl 계산 (역대입)
//
// 즉 "먼저 풀리는 것" = 카메라 포즈(Δp)
//
// ★ 왜 3D 점을 소거하는가?
//   - 3D 점 수 >> 포즈 수 (수만 vs 수백)
//   - Hll이 블록 대각 → Hll⁻¹ 계산이 O(n)으로 효율적
//   - 결과: 큰 시스템 → 작은 시스템으로 축소
void problem1_schur_order()
{
    std::cout << "Q1. Schur Complement에서 먼저 최적화하는 것은?\n";
    std::cout << "   a) 3D 점\n";
    std::cout << "   b) 카메라 포즈\n";
    std::cout << "   c) 동시에\n";
    std::cout << "   d) 순서 상관 없음\n";
    std::cout << "Your answer: ";
    char ans1;
    std::cin >> ans1;
}

// Q2: setMarginalized(true) 대상
//
// g2o에서 setMarginalized(true):
//   "이 Vertex를 Schur Complement로 소거하라"는 지시
//
//   v_point->setMarginalized(true);   // 3D 점을 소거 대상으로 지정
//
// 소거 대상 = 3D 점 (VertexPointXYZ)
//   - 3D 점은 수가 많고, Hll이 블록 대각이라 소거 효율적
//   - 포즈를 소거하면 Hpp가 복잡해져 비효율적
//
// ★ 주의: 카메라 포즈에는 setMarginalized를 설정하지 않음!
//   포즈끼리는 3D 점을 통해 간접 연결 → Hpp가 블록 대각이 아님
void problem2_marginalized()
{
    std::cout << "\nQ2. setMarginalized(true)를 설정하는 Vertex는?\n";
    std::cout << "   a) 카메라 포즈\n";
    std::cout << "   b) 3D 점\n";
    std::cout << "   c) 둘 다\n";
    std::cout << "   d) 없음\n";
    std::cout << "Your answer: ";
    char ans2;
    std::cin >> ans2;
}

// Q3: setFixed(true)의 이유
//
// Gauge Freedom (게이지 자유도):
//   BA의 목적 함수는 "상대적" 관계만 정의
//   → 전체 맵을 평행 이동/회전/스케일링해도 비용이 동일
//   → 해가 유일하지 않음 (무한히 많은 해)
//
//   자유도: SE(3) → 6 DoF (3 회전 + 3 이동)
//           + 단안이면 1 DoF (스케일) = 총 7 DoF
//
// 해결: 첫 카메라를 setFixed(true)
//   → 첫 포즈가 원점(I, 0)에 고정
//   → 나머지 포즈/점이 이 기준으로 최적화
//   → Gauge freedom 제거 → 유일한 해
//
// ★ 고정하지 않으면 헤시안이 특이(singular) → 풀이 불가
//   실제로는 첫 포즈의 행/열이 H에서 제거됨
void problem3_set_fixed()
{
    std::cout << "\nQ3. 첫 카메라를 setFixed(true)하는 이유는?\n";
    std::cout << "   a) 속도 향상\n";
    std::cout << "   b) Gauge freedom 제거\n";
    std::cout << "   c) 메모리 절약\n";
    std::cout << "   d) Outlier 제거\n";
    std::cout << "Your answer: ";
    char ans3;
    std::cin >> ans3;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7 Quiz (Medium)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_schur_order();
    problem2_marginalized();
    problem3_set_fixed();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "답안 제출 완료!" << std::endl;
    std::cout << "quiz_solutions/medium_sol.cpp에서 정답을 확인하세요." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
