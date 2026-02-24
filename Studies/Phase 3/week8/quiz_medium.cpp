/**
 * Quiz Medium - Week 8: Ceres Bundle Adjustment
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. Jet 타입의 내부 구조 — 이중수(dual number) 기반 자동 미분
 *   2. DENSE_SCHUR solver — BA의 희소 구조를 활용한 효율적 풀이
 *   3. Ceres vs g2o 사용 시나리오 — 각각의 강점과 적합한 상황
 *
 * Jet 타입의 원리 (이중수 산술):
 *
 *   Jet<double, N> = { a + εv }
 *     a = 함수값 (double)
 *     v = N개의 편미분 값 (∂f/∂x₁, ∂f/∂x₂, ..., ∂f/∂xₙ)
 *
 *   연산 규칙 (체인 룰 자동 적용):
 *     (a + εu) + (b + εv) = (a+b) + ε(u+v)
 *     (a + εu) × (b + εv) = ab + ε(av + bu)
 *     f(a + εv) = f(a) + εf'(a)v
 *
 * DENSE_SCHUR가 BA에 적합한 이유:
 *
 *   BA 헤시안: [Hpp  Hpl]     ← Hpp: 포즈-포즈 (작음, 밀집)
 *              [Hpl' Hll]     ← Hll: 점-점 (크지만 블록 대각)
 *
 *   Schur 소거: 3D 점 소거 → 포즈만의 밀집 시스템
 *   DENSE_SCHUR: reduced 시스템이 밀집(dense)이므로 밀집 행렬 풀이 사용
 *
 * 난이도: ★★☆ (서술형)
 * 선수 지식: quiz_easy (Ceres 기본), Week 7 (g2o, Schur Complement)
 */

#include <iostream>

// Q1: Jet 타입이 저장하는 것
//
// Jet<double, N>의 구조:
//   - a (double): 현재 함수값
//   - v (double[N]): N개의 편미분 값
//
// 예시: f(x, y) = x² + xy, x=3, y=2
//   x = Jet(3, [1, 0])  ← 값=3, ∂x/∂x=1, ∂x/∂y=0
//   y = Jet(2, [0, 1])  ← 값=2, ∂y/∂x=0, ∂y/∂y=1
//
//   x² = Jet(9, [6, 0])     ← 값=9, ∂(x²)/∂x=2x=6
//   xy = Jet(6, [2, 3])     ← 값=6, ∂(xy)/∂x=y=2, ∂(xy)/∂y=x=3
//   f  = Jet(15, [8, 3])    ← 값=15, ∂f/∂x=8, ∂f/∂y=3
//
// ★ 하나의 순방향 패스로 함수값과 모든 편미분을 동시에 계산
//   이것이 자동 미분이 수치 미분보다 정확하고 빠른 이유
void problem1_jet_type()
{
    std::cout << "Q1. Ceres의 Jet 타입은 무엇을 저장하나요?\n";
    std::cout << "   설명: ___________________\n" << std::endl;
}

// Q2: DENSE_SCHUR solver를 사용하는 이유
//
// BA의 구조적 특징:
//   1. 3D 점끼리는 직접 연결 안 됨 → Hll이 블록 대각
//   2. 3D 점 수 >> 포즈 수 (수만 vs 수십~수백)
//   3. Schur 소거 후 reduced system 크기 = 포즈 수 × 6
//
// DENSE_SCHUR 선택 이유:
//   - reduced system이 "밀집"(dense) 행렬 → 밀집 솔버 적합
//   - 포즈 수가 적으면(~100) 밀집 풀이가 더 빠름
//   - 포즈 수가 많으면(1000+) → SPARSE_SCHUR 또는 ITERATIVE_SCHUR
//
// Ceres에서 설정:
//   options.linear_solver_type = ceres::DENSE_SCHUR;
//
// ★ g2o에서는 setMarginalized(true)가 동일한 역할
void problem2_dense_schur()
{
    std::cout << "Q2. DENSE_SCHUR solver를 사용하는 이유는?\n";
    std::cout << "   (BA의 구조적 특징과 연관지어 설명)\n";
    std::cout << "   답: ___________________\n" << std::endl;
}

// Q3: Ceres vs g2o 사용 시나리오
//
// Ceres 권장 상황:
//   - 새로운 비용 함수를 빠르게 프로토타이핑할 때 (AutoDiff)
//   - Jacobian 유도가 복잡한 문제 (VIO, 복합 센서 융합)
//   - 다양한 Loss 함수/solver를 실험할 때
//   - 대표 시스템: VINS-Mono, Cartographer
//
// g2o 권장 상황:
//   - SLAM에 특화된 타입이 필요할 때 (SE3, Sim3)
//   - 기존 SLAM 시스템에 통합할 때 (ORB-SLAM 생태계)
//   - 최대 속도가 필요할 때 (수동 Jacobian + marginalization)
//   - Pose Graph Optimization (BA 외에도 활용)
//   - 대표 시스템: ORB-SLAM2/3, LSD-SLAM
//
// ★ 둘 다 Schur Complement를 지원하며 BA 결과는 동일
//   차이는 API 편의성 vs SLAM 특화 기능
void problem3_ceres_vs_g2o()
{
    std::cout << "Q3. Ceres vs g2o를 각각 언제 사용할지 설명하세요:\n";
    std::cout << "   Ceres 권장: ___________________\n";
    std::cout << "   g2o 권장: ___________________\n" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 8 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_jet_type();
    problem2_dense_schur();
    problem3_ceres_vs_g2o();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
