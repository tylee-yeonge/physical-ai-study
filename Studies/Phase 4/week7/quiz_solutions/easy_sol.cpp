/**
 * Phase 4 Week 7 - Pre-integration 수식 기초 퀴즈 풀이
 */

#include <iostream>
#include <cmath>

void problem1_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: IMU 측정 모델" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) 9.81 m/s^2 (중력 반대 방향)\n" << std::endl;

    std::cout << "  가속도계 모델: a_m = R^T(a_w - g) + b_a + n_a\n" << std::endl;
    std::cout << "  정지 상태: a_w = 0 (실제 가속도 없음)" << std::endl;
    std::cout << "  g = [0, 0, -9.81]\n" << std::endl;
    std::cout << "  a_m = R^T(0 - [0,0,-9.81]) + 0 + 0" << std::endl;
    std::cout << "      = R^T[0, 0, 9.81]" << std::endl;
    std::cout << "      ≈ [0, 0, 9.81] (수평 정지 시)\n" << std::endl;

    std::cout << "  직관:" << std::endl;
    std::cout << "  - 가속도계는 '관성력'을 측정합니다" << std::endl;
    std::cout << "  - 정지 상태 = 중력에 대항하는 힘 = +9.81" << std::endl;
    std::cout << "  - 자유낙하 = 중력에 따르는 것 = 0 (무중력)" << std::endl;
    std::cout << "  - 이것이 a_w - g에서 g를 '빼는' 이유입니다" << std::endl;
}

void problem2_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: Pre-integrated 측정값의 의미" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (D) 위 모두 맞음\n" << std::endl;

    std::cout << "  (A) 재적분 불필요:" << std::endl;
    std::cout << "    최적화에서 p_i, v_i, R_i가 바뀌어도" << std::endl;
    std::cout << "    ΔR, Δv, Δp는 그대로 사용 가능!" << std::endl;
    std::cout << "    → 이것이 Pre-integration의 핵심 장점\n" << std::endl;

    std::cout << "  (B) 수식에 출발 포즈 없음:" << std::endl;
    std::cout << "    ΔR_ij = Π Exp((ω_m - b_g)·dt)" << std::endl;
    std::cout << "    Δv_ij = Σ ΔR_ik·(a_m - b_a)·dt" << std::endl;
    std::cout << "    Δp_ij = Σ [Δv_ik·dt + 0.5·ΔR_ik·(a_m - b_a)·dt²]" << std::endl;
    std::cout << "    → p_i, v_i, R_i가 어디에도 없음!\n" << std::endl;

    std::cout << "  (C) 바이어스 보정:" << std::endl;
    std::cout << "    바이어스만 변하면 자코비안으로 1차 보정:" << std::endl;
    std::cout << "    Δv' ≈ Δv + J_v_ba·δb_a + J_v_bg·δb_g" << std::endl;
    std::cout << "    → 완전한 재적분보다 훨씬 빠름" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 업데이트 순서" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) Δp → Δv → ΔR (아래에서 위)\n" << std::endl;

    std::cout << "  이유: 각 업데이트가 '현재' 값을 사용하기 때문\n" << std::endl;

    std::cout << "  Δp 업데이트: 현재 Δv와 현재 ΔR을 사용" << std::endl;
    std::cout << "    Δp += Δv·dt + 0.5·ΔR·acc·dt²" << std::endl;
    std::cout << "    → Δv나 ΔR이 먼저 바뀌면 '다음 시점' 값 사용 = 오차!\n" << std::endl;

    std::cout << "  Δv 업데이트: 현재 ΔR을 사용" << std::endl;
    std::cout << "    Δv += ΔR·acc·dt" << std::endl;
    std::cout << "    → ΔR이 먼저 바뀌면 안됨\n" << std::endl;

    std::cout << "  ΔR 업데이트: 독립적" << std::endl;
    std::cout << "    ΔR *= Exp(gyro·dt)" << std::endl;
    std::cout << "    → 마지막에 업데이트\n" << std::endl;

    std::cout << "  따라서: Δp(현재 Δv,ΔR) → Δv(현재 ΔR) → ΔR" << std::endl;
    std::cout << "  (또는 mid-point 방법 사용 시 순서 무관)" << std::endl;
}

void problem4_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 풀이: 상태 복원 공식" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (A) Δv, Δp가 시각 i 기준 좌표이므로 월드로 변환\n" << std::endl;

    std::cout << "  Pre-integrated 값의 좌표 프레임:" << std::endl;
    std::cout << "    ΔR_ij: 시각 i → 시각 j의 상대 회전" << std::endl;
    std::cout << "    Δv_ij: 시각 i 프레임에서의 상대 속도 변화" << std::endl;
    std::cout << "    Δp_ij: 시각 i 프레임에서의 상대 위치 변화\n" << std::endl;

    std::cout << "  복원 공식에서 R_i의 역할:" << std::endl;
    std::cout << "    v_j = v_i + g·Δt + R_i · Δv_ij" << std::endl;
    std::cout << "                        ↑ i프레임→월드 변환\n" << std::endl;

    std::cout << "    p_j = p_i + v_i·Δt + 0.5·g·Δt² + R_i · Δp_ij" << std::endl;
    std::cout << "                                        ↑ i프레임→월드 변환\n" << std::endl;

    std::cout << "  왜 시각 i 기준인가?" << std::endl;
    std::cout << "    → Pre-integration이 IMU 바디 프레임에서 계산되기 때문" << std::endl;
    std::cout << "    → ΔR_ik가 시각 i 기준으로 가속도를 변환하므로" << std::endl;
    std::cout << "    → Δv, Δp도 자연스럽게 시각 i 프레임" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7 Quiz Easy - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
