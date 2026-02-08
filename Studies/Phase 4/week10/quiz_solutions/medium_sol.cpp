/**
 * Phase 4 Week 10 - IMU 적분 중급 퀴즈 풀이
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <random>

void problem1_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: 중력 누출 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double b_g = 0.005;  // rad/s
    double t = 2.0;
    double g = 9.81;

    double angle_error = b_g * t;
    double fake_accel = std::sin(angle_error) * g;
    double p_drift = 0.5 * fake_accel * t * t;

    std::cout << "  Step 1: 회전 오차 누적" << std::endl;
    std::cout << "    δθ = b_g × t = " << b_g << " × " << t << " = " << angle_error << " rad" << std::endl;
    std::cout << "    = " << angle_error * 180 / M_PI << "도\n" << std::endl;

    std::cout << "  Step 2: 중력 누출" << std::endl;
    std::cout << "    중력이 수평으로 누출되는 양:" << std::endl;
    std::cout << "    a_fake = sin(δθ) × g" << std::endl;
    std::cout << "           = sin(" << angle_error << ") × " << g << std::endl;
    std::cout << "           = " << fake_accel << " m/s²\n" << std::endl;

    std::cout << "  Step 3: 위치 드리프트 (2초간)" << std::endl;
    std::cout << "    p = 0.5 × a_fake × t²" << std::endl;
    std::cout << "    = 0.5 × " << fake_accel << " × " << t*t << std::endl;
    std::cout << "    = " << p_drift << " m\n" << std::endl;

    std::cout << "  더 긴 시간:" << std::endl;
    for (double ti : {1.0, 2.0, 5.0, 10.0}) {
        double ae = b_g * ti;
        double fa = std::sin(ae) * g;
        double pd = 0.5 * fa * ti * ti;
        printf("    t=%4.1fs: δθ=%6.4f rad, a_fake=%5.3f m/s², p=%6.3f m\n",
               ti, ae, fa, pd);
    }
    std::cout << "\n  → 5초 만에 미터 단위 오차!" << std::endl;
    std::cout << "  → 자이로 바이어스 0.005 rad/s는 '매우 작은' 값인데도!" << std::endl;
}

void problem2_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 노이즈 vs 바이어스 드리프트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double sigma_a = 0.1;
    double b_a = 0.01;
    double dt = 0.005;
    double T = 10.0;

    // 바이어스 드리프트 (확정적)
    double p_bias = 0.5 * b_a * T * T;

    // 노이즈 드리프트 (확률적, RMS)
    // σ_p ≈ σ_a × dt × √(N) × T / √6 (근사)
    // 정확한 공식: σ_p = σ_a × √(dt) × T^{3/2} / √3
    int N = T / dt;
    double sigma_p = sigma_a * std::sqrt(dt) * std::pow(T, 1.5) / std::sqrt(3.0);

    std::cout << "  (가) 노이즈 드리프트 (10초 후):" << std::endl;
    std::cout << "    이론적 σ_p = σ_a × √dt × T^{3/2} / √3" << std::endl;
    std::cout << "                = " << sigma_a << " × " << std::sqrt(dt) << " × " << std::pow(T, 1.5) << " / " << std::sqrt(3.0) << std::endl;
    std::cout << "                ≈ " << sigma_p << " m (1σ)\n" << std::endl;

    std::cout << "  (나) 바이어스 드리프트 (10초 후):" << std::endl;
    std::cout << "    p = 0.5 × b_a × t² = 0.5 × " << b_a << " × " << T*T << std::endl;
    std::cout << "    = " << p_bias << " m (확정적)\n" << std::endl;

    std::cout << "  비교:" << std::endl;
    std::cout << "    노이즈 (1σ): " << sigma_p << " m" << std::endl;
    std::cout << "    바이어스:     " << p_bias << " m" << std::endl;
    std::cout << "    → 바이어스가 약 " << p_bias / sigma_p << "배\n" << std::endl;

    std::cout << "  핵심 차이:" << std::endl;
    std::cout << "    노이즈: 랜덤, ∝ t^{3/2}, 때로 상쇄" << std::endl;
    std::cout << "    바이어스: 확정적, ∝ t², 항상 같은 방향으로 축적" << std::endl;
    std::cout << "    → 바이어스 추정이 VIO에서 매우 중요한 이유!" << std::endl;
}

void problem3_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 직접 적분 vs Pre-integration" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    int kf = 10, imu_per = 40, iter = 10;
    int total_imu = (kf - 1) * imu_per;

    std::cout << "  직접 적분:" << std::endl;
    std::cout << "    매 최적화 반복마다 모든 IMU 재적분" << std::endl;
    std::cout << "    = " << total_imu << " × " << iter << " = " << total_imu * iter << " 적분\n" << std::endl;

    std::cout << "  Pre-integration:" << std::endl;
    std::cout << "    최초 1회: " << total_imu << " 적분" << std::endl;
    std::cout << "    이후 반복: 잔차 계산만 (9 intervals × 행렬 곱)" << std::endl;
    std::cout << "    바이어스 변경: 자코비안 보정 (9 × 행렬·벡터 곱)\n" << std::endl;

    std::cout << "  연산량 비교:" << std::endl;
    std::cout << "    직접: " << total_imu * iter << " 적분 연산" << std::endl;
    std::cout << "    Pre-int: " << total_imu << " + " << (kf-1)*iter << " ≈ " << total_imu + (kf-1)*iter << " 연산\n" << std::endl;

    std::cout << "    효율: 약 " << (double)(total_imu * iter) / (total_imu + (kf-1)*iter) << "배\n" << std::endl;

    std::cout << "  실제 시스템에서:" << std::endl;
    std::cout << "    IMU 200Hz, Vision 20Hz → interval당 10 IMU" << std::endl;
    std::cout << "    Window 10 KF, 최적화 8회 반복:" << std::endl;
    int real_imu = 9 * 10;
    int real_direct = real_imu * 8;
    int real_preint = real_imu + 9 * 8;
    std::cout << "    직접: " << real_direct << " vs Pre-int: " << real_preint << std::endl;
    std::cout << "    → 약 " << (double)real_direct / real_preint << "배 효율" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 10 Quiz Medium - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
