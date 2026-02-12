/**
 * Phase 4 Week 3 - 칼만 필터 기초 퀴즈
 */

#include <iostream>

void problem1_predict_update()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 예측-업데이트 사이클" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 칼만 필터에서 예측(Prediction) 단계와\n"
              << "      업데이트(Update) 단계의 역할은?\n"
              << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   예측: 모델(물리 법칙)로 다음 상태 추정" << std::endl;
    std::cout << "   → 불확실성(P)이 증가" << std::endl;
    std::cout << "   업데이트: 측정값으로 예측을 보정" << std::endl;
    std::cout << "   → 불확실성(P)이 감소" << std::endl;
}

void problem2_kalman_gain()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 칼만 게인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 칼만 게인 K가 1에 가까우면 무슨 의미?\n" << std::endl;

    std::cout << "💡 답: 측정값을 거의 100% 신뢰" << std::endl;
    std::cout << "   K = P/(P+R)에서 K→1이면 P >> R" << std::endl;
    std::cout << "   → 예측의 불확실성이 매우 큼" << std::endl;
    std::cout << "   → 측정값에 거의 그대로 따라감" << std::endl;
}

void problem3_covariance()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 공분산의 변화" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 측정 업데이트 없이 예측만 반복하면\n"
              << "      공분산 P는 어떻게 변하나요?\n"
              << std::endl;

    std::cout << "💡 답: P가 계속 증가합니다" << std::endl;
    std::cout << "   P⁻ = F·P·Fᵀ + Q" << std::endl;
    std::cout << "   → 프로세스 노이즈 Q가 매번 누적" << std::endl;
    std::cout << "   → IMU만 사용하면 drift 하는 이유!" << std::endl;
}

void problem4_weighted_average()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 가중 평균" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 예측값 20.0(σ²=9), 측정값 22.0(σ²=1)\n"
              << "      최적 추정값은?\n"
              << std::endl;

    double pred = 20.0, pred_var = 9.0;
    double meas = 22.0, meas_var = 1.0;
    double K = pred_var / (pred_var + meas_var);
    double result = pred + K * (meas - pred);

    std::cout << "💡 답: K = " << K << std::endl;
    std::cout << "   결과 = " << pred << " + " << K << " × (" << meas << " - " << pred << ")"
              << std::endl;
    std::cout << "   결과 = " << result << std::endl;
    std::cout << "   → 측정값(22.0)에 가까움 (더 확실하니까)" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 3 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_predict_update();
    problem2_kalman_gain();
    problem3_covariance();
    problem4_weighted_average();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
