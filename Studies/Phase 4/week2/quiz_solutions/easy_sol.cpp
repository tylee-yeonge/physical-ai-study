/**
 * Phase 4 Week 2 - IMU 노이즈 모델 기초 퀴즈 풀이
 */

#include <iostream>
#include <cmath>

void problem1_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: IMU 노이즈 종류" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) White Noise는 매 순간 독립적, Bias는 천천히 변함\n" << std::endl;

    std::cout << "  White Noise (백색 소음):" << std::endl;
    std::cout << "    → 매 측정마다 독립적인 랜덤 값" << std::endl;
    std::cout << "    → 시간 상관 없음" << std::endl;
    std::cout << "    → 적분하면 Random Walk (√t에 비례)\n" << std::endl;

    std::cout << "  Bias (바이어스):" << std::endl;
    std::cout << "    → 천천히 변하는 오프셋" << std::endl;
    std::cout << "    → Random Walk 모델: db/dt = n_w" << std::endl;
    std::cout << "    → 적분하면 t에 비례하여 누적\n" << std::endl;

    std::cout << "  (C)가 아닌 이유:" << std::endl;
    std::cout << "    White Noise는 적분해도 사라지지 않음" << std::endl;
    std::cout << "    → Random Walk로 변환되어 √t에 비례 성장" << std::endl;
    std::cout << "    → 다만 바이어스(t에 비례)보다는 느리게 성장" << std::endl;
}

void problem2_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: Bias Random Walk" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) 바이어스가 시간에 따라 랜덤하게 '걸어다님'\n" << std::endl;

    std::cout << "  모델: db/dt = n_w (white noise)" << std::endl;
    std::cout << "  → b(t) = b(0) + ∫n_w dt\n" << std::endl;

    std::cout << "  특성:" << std::endl;
    std::cout << "    → 바이어스가 현재 값 주변에서 무작위로 변동" << std::endl;
    std::cout << "    → 천천히 드리프트하는 모양" << std::endl;
    std::cout << "    → 표준편차: σ_b(t) = σ_bw × √t\n" << std::endl;

    std::cout << "  수치 예시 (σ_bw = 0.0001 rad/s²/√Hz):" << std::endl;
    for (double t : {1.0, 60.0, 3600.0})
    {
        double sigma_b = 0.0001 * std::sqrt(t);
        printf("    t = %6.0fs: σ_b = %.6f rad/s\n", t, sigma_b);
    }
    std::cout << "    → 1시간 후 바이어스가 0.006 rad/s 변할 수 있음" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: Allan Variance" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (C) White Noise density와 Bias random walk 둘 다\n" << std::endl;

    std::cout << "  Allan Variance (ADEV) 로그-로그 그래프:" << std::endl;
    std::cout << "    ┌──────────────────────────────────┐" << std::endl;
    std::cout << "    │  \\                          /    │" << std::endl;
    std::cout << "    │   \\   기울기 -1/2        /      │" << std::endl;
    std::cout << "    │    \\  (White Noise)   / 기울기   │" << std::endl;
    std::cout << "    │     \\              /   +1/2     │" << std::endl;
    std::cout << "    │      \\           / (Bias RW)    │" << std::endl;
    std::cout << "    │       \\________/                │" << std::endl;
    std::cout << "    │              ↑                   │" << std::endl;
    std::cout << "    │         최소점 (Bias Instability) │" << std::endl;
    std::cout << "    └──────────────────────────────────┘" << std::endl;
    std::cout << "              τ (적분 시간) →\n" << std::endl;

    std::cout << "  읽는 방법:" << std::endl;
    std::cout << "    기울기 -1/2 영역: N (Noise density) = ADEV(τ=1)" << std::endl;
    std::cout << "    기울기 +1/2 영역: K (Bias RW) = ADEV / √3 at τ=3" << std::endl;
    std::cout << "    최소점: Bias Instability (σ_BI)" << std::endl;
}

void problem4_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 풀이: 노이즈 파라미터 단위" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) m/s²/√Hz (= m/s²/sqrt(Hz))\n" << std::endl;

    std::cout << "  Noise density (연속 시간):" << std::endl;
    std::cout << "    가속도계: σ_a [m/s²/√Hz]" << std::endl;
    std::cout << "    자이로:   σ_g [rad/s/√Hz]\n" << std::endl;

    std::cout << "  이산 시간 변환:" << std::endl;
    std::cout << "    σ_discrete = σ_continuous / √dt\n" << std::endl;

    double sigma_c = 0.04;
    double dt = 0.005;
    double sigma_d = sigma_c / std::sqrt(dt);

    std::cout << "  예시: σ_a = 0.04 m/s²/√Hz, dt = 0.005s (200Hz)" << std::endl;
    std::cout << "    σ_discrete = " << sigma_c << " / √" << dt << " = " << sigma_d << " m/s²\n"
              << std::endl;

    std::cout << "  직관: noise density는 '주파수 대역당' 노이즈" << std::endl;
    std::cout << "    → 샘플링이 빠르면 각 샘플의 노이즈는 커짐" << std::endl;
    std::cout << "    → 하지만 적분 후 최종 오차는 동일" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 2 Quiz Easy - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
