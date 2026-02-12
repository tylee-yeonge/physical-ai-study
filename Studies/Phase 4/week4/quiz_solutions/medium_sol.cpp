/**
 * Phase 4 Week 4 - EKF 중급 퀴즈 풀이
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

void problem1_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: 자코비안 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double theta = M_PI / 4;
    double v = 2.0, dt = 0.1;

    std::cout << "  상태 전이:" << std::endl;
    std::cout << "  px' = px + v·cos(θ)·dt" << std::endl;
    std::cout << "  py' = py + v·sin(θ)·dt" << std::endl;
    std::cout << "  θ'  = θ" << std::endl;
    std::cout << "  v'  = v\n" << std::endl;

    std::cout << "  자코비안 F = ∂f/∂[px, py, θ, v]:\n" << std::endl;
    std::cout << "  F(0,0)=1  F(0,1)=0  F(0,2)=-v·sin(θ)·dt  F(0,3)=cos(θ)·dt" << std::endl;
    std::cout << "  F(1,0)=0  F(1,1)=1  F(1,2)= v·cos(θ)·dt  F(1,3)=sin(θ)·dt" << std::endl;
    std::cout << "  F(2,0)=0  F(2,1)=0  F(2,2)=1             F(2,3)=0" << std::endl;
    std::cout << "  F(3,0)=0  F(3,1)=0  F(3,2)=0             F(3,3)=1\n" << std::endl;

    Eigen::Matrix4d F = Eigen::Matrix4d::Identity();
    F(0, 2) = -v * std::sin(theta) * dt;
    F(0, 3) = std::cos(theta) * dt;
    F(1, 2) = v * std::cos(theta) * dt;
    F(1, 3) = std::sin(theta) * dt;

    std::cout << "  θ=π/4에서 수치값:\n" << F << std::endl;
    std::cout << "\n  핵심: F(0,2), F(1,2)에 sin, cos → 비선형!" << std::endl;
    std::cout << "  → 매 스텝마다 현재 θ에서 재계산 필요" << std::endl;
}

void problem2_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 수치적 자코비안 검증" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double dt = 0.1;
    Eigen::Vector4d x(5.0, 3.0, M_PI / 4, 2.0);

    auto f = [dt](const Eigen::Vector4d& x) -> Eigen::Vector4d
    {
        Eigen::Vector4d xn;
        xn(0) = x(0) + x(3) * std::cos(x(2)) * dt;
        xn(1) = x(1) + x(3) * std::sin(x(2)) * dt;
        xn(2) = x(2);
        xn(3) = x(3);
        return xn;
    };

    // 해석적
    Eigen::Matrix4d F_a = Eigen::Matrix4d::Identity();
    F_a(0, 2) = -x(3) * std::sin(x(2)) * dt;
    F_a(0, 3) = std::cos(x(2)) * dt;
    F_a(1, 2) = x(3) * std::cos(x(2)) * dt;
    F_a(1, 3) = std::sin(x(2)) * dt;

    // 수치적
    double eps = 1e-7;
    Eigen::Vector4d f0 = f(x);
    Eigen::Matrix4d F_n;
    for (int i = 0; i < 4; i++)
    {
        Eigen::Vector4d xp = x;
        xp(i) += eps;
        F_n.col(i) = (f(xp) - f0) / eps;
    }

    double max_diff = (F_a - F_n).cwiseAbs().maxCoeff();

    std::cout << "  해석적 F:\n" << F_a << "\n" << std::endl;
    std::cout << "  수치적 F:\n" << F_n << "\n" << std::endl;
    std::cout << "  최대 차이: " << max_diff << std::endl;
    std::cout << "  → " << (max_diff < 1e-5 ? "✅ 정확!" : "❌ 오류!") << std::endl;
    std::cout << "\n  팁: 자코비안 디버깅에 항상 수치적 검증 사용!" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 공분산 전파" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Eigen::Matrix2d P, F, Q;
    P << 1, 0, 0, 1;
    F << 1, 0.1, 0, 1;
    Q << 0.01, 0, 0, 0.01;

    std::cout << "  Step 1: F·P" << std::endl;
    Eigen::Matrix2d FP = F * P;
    std::cout << FP << "\n" << std::endl;

    std::cout << "  Step 2: F·P·Fᵀ" << std::endl;
    Eigen::Matrix2d FPFt = FP * F.transpose();
    std::cout << FPFt << "\n" << std::endl;

    std::cout << "  Step 3: P⁻ = F·P·Fᵀ + Q" << std::endl;
    Eigen::Matrix2d P_pred = FPFt + Q;
    std::cout << P_pred << "\n" << std::endl;

    std::cout << "  관찰:" << std::endl;
    std::cout << "  - P(0,0): 1.0 → " << P_pred(0, 0) << " (위치 불확실성 증가)" << std::endl;
    std::cout << "  - P(1,1): 1.0 → " << P_pred(1, 1) << " (속도 불확실성 증가)" << std::endl;
    std::cout << "  - P(0,1): 0.0 → " << P_pred(0, 1) << " (상관관계 생김!)" << std::endl;
    std::cout << "  → 위치가 속도에 의존하므로 상관관계 발생" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 4 Quiz Medium - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
