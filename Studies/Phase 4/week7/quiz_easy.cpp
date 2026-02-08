/**
 * Phase 4 Week 7 - Pre-integration 수식 기초 퀴즈
 */

#include <iostream>
#include <cmath>

void problem1_imu_measurement_model() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: IMU 측정 모델" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "가속도계 측정 모델: a_m = R^T(a_w - g) + b_a + n_a\n" << std::endl;
    std::cout << "질문: IMU가 완전히 정지 상태일 때, 가속도계가 측정하는 값은?\n" << std::endl;
    std::cout << "  (A) 0 m/s^2 (정지이므로)" << std::endl;
    std::cout << "  (B) 9.81 m/s^2 (중력 반대 방향)" << std::endl;
    std::cout << "  (C) -9.81 m/s^2 (중력 방향)" << std::endl;
    std::cout << "  (D) 바이어스 값만큼\n" << std::endl;

    std::cout << "💡 힌트: 정지 = a_w = 0, g = [0,0,-9.81]" << std::endl;
    std::cout << "   a_m = R^T(0 - [0,0,-9.81]) = R^T[0,0,9.81]" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem2_preintegrated_meaning() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Pre-integrated 측정값의 의미" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Pre-integration의 결과물: ΔR_ij, Δv_ij, Δp_ij\n" << std::endl;
    std::cout << "질문: 이 값들이 '출발 포즈와 무관'하다는 것의 의미는?\n" << std::endl;
    std::cout << "  (A) 최적화에서 p_i, v_i, R_i가 바뀌어도 재적분 불필요" << std::endl;
    std::cout << "  (B) ΔR, Δv, Δp 수식에 p_i, v_i, R_i가 포함되지 않음" << std::endl;
    std::cout << "  (C) 바이어스가 변하면 자코비안으로 1차 보정 가능" << std::endl;
    std::cout << "  (D) 위 모두 맞음\n" << std::endl;

    std::cout << "💡 힌트: Pre-integration의 핵심 동기를 생각해보세요" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem3_update_order() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 업데이트 순서" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Pre-integration 이산 업데이트:\n" << std::endl;
    std::cout << "  ΔR ← ΔR · Exp((ω_m - b_g)·dt)" << std::endl;
    std::cout << "  Δv ← Δv + ΔR · (a_m - b_a)·dt" << std::endl;
    std::cout << "  Δp ← Δp + Δv·dt + 0.5·ΔR·(a_m - b_a)·dt²\n" << std::endl;
    std::cout << "질문: 코드 구현 시 올바른 업데이트 순서는?\n" << std::endl;
    std::cout << "  (A) ΔR → Δv → Δp (위에서 아래)" << std::endl;
    std::cout << "  (B) Δp → Δv → ΔR (아래에서 위)" << std::endl;
    std::cout << "  (C) 순서 상관없음" << std::endl;
    std::cout << "  (D) Δv → Δp → ΔR\n" << std::endl;

    std::cout << "💡 힌트: Δp는 현재 Δv를 사용, Δv는 현재 ΔR을 사용" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem4_state_recovery() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 상태 복원 공식" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "상태 복원 공식:" << std::endl;
    std::cout << "  R_j = R_i · ΔR_ij" << std::endl;
    std::cout << "  v_j = v_i + g·Δt + R_i · Δv_ij" << std::endl;
    std::cout << "  p_j = p_i + v_i·Δt + 0.5·g·Δt² + R_i · Δp_ij\n" << std::endl;

    std::cout << "질문: R_i를 Δv_ij, Δp_ij에 곱하는 이유는?\n" << std::endl;
    std::cout << "  (A) Δv, Δp가 '시각 i' 기준 좌표이므로 월드로 변환" << std::endl;
    std::cout << "  (B) 정규화를 위해" << std::endl;
    std::cout << "  (C) 바이어스 보정을 위해" << std::endl;
    std::cout << "  (D) 노이즈 제거를 위해\n" << std::endl;

    std::cout << "💡 힌트: Pre-integrated 값은 어떤 프레임 기준인가?" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7 Quiz - Easy (Pre-integration 수식 개념)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_imu_measurement_model();
    problem2_preintegrated_meaning();
    problem3_update_order();
    problem4_state_recovery();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
