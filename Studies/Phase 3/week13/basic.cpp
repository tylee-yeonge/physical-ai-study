#include "scale_recovery.h"
#include <iostream>
#include <cmath>
#include <random>

// ====================
// ScaleRecovery 구현
// ====================

// Stereo 깊이 계산 — 기하학적으로 절대 스케일을 복구하는 방법
//
// Stereo 카메라의 원리:
//   두 카메라 사이 거리(baseline)가 물리적으로 알려져 있으므로
//   삼각측량으로 절대 깊이(미터 단위)를 구할 수 있음.
//
// 공식 유도:
//   좌 카메라 ←── baseline (b) ──→ 우 카메라
//
//   좌 이미지에서 점의 x좌표: u_L = f · X/Z + cx
//   우 이미지에서 점의 x좌표: u_R = f · (X-b)/Z + cx
//   disparity = u_L - u_R = f · b / Z
//   ∴ depth (Z) = f · b / disparity
//
// 파라미터:
//   f: 초점 거리 (pixel) — 캘리브레이션으로 알려진 값
//   baseline: 두 카메라 사이 거리 (미터) — 제조 시 고정
//   disparity: 좌우 이미지에서 같은 점의 x좌표 차이 (pixel) — 매칭으로 측정
//
// ★ disparity가 작을수록 먼 물체 → 정밀도 저하
//   disparity = 1 pixel 미만이면 깊이 추정이 매우 불안정
//   → 이것이 stereo의 유효 거리 한계 (baseline에 비례)
//
// 💡 quiz_easy Q1: depth = f * b / disparity 공식
// 💡 quiz_medium Q1: disparity별 깊이 계산
double ScaleRecovery::compute_stereo_depth(double f, double baseline, double disparity)
{
    // disparity ≤ 0: 물리적으로 불가능 (좌우 매칭 실패)
    if (disparity <= 0) return -1.0;
    return f * baseline / disparity;
}

// IMU 가속도 적분 시뮬레이션 — 관성 센서로 위치를 추정하는 과정
//
// IMU(Inertial Measurement Unit)의 가속도계 출력을 이중 적분하여 위치 추정:
//   가속도 a(t) → 속도 v(t) = ∫a dt → 위치 p(t) = ∫v dt
//   [m/s²]         [m/s]                  [m]
//
// 이산 적분 (Euler 방법):
//   v(k+1) = v(k) + a(k) · dt               ← 속도 적분
//   p(k+1) = p(k) + v(k) · dt + 0.5 · a(k) · dt²  ← 위치 적분
//
// ★ IMU의 핵심 장점: 출력 단위가 m/s² (절대 단위!)
//   → 적분 결과도 m 단위 → 절대 스케일 제공
//   → 이것이 단안 카메라의 스케일 모호성을 해결하는 핵심
//
// ★ IMU의 핵심 약점: 바이어스 드리프트
//   가속도계 바이어스 b_a = 0.01 m/s² (작은 값)
//   위치 오차 = 0.5 · b_a · t²
//     1초: 0.005m (OK)
//     10초: 0.5m (문제!)
//     60초: 18m (심각!)
//   → 이중 적분이므로 오차가 시간의 제곱에 비례하여 증가
//   → Vision으로 바이어스를 보정해야 함 → 이것이 VIO!
//
// 💡 quiz_easy Q2: Vision-IMU 상호 보완
// 💡 quiz_medium Q2: IMU 적분 수식
void ScaleRecovery::simulate_imu(const std::vector<double>& accel, double dt,
                                 std::vector<double>& positions, std::vector<double>& velocities)
{
    positions.clear();
    velocities.clear();
    positions.push_back(0.0);    // 초기 위치 = 0
    velocities.push_back(0.0);   // 초기 속도 = 0

    for (size_t i = 0; i < accel.size(); i++)
    {
        // 속도 적분: v(k+1) = v(k) + a(k) · dt
        double v_new = velocities.back() + accel[i] * dt;
        // 위치 적분: p(k+1) = p(k) + v(k) · dt + 0.5 · a(k) · dt²
        //   → 2차 항(0.5·a·dt²)을 포함하여 정밀도 향상
        double p_new = positions.back() + velocities.back() * dt + 0.5 * accel[i] * dt * dt;
        velocities.push_back(v_new);
        positions.push_back(p_new);
    }
}

