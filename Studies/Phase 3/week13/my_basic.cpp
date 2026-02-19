/**
 * Phase 3 Week 13 - 스케일 복구 직접 구현
 *
 * scale_recovery.h의 함수들을 직접 구현해보세요.
 *
 * ┌──────┬──────────────────────────┬────────┬──────────────┐
 * │ Step │ 함수                     │ 난이도  │ 검증 방법     │
 * ├──────┼──────────────────────────┼────────┼──────────────┤
 * │  1   │ compute_stereo_depth     │ 쉬움   │ ./my_basic   │
 * │  2   │ simulate_imu             │ 핵심   │ ./my_basic   │
 * │  3   │ compare_scale_methods    │ 보통   │ ./my_basic   │
 * │  4   │ fuse_vision_imu          │ 쉬움   │ ./my_basic   │
 * │  5   │ analyze_results          │ 쉬움   │ ./my_basic   │
 * └──────┴──────────────────────────┴────────┴──────────────┘
 */
#include "scale_recovery.h"
#include <iostream>
#include <cmath>
#include <random>

// [Step 1] compute_stereo_depth — Stereo 깊이 계산
// 힌트: depth = f * baseline / disparity
// 주의: disparity <= 0이면 -1 반환 (무한대 방지)
// 기대값: f=500, b=0.12, d=10 → depth=6.0
double ScaleRecovery::compute_stereo_depth(double f, double baseline, double disparity)
{
    // TODO: 공식 구현 + disparity 체크
    return -1.0;
}

// [Step 2] simulate_imu — IMU 가속도 적분 (핵심!)
// 힌트:
//   v(k+1) = v(k) + a(k) * dt
//   p(k+1) = p(k) + v(k) * dt + 0.5 * a(k) * dt²
// 참고: basic.cpp demoIMUIntegration()
// 기대값: a=1.0 (10회), dt=0.1 → 최종 위치 ≈ 0.045
void ScaleRecovery::simulate_imu(const std::vector<double>& accel, double dt,
                                 std::vector<double>& positions, std::vector<double>& velocities)
{
    positions.clear();
    velocities.clear();
    // TODO:
    // 1. 초기값: positions[0]=0, velocities[0]=0
    // 2. 반복: v_new, p_new 계산 후 push_back
    positions.push_back(0.0);
    velocities.push_back(0.0);
}

// [Step 3] compare_scale_methods — Mono/Stereo/IMU 비교
// 힌트: 각 방법의 특성에 맞는 노이즈 시뮬레이션
// 참고: basic.cpp demoMethodComparison()
void ScaleRecovery::compare_scale_methods(double true_distance, double mono_scale_noise,
                                          double stereo_noise, double imu_bias, int n_frames)
{
    // TODO:
    // 1. Mono: 매 프레임 scale * step 누적 (scale = N(1, noise))
    // 2. Stereo: true_distance + N(0, noise)
    // 3. IMU: 바이어스 있는 적분
    std::cout << "  TODO: 비교 결과 구현" << std::endl;
}

// [Step 4] fuse_vision_imu — 가중 평균 융합
// 힌트: result = w * vision + (1-w) * imu
// 기대값: vision=12, imu=10, w=0.3 → 10.6
double ScaleRecovery::fuse_vision_imu(double vision_estimate, double imu_estimate,
                                      double vision_weight)
{
    // TODO: 가중 평균
    return 0.0;
}

// [Step 5] analyze_results — 오차 분석 출력
// 힌트: 각 추정값과 참값의 절대/백분율 오차 출력
void ScaleRecovery::analyze_results(double true_val, const std::vector<double>& estimates,
                                    const std::vector<std::string>& labels)
{
    // TODO:
    // error = |estimate - true_val|
    // pct = error / true_val * 100
    for (size_t i = 0; i < labels.size(); i++)
    {
        std::cout << "  " << labels[i] << ": TODO" << std::endl;
    }
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 스케일 복구 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ── Step 1 테스트: compute_stereo_depth ──
    std::cout << "[Step 1] compute_stereo_depth" << std::endl;
    double d1 = ScaleRecovery::compute_stereo_depth(500, 0.12, 10);
    double d2 = ScaleRecovery::compute_stereo_depth(500, 0.12, 0);
    bool step1 = (std::abs(d1 - 6.0) < 1e-6 && d2 < 0);
    std::cout << "  d=10: " << d1 << " m" << (std::abs(d1 - 6.0) < 1e-6 ? " ✅" : " ❌ (기대: 6.0)") << std::endl;
    std::cout << "  d=0:  " << d2 << (d2 < 0 ? " ✅ (무효)" : " ❌") << "\n" << std::endl;

    // ── Step 2 테스트: simulate_imu ──
    std::cout << "[Step 2] simulate_imu" << std::endl;
    std::vector<double> accel = {1.0, 1.0, 0.0, 0.0, -1.0};
    std::vector<double> pos, vel;
    ScaleRecovery::simulate_imu(accel, 0.1, pos, vel);
    bool step2 = (pos.size() == 6 && vel.size() == 6 && pos.back() > 0);
    std::cout << "  결과 크기: " << pos.size() << (pos.size() == 6 ? " ✅" : " ❌") << std::endl;
    std::cout << "  최종 위치: " << pos.back() << " m" << std::endl;
    std::cout << "  최종 속도: " << vel.back() << " m/s\n" << std::endl;

    // ── Step 3 테스트: compare_scale_methods ──
    std::cout << "[Step 3] compare_scale_methods" << std::endl;
    ScaleRecovery::compare_scale_methods(10.0, 0.02, 0.1, 0.01, 50);
    std::cout << std::endl;

    // ── Step 4 테스트: fuse_vision_imu ──
    std::cout << "[Step 4] fuse_vision_imu" << std::endl;
    double fused = ScaleRecovery::fuse_vision_imu(12.0, 10.0, 0.3);
    bool step4 = (std::abs(fused - 10.6) < 1e-6);
    std::cout << "  융합 결과: " << fused
              << (step4 ? " ✅" : " ❌ (기대: 10.6)") << "\n" << std::endl;

    // ── Step 5 테스트: analyze_results ──
    std::cout << "[Step 5] analyze_results" << std::endl;
    ScaleRecovery::analyze_results(10.0, {12.5, 10.1, 10.3}, {"Mono", "Stereo", "VIO"});

    // ── 종합 결과 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  종합: Step1 " << (step1 ? "✅" : "❌")
              << " Step2 " << (step2 ? "✅" : "❌")
              << " Step4 " << (step4 ? "✅" : "❌") << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
