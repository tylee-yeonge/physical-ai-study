/**
 * Phase 4 Week 4 - EKF 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

void problem1_jacobian_computation() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 자코비안 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "상태: x = [px, py, θ, v]" << std::endl;
    std::cout << "전이: px' = px + v·cosθ·dt" << std::endl;
    std::cout << "      py' = py + v·sinθ·dt" << std::endl;
    std::cout << "θ=π/4, v=2.0, dt=0.1 에서 자코비안 F를 구하시오.\n" << std::endl;

    double theta = M_PI / 4;
    double v = 2.0;
    double dt = 0.1;

    Eigen::Matrix4d F = Eigen::Matrix4d::Identity();
    F(0, 2) = -v * std::sin(theta) * dt;
    F(0, 3) = std::cos(theta) * dt;
    F(1, 2) = v * std::cos(theta) * dt;
    F(1, 3) = std::sin(theta) * dt;

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "   F(0,2) = -v·sin(θ)·dt = -" << v << "×" << std::sin(theta) << "×" << dt
              << " = " << F(0,2) << std::endl;
    std::cout << "   F(0,3) = cos(θ)·dt = " << std::cos(theta) << "×" << dt
              << " = " << F(0,3) << std::endl;
    std::cout << "   F(1,2) = v·cos(θ)·dt = " << v << "×" << std::cos(theta) << "×" << dt
              << " = " << F(1,2) << std::endl;
    std::cout << "   F(1,3) = sin(θ)·dt = " << std::sin(theta) << "×" << dt
              << " = " << F(1,3) << std::endl;

    std::cout << "\n   F =\n" << F << std::endl;
}

void problem2_numerical_jacobian() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 수치적 자코비안 검증" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "해석적 자코비안과 수치적 자코비안을 비교\n" << std::endl;

    double dt = 0.1;
    Eigen::Vector4d x(5.0, 3.0, M_PI/4, 2.0);

    // 비선형 함수
    auto f = [dt](const Eigen::Vector4d& x) -> Eigen::Vector4d {
        Eigen::Vector4d xn;
        xn(0) = x(0) + x(3) * std::cos(x(2)) * dt;
        xn(1) = x(1) + x(3) * std::sin(x(2)) * dt;
        xn(2) = x(2);
        xn(3) = x(3);
        return xn;
    };

    // 해석적 자코비안
    Eigen::Matrix4d F_analytic = Eigen::Matrix4d::Identity();
    F_analytic(0, 2) = -x(3) * std::sin(x(2)) * dt;
    F_analytic(0, 3) = std::cos(x(2)) * dt;
    F_analytic(1, 2) = x(3) * std::cos(x(2)) * dt;
    F_analytic(1, 3) = std::sin(x(2)) * dt;

    // 수치적 자코비안
    double eps = 1e-7;
    Eigen::Vector4d f0 = f(x);
    Eigen::Matrix4d F_numeric;

    for (int i = 0; i < 4; i++) {
        Eigen::Vector4d x_plus = x;
        x_plus(i) += eps;
        F_numeric.col(i) = (f(x_plus) - f0) / eps;
    }

    Eigen::Matrix4d diff = F_analytic - F_numeric;

    std::cout << "💡 결과:" << std::endl;
    std::cout << "   최대 차이: " << diff.cwiseAbs().maxCoeff() << std::endl;
    std::cout << "   → 1e-7 이하면 해석적 자코비안이 정확!" << std::endl;
    std::cout << "\n   이 방법으로 복잡한 자코비안의 버그를 찾을 수 있습니다." << std::endl;
}

void problem3_covariance_propagation() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 공분산 전파" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "P = [[1,0],[0,1]], F = [[1,0.1],[0,1]], Q = [[0.01,0],[0,0.01]]" << std::endl;
    std::cout << "예측 후 P⁻를 구하시오.\n" << std::endl;

    Eigen::Matrix2d P, F, Q;
    P << 1, 0, 0, 1;
    F << 1, 0.1, 0, 1;
    Q << 0.01, 0, 0, 0.01;

    Eigen::Matrix2d P_pred = F * P * F.transpose() + Q;

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "   F·P = \n" << F * P << "\n" << std::endl;
    std::cout << "   F·P·Fᵀ = \n" << F * P * F.transpose() << "\n" << std::endl;
    std::cout << "   P⁻ = F·P·Fᵀ + Q = \n" << P_pred << std::endl;
    std::cout << "\n   관찰: P(0,0)이 1→1.02로 증가 (불확실성 증가)" << std::endl;
    std::cout << "   P(0,1)=0.1 → 위치-속도 상관관계 생김!" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 4 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_jacobian_computation();
    problem2_numerical_jacobian();
    problem3_covariance_propagation();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