// Mono vs Stereo vs IMU 방법 비교 — 스케일 복구 능력의 차이를 수치로 확인
//
// 같은 이동 거리(true_distance)를 세 가지 방법으로 추정하고 오차를 비교.
//
// 각 방법의 오차 특성:
//   ① Mono: 프레임마다 스케일 노이즈 누적 (랜덤 워크)
//     → 오차 ∝ √N (프레임 수의 제곱근에 비례)
//     → 장거리에서 크게 벗어남
//
//   ② Stereo: 절대 깊이 기반, 프레임 간 독립
//     → 오차 = 일정한 가우시안 노이즈 (누적 없음)
//     → 장거리에서도 안정적
//
//   ③ IMU: 가속도 적분, 바이어스 누적
//     → 오차 ∝ t² (시간의 제곱에 비례)
//     → 단시간: 정확, 장시간: 발산
//
// ★ 결론: 세 방법 모두 장단점이 있으므로 융합(VIO)이 최적!
void ScaleRecovery::compare_scale_methods(double true_distance, double mono_scale_noise,
                                          double stereo_noise, double imu_bias, int n_frames)
{
    std::default_random_engine gen(42);
    double step = true_distance / n_frames;

    // ── Mono: 매 프레임 스케일 노이즈 누적 (Week 12의 드리프트) ──
    //   각 프레임의 이동에 N(1.0, mono_scale_noise) 스케일을 곱함
    //   → 프레임이 쌓일수록 누적 오차 증가
    std::normal_distribution<> mono_noise(1.0, mono_scale_noise);
    double mono_pos = 0;
    for (int i = 0; i < n_frames; i++) mono_pos += step * mono_noise(gen);

    // ── Stereo: 절대 깊이 기반 (누적 없음) ──
    //   baseline이 알려져 있으므로 각 프레임에서 독립적으로 깊이 측정
    //   → 최종 거리에 한 번의 가우시안 노이즈만 추가 (단순화)
    //   → 실제로는 프레임마다 독립적인 깊이 오차가 있지만 누적되지 않음
    std::normal_distribution<> st_noise(0, stereo_noise);
    double stereo_pos = true_distance + st_noise(gen);

    // ── IMU: 가속도 적분 + 바이어스 누적 ──
    //   등가속도 운동을 가정하고, 여기에 바이어스를 추가
    //   true_distance = 0.5 · a · (N·dt)² → a = 2·d / (N·dt)²
    //   매 프레임: a_measured = a_true + imu_bias
    //   → 바이어스가 이중 적분으로 인해 t²에 비례하여 위치 오차 누적
    double dt = 0.1;
    double accel = true_distance / (n_frames * dt * dt * 0.5);
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

// Vision-IMU 융합 — 가중 평균으로 두 추정값을 결합
//
// VIO(Visual-Inertial Odometry)의 단순화된 버전:
//   fused = vision_weight · vision + (1 - vision_weight) · imu
//
// 실제 VIO(VINS-Mono 등)에서는:
//   Kalman Filter 또는 최적화 기반 융합을 사용
//   IMU pre-integration으로 고속 상태 예측
//   Vision이 IMU 바이어스를 온라인으로 추정/보정
//   → 단순 가중 평균보다 훨씬 정교하지만, 핵심 아이디어는 동일
//
// Vision과 IMU의 상호 보완:
//   Vision: 방향 정확, 스케일 모호, 느린 움직임에 강함
//   IMU:    스케일 관측, 바이어스 드리프트, 빠른 움직임에 강함
//   → 융합하면 둘의 약점을 상호 보완!
double ScaleRecovery::fuse_vision_imu(double vision_estimate, double imu_estimate,
                                      double vision_weight)
{
    return vision_weight * vision_estimate + (1.0 - vision_weight) * imu_estimate;
}

// 스케일 복구 결과 분석 — 여러 추정값을 참값과 비교
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

// ── Stereo Depth 시연 ──
//
// depth = f · b / disparity 공식을 다양한 disparity 값으로 확인.
//
// 핵심 관찰:
//   disparity 10px → depth 6m (적절한 거리)
//   disparity 2px → depth 30m (원거리, 정밀도 저하)
//   disparity 0.5px → depth 120m (사실상 추정 불가)
//   → disparity가 1/2이 되면 depth가 2배 → 원거리에서 비선형 오차 증가
//   → baseline을 늘리면 같은 depth에서 disparity 증가 → 원거리 정밀도 향상
//
// 💡 quiz_easy Q1: Stereo Depth 공식
// 💡 quiz_medium Q1: Stereo depth 계산
void demoStereoDepth()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Stereo Depth 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "공식: depth = f * b / disparity\n" << std::endl;

    // f = 500 pixel (일반적인 초점거리)
    // b = 0.12 m = 12cm (일반적인 stereo baseline, 예: RealSense D435)
    double f = 500, b = 0.12;

    double disparities[] = {10, 30, 5, 2, 0.5};
    for (double d : disparities)
    {
        double depth = ScaleRecovery::compute_stereo_depth(f, b, d);
        std::cout << "  disparity " << d << " px → depth " << depth << " m" << std::endl;
    }

    std::cout << "\n💡 disparity가 작을수록 멀리 (정밀도 저하)" << std::endl;
    std::cout << "   baseline이 클수록 원거리 정밀도 향상" << std::endl;
}

