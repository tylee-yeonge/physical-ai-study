/**
 * Quiz Medium - Week 11: VIO 초기화 문제
 *
 * Eigen을 사용한 계산 문제
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <iomanip>

using namespace Eigen;
using namespace std;

void problem1_gravity_error_propagation() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 1: 중력 방향 오차 전파" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "중력 벡터가 잘못 추정되었을 때의 영향을 계산합니다.\n" << endl;
    cout << "실제 중력: g_true = [0, 0, -9.81]^T m/s²" << endl;
    cout << "추정 중력: x축으로 theta만큼 기울어진 경우\n" << endl;

    double g_mag = 9.81;

    // 실제 중력
    Vector3d g_true(0, 0, -g_mag);

    // 다양한 각도 오차에 대해
    vector<double> thetas_deg = {0.5, 1.0, 3.0, 5.0};

    cout << "과제: 각 theta에 대해 아래를 계산하세요.\n" << endl;
    cout << "  1. 잘못된 중력 벡터 g_wrong" << endl;
    cout << "     g_wrong = [g_mag * sin(theta), 0, -g_mag * cos(theta)]^T\n" << endl;
    cout << "  2. 중력 보상 오차 벡터 delta_g = g_wrong - g_true\n" << endl;
    cout << "  3. 이 오차가 5초간 이중 적분되면?" << endl;
    cout << "     위치 오차 = 0.5 * ||delta_g|| * t^2  (t=5초)\n" << endl;

    // TODO: 여기에 코드를 작성하세요
    // for (double theta_deg : thetas_deg) {
    //     double theta_rad = theta_deg * M_PI / 180.0;
    //     Vector3d g_wrong = ???;
    //     Vector3d delta_g = ???;
    //     double pos_error = ???;
    // }

    cout << "  theta (°) │ delta_g (m/s²) │ 5초 후 위치 오차 (m)" << endl;
    cout << "  ──────────┼───────────────┼─────────────────────" << endl;
    for (double theta_deg : thetas_deg) {
        cout << "    " << setw(4) << fixed << setprecision(1) << theta_deg
             << "     │    _________   │      ___________" << endl;
    }

    cout << "\n  💡 힌트: sin(1°) ≈ 0.01745, cos(1°) ≈ 0.99985\n" << endl;
    cout << "  추가 질문: 0.1° 이내의 정확도로 중력 방향을 추정하면,\n";
    cout << "  5초 후 위치 오차는 몇 cm인가?\n" << endl;
}

void problem2_scale_alignment() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 2: 스케일 추정 (최소자승법)" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "Vision SfM에서 얻은 상대 이동과 IMU에서 얻은 실제 이동이\n";
    cout << "주어졌을 때, 최적의 스케일 s를 추정합니다.\n" << endl;

    // SfM 상대 이동 (up-to-scale)
    MatrixXd sfm_displacements(5, 3);
    sfm_displacements << 0.30, 0.00, 0.02,
                          0.28, 0.05, -0.01,
                          0.32, -0.03, 0.01,
                          0.15, 0.20, 0.00,
                          0.25, 0.10, -0.02;

    // IMU 기반 실제 이동 (미터)
    MatrixXd imu_displacements(5, 3);
    imu_displacements << 0.90, 0.01, 0.06,
                          0.85, 0.14, -0.02,
                          0.95, -0.08, 0.04,
                          0.45, 0.61, 0.01,
                          0.76, 0.29, -0.05;

    cout << "  SfM 이동 (up-to-scale)    │  IMU 이동 (미터)" << endl;
    cout << "  ──────────────────────────┼───────────────────────────" << endl;
    for (int i = 0; i < 5; i++) {
        printf("  [%5.2f, %5.2f, %5.2f]    │  [%5.2f, %5.2f, %5.2f]\n",
               sfm_displacements(i,0), sfm_displacements(i,1), sfm_displacements(i,2),
               imu_displacements(i,0), imu_displacements(i,1), imu_displacements(i,2));
    }

    cout << "\n과제: 스케일 s를 최소자승법으로 추정하세요.\n" << endl;
    cout << "  모델: imu_displacement ≈ s * sfm_displacement" << endl;
    cout << "  비용: min_s Σ ||imu_i - s * sfm_i||²\n" << endl;
    cout << "  해석적 해: s = Σ(sfm_i · imu_i) / Σ(sfm_i · sfm_i)\n" << endl;

    // TODO: 여기에 코드를 작성하세요
    // double numerator = 0.0;   // Σ(sfm_i · imu_i)
    // double denominator = 0.0; // Σ(sfm_i · sfm_i)
    // for (int i = 0; i < 5; i++) {
    //     Vector3d sfm_i = sfm_displacements.row(i).transpose();
    //     Vector3d imu_i = imu_displacements.row(i).transpose();
    //     numerator += ???;
    //     denominator += ???;
    // }
    // double s = numerator / denominator;

    cout << "  추정된 스케일 s = _____\n" << endl;
    cout << "  💡 힌트: Eigen의 dot product를 사용하세요: v1.dot(v2)\n" << endl;

    cout << "  추가 질문: 추정된 스케일을 적용한 후 잔차(residual)를 계산하세요.\n";
    cout << "  residual_i = ||imu_i - s * sfm_i||\n" << endl;
}

void problem3_bias_estimation_simulation() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 3: 자이로 바이어스 추정" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "정지 상태에서 IMU 자이로스코프를 10회 측정했습니다.\n";
    cout << "정지 상태이므로 실제 각속도 = 0입니다.\n";
    cout << "측정값에서 바이어스를 추정하세요.\n" << endl;

    // 정지 상태 자이로 측정값 (실제 b_g + 노이즈)
    // 실제 바이어스: b_g = [0.002, -0.001, 0.0015]
    MatrixXd gyro_measurements(10, 3);
    gyro_measurements <<  0.0023, -0.0008,  0.0018,
                           0.0018, -0.0013,  0.0012,
                           0.0021, -0.0009,  0.0016,
                           0.0019, -0.0011,  0.0014,
                           0.0022, -0.0007,  0.0017,
                           0.0017, -0.0012,  0.0013,
                           0.0024, -0.0010,  0.0019,
                           0.0020, -0.0009,  0.0015,
                           0.0021, -0.0011,  0.0016,
                           0.0016, -0.0010,  0.0010;

    cout << "  측정 │   ω_x (rad/s)  │   ω_y (rad/s)  │   ω_z (rad/s)" << endl;
    cout << "  ─────┼────────────────┼────────────────┼────────────────" << endl;
    for (int i = 0; i < 10; i++) {
        printf("   %2d   │    %8.4f     │    %8.4f     │    %8.4f\n",
               i+1, gyro_measurements(i,0), gyro_measurements(i,1), gyro_measurements(i,2));
    }

    cout << "\n과제:" << endl;
    cout << "  1. 각 축별 평균을 구하여 바이어스를 추정하세요." << endl;
    cout << "     b_g_hat = (1/N) * Σ ω_measured_i\n" << endl;
    cout << "  2. 각 축별 표준편차를 구하세요 (노이즈 크기 추정).\n" << endl;
    cout << "  3. 추정된 바이어스의 크기 ||b_g_hat||를 구하세요.\n" << endl;

    // TODO: 여기에 코드를 작성하세요
    // Vector3d b_g_hat = Vector3d::Zero();
    // for (int i = 0; i < 10; i++) {
    //     b_g_hat += gyro_measurements.row(i).transpose();
    // }
    // b_g_hat /= 10.0;

    cout << "  추정된 바이어스: b_g_hat = [_____, _____, _____] rad/s" << endl;
    cout << "  표준편차:        σ      = [_____, _____, _____] rad/s" << endl;
    cout << "  바이어스 크기:   ||b_g|| = _____ rad/s\n" << endl;

    cout << "  💡 힌트: 실제 바이어스는 [0.002, -0.001, 0.0015] rad/s입니다.\n";
    cout << "           노이즈가 있으므로 정확히 일치하지는 않습니다.\n" << endl;

    cout << "  추가 질문: 바이어스를 보정한 후 각 측정의 잔차를 계산하면\n";
    cout << "  얼마나 작아지는지 확인하세요.\n" << endl;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 11 Quiz - Medium" << endl;
    cout << "VIO 초기화 문제" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    problem1_gravity_error_propagation();
    problem2_scale_alignment();
    problem3_bias_estimation_simulation();

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
