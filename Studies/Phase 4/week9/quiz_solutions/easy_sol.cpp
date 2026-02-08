/**
 * Phase 4 Week 9 - Pre-integration 심화 기초 퀴즈 풀이
 */

#include <iostream>

void problem1_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: 공분산이 필요한 이유" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) Factor Graph에서 IMU Factor의 가중치를 정할 수 없음\n" << std::endl;

    std::cout << "  비용 함수: E = r^T · Σ^{-1} · r\n" << std::endl;
    std::cout << "  Σ가 없으면:" << std::endl;
    std::cout << "    → Σ^{-1}을 계산할 수 없음" << std::endl;
    std::cout << "    → 0.1초 적분과 5초 적분의 가중치가 동일" << std::endl;
    std::cout << "    → 부정확한 장시간 적분도 정확한 것처럼 취급" << std::endl;
    std::cout << "    → 최적화 결과가 비합리적!\n" << std::endl;

    std::cout << "  Σ가 있으면:" << std::endl;
    std::cout << "    → 짧은 적분: Σ 작음 → Σ^{-1} 큼 → 큰 가중치" << std::endl;
    std::cout << "    → 긴 적분: Σ 큼 → Σ^{-1} 작음 → 작은 가중치" << std::endl;
    std::cout << "    → 정확한 Factor에 더 의존 → 합리적 최적화!" << std::endl;
}

void problem2_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 공분산 증가 패턴" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (C) 위치 σ_p (∝ t^{3/2})\n" << std::endl;

    std::cout << "  노이즈 누적 과정:\n" << std::endl;
    std::cout << "  1. 자이로 노이즈 → 회전 오차" << std::endl;
    std::cout << "     σ_φ ∝ √(N·σ²_gyro) ∝ √t" << std::endl;
    std::cout << "     (N개 독립 노이즈의 합 → √N)\n" << std::endl;

    std::cout << "  2. 가속도 노이즈 → 속도 오차" << std::endl;
    std::cout << "     σ_v ∝ √(N·σ²_acc) ∝ √t" << std::endl;
    std::cout << "     (+ 회전 오차의 간접 영향)\n" << std::endl;

    std::cout << "  3. 속도 오차 적분 → 위치 오차" << std::endl;
    std::cout << "     σ_p ∝ ∫σ_v dt ∝ ∫√t dt ∝ t^{3/2}" << std::endl;
    std::cout << "     → 가장 빠르게 증가!\n" << std::endl;

    std::cout << "  실무적 의미:" << std::endl;
    std::cout << "    키프레임 간격 2배 → 위치 불확실성 2^{1.5} ≈ 2.83배" << std::endl;
    std::cout << "    → 짧은 키프레임 간격이 중요!" << std::endl;
}

void problem3_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: IMU 노이즈 파라미터" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (C) 모두 사용\n" << std::endl;

    std::cout << "  acc_n, gyr_n (white noise density):" << std::endl;
    std::cout << "    → Pre-integration 적분 노이즈" << std::endl;
    std::cout << "    → ΔR, Δv, Δp의 불확실성에 직접 기여" << std::endl;
    std::cout << "    → Q_d = diag(gyr_n²/dt, acc_n²/dt)\n" << std::endl;

    std::cout << "  acc_w, gyr_w (bias random walk):" << std::endl;
    std::cout << "    → 바이어스의 변화 불확실성" << std::endl;
    std::cout << "    → VINS 15D 공분산에서 바이어스 부분에 기여" << std::endl;
    std::cout << "    → Q_bias = diag(acc_w²·dt, gyr_w²·dt)\n" << std::endl;

    std::cout << "  9D 공분산 (기본): acc_n, gyr_n만 사용" << std::endl;
    std::cout << "  15D 공분산 (VINS): 4개 모두 사용" << std::endl;
}

void problem4_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 풀이: VINS의 15D 공분산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) 가속도 바이어스 δb_a (3D) + 자이로 바이어스 δb_g (3D)\n" << std::endl;

    std::cout << "  VINS 15D Error State:" << std::endl;
    std::cout << "    [δφ(3), δp(3), δv(3), δb_a(3), δb_g(3)]\n" << std::endl;

    std::cout << "  왜 바이어스를 포함하나?" << std::endl;
    std::cout << "    1. 바이어스는 Random Walk로 천천히 변함" << std::endl;
    std::cout << "    2. 적분 중 바이어스 변화도 불확실성에 기여" << std::endl;
    std::cout << "    3. 바이어스와 다른 상태 간 상관관계도 추적\n" << std::endl;

    std::cout << "  15×15 공분산 구조:" << std::endl;
    std::cout << "    | Σ_φφ  Σ_φp  Σ_φv  Σ_φba  Σ_φbg |" << std::endl;
    std::cout << "    | Σ_pφ  Σ_pp  Σ_pv  Σ_pba  Σ_pbg |" << std::endl;
    std::cout << "    | Σ_vφ  Σ_vp  Σ_vv  Σ_vba  Σ_vbg |" << std::endl;
    std::cout << "    | Σ_baφ ...              Σ_baba ... |" << std::endl;
    std::cout << "    | Σ_bgφ ...                    Σ_bgbg |" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 9 Quiz Easy - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
