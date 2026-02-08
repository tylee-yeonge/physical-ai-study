/**
 * Phase 4 Week 9 - Pre-integration 심화 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
    Eigen::Matrix3d m;
    m <<    0, -v.z(),  v.y(),
         v.z(),     0, -v.x(),
        -v.y(),  v.x(),     0;
    return m;
}

void problem1_covariance_propagation() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 공분산 전파 1스텝" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "초기 Σ = 0 (9×9), dt = 0.005s" << std::endl;
    std::cout << "σ_acc = 0.1, σ_gyro = 0.01" << std::endl;
    std::cout << "ΔR = I, acc = [0,0,9.81], gyro = [0,0,0]\n" << std::endl;
    std::cout << "1스텝 후 Σ의 대각 원소를 구하시오.\n" << std::endl;

    double dt = 0.005;
    double sigma_acc = 0.1;
    double sigma_gyro = 0.01;

    Eigen::Matrix3d delta_R = Eigen::Matrix3d::Identity();
    Eigen::Vector3d acc(0, 0, 9.81);
    Eigen::Vector3d gyro(0, 0, 0);

    // A 행렬
    Eigen::Matrix<double, 9, 9> A = Eigen::Matrix<double, 9, 9>::Identity();
    A.block<3,3>(0,0) += -skew(gyro) * dt;  // = I (gyro=0)
    A.block<3,3>(3,0) = -delta_R * skew(acc) * dt;
    A.block<3,3>(6,3) = Eigen::Matrix3d::Identity() * dt;

    // B 행렬
    Eigen::Matrix<double, 9, 6> B = Eigen::Matrix<double, 9, 6>::Zero();
    B.block<3,3>(0,0) = -Eigen::Matrix3d::Identity() * dt;
    B.block<3,3>(3,3) = -delta_R * dt;

    // Q 행렬
    Eigen::Matrix<double, 6, 6> Q = Eigen::Matrix<double, 6, 6>::Zero();
    Q.block<3,3>(0,0) = Eigen::Matrix3d::Identity() * sigma_gyro * sigma_gyro / dt;
    Q.block<3,3>(3,3) = Eigen::Matrix3d::Identity() * sigma_acc * sigma_acc / dt;

    // 초기 Σ = 0
    Eigen::Matrix<double, 9, 9> Sigma = Eigen::Matrix<double, 9, 9>::Zero();

    // 1스텝 전파: Σ = A·0·A^T + B·Q·B^T = B·Q·B^T
    Sigma = A * Sigma * A.transpose() + B * Q * B.transpose();

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  초기 Σ=0이므로: Σ' = B·Q·B^T\n" << std::endl;
    std::cout << "  B·Q·B^T의 대각:" << std::endl;
    std::cout << "    σ²_rotation = dt² · σ²_gyro/dt = dt·σ²_gyro" << std::endl;
    std::cout << "                = " << dt * sigma_gyro * sigma_gyro << std::endl;
    std::cout << "    σ²_velocity = dt² · σ²_acc/dt = dt·σ²_acc" << std::endl;
    std::cout << "                = " << dt * sigma_acc * sigma_acc << std::endl;
    std::cout << "    σ²_position = 0 (속도 불확실성이 아직 적분되지 않음)\n" << std::endl;

    std::cout << "  Σ 대각:\n";
    for (int i = 0; i < 9; i++) {
        std::cout << "    Σ(" << i << "," << i << ") = " << Sigma(i,i) << std::endl;
    }
    std::cout << "\n  √Σ(0,0) = σ_rotation = " << std::sqrt(Sigma(0,0)) << " rad" << std::endl;
    std::cout << "  √Σ(3,3) = σ_velocity = " << std::sqrt(Sigma(3,3)) << " m/s" << std::endl;
}

void problem2_time_dependence() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 적분 시간에 따른 공분산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "0.1초 vs 1.0초 적분의 위치 불확실성 비를 구하시오.\n" << std::endl;

    double dt = 0.005;
    double sigma_acc = 0.1;
    double sigma_gyro = 0.01;

    auto runIntegration = [&](int steps) -> double {
        Eigen::Matrix3d delta_R = Eigen::Matrix3d::Identity();
        Eigen::Vector3d acc(0, 0, 9.81);
        Eigen::Vector3d gyro(0, 0, 0);
        Eigen::Matrix<double, 9, 9> Sigma = Eigen::Matrix<double, 9, 9>::Zero();

        for (int i = 0; i < steps; i++) {
            Eigen::Matrix<double, 9, 9> A = Eigen::Matrix<double, 9, 9>::Identity();
            A.block<3,3>(3,0) = -delta_R * skew(acc) * dt;
            A.block<3,3>(6,3) = Eigen::Matrix3d::Identity() * dt;

            Eigen::Matrix<double, 9, 6> B = Eigen::Matrix<double, 9, 6>::Zero();
            B.block<3,3>(0,0) = -Eigen::Matrix3d::Identity() * dt;
            B.block<3,3>(3,3) = -delta_R * dt;

            Eigen::Matrix<double, 6, 6> Q = Eigen::Matrix<double, 6, 6>::Zero();
            Q.block<3,3>(0,0) = Eigen::Matrix3d::Identity() * sigma_gyro * sigma_gyro / dt;
            Q.block<3,3>(3,3) = Eigen::Matrix3d::Identity() * sigma_acc * sigma_acc / dt;

            Sigma = A * Sigma * A.transpose() + B * Q * B.transpose();
        }
        return std::sqrt(Sigma(6,6));  // σ_position_x
    };

    double sigma_01 = runIntegration(20);    // 0.1초
    double sigma_05 = runIntegration(100);   // 0.5초
    double sigma_10 = runIntegration(200);   // 1.0초
    double sigma_20 = runIntegration(400);   // 2.0초

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  t=0.1s: σ_p = " << sigma_01 << " m" << std::endl;
    std::cout << "  t=0.5s: σ_p = " << sigma_05 << " m" << std::endl;
    std::cout << "  t=1.0s: σ_p = " << sigma_10 << " m" << std::endl;
    std::cout << "  t=2.0s: σ_p = " << sigma_20 << " m\n" << std::endl;

    std::cout << "  비율:" << std::endl;
    std::cout << "    σ_p(1.0) / σ_p(0.1) = " << sigma_10 / sigma_01 << std::endl;
    std::cout << "    이론적 (t^{3/2} 기준): " << std::pow(10, 1.5) << std::endl;
    std::cout << "    σ_p(2.0) / σ_p(1.0) = " << sigma_20 / sigma_10 << std::endl;
    std::cout << "    이론적 (t^{3/2} 기준): " << std::pow(2, 1.5) << "\n" << std::endl;

    std::cout << "  → 위치 불확실성은 t^{3/2}에 비례!" << std::endl;
    std::cout << "  → 키프레임 간격이 2배 → 불확실성 2.83배!" << std::endl;
}

void problem3_weight_comparison() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Factor 가중치 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "두 IMU Factor의 공분산 (위치 부분만, 1D 단순화):\n" << std::endl;
    std::cout << "  Factor A (0.1초 적분): σ²_p = 0.0001 m²" << std::endl;
    std::cout << "  Factor B (1.0초 적분): σ²_p = 0.01 m²\n" << std::endl;
    std::cout << "같은 크기의 잔차 r=0.05m가 있을 때," << std::endl;
    std::cout << "각 Factor의 비용(마하라노비스 거리)을 비교하시오.\n" << std::endl;

    double sigma2_A = 0.0001;
    double sigma2_B = 0.01;
    double r = 0.05;

    double cost_A = r * r / sigma2_A;
    double cost_B = r * r / sigma2_B;

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  Factor A: E = r²/σ² = " << r*r << "/" << sigma2_A << " = " << cost_A << std::endl;
    std::cout << "  Factor B: E = r²/σ² = " << r*r << "/" << sigma2_B << " = " << cost_B << "\n" << std::endl;

    std::cout << "  Factor A의 비용이 " << cost_A / cost_B << "배 높음!" << std::endl;
    std::cout << "\n  의미:" << std::endl;
    std::cout << "    Factor A (짧은 적분): 0.05m 오차는 매우 큰 것 → 강하게 보정" << std::endl;
    std::cout << "    Factor B (긴 적분): 0.05m 오차는 무시할 수준 → 약하게 보정" << std::endl;
    std::cout << "    → 정확한 Factor에 더 의존하는 합리적 최적화!" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 9 Quiz - Medium (공분산 전파 계산)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_covariance_propagation();
    problem2_time_dependence();
    problem3_weight_comparison();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
