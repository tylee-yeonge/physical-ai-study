/**
 * Phase 4 Week 2 - IMU 노이즈 모델 기초 퀴즈
 */

#include <iostream>
#include <cmath>

void problem1_noise_types()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: IMU 노이즈 종류" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "IMU 측정 모델에서 White Noise와 Bias의 차이는?\n" << std::endl;
    std::cout << "  (A) White Noise는 상수, Bias는 랜덤" << std::endl;
    std::cout << "  (B) White Noise는 매 순간 독립적, Bias는 천천히 변함" << std::endl;
    std::cout << "  (C) White Noise는 적분으로 사라지고, Bias는 커짐" << std::endl;
    std::cout << "  (D) 둘 다 같은 성질\n" << std::endl;

    std::cout << "  힌트: 적분했을 때 어떻게 변하는지 생각하세요." << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem2_bias_random_walk()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Bias Random Walk" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "자이로 바이어스 모델: db/dt = n_w (white noise)\n" << std::endl;
    std::cout << "이 모델의 의미로 가장 적절한 것은?\n" << std::endl;
    std::cout << "  (A) 바이어스가 일정하게 유지됨" << std::endl;
    std::cout << "  (B) 바이어스가 시간에 따라 랜덤하게 '걸어다님'" << std::endl;
    std::cout << "  (C) 바이어스가 지수적으로 감소함" << std::endl;
    std::cout << "  (D) 바이어스가 주기적으로 변함\n" << std::endl;

    std::cout << "  힌트: 'Random Walk' = 무작위 걸음" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem3_allan_variance()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Allan Variance" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Allan Variance 그래프에서 읽을 수 있는 노이즈 파라미터는?\n" << std::endl;
    std::cout << "  (A) White Noise 크기만" << std::endl;
    std::cout << "  (B) Bias 크기만" << std::endl;
    std::cout << "  (C) White Noise density와 Bias random walk 둘 다" << std::endl;
    std::cout << "  (D) 센서의 최대 측정 범위\n" << std::endl;

    std::cout << "  힌트: Allan Variance 로그 그래프의 기울기가 다른 두 영역" << std::endl;
    std::cout << "    기울기 -1/2: ???,  기울기 +1/2: ???" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem4_noise_units()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 노이즈 파라미터 단위" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "가속도계 White Noise density의 단위는?\n" << std::endl;
    std::cout << "  (A) m/s²" << std::endl;
    std::cout << "  (B) m/s²/sqrt(Hz)" << std::endl;
    std::cout << "  (C) m/s³" << std::endl;
    std::cout << "  (D) rad/s\n" << std::endl;

    std::cout << "  힌트: noise density는 주파수 대역에 따라 스케일링됩니다." << std::endl;
    std::cout << "  이산 시간에서 σ_d = σ_c / sqrt(dt)" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 2 Quiz - Easy (IMU 노이즈 모델)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_noise_types();
    problem2_bias_random_walk();
    problem3_allan_variance();
    problem4_noise_units();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
