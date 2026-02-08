/**
 * Phase 4 Week 9 - Pre-integration 심화 중급 퀴즈 풀이
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

void problem1_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: 공분산 전파 1스텝" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double dt = 0.005;
    double sigma_acc = 0.1, sigma_gyro = 0.01;
    Eigen::Vector3d acc(0, 0, 9.81);

    std::cout << "  Σ' = A·0·A^T + B·Q·B^T = B·Q·B^T\n" << std::endl;

    std::cout << "  B 행렬 (9×6):" << std::endl;
    std::cout << "    B = | -I·dt    0    |" << std::endl;
    std::cout << "        |  0     -ΔR·dt |" << std::endl;
    std::cout << "        |  0       0    |\n" << std::endl;

    std::cout << "  Q 행렬 (6×6):" << std::endl;
    std::cout << "    Q = | σ²_g/dt · I    0         |" << std::endl;
    std::cout << "        |    0      σ²_a/dt · I   |" << std::endl;
    std::cout << "    σ²_g/dt = " << sigma_gyro*sigma_gyro/dt << std::endl;
    std::cout << "    σ²_a/dt = " << sigma_acc*sigma_acc/dt << "\n" << std::endl;

    // B·Q·B^T 계산
    Eigen::Matrix<double, 9, 6> B = Eigen::Matrix<double, 9, 6>::Zero();
    B.block<3,3>(0,0) = -Eigen::Matrix3d::Identity() * dt;
    B.block<3,3>(3,3) = -Eigen::Matrix3d::Identity() * dt;

    Eigen::Matrix<double, 6, 6> Q = Eigen::Matrix<double, 6, 6>::Zero();
    Q.block<3,3>(0,0) = Eigen::Matrix3d::Identity() * sigma_gyro * sigma_gyro / dt;
    Q.block<3,3>(3,3) = Eigen::Matrix3d::Identity() * sigma_acc * sigma_acc / dt;

    Eigen::Matrix<double, 9, 9> BQBt = B * Q * B.transpose();

    std::cout << "  B·Q·B^T 결과 (대각만):" << std::endl;
    std::cout << "    회전: " << BQBt(0,0) << " = dt·σ²_g = " << dt*sigma_gyro*sigma_gyro << std::endl;
    std::cout << "    속도: " << BQBt(3,3) << " = dt·σ²_a = " << dt*sigma_acc*sigma_acc << std::endl;
    std::cout << "    위치: " << BQBt(6,6) << " = 0 (1스텝으로는 위치 불확실성 없음)\n" << std::endl;

    std::cout << "  √Σ:" << std::endl;
    std::cout << "    σ_rotation = " << std::sqrt(BQBt(0,0)) << " rad" << std::endl;
    std::cout << "    σ_velocity = " << std::sqrt(BQBt(3,3)) << " m/s" << std::endl;
    std::cout << "    σ_position = 0 m (다음 스텝부터 증가)" << std::endl;
}

void problem2_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 적분 시간에 따른 공분산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double dt = 0.005;
    double sigma_acc = 0.1, sigma_gyro = 0.01;
    Eigen::Vector3d acc(0, 0, 9.81);

    auto run = [&](int steps) {
        Eigen::Matrix<double, 9, 9> S = Eigen::Matrix<double, 9, 9>::Zero();
        for (int i = 0; i < steps; i++) {
            Eigen::Matrix<double, 9, 9> A = Eigen::Matrix<double, 9, 9>::Identity();
            A.block<3,3>(3,0) = -skew(acc) * dt;
            A.block<3,3>(6,3) = Eigen::Matrix3d::Identity() * dt;

            Eigen::Matrix<double, 9, 6> B = Eigen::Matrix<double, 9, 6>::Zero();
            B.block<3,3>(0,0) = -Eigen::Matrix3d::Identity() * dt;
            B.block<3,3>(3,3) = -Eigen::Matrix3d::Identity() * dt;

            Eigen::Matrix<double, 6, 6> Q = Eigen::Matrix<double, 6, 6>::Zero();
            Q.block<3,3>(0,0) = Eigen::Matrix3d::Identity() * sigma_gyro * sigma_gyro / dt;
            Q.block<3,3>(3,3) = Eigen::Matrix3d::Identity() * sigma_acc * sigma_acc / dt;

            S = A * S * A.transpose() + B * Q * B.transpose();
        }
        return S;
    };

    auto S01 = run(20);   auto S10 = run(200);

    std::cout << "  0.1초 적분:" << std::endl;
    std::cout << "    σ_position = " << std::sqrt(S01(6,6)) << " m\n" << std::endl;
    std::cout << "  1.0초 적분:" << std::endl;
    std::cout << "    σ_position = " << std::sqrt(S10(6,6)) << " m\n" << std::endl;

    std::cout << "  비율: " << std::sqrt(S10(6,6)) / std::sqrt(S01(6,6)) << std::endl;
    std::cout << "  이론값 (10^{1.5}): " << std::pow(10, 1.5) << "\n" << std::endl;

    std::cout << "  → 위치 불확실성이 t^{3/2}에 비례 확인!" << std::endl;
}

void problem3_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: Factor 가중치 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double r = 0.05;
    double s2_A = 0.0001, s2_B = 0.01;

    std::cout << "  Factor A (0.1초): E = r²/σ² = " << r*r << "/" << s2_A
              << " = " << r*r/s2_A << std::endl;
    std::cout << "  Factor B (1.0초): E = r²/σ² = " << r*r << "/" << s2_B
              << " = " << r*r/s2_B << "\n" << std::endl;

    std::cout << "  Factor A 비용 / Factor B 비용 = " << (r*r/s2_A) / (r*r/s2_B) << "배\n" << std::endl;

    std::cout << "  실무적 의미:" << std::endl;
    std::cout << "    같은 0.05m 오차라도:" << std::endl;
    std::cout << "    - 0.1초 적분에서는 '큰 문제' (σ=0.01m 대비)" << std::endl;
    std::cout << "    - 1.0초 적분에서는 '무시 가능' (σ=0.1m 대비)" << std::endl;
    std::cout << "    → 최적화가 자동으로 신뢰도를 조절!" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 9 Quiz Medium - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
