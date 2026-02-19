#include "scale_recovery.h"
#include <iostream>
#include <cmath>
#include <random>

// ====================
// ScaleRecovery 구현
// ====================

double ScaleRecovery::compute_stereo_depth(double f, double baseline, double disparity)
{
    if (disparity <= 0) return -1.0;
    return f * baseline / disparity;
}

void ScaleRecovery::simulate_imu(const std::vector<double>& accel, double dt,
                                 std::vector<double>& positions, std::vector<double>& velocities)
{
    positions.clear();
    velocities.clear();
    positions.push_back(0.0);
    velocities.push_back(0.0);

    for (size_t i = 0; i < accel.size(); i++)
    {
        double v_new = velocities.back() + accel[i] * dt;
        double p_new = positions.back() + velocities.back() * dt + 0.5 * accel[i] * dt * dt;
        velocities.push_back(v_new);
        positions.push_back(p_new);
    }
}

void ScaleRecovery::compare_scale_methods(double true_distance, double mono_scale_noise,
                                          double stereo_noise, double imu_bias, int n_frames)
{
    std::default_random_engine gen(42);
    double step = true_distance / n_frames;

    // Mono: 매 프레임 스케일 노이즈 누적
    std::normal_distribution<> mono_noise(1.0, mono_scale_noise);
    double mono_pos = 0;
    for (int i = 0; i < n_frames; i++) mono_pos += step * mono_noise(gen);

    // Stereo: 절대 깊이 기반 (노이즈 작음)
    std::normal_distribution<> st_noise(0, stereo_noise);
    double stereo_pos = true_distance + st_noise(gen);

    // IMU: 바이어스 누적
    double dt = 0.1;
    double accel = true_distance / (n_frames * dt * dt * 0.5);  // 등가속도
    double imu_pos = 0, imu_vel = 0;
    for (int i = 0; i < n_frames; i++)
    {
        double a = accel + imu_bias;
        imu_pos += imu_vel * dt + 0.5 * a * dt * dt;
        imu_vel += a * dt;
    }

    std::cout << "  Mono:   " << mono_pos << " m (오차: "
              << std::abs(mono_pos - true_distance) << " m)" << std::endl;
    std::cout << "  Stereo: " << stereo_pos << " m (오차: "
              << std::abs(stereo_pos - true_distance) << " m)" << std::endl;
    std::cout << "  IMU:    " << imu_pos << " m (오차: "
              << std::abs(imu_pos - true_distance) << " m)" << std::endl;
}

double ScaleRecovery::fuse_vision_imu(double vision_estimate, double imu_estimate,
                                      double vision_weight)
{
    return vision_weight * vision_estimate + (1.0 - vision_weight) * imu_estimate;
}

void ScaleRecovery::analyze_results(double true_val, const std::vector<double>& estimates,
                                    const std::vector<std::string>& labels)
{
    for (size_t i = 0; i < estimates.size(); i++)
    {
        double error = std::abs(estimates[i] - true_val);
        double pct = error / true_val * 100.0;
        std::cout << "  " << labels[i] << ": " << estimates[i]
                  << " m (오차 " << error << " m, " << pct << "%)" << std::endl;
    }
}

// ====================
// 교육 블록
// ====================

// 💡 quiz_easy Q1: Stereo Depth 공식
// 💡 quiz_medium Q1: Stereo depth 계산
void demoStereoDepth()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Stereo Depth 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "공식: depth = f * b / disparity\n" << std::endl;

    double f = 500, b = 0.12;  // 픽셀, 미터

    double disparities[] = {10, 30, 5, 2, 0.5};
    for (double d : disparities)
    {
        double depth = ScaleRecovery::compute_stereo_depth(f, b, d);
        std::cout << "  disparity " << d << " px → depth " << depth << " m" << std::endl;
    }

    std::cout << "\n💡 disparity가 작을수록 멀리 (정밀도 저하)" << std::endl;
    std::cout << "   baseline이 클수록 원거리 정밀도 향상" << std::endl;
}

// 💡 quiz_easy Q2: Vision-IMU 상호 보완
// 💡 quiz_medium Q2: IMU 적분
void demoIMUIntegration()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 IMU 적분과 바이어스 드리프트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "적분 공식:" << std::endl;
    std::cout << "  v(k+1) = v(k) + a(k) * dt" << std::endl;
    std::cout << "  p(k+1) = p(k) + v(k) * dt + 0.5 * a(k) * dt²\n" << std::endl;

    // 바이어스 없는 경우
    std::vector<double> accel_clean = {1.0, 1.0, 0.0, 0.0, -1.0, -1.0, 0.0, 0.0, 0.0, 0.0};
    std::vector<double> pos_clean, vel_clean;
    ScaleRecovery::simulate_imu(accel_clean, 0.1, pos_clean, vel_clean);

    // 바이어스 0.01 m/s² 추가
    std::vector<double> accel_biased = accel_clean;
    for (auto& a : accel_biased) a += 0.01;
    std::vector<double> pos_biased, vel_biased;
    ScaleRecovery::simulate_imu(accel_biased, 0.1, pos_biased, vel_biased);

    std::cout << "  바이어스 없음: 최종 위치 = " << pos_clean.back() << " m" << std::endl;
    std::cout << "  바이어스 0.01: 최종 위치 = " << pos_biased.back() << " m" << std::endl;
    std::cout << "  바이어스 오차: " << std::abs(pos_clean.back() - pos_biased.back()) << " m\n" << std::endl;

    std::cout << "💡 Vision → 방향 정확, 스케일 모호" << std::endl;
    std::cout << "   IMU    → 스케일 관측, 바이어스 드리프트" << std::endl;
    std::cout << "   VIO    → 서로 보완! (Vision이 IMU 바이어스 보정)" << std::endl;
}

// 💡 quiz_easy Q4: VIO 정의
// 💡 quiz_medium Q3: 센서 비교
void demoMethodComparison()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Mono vs Stereo vs VIO 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "10m 이동 시뮬레이션:\n" << std::endl;
    ScaleRecovery::compare_scale_methods(10.0, 0.02, 0.1, 0.01, 100);

    std::cout << "\n융합 효과 (Vision + IMU):" << std::endl;
    double mono = 12.5, imu = 10.3;
    double fused = ScaleRecovery::fuse_vision_imu(mono, imu, 0.3);
    std::cout << "  Mono:    " << mono << " m" << std::endl;
    std::cout << "  IMU:     " << imu << " m" << std::endl;
    std::cout << "  융합:    " << fused << " m (vision_w=0.3)" << std::endl;
    std::cout << "  참값:    10.0 m\n" << std::endl;

    std::cout << "💡 VIO = Visual-Inertial Odometry" << std::endl;
    std::cout << "   → 단안의 스케일 모호성을 IMU로 해결!" << std::endl;
}

// ====================
// Main
// ====================

#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 13: 스케일 복구" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    demoStereoDepth();
    demoIMUIntegration();
    demoMethodComparison();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📌 다음 단계:" << std::endl;
    std::cout << "  1. quiz_easy    → Stereo, Vision-IMU 보완, VIO 정의" << std::endl;
    std::cout << "  2. quiz_medium  → Depth 계산, IMU 적분, 센서 비교" << std::endl;
    std::cout << "  3. my_basic.cpp → 직접 구현 (5 Step)" << std::endl;
    std::cout << "  4. Phase 4      → VINS-Mono 실제 구현!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