// ── IMU 적분과 바이어스 드리프트 시연 ──
//
// 바이어스가 없는 IMU와 0.01 m/s² 바이어스가 있는 IMU를 비교.
//
// 시뮬레이션:
//   가속도 시퀀스: [1, 1, 0, 0, -1, -1, 0, 0, 0, 0] (가감속 운동)
//   dt = 0.1초, 총 10 스텝 = 1초
//
// 바이어스 0.01 m/s²의 영향:
//   1초 후: ~0.005m 오차 (작음)
//   하지만 실제로는 수백 초 운용하므로:
//     10초: 0.5m, 60초: 18m, 300초: 450m (사용 불가!)
//   → 이것이 IMU만으로는 위치 추정이 불가능한 이유
//   → Vision이 IMU의 바이어스를 주기적으로 보정 → VIO의 핵심
//
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

    // 바이어스 없는 경우 (이상적인 IMU)
    std::vector<double> accel_clean = {1.0, 1.0, 0.0, 0.0, -1.0, -1.0, 0.0, 0.0, 0.0, 0.0};
    std::vector<double> pos_clean, vel_clean;
    ScaleRecovery::simulate_imu(accel_clean, 0.1, pos_clean, vel_clean);

    // 바이어스 0.01 m/s² 추가 (현실적인 MEMS IMU)
    //   MEMS 가속도계의 전형적 바이어스: 0.01~0.1 m/s²
    //   각 가속도에 0.01을 더함 → 이중 적분으로 오차 누적
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

// ── Mono vs Stereo vs VIO 비교 시연 ──
//
// 10m 이동을 세 가지 방법으로 추정하고 오차를 비교.
// 추가로 Vision-IMU 가중 평균 융합의 효과도 시연.
//
// 결과 해석:
//   Mono: 스케일 드리프트로 큰 오차 (σ=2%)
//   Stereo: 절대 깊이 기반으로 작은 오차 (σ=0.1m)
//   IMU: 바이어스(0.01 m/s²)로 중간 오차
//   VIO(융합): 두 추정의 가중 평균 → 오차 감소
//
// ★ 실제 VIO에서는 가중 평균이 아닌 최적화 기반 융합:
//   VINS-Mono: Sliding Window + IMU Pre-integration + Vision Factor
//   → Phase 4에서 학습!
//
// 💡 quiz_easy Q4: VIO 정의
// 💡 quiz_medium Q3: 센서 비교
void demoMethodComparison()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Mono vs Stereo vs VIO 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "10m 이동 시뮬레이션:\n" << std::endl;
    ScaleRecovery::compare_scale_methods(10.0, 0.02, 0.1, 0.01, 100);

    // Vision-IMU 융합 시연 (단순 가중 평균)
    //   vision_weight = 0.3: IMU를 더 신뢰 (스케일에 강함)
    //   실제 VIO에서는 각 센서의 불확실성(공분산)에 기반한 가중치 사용
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

// 전체 데모 — 스케일 복구 방법을 3가지 관점에서 시연
//
// 학습 흐름:
//   ① Stereo depth: baseline으로 절대 깊이 계산 → 스케일 복구
//   ② IMU 적분: 가속도 → 속도 → 위치, 바이어스의 위험성
//   ③ 방법 비교: Mono vs Stereo vs IMU, 그리고 VIO 융합
//
// Phase 3 전체 요약:
//   Week 1-5: VO 파이프라인 (특징점, 에피폴라, PnP, ICP, Mini VO)
//   Week 6: 키프레임 관리
//   Week 7-11: BA (g2o, Ceres, Schur, Solver)
//   Week 12: 스케일 모호성 (원인, 드리프트, Sim(3))
//   Week 13: 스케일 복구 (Stereo, IMU, VIO)
//   → Phase 4: VINS-Mono로 VIO를 실제 구현!
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
