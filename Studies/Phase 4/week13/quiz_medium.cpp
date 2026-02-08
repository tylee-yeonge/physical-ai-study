/**
 * Phase 4 Week 13 - Camera-IMU 외부 캘리브레이션 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <iomanip>

using namespace Eigen;
using namespace std;

Matrix3d skew(const Vector3d& v) {
    Matrix3d m;
    m <<    0, -v.z(),  v.y(),
         v.z(),     0, -v.x(),
        -v.y(),  v.x(),     0;
    return m;
}

Matrix3d expSO3(const Vector3d& omega) {
    double angle = omega.norm();
    if (angle < 1e-10) return Matrix3d::Identity();
    Vector3d axis = omega / angle;
    Matrix3d K = skew(axis);
    return Matrix3d::Identity()
         + sin(angle) * K + (1.0 - cos(angle)) * K * K;
}

Vector3d logSO3(const Matrix3d& R) {
    double cos_angle = (R.trace() - 1.0) / 2.0;
    cos_angle = max(-1.0, min(1.0, cos_angle));
    double angle = acos(cos_angle);
    if (angle < 1e-10) return Vector3d::Zero();
    Matrix3d log_R = (angle / (2.0 * sin(angle))) * (R - R.transpose());
    return Vector3d(log_R(2,1), log_R(0,2), log_R(1,0));
}

void problem1_extrinsic_error_analysis() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 1: Extrinsic 오차 영향 분석" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "실제 Extrinsic: R_ci = Exp([0.5, 0.0, 0.0]) (x축 28.6도)" << endl;
    cout << "추정에 y축 오차 delta를 추가: R_ci_est = R_ci · Exp([0, delta, 0])\n" << endl;

    Matrix3d R_ci_true = expSO3(Vector3d(0.5, 0.0, 0.0));
    Vector3d g_imu(0, 0, -9.81);

    cout << "과제: 다양한 delta에 대해 가속도 변환 오차를 계산하세요.\n" << endl;
    cout << "  1. a_cam_true = R_ci_true * g_imu" << endl;
    cout << "  2. a_cam_est  = R_ci_est * g_imu" << endl;
    cout << "  3. error = ||a_cam_est - a_cam_true||" << endl;
    cout << "  4. 5초 위치 드리프트 = 0.5 * error * 25\n" << endl;

    vector<double> deltas_deg = {0.1, 0.5, 1.0, 2.0};

    // TODO: 학생이 구현
    cout << "  delta (도) │ 가속도 오차(m/s²) │ 5초 드리프트(m)" << endl;
    cout << "  ───────────┼──────────────────┼────────────────" << endl;
    for (double delta_deg : deltas_deg) {
        cout << "    " << setw(4) << fixed << setprecision(1)
             << delta_deg << "     │    ___________    │   ___________" << endl;
    }

    cout << "\n  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

void problem2_time_offset_effect() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 2: 시간 오프셋 영향 계산" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "로봇이 z축으로 일정 각속도 omega = 3 rad/s로 회전합니다.\n" << endl;
    cout << "Camera와 IMU 사이에 td = 15ms의 시간 오프셋이 있습니다.\n" << endl;

    double omega = 3.0;  // rad/s
    double td = 0.015;   // 15 ms

    cout << "과제:" << endl;
    cout << "  1. td 동안의 회전 각도 (rad, 도)를 구하세요.\n" << endl;
    cout << "  2. 이 회전 오차로 인한 중력 누출을 계산하세요." << endl;
    cout << "     gravity_leak = sin(angle_error) * 9.81\n" << endl;
    cout << "  3. 이 가속도 오차가 1초간 적분되면?" << endl;
    cout << "     velocity_error = gravity_leak * 1.0" << endl;
    cout << "     position_error = 0.5 * gravity_leak * 1.0^2\n" << endl;

    // TODO: 학생이 구현
    cout << "  회전 오차: _____ rad = _____ 도" << endl;
    cout << "  중력 누출: _____ m/s²" << endl;
    cout << "  1초 속도 오차: _____ m/s" << endl;
    cout << "  1초 위치 오차: _____ m\n" << endl;

    cout << "  추가: omega가 0.5 rad/s(저속)일 때는?" << endl;
    cout << "  → 같은 td에서 오차가 어떻게 달라지는지 비교하세요.\n" << endl;
    cout << "  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

void problem3_hand_eye_simple() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 3: 간단한 핸드-아이 문제" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "실제 R_ci = Exp([0, 0, pi/4])  (z축 45도)\n" << endl;
    cout << "3개의 운동에서 Camera와 IMU의 상대 회전이 관측되었습니다.\n" << endl;

    Matrix3d R_ci_true = expSO3(Vector3d(0, 0, M_PI/4));

    // 3개의 IMU 상대 회전
    vector<Vector3d> imu_rotations = {
        {0.3, 0.0, 0.0},   // x축 회전
        {0.0, 0.2, 0.0},   // y축 회전
        {0.0, 0.0, 0.15}   // z축 회전
    };

    cout << "  IMU 상대 회전 (angle-axis):" << endl;
    for (int i = 0; i < 3; i++) {
        Matrix3d R_imu = expSO3(imu_rotations[i]);
        Matrix3d R_cam = R_ci_true * R_imu * R_ci_true.transpose();

        cout << "  쌍 " << i+1 << ":" << endl;
        cout << "    B (IMU): " << logSO3(R_imu).transpose() << endl;
        cout << "    A (Cam): " << logSO3(R_cam).transpose() << "\n" << endl;
    }

    cout << "과제:" << endl;
    cout << "  각 쌍에 대해 A * X = X * B 잔차를 계산하세요.\n" << endl;
    cout << "  1. X = I (초기 추측)에서의 잔차:" << endl;
    cout << "     residual_i = ||A_i * X - X * B_i||_F\n" << endl;
    cout << "  2. X = R_ci_true에서의 잔차:" << endl;
    cout << "     residual_i = ||A_i * X - X * B_i||_F\n" << endl;
    cout << "  (||.||_F = Frobenius norm)\n" << endl;

    // TODO: 학생이 구현
    cout << "  X = I:" << endl;
    cout << "    잔차 1: _____" << endl;
    cout << "    잔차 2: _____" << endl;
    cout << "    잔차 3: _____" << endl;
    cout << "  X = R_ci_true:" << endl;
    cout << "    잔차 1: _____" << endl;
    cout << "    잔차 2: _____" << endl;
    cout << "    잔차 3: _____\n" << endl;

    cout << "  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 13 Quiz - Medium" << endl;
    cout << "Camera-IMU 외부 캘리브레이션" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    problem1_extrinsic_error_analysis();
    problem2_time_offset_effect();
    problem3_hand_eye_simple();

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
