/**
 * Quiz Solutions - Medium
 * Week 13: 스케일 복구 방법
 *
 * Eigen을 사용한 계산 문제 풀이
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <iomanip>

using namespace Eigen;
using namespace std;

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 13 Quiz Solutions (Medium)" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    // =========================================
    // Q1. Stereo Depth 계산
    // =========================================
    cout << "Q1. Stereo Depth 계산" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    double f = 500.0;  // 초점 거리 (pixel)
    double b = 0.12;   // 베이스라인 (m)

    Eigen::VectorXd left_x(5);
    Eigen::VectorXd right_x(5);
    left_x  << 320.0, 250.0, 400.0, 310.0, 350.0;
    right_x << 310.0, 220.0, 395.0, 280.0, 348.0;

    // disparity 계산
    Eigen::VectorXd disparity = left_x - right_x;

    // depth 계산: depth = f * b / disparity
    Eigen::VectorXd depth(5);
    for (int i = 0; i < 5; i++) {
        if (disparity(i) > 0) {
            depth(i) = f * b / disparity(i);
        } else {
            depth(i) = -1.0;  // 무한대 (오류)
        }
    }

    cout << "  파라미터: f = " << f << " pixel, b = " << b << " m\n" << endl;
    cout << "  점 │ 좌 (pixel) │ 우 (pixel) │ Disparity │ Depth (m)" << endl;
    cout << "  ───┼───────────┼───────────┼───────────┼──────────" << endl;
    for (int i = 0; i < 5; i++) {
        printf("   %d  │  %7.1f   │  %7.1f   │  %7.1f   │  %7.2f\n",
               i+1, left_x(i), right_x(i), disparity(i), depth(i));
    }

    cout << "\n  분석:" << endl;
    cout << "  - 점 3: disparity=5 → depth=12m (가장 멀리)" << endl;
    cout << "  - 점 4: disparity=30 → depth=2m (가장 가까이)" << endl;
    cout << "  - 점 5: disparity=2 → depth=30m (매우 멀리, 정밀도 낮음)\n" << endl;

    cout << "  disparity=0이면?" << endl;
    cout << "  → depth = f*b/0 = 무한대! (0으로 나누기 오류)" << endl;
    cout << "  → 무한히 먼 물체 또는 매칭 실패" << endl;
    cout << "  → 실제로는 최소 disparity 임계값 필요 (예: > 1 pixel)\n" << endl;

    cout << "  핵심:" << endl;
    cout << "  - Stereo는 baseline 덕분에 절대 스케일 복구 가능" << endl;
    cout << "  - depth 단위가 자동으로 미터(m)" << endl;
    cout << "  - 원거리(disparity 작음)에서 정밀도 저하 주의\n" << endl;

    // =========================================
    // Q2. IMU 가속도 적분
    // =========================================
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Q2. IMU 가속도 적분" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    double dt = 0.1;

    Eigen::VectorXd accel(10);
    accel << 1.0, 1.0, 0.5, 0.0, 0.0, -0.5, -1.0, 0.0, 0.5, 0.0;

    // 적분
    Eigen::VectorXd velocity(11);  // 0~10
    Eigen::VectorXd position(11);  // 0~10
    velocity(0) = 0.0;
    position(0) = 0.0;

    for (int i = 0; i < 10; i++) {
        // v(k+1) = v(k) + a(k) * dt
        velocity(i + 1) = velocity(i) + accel(i) * dt;

        // p(k+1) = p(k) + v(k) * dt + 0.5 * a(k) * dt²
        position(i + 1) = position(i) + velocity(i) * dt + 0.5 * accel(i) * dt * dt;
    }

    cout << "  바이어스 없는 경우 (정상):" << endl;
    cout << "  스텝 │ 시간 (초) │ 가속도 (m/s²) │ 속도 (m/s) │ 위치 (m)" << endl;
    cout << "  ─────┼──────────┼──────────────┼────────────┼─────────" << endl;
    printf("    0   │   0.0     │      -       │   %6.3f    │  %6.3f\n",
           velocity(0), position(0));
    for (int i = 0; i < 10; i++) {
        printf("   %2d   │   %4.1f    │    %5.2f     │   %6.3f    │  %6.3f\n",
               i + 1, (i + 1) * dt, accel(i), velocity(i + 1), position(i + 1));
    }

    cout << "\n  최종: 속도 = " << fixed << setprecision(3) << velocity(10)
         << " m/s, 위치 = " << position(10) << " m\n" << endl;

    // 바이어스 추가
    cout << "  바이어스 0.01 m/s² 추가 시:" << endl;

    double bias = 0.01;
    Eigen::VectorXd velocity_biased(11);
    Eigen::VectorXd position_biased(11);
    velocity_biased(0) = 0.0;
    position_biased(0) = 0.0;

    for (int i = 0; i < 10; i++) {
        double biased_accel = accel(i) + bias;
        velocity_biased(i + 1) = velocity_biased(i) + biased_accel * dt;
        position_biased(i + 1) = position_biased(i) + velocity_biased(i) * dt
                                  + 0.5 * biased_accel * dt * dt;
    }

    cout << "  최종: 속도 = " << velocity_biased(10)
         << " m/s, 위치 = " << position_biased(10) << " m" << endl;
    cout << "  위치 차이 = " << abs(position_biased(10) - position(10)) << " m\n" << endl;

    cout << "  분석:" << endl;
    cout << "  - 1초 동안은 바이어스 영향이 작음 (0.005m)" << endl;
    cout << "  - 하지만 시간이 길어지면 오차가 t²에 비례하여 증가" << endl;
    cout << "  - 이론값: 0.5 * 0.01 * 1.0² = 0.005m" << endl;
    cout << "  - 10초 후: 0.5 * 0.01 * 100 = 0.5m" << endl;
    cout << "  - 60초 후: 0.5 * 0.01 * 3600 = 18m" << endl;
    cout << "  → Vision으로 주기적 보정 필수! (VIO)\n" << endl;

    // =========================================
    // Q3. 센서 융합 비교 분석
    // =========================================
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Q3. 센서 융합 비교 분석" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    Eigen::Vector3d true_position(10.0, 0.0, 0.0);
    Eigen::Vector3d mono_estimate(12.5, 0.3, -0.1);
    Eigen::Vector3d stereo_estimate(10.1, 0.05, 0.02);
    Eigen::Vector3d vio_estimate(10.3, 0.1, -0.05);

    // 절대 오차 (유클리디안 거리)
    double mono_error = (mono_estimate - true_position).norm();
    double stereo_error = (stereo_estimate - true_position).norm();
    double vio_error = (vio_estimate - true_position).norm();

    // 스케일 오차 (x축 기준)
    double mono_scale_error = abs(mono_estimate(0) - true_position(0)) / true_position(0) * 100.0;
    double stereo_scale_error = abs(stereo_estimate(0) - true_position(0)) / true_position(0) * 100.0;
    double vio_scale_error = abs(vio_estimate(0) - true_position(0)) / true_position(0) * 100.0;

    cout << "  실제 위치: (" << true_position.transpose() << ") m\n" << endl;

    cout << "  방법       │ 추정 위치             │ 절대 오차 (m) │ 스케일 오차 (%)" << endl;
    cout << "  ───────────┼──────────────────────┼──────────────┼────────────────" << endl;
    printf("  Monocular  │ (%5.1f, %5.2f, %5.2f) │    %6.3f     │     %5.1f%%\n",
           mono_estimate(0), mono_estimate(1), mono_estimate(2),
           mono_error, mono_scale_error);
    printf("  Stereo     │ (%5.1f, %5.2f, %5.2f) │    %6.3f     │     %5.1f%%\n",
           stereo_estimate(0), stereo_estimate(1), stereo_estimate(2),
           stereo_error, stereo_scale_error);
    printf("  VIO        │ (%5.1f, %5.2f, %5.2f) │    %6.3f     │     %5.1f%%\n",
           vio_estimate(0), vio_estimate(1), vio_estimate(2),
           vio_error, vio_scale_error);

    cout << "\n  분석:" << endl;
    cout << "  1. Monocular (절대 오차: " << fixed << setprecision(3) << mono_error << "m)" << endl;
    cout << "     - 스케일 오차가 25%로 가장 큼" << endl;
    cout << "     - 원인: 스케일 모호성 (Week 12)" << endl;
    cout << "     - ||t||=1 정규화 → 누적 드리프트 → 실제보다 25% 더 멀리 추정" << endl;
    cout << endl;
    cout << "  2. Stereo (절대 오차: " << stereo_error << "m)" << endl;
    cout << "     - 가장 정확! 스케일 오차 1%" << endl;
    cout << "     - 원인: baseline으로 절대 depth 복구" << endl;
    cout << "     - 스케일 누적 오차 없음" << endl;
    cout << endl;
    cout << "  3. VIO (절대 오차: " << vio_error << "m)" << endl;
    cout << "     - Stereo 다음으로 정확, 스케일 오차 3%" << endl;
    cout << "     - IMU가 스케일 정보 제공" << endl;
    cout << "     - Vision이 IMU 드리프트 보정" << endl;
    cout << "     - 단안 카메라 + IMU로 좋은 성능!\n" << endl;

    cout << "  결론:" << endl;
    cout << "  - Stereo > VIO > Monocular (스케일 정확도 순)" << endl;
    cout << "  - VIO는 추가 카메라 없이도 Stereo에 근접한 성능" << endl;
    cout << "  - AMR에 IMU가 이미 있으므로 VIO가 가장 실용적!\n" << endl;

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Phase 3 Week 13 - Medium Quiz 완료!" << endl;
    cout << "Phase 3 전체 학습 완료! 다음은 Phase 4: VIO" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
