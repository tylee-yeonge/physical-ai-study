/**
 * Phase 4 Week 4 - EKF 기초 퀴즈
 */

#include <iostream>
#include <cmath>

void problem1_why_ekf()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: EKF가 필요한 이유" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 일반 칼만 필터 대신 EKF를 써야 하는\n"
              << "      상황 3가지는?\n"
              << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   1. IMU 적분: R(θ)·a → 회전×가속도 (비선형)" << std::endl;
    std::cout << "   2. 카메라 투영: 3D→2D 나눗셈 (비선형)" << std::endl;
    std::cout << "   3. 쿼터니언 회전: q⊗δq (비선형 곱셈)" << std::endl;
}

void problem2_jacobian_meaning()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 자코비안의 의미" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: EKF에서 자코비안 F의 역할은?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   비선형 함수 f(x)를 현재 추정값 근처에서 선형 근사" << std::endl;
    std::cout << "   F = ∂f/∂x (편미분 행렬)" << std::endl;
    std::cout << "   용도: 공분산 전파 P⁻ = F·P·Fᵀ + Q" << std::endl;
    std::cout << "   주의: 상태 예측은 f(x) 그대로 사용!" << std::endl;
}

void problem3_linearization_error()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 선형화 오차" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: f(x) = x²을 x=3에서 선형화하면?\n"
              << "      x=4에서의 실제값과 근사값의 차이는?\n"
              << std::endl;

    double x0 = 3.0;
    double f_x0 = x0 * x0;
    double df_x0 = 2 * x0;  // f'(x) = 2x

    double x1 = 4.0;
    double f_actual = x1 * x1;
    double f_approx = f_x0 + df_x0 * (x1 - x0);

    std::cout << "💡 답:" << std::endl;
    std::cout << "   f(3) = " << f_x0 << ", f'(3) = " << df_x0 << std::endl;
    std::cout << "   선형 근사: f(4) ≈ " << f_x0 << " + " << df_x0 << "×(4-3) = " << f_approx
              << std::endl;
    std::cout << "   실제값:    f(4) = " << f_actual << std::endl;
    std::cout << "   오차: " << std::abs(f_actual - f_approx) << " (비선형 → 근사 오차)"
              << std::endl;
}

void problem4_ekf_vs_optimization()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: EKF vs 최적화" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: VINS가 EKF 대신 최적화를 쓰는 이유는?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   1. 최적화: 반복 재선형화 → 더 정확" << std::endl;
    std::cout << "   2. 최적화: 여러 프레임 동시 고려 → 일관성" << std::endl;
    std::cout << "   3. EKF: 마르코프 (직전만) → 정보 손실" << std::endl;
    std::cout << "   4. 단, EKF가 더 빠름 → 리소스 제한 시 유리" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 4 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_why_ekf();
    problem2_jacobian_meaning();
    problem3_linearization_error();
    problem4_ekf_vs_optimization();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
