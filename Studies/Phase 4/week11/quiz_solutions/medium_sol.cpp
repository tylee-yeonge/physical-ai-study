/**
 * Phase 4 Week 11 - VIO 초기화 중급 퀴즈 풀이
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <iomanip>

using namespace Eigen;
using namespace std;

void problem1_solution() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 1 풀이: 중력 방향 오차 전파" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    double g_mag = 9.81;
    Vector3d g_true(0, 0, -g_mag);
    double t = 5.0;

    vector<double> thetas_deg = {0.1, 0.5, 1.0, 3.0, 5.0};

    cout << "  Step 1: 각 theta에 대해 잘못된 중력 벡터 계산\n" << endl;
    cout << "  g_wrong = [g*sin(theta), 0, -g*cos(theta)]^T" << endl;
    cout << "  delta_g = g_wrong - g_true\n" << endl;

    cout << "  theta (deg) │ delta_g (m/s²)  │ 5초 후 위치 오차 (m)" << endl;
    cout << "  ────────────┼─────────────────┼─────────────────────" << endl;

    for (double theta_deg : thetas_deg) {
        double theta_rad = theta_deg * M_PI / 180.0;

        Vector3d g_wrong(g_mag * sin(theta_rad), 0, -g_mag * cos(theta_rad));
        Vector3d delta_g = g_wrong - g_true;
        double delta_g_norm = delta_g.norm();
        double pos_error = 0.5 * delta_g_norm * t * t;

        printf("    %4.1f°      │     %7.4f      │      %7.3f\n",
               theta_deg, delta_g_norm, pos_error);
    }

    cout << "\n  상세 풀이 (theta = 1.0도):" << endl;
    double theta_1 = 1.0 * M_PI / 180.0;
    Vector3d g_wrong_1(g_mag * sin(theta_1), 0, -g_mag * cos(theta_1));
    Vector3d delta_g_1 = g_wrong_1 - g_true;

    cout << "    g_wrong = [" << g_wrong_1.transpose() << "]" << endl;
    cout << "    g_true  = [" << g_true.transpose() << "]" << endl;
    cout << "    delta_g = [" << delta_g_1.transpose() << "]" << endl;
    cout << "    |delta_g| = " << delta_g_1.norm() << " m/s^2" << endl;
    cout << "    pos_error = 0.5 * " << delta_g_1.norm() << " * 25 = "
         << 0.5 * delta_g_1.norm() * 25 << " m\n" << endl;

    cout << "  추가 질문 답: theta = 0.1도일 때" << endl;
    double theta_01 = 0.1 * M_PI / 180.0;
    Vector3d g_wrong_01(g_mag * sin(theta_01), 0, -g_mag * cos(theta_01));
    Vector3d delta_g_01 = g_wrong_01 - g_true;
    double pos_01 = 0.5 * delta_g_01.norm() * t * t;
    printf("    위치 오차 = %.3f m = %.1f cm\n", pos_01, pos_01 * 100);
    cout << "    → 0.1도 정확도에서도 5초 후 ~21cm 오차!\n" << endl;

    cout << "  핵심 교훈:" << endl;
    cout << "    → 중력 방향 1도 오차 → 5초 후 2.14m 드리프트" << endl;
    cout << "    → VIO 초기화에서 중력 추정이 매우 중요한 이유" << endl;
}

void problem2_solution() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 2 풀이: 스케일 추정 (최소자승법)" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    MatrixXd sfm_displacements(5, 3);
    sfm_displacements << 0.30, 0.00, 0.02,
                          0.28, 0.05, -0.01,
                          0.32, -0.03, 0.01,
                          0.15, 0.20, 0.00,
                          0.25, 0.10, -0.02;

    MatrixXd imu_displacements(5, 3);
    imu_displacements << 0.90, 0.01, 0.06,
                          0.85, 0.14, -0.02,
                          0.95, -0.08, 0.04,
                          0.45, 0.61, 0.01,
                          0.76, 0.29, -0.05;

    cout << "  Step 1: 최소자승 해 유도\n" << endl;
    cout << "    모델: imu_i = s * sfm_i + e_i" << endl;
    cout << "    비용: J(s) = Σ ||imu_i - s * sfm_i||^2" << endl;
    cout << "    미분: dJ/ds = -2 Σ sfm_i^T (imu_i - s * sfm_i) = 0" << endl;
    cout << "    해:   s = Σ(sfm_i . imu_i) / Σ(sfm_i . sfm_i)\n" << endl;

    cout << "  Step 2: 수치 계산\n" << endl;

    double numerator = 0.0;
    double denominator = 0.0;

    for (int i = 0; i < 5; i++) {
        Vector3d sfm_i = sfm_displacements.row(i).transpose();
        Vector3d imu_i = imu_displacements.row(i).transpose();
        double dot_si = sfm_i.dot(imu_i);
        double dot_ss = sfm_i.dot(sfm_i);
        numerator += dot_si;
        denominator += dot_ss;

        printf("    i=%d: sfm.imu = %7.4f, sfm.sfm = %7.4f\n",
               i, dot_si, dot_ss);
    }

    double s = numerator / denominator;

    cout << "\n    분자 (Σ sfm.imu) = " << numerator << endl;
    cout << "    분모 (Σ sfm.sfm) = " << denominator << endl;
    cout << "    s = " << numerator << " / " << denominator << " = " << s << endl;
    cout << "\n    → 추정된 스케일 s ≈ " << fixed << setprecision(4) << s << "\n" << endl;

    cout << "  Step 3: 잔차 계산\n" << endl;
    cout << "    i │ ||imu_i - s*sfm_i|| (m)" << endl;
    cout << "    ──┼───────────────────────" << endl;

    double total_residual = 0.0;
    for (int i = 0; i < 5; i++) {
        Vector3d sfm_i = sfm_displacements.row(i).transpose();
        Vector3d imu_i = imu_displacements.row(i).transpose();
        double residual = (imu_i - s * sfm_i).norm();
        total_residual += residual * residual;
        printf("    %d │       %7.4f\n", i, residual);
    }
    cout << "\n    총 잔차 (RMSE) = " << sqrt(total_residual / 5) << " m" << endl;
    cout << "    → 잔차가 작으면 스케일 추정이 신뢰할 만함" << endl;
}

void problem3_solution() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 3 풀이: 자이로 바이어스 추정" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

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

    Vector3d b_g_true(0.002, -0.001, 0.0015);

    cout << "  Step 1: 평균으로 바이어스 추정\n" << endl;
    cout << "    b_g_hat = (1/N) * Σ omega_i\n" << endl;

    Vector3d b_g_hat = Vector3d::Zero();
    for (int i = 0; i < 10; i++) {
        b_g_hat += gyro_measurements.row(i).transpose();
    }
    b_g_hat /= 10.0;

    cout << "    추정된 바이어스:" << endl;
    printf("      b_g_hat = [%8.5f, %8.5f, %8.5f] rad/s\n",
           b_g_hat(0), b_g_hat(1), b_g_hat(2));
    printf("    실제 바이어스:\n");
    printf("      b_g_true = [%8.5f, %8.5f, %8.5f] rad/s\n",
           b_g_true(0), b_g_true(1), b_g_true(2));
    printf("    추정 오차:\n");
    Vector3d error = b_g_hat - b_g_true;
    printf("      error   = [%8.5f, %8.5f, %8.5f] rad/s\n",
           error(0), error(1), error(2));

    cout << "\n  Step 2: 표준편차 계산\n" << endl;
    cout << "    sigma = sqrt( (1/(N-1)) * Σ(omega_i - b_g_hat)^2 )\n" << endl;

    Vector3d sigma = Vector3d::Zero();
    for (int i = 0; i < 10; i++) {
        Vector3d diff = gyro_measurements.row(i).transpose() - b_g_hat;
        sigma += diff.cwiseProduct(diff);
    }
    sigma = (sigma / 9.0).cwiseSqrt();  // N-1 = 9 (표본 표준편차)

    printf("    sigma = [%8.5f, %8.5f, %8.5f] rad/s\n",
           sigma(0), sigma(1), sigma(2));

    cout << "\n  Step 3: 바이어스 크기\n" << endl;
    printf("    ||b_g_hat|| = %8.5f rad/s\n", b_g_hat.norm());
    printf("    ||b_g_true|| = %8.5f rad/s\n", b_g_true.norm());

    cout << "\n  Step 4: 바이어스 보정 후 잔차\n" << endl;
    cout << "    i │ 보정 전 ||omega_i|| │ 보정 후 ||omega_i - b_g_hat||" << endl;
    cout << "    ──┼─────────────────────┼───────────────────────────" << endl;

    double sum_before = 0, sum_after = 0;
    for (int i = 0; i < 10; i++) {
        Vector3d omega_i = gyro_measurements.row(i).transpose();
        double before = omega_i.norm();
        double after = (omega_i - b_g_hat).norm();
        sum_before += before;
        sum_after += after;
        printf("    %2d│       %8.5f       │          %8.5f\n",
               i+1, before, after);
    }

    cout << "\n    평균 잔차:" << endl;
    printf("      보정 전: %8.5f rad/s\n", sum_before / 10.0);
    printf("      보정 후: %8.5f rad/s\n", sum_after / 10.0);
    printf("      개선율: %.1f%%\n", (1.0 - sum_after / sum_before) * 100);

    cout << "\n  핵심 교훈:" << endl;
    cout << "    → 정지 상태 측정의 평균 = 바이어스 추정 (가장 간단한 방법)" << endl;
    cout << "    → 표준편차 = 센서 노이즈 크기" << endl;
    cout << "    → 10개 샘플로도 바이어스를 상당히 정확하게 추정 가능" << endl;
    cout << "    → 실제 VIO에서는 더 정교한 방법 사용 (움직이면서 추정)" << endl;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 11 Quiz Medium - 풀이" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    return 0;
}
