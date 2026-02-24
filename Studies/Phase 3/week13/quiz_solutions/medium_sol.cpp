/**
 * Quiz Medium - Week 13: 스케일 복구 방법 (정답)
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. Stereo Depth 계산 — disparity로 깊이를 구하는 실습
 *   2. IMU 가속도 적분 — 이중 적분의 오차 누적 이해
 *   3. 센서 융합 비교 — Mono/Stereo/VIO 추정 정확도 비교
 *
 * IMU 이중 적분 (Dead Reckoning):
 *
 *   가속도 a(t) → 1차 적분 → 속도 v(t) → 2차 적분 → 위치 p(t)
 *
 *   이산화:
 *     v(k+1) = v(k) + a(k) · dt
 *     p(k+1) = p(k) + v(k) · dt + 0.5 · a(k) · dt²
 *
 *   바이어스 b가 있으면:
 *     a_meas = a_true + b
 *     위치 오차 ∝ 0.5 · b · t² → 시간의 제곱에 비례하여 발산!
 *
 * Stereo vs Monocular vs VIO 비교:
 *
 *   Monocular: 스케일 모호 → 드리프트 심함 (25% 이상 오차 가능)
 *   Stereo:    절대 스케일 → 가장 정확 (~1% 오차)
 *   VIO:       IMU로 스케일 보정 → 양호 (~3% 오차)
 *
 * 난이도: ★★☆ (수치 계산, Eigen 활용)
 * 선수 지식: quiz_easy (스케일 복구 개념), Week 12 (스케일 모호성)
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <iomanip>

using namespace Eigen;

// 문제 1: Stereo Depth 계산 실습
//
// 깊이 공식: Z = f · b / d
//   f = 500 pixel (초점 거리)
//   b = 0.12 m (baseline)
//   d = left_x - right_x (disparity)
//
// disparity와 depth의 관계:
//   d가 크면 → 가까운 물체 (Z 작음)
//   d가 작으면 → 먼 물체 (Z 큼)
//   d = 0이면 → Z = ∞ (무한 원점, 0으로 나누기!)
//
// 정밀도 한계:
//   disparity 1 pixel 변화 → depth 크게 변동 (특히 먼 거리에서)
//   예: f=500, b=0.12일 때
//       d=10 → Z=6.0m, d=9 → Z=6.67m (차이 0.67m)
//       d=2 → Z=30m, d=1 → Z=60m (차이 30m!)
//
// ★ Stereo 깊이 추정은 가까운 거리에서 정확, 먼 거리에서 부정확
void problem1_stereo_depth_calculation()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Stereo Depth 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Stereo 카메라의 파라미터:" << std::endl;
    std::cout << "  초점 거리 f = 500 pixel" << std::endl;
    std::cout << "  베이스라인 b = 0.12 m" << std::endl;
    std::cout << std::endl;

    // 주어진 데이터: 5개 특징점의 좌우 카메라 x좌표
    Eigen::VectorXd left_x(5);   // 좌 카메라 x좌표
    Eigen::VectorXd right_x(5);  // 우 카메라 x좌표

    left_x << 320.0, 250.0, 400.0, 310.0, 350.0;
    right_x << 310.0, 220.0, 395.0, 280.0, 348.0;

    double f = 500.0;  // 초점 거리
    double b = 0.12;   // 베이스라인

    std::cout << "5개 특징점의 좌우 카메라 x좌표:" << std::endl;
    std::cout << "  점 │ 좌 카메라 (pixel) │ 우 카메라 (pixel)" << std::endl;
    std::cout << "  ───┼──────────────────┼──────────────────" << std::endl;
    for (int i = 0; i < 5; i++)
    {
        printf("   %d  │      %6.1f       │      %6.1f\n", i + 1, left_x(i), right_x(i));
    }
    std::cout << std::endl;

    std::cout << "과제: 각 점의 disparity와 depth를 계산하세요.\n" << std::endl;
    std::cout << "  disparity = left_x - right_x" << std::endl;
    std::cout << "  depth = f * b / disparity\n" << std::endl;

    // ✅ 정답:
    Eigen::VectorXd disparity = left_x - right_x;
    Eigen::VectorXd depth(5);
    for (int i = 0; i < 5; i++)
    {
        depth(i) = f * b / disparity(i);
    }

    std::cout << "  점 │ Disparity (pixel) │ Depth (m)" << std::endl;
    std::cout << "  ───┼──────────────────┼──────────" << std::endl;

    for (int i = 0; i < 5; i++)
    {
        printf("   %d  │      %5.1f        │   %6.2f\n", i + 1, disparity(i), depth(i));
    }

    std::cout << "\n  💡 disparity가 작은 점일수록 멀리 있음 (d→0이면 Z→∞)\n" << std::endl;
}

// 문제 2: IMU 가속도 적분 실습
//
// IMU 이중 적분 (Euler 방법):
//   v(k+1) = v(k) + a(k) · dt         (1차 적분: 가속도 → 속도)
//   p(k+1) = p(k) + v(k) · dt + 0.5 · a(k) · dt²  (2차 적분: 속도 → 위치)
//
// 바이어스의 영향:
//   a_meas = a_true + b (b = 바이어스, 상수 오차)
//   속도 오차: Δv = b · t (시간에 비례)
//   위치 오차: Δp = 0.5 · b · t² (시간의 제곱에 비례!)
//
//   b = 0.01 m/s², t = 1초 → Δp = 0.005m (무시 가능)
//   b = 0.01 m/s², t = 100초 → Δp = 50m (심각!)
//
// ★ 이것이 IMU 단독으로는 장시간 위치 추정이 불가능한 이유
//   → 카메라 등 다른 센서와 융합 필수 (VIO)
void problem2_imu_integration()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: IMU 가속도 적분" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "로봇이 x축 방향으로 이동합니다." << std::endl;
    std::cout << "IMU에서 측정된 가속도 데이터 (10 스텝, dt = 0.1초):\n" << std::endl;

    double dt = 0.1;  // 시간 간격 (초)

    // IMU 가속도 측정값 (x축만, 단순화)
    Eigen::VectorXd accel(10);
    accel << 1.0, 1.0, 0.5, 0.0, 0.0, -0.5, -1.0, 0.0, 0.5, 0.0;

    std::cout << "  스텝 │ 시간 (초) │ 가속도 (m/s²)" << std::endl;
    std::cout << "  ─────┼──────────┼──────────────" << std::endl;
    for (int i = 0; i < 10; i++)
    {
        printf("    %2d  │   %4.1f    │    %5.2f\n", i + 1, (i + 1) * dt, accel(i));
    }

    std::cout << "\n과제: 가속도를 적분하여 각 스텝의 속도와 위치를 구하세요." << std::endl;
    std::cout << "       초기 조건: v(0) = 0 m/s, p(0) = 0 m\n" << std::endl;

    std::cout << "  적분 공식:" << std::endl;
    std::cout << "    v(k+1) = v(k) + a(k) * dt" << std::endl;
    std::cout << "    p(k+1) = p(k) + v(k) * dt + 0.5 * a(k) * dt²\n" << std::endl;

    // ✅ 정답:
    Eigen::VectorXd velocity = Eigen::VectorXd::Zero(11);
    Eigen::VectorXd position = Eigen::VectorXd::Zero(11);

    for (int i = 0; i < 10; i++)
    {
        velocity(i + 1) = velocity(i) + accel(i) * dt;
        position(i + 1) = position(i) + velocity(i) * dt + 0.5 * accel(i) * dt * dt;
    }

    std::cout << "  스텝 │ 시간 (초) │ 속도 (m/s) │ 위치 (m)" << std::endl;
    std::cout << "  ─────┼──────────┼────────────┼─────────" << std::endl;
    for (int i = 0; i <= 10; i++)
    {
        printf("   %2d   │   %4.1f    │   %6.3f   │  %6.3f\n",
               i, i * dt, velocity(i), position(i));
    }

    // 바이어스 영향 분석
    double bias = 0.01;
    double total_time = 10 * dt;
    double bias_error = 0.5 * bias * total_time * total_time;
    std::cout << "\n  바이어스 0.01 m/s² 추가 시 위치 오차: "
              << bias_error << " m (0.5·b·t²)\n" << std::endl;
}

// 문제 3: 센서 융합 비교 분석
//
// 3가지 방법으로 10m 직선 경로 추정:
//
// Monocular (12.5, 0.3, -0.1):
//   X축 25% 과대 추정 → 스케일 모호성 때문
//   Y, Z 축 소량 오차 → 방향은 비교적 정확
//
// Stereo (10.1, 0.05, 0.02):
//   전체적으로 매우 정확 → baseline으로 절대 스케일 복원
//   잔여 오차는 매칭 노이즈와 캘리브레이션 오차
//
// VIO (10.3, 0.1, -0.05):
//   스케일은 Mono보다 훨씬 정확 → IMU의 스케일 보정 효과
//   Stereo보다는 약간 부정확 → IMU 바이어스의 잔여 영향
//
// 절대 오차 = ||estimate - true||₂
// 스케일 오차 = |estimate_x - true_x| / true_x × 100%
//
// ★ Mono의 25% 스케일 오차 = Week 12에서 배운 스케일 모호성의 직접적 결과
void problem3_sensor_comparison()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 센서 융합 비교 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "3가지 방법으로 10m 직선 경로를 추정한 결과입니다.\n" << std::endl;

    // 실제 경로: 10m 직선 (x축)
    double true_distance = 10.0;

    // 각 방법의 추정 결과 (시뮬레이션 데이터)
    Eigen::Vector3d mono_estimate(12.5, 0.3, -0.1);     // Monocular: 스케일 드리프트
    Eigen::Vector3d stereo_estimate(10.1, 0.05, 0.02);  // Stereo: 안정적
    Eigen::Vector3d vio_estimate(10.3, 0.1, -0.05);     // VIO: 양호

    std::cout << "  실제 경로: (10.0, 0.0, 0.0) m" << std::endl;
    std::cout << "  Monocular 추정: (" << mono_estimate.transpose() << ") m" << std::endl;
    std::cout << "  Stereo 추정:    (" << stereo_estimate.transpose() << ") m" << std::endl;
    std::cout << "  VIO 추정:       (" << vio_estimate.transpose() << ") m\n" << std::endl;

    std::cout << "과제:" << std::endl;
    std::cout << "  1. 각 방법의 절대 위치 오차 (Euclidean distance)를 계산하세요." << std::endl;
    std::cout << "  2. 각 방법의 스케일 오차를 백분율로 계산하세요." << std::endl;
    std::cout << "  3. 어떤 방법이 가장 정확한지, 그 이유를 설명하세요.\n" << std::endl;

    // ✅ 정답:
    Eigen::Vector3d true_position(10.0, 0.0, 0.0);

    double mono_error = (mono_estimate - true_position).norm();
    double stereo_error = (stereo_estimate - true_position).norm();
    double vio_error = (vio_estimate - true_position).norm();

    double mono_scale_error = std::abs(mono_estimate(0) - true_distance) / true_distance * 100.0;
    double stereo_scale_error = std::abs(stereo_estimate(0) - true_distance) / true_distance * 100.0;
    double vio_scale_error = std::abs(vio_estimate(0) - true_distance) / true_distance * 100.0;

    std::cout << "  방법       │ 절대 오차 (m) │ 스케일 오차 (%)" << std::endl;
    std::cout << "  ───────────┼──────────────┼────────────────" << std::endl;
    printf("  Monocular  │    %5.3f     │     %5.1f%%\n", mono_error, mono_scale_error);
    printf("  Stereo     │    %5.3f     │     %5.1f%%\n", stereo_error, stereo_scale_error);
    printf("  VIO        │    %5.3f     │     %5.1f%%\n", vio_error, vio_scale_error);

    std::cout << "\n  Monocular의 오차가 가장 큰 이유:" << std::endl;
    std::cout << "  → 단안 카메라는 절대 스케일을 알 수 없어 (Week 12 스케일 모호성)" << std::endl;
    std::cout << "  → 스케일 드리프트가 누적되어 25% 과대 추정\n" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 13 Quiz - Medium (정답)" << std::endl;
    std::cout << "스케일 복구 방법" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_stereo_depth_calculation();
    problem2_imu_integration();
    problem3_sensor_comparison();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
