/**
 * Phase 4 Week 10 - IMU 적분 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <random>

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
    Eigen::Matrix3d m;
    m <<    0, -v.z(),  v.y(),
         v.z(),     0, -v.x(),
        -v.y(),  v.x(),     0;
    return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& omega) {
    double angle = omega.norm();
    if (angle < 1e-10) return Eigen::Matrix3d::Identity();
    Eigen::Vector3d axis = omega / angle;
    Eigen::Matrix3d K = skew(axis);
    return Eigen::Matrix3d::Identity()
         + std::sin(angle) * K + (1.0 - std::cos(angle)) * K * K;
}

void problem1_gravity_leak() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 중력 누출 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "자이로 바이어스 b_g = [0, 0.005, 0] rad/s (y축)" << std::endl;
    std::cout << "2초 후 회전 오차와 그로 인한 가짜 수평 가속도를 구하시오.\n" << std::endl;

    double b_g_y = 0.005;  // rad/s
    double t = 2.0;
    double g = 9.81;

    // 회전 오차
    double angle_error = b_g_y * t;  // 0.01 rad

    // 중력 누출: sin(angle_error) × g
    double fake_accel = std::sin(angle_error) * g;

    // 2초간의 위치 드리프트
    double p_drift = 0.5 * fake_accel * t * t;

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  회전 오차: " << b_g_y << " × " << t << " = " << angle_error << " rad" << std::endl;
    std::cout << "  (≈ " << angle_error * 180 / M_PI << "도)\n" << std::endl;

    std::cout << "  중력 누출 (가짜 수평 가속도):" << std::endl;
    std::cout << "    sin(" << angle_error << ") × " << g << " = " << fake_accel << " m/s²\n" << std::endl;

    std::cout << "  이 가짜 가속도가 2초간 적분되면:" << std::endl;
    std::cout << "    p = 0.5 × " << fake_accel << " × " << t*t << " = " << p_drift << " m\n" << std::endl;

    std::cout << "  핵심: 0.005 rad/s의 작은 바이어스가" << std::endl;
    std::cout << "  → 2초 만에 " << p_drift << "m 위치 오차를 만듦!" << std::endl;
    std::cout << "  → 자이로 바이어스가 가장 치명적인 이유" << std::endl;
}

void problem2_noise_vs_bias_drift() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 노이즈 vs 바이어스 드리프트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "1D 가속도 적분에서:" << std::endl;
    std::cout << "  (가) 노이즈만: σ_a = 0.1 m/s², dt = 0.005s" << std::endl;
    std::cout << "  (나) 바이어스만: b_a = 0.01 m/s²" << std::endl;
    std::cout << "10초 후 각각의 위치 드리프트를 비교하시오.\n" << std::endl;

    double sigma_a = 0.1;
    double b_a = 0.01;
    double dt = 0.005;
    double T = 10.0;
    int N = T / dt;

    // 노이즈 드리프트 (이론적): σ_p ≈ σ_a · dt · √N · T / √6
    // 간단 추정: 시뮬레이션으로 확인
    std::default_random_engine gen(42);
    std::normal_distribution<double> noise(0, sigma_a);

    // 시뮬레이션 (가) 노이즈만
    double v_noise = 0, p_noise = 0;
    for (int i = 0; i < N; i++) {
        double a = noise(gen);
        v_noise += a * dt;
        p_noise += v_noise * dt;
    }

    // 시뮬레이션 (나) 바이어스만
    double v_bias = 0, p_bias = 0;
    for (int i = 0; i < N; i++) {
        v_bias += b_a * dt;
        p_bias += v_bias * dt;
    }

    // 이론값
    double p_bias_theory = 0.5 * b_a * T * T;

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  (가) 노이즈 드리프트 (10초 후):" << std::endl;
    std::cout << "    시뮬레이션: |p| = " << std::abs(p_noise) << " m" << std::endl;
    std::cout << "    (랜덤이므로 실행마다 다름, ±1σ 범위)\n" << std::endl;

    std::cout << "  (나) 바이어스 드리프트 (10초 후):" << std::endl;
    std::cout << "    시뮬레이션: p = " << p_bias << " m" << std::endl;
    std::cout << "    이론값: 0.5 × " << b_a << " × " << T*T << " = " << p_bias_theory << " m\n" << std::endl;

    std::cout << "  비교:" << std::endl;
    std::cout << "    바이어스가 노이즈보다 " << std::abs(p_bias / p_noise) << "배 큰 드리프트!" << std::endl;
    std::cout << "    → 바이어스: t²에 비례 (확정적, 항상 같은 방향)" << std::endl;
    std::cout << "    → 노이즈: t^{3/2}에 비례 (확률적, 방향 무작위)" << std::endl;
}

void problem3_integration_comparison() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 직접 적분 vs Pre-integration" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "최적화에서 포즈 R_i가 수정되었습니다.\n" << std::endl;
    std::cout << "직접 적분: IMU 200개를 다시 적분해야 함" << std::endl;
    std::cout << "Pre-integration: 재적분 불필요 (Δp는 R_i와 무관)\n" << std::endl;
    std::cout << "VINS에서 10 키프레임, 각 간격에 IMU 40개:" << std::endl;
    std::cout << "  총 IMU = 9 × 40 = 360개\n" << std::endl;
    std::cout << "최적화 10회 반복 시 연산량을 비교하시오.\n" << std::endl;

    int keyframes = 10;
    int imu_per_interval = 40;
    int total_imu = (keyframes - 1) * imu_per_interval;
    int iterations = 10;

    int direct_ops = total_imu * iterations;
    int preint_ops = total_imu;  // 최초 1회만

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  직접 적분:" << std::endl;
    std::cout << "    매 반복마다 " << total_imu << "개 재적분" << std::endl;
    std::cout << "    총: " << total_imu << " × " << iterations << " = " << direct_ops << " 적분 연산\n" << std::endl;

    std::cout << "  Pre-integration:" << std::endl;
    std::cout << "    최초 1회: " << total_imu << " 적분 연산" << std::endl;
    std::cout << "    이후: 잔차 계산만 (행렬 곱 수회)" << std::endl;
    std::cout << "    바이어스 보정: 자코비안 곱 (무시할 수준)\n" << std::endl;

    std::cout << "  효율: " << (double)direct_ops / preint_ops << "배 차이!" << std::endl;
    std::cout << "  → Pre-integration이 실시간 VIO를 가능하게 함" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 10 Quiz - Medium (IMU 적분 계산)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_gravity_leak();
    problem2_noise_vs_bias_drift();
    problem3_integration_comparison();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
