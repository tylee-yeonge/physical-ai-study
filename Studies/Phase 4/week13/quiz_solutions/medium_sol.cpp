/**
 * Phase 4 Week 13 - Camera-IMU 외부 캘리브레이션 중급 퀴즈 풀이
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <iomanip>
#include <vector>

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

void problem1_solution() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 1 풀이: Extrinsic 오차 영향 분석" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    Matrix3d R_ci_true = expSO3(Vector3d(0.5, 0.0, 0.0));
    Vector3d g_imu(0, 0, -9.81);

    Vector3d a_cam_true = R_ci_true * g_imu;

    cout << "  Step 1: 정확한 변환" << endl;
    cout << "    a_cam_true = R_ci_true * g_imu" << endl;
    cout << "    = [" << a_cam_true.transpose() << "]\n" << endl;

    vector<double> deltas_deg = {0.1, 0.5, 1.0, 2.0};

    cout << "  Step 2: 각 delta에 대한 오차\n" << endl;
    cout << "  delta (도) │ 가속도 오차(m/s²) │ 5초 드리프트(m)" << endl;
    cout << "  ───────────┼──────────────────┼────────────────" << endl;

    for (double delta_deg : deltas_deg) {
        double delta_rad = delta_deg * M_PI / 180.0;
        Matrix3d R_ci_est = R_ci_true * expSO3(Vector3d(0, delta_rad, 0));

        Vector3d a_cam_est = R_ci_est * g_imu;
        double error = (a_cam_est - a_cam_true).norm();
        double drift_5s = 0.5 * error * 25.0;

        printf("    %4.1f°     │     %8.4f      │     %8.3f\n",
               delta_deg, error, drift_5s);
    }

    cout << "\n  상세 (delta = 1.0도):" << endl;
    double d1 = 1.0 * M_PI / 180.0;
    Matrix3d R_ci_1 = R_ci_true * expSO3(Vector3d(0, d1, 0));
    Vector3d a_cam_1 = R_ci_1 * g_imu;
    Vector3d diff = a_cam_1 - a_cam_true;

    cout << "    a_cam_est  = [" << a_cam_1.transpose() << "]" << endl;
    cout << "    a_cam_true = [" << a_cam_true.transpose() << "]" << endl;
    cout << "    차이       = [" << diff.transpose() << "]" << endl;
    cout << "    ||차이||   = " << diff.norm() << " m/s²\n" << endl;

    cout << "  핵심: 1도 오차 → ~0.17 m/s² → 5초 후 ~2.1m 드리프트!" << endl;
    cout << "  → Extrinsic 회전은 0.1도 이내 정확도가 필요" << endl;
}

void problem2_solution() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 2 풀이: 시간 오프셋 영향 계산" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    double omega = 3.0;
    double td = 0.015;

    cout << "  고속 회전 (omega = 3 rad/s):\n" << endl;

    double angle_error = omega * td;
    double angle_error_deg = angle_error * 180.0 / M_PI;
    double gravity_leak = sin(angle_error) * 9.81;
    double vel_error = gravity_leak * 1.0;
    double pos_error = 0.5 * gravity_leak * 1.0;

    cout << "  Step 1: td 동안의 회전" << endl;
    cout << "    angle = omega * td = " << omega << " * " << td << endl;
    cout << "    = " << angle_error << " rad" << endl;
    cout << "    = " << angle_error_deg << " 도\n" << endl;

    cout << "  Step 2: 중력 누출" << endl;
    cout << "    gravity_leak = sin(" << angle_error << ") * 9.81" << endl;
    cout << "    = " << gravity_leak << " m/s²\n" << endl;

    cout << "  Step 3: 1초간 적분" << endl;
    cout << "    velocity_error = " << gravity_leak << " * 1.0 = " << vel_error << " m/s" << endl;
    cout << "    position_error = 0.5 * " << gravity_leak << " * 1.0 = " << pos_error << " m\n" << endl;

    cout << "  ────────────────────────────────\n" << endl;

    double omega_slow = 0.5;
    double angle_slow = omega_slow * td;
    double leak_slow = sin(angle_slow) * 9.81;

    cout << "  저속 회전 (omega = 0.5 rad/s):\n" << endl;
    cout << "    angle = " << omega_slow << " * " << td << " = " << angle_slow << " rad" << endl;
    cout << "    = " << angle_slow * 180 / M_PI << " 도" << endl;
    cout << "    gravity_leak = " << leak_slow << " m/s²" << endl;
    cout << "    1초 위치 오차 = " << 0.5 * leak_slow << " m\n" << endl;

    cout << "  비교:" << endl;
    cout << "    고속(" << omega << " rad/s): " << gravity_leak << " m/s²" << endl;
    cout << "    저속(" << omega_slow << " rad/s): " << leak_slow << " m/s²" << endl;
    cout << "    비율: " << gravity_leak / leak_slow << "배\n" << endl;

    cout << "  결론:" << endl;
    cout << "    → 동일한 td라도 회전 속도에 비례하여 오차 증가" << endl;
    cout << "    → 고속 회전 시나리오에서 td 보정이 특히 중요" << endl;
    cout << "    → VINS는 td를 온라인 추정으로 해결" << endl;
}

void problem3_solution() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 3 풀이: 간단한 핸드-아이 문제" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    Matrix3d R_ci_true = expSO3(Vector3d(0, 0, M_PI/4));

    vector<Vector3d> imu_rotations = {
        {0.3, 0.0, 0.0},
        {0.0, 0.2, 0.0},
        {0.0, 0.0, 0.15}
    };

    // 회전 쌍 생성
    vector<Matrix3d> A_list, B_list;
    for (const auto& omega : imu_rotations) {
        Matrix3d B = expSO3(omega);
        Matrix3d A = R_ci_true * B * R_ci_true.transpose();
        A_list.push_back(A);
        B_list.push_back(B);
    }

    // X = I 에서의 잔차
    cout << "  Case 1: X = I (초기 추측)\n" << endl;
    Matrix3d X_init = Matrix3d::Identity();

    double total_residual_I = 0;
    for (int i = 0; i < 3; i++) {
        Matrix3d AX = A_list[i] * X_init;
        Matrix3d XB = X_init * B_list[i];
        double residual = (AX - XB).norm();  // Frobenius norm
        total_residual_I += residual;

        cout << "    쌍 " << i+1 << ": ||A*X - X*B||_F = " << residual << endl;
    }
    cout << "    합계: " << total_residual_I << "\n" << endl;

    // X = R_ci_true 에서의 잔차
    cout << "  Case 2: X = R_ci_true (정답)\n" << endl;
    Matrix3d X_true = R_ci_true;

    double total_residual_true = 0;
    for (int i = 0; i < 3; i++) {
        Matrix3d AX = A_list[i] * X_true;
        Matrix3d XB = X_true * B_list[i];
        double residual = (AX - XB).norm();
        total_residual_true += residual;

        cout << "    쌍 " << i+1 << ": ||A*X - X*B||_F = " << residual << endl;
    }
    cout << "    합계: " << total_residual_true << "\n" << endl;

    cout << "  검증:" << endl;
    cout << "    X=I 잔차 합: " << total_residual_I << endl;
    cout << "    X=R_ci 잔차 합: " << total_residual_true << " (≈0)\n" << endl;

    cout << "  X = R_ci_true일 때 잔차가 0인 이유:" << endl;
    cout << "    A * X = R_ci * B * R_ci^T * R_ci = R_ci * B" << endl;
    cout << "    X * B = R_ci * B" << endl;
    cout << "    → A * X = X * B (정확히 성립)\n" << endl;

    cout << "  핵심:" << endl;
    cout << "    → AX=XB를 만족하는 X가 바로 Extrinsic 회전" << endl;
    cout << "    → 잔차를 최소화하는 X를 SVD로 풀면 됨" << endl;
    cout << "    → 3축 회전이 모두 필요 (관측성 확보)" << endl;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 13 Quiz Medium - 풀이" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    return 0;
}
