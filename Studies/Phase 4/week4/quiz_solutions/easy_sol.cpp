/**
 * Phase 4 Week 4 - EKF 기초 퀴즈 풀이
 */

#include <iostream>
#include <cmath>

void problem1_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: EKF가 필요한 이유" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "선형 칼만 필터: x_k = F·x_{k-1} (행렬 곱 = 선형)" << std::endl;
    std::cout << "실제 시스템: x_k = f(x_{k-1}) (비선형 함수)\n" << std::endl;

    std::cout << "비선형 상황 3가지:" << std::endl;
    std::cout << "  1. IMU 적분: a_world = R(θ)·a_body" << std::endl;
    std::cout << "     → R(θ)가 sin/cos 포함 → 비선형" << std::endl;
    std::cout << "  2. 카메라 투영: u = fx·X/Z + cx" << std::endl;
    std::cout << "     → 나눗셈(X/Z) → 비선형" << std::endl;
    std::cout << "  3. 쿼터니언 곱: q_new = q ⊗ δq" << std::endl;
    std::cout << "     → 쿼터니언 곱셈 → 비선형" << std::endl;
}

void problem2_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 자코비안의 역할" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "자코비안 F = ∂f/∂x의 역할:\n" << std::endl;
    std::cout << "  1. 비선형 f(x)를 현재점에서 선형 근사" << std::endl;
    std::cout << "     f(x) ≈ f(x̂) + F·(x - x̂)" << std::endl;
    std::cout << "  2. 공분산 전파에 사용" << std::endl;
    std::cout << "     P⁻ = F·P·Fᵀ + Q" << std::endl;
    std::cout << "  3. 상태 예측은 f(x) 직접 사용" << std::endl;
    std::cout << "     x̂⁻ = f(x̂) (자코비안 아님!)\n" << std::endl;

    std::cout << "핵심 구분:" << std::endl;
    std::cout << "  상태 예측: f(x) 사용 (비선형 그대로)" << std::endl;
    std::cout << "  공분산 예측: F 사용 (선형 근사)" << std::endl;
}

void problem3_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 선형화 오차" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double x0 = 3.0, x1 = 4.0;
    double f_actual = x1 * x1;          // 16
    double f_approx = x0*x0 + 2*x0*(x1-x0);  // 9 + 6 = 15

    std::cout << "  f(x) = x², f'(x) = 2x" << std::endl;
    std::cout << "  x₀=3에서: f(3)=9, f'(3)=6" << std::endl;
    std::cout << "  선형 근사: f(4) ≈ 9 + 6·(4-3) = 15" << std::endl;
    std::cout << "  실제값:    f(4) = 16" << std::endl;
    std::cout << "  오차: 1 (= 2차 항 (x-x₀)² = 1)\n" << std::endl;

    std::cout << "  교훈:" << std::endl;
    std::cout << "  - x₀에서 멀어질수록 오차 증가" << std::endl;
    std::cout << "  - 비선형이 강할수록 오차 큼" << std::endl;
    std::cout << "  - EKF는 매 스텝 재선형화로 오차 최소화" << std::endl;
}

void problem4_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 풀이: EKF vs 최적화" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "EKF:" << std::endl;
    std::cout << "  + 빠름 (O(n²) per step)" << std::endl;
    std::cout << "  + 메모리 적음 (현재 상태만)" << std::endl;
    std::cout << "  - 선형화 오차 누적" << std::endl;
    std::cout << "  - 마르코프 (직전만 사용, 정보 손실)\n" << std::endl;

    std::cout << "최적화 (VINS):" << std::endl;
    std::cout << "  + 반복 재선형화 → 더 정확" << std::endl;
    std::cout << "  + 여러 프레임 동시 고려" << std::endl;
    std::cout << "  + Loop Closure 자연스럽게 통합" << std::endl;
    std::cout << "  - 계산량 많음" << std::endl;
    std::cout << "  - 메모리 많음 (윈도우 내 모든 상태)\n" << std::endl;

    std::cout << "실무 선택:" << std::endl;
    std::cout << "  리소스 제한 (임베디드): EKF/ESKF" << std::endl;
    std::cout << "  정확도 우선 (PC/Jetson): 최적화 기반" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 4 Quiz Easy - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
