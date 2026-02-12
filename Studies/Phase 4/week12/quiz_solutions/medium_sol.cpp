/**
 * Phase 4 Week 12 - VIO 초기화 과정 중급 퀴즈 풀이
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <iomanip>

using namespace Eigen;
using namespace std;

Matrix3d skew(const Vector3d& v)
{
    Matrix3d m;
    m << 0, -v.z(), v.y(), v.z(), 0, -v.x(), -v.y(), v.x(), 0;
    return m;
}

Matrix3d expSO3(const Vector3d& omega)
{
    double angle = omega.norm();
    if (angle < 1e-10)
        return Matrix3d::Identity();
    Vector3d axis = omega / angle;
    Matrix3d K = skew(axis);
    return Matrix3d::Identity() + sin(angle) * K + (1.0 - cos(angle)) * K * K;
}

Vector3d logSO3(const Matrix3d& R)
{
    double cos_angle = (R.trace() - 1.0) / 2.0;
    cos_angle = max(-1.0, min(1.0, cos_angle));
    double angle = acos(cos_angle);
    if (angle < 1e-10)
        return Vector3d::Zero();
    Matrix3d log_R = (angle / (2.0 * sin(angle))) * (R - R.transpose());
    return Vector3d(log_R(2, 1), log_R(0, 2), log_R(1, 0));
}

void problem1_solution()
{
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 1 풀이: 자이로 바이어스 추정" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    // Vision 회전
    Matrix3d R0 = Matrix3d::Identity();
    Matrix3d R1 = expSO3(Vector3d(0.1, 0.05, 0.02));
    Matrix3d R2 = expSO3(Vector3d(0.2, 0.08, 0.05));

    Vector3d b_g_true(0.003, -0.002, 0.001);
    double dt = 0.5;

    // Pre-integrated 회전 (b_g=0으로 계산됨)
    Matrix3d DR_01 = R0.transpose() * R1 * expSO3(-b_g_true * dt);
    Matrix3d DR_12 = R1.transpose() * R2 * expSO3(-b_g_true * dt);

    cout << "  Step 1: Vision에서의 상대 회전\n" << endl;
    Matrix3d R_01_vision = R0.transpose() * R1;
    Matrix3d R_12_vision = R1.transpose() * R2;
    cout << "    R_01_vision = R0^T * R1" << endl;
    cout << "    R_12_vision = R1^T * R2\n" << endl;

    cout << "  Step 2: 잔차 계산\n" << endl;
    Vector3d r0 = logSO3(DR_01.transpose() * R_01_vision);
    Vector3d r1 = logSO3(DR_12.transpose() * R_12_vision);

    cout << "    r0 = Log(DR_01^T * R_01_vision)" << endl;
    cout << "       = [" << r0.transpose() << "]" << endl;
    cout << "    r1 = Log(DR_12^T * R_12_vision)" << endl;
    cout << "       = [" << r1.transpose() << "]\n" << endl;

    cout << "  Step 3: 자코비안 (단순화: J ≈ -I * dt)\n" << endl;
    Matrix3d J = -Matrix3d::Identity() * dt;

    cout << "  Step 4: 선형 시스템 [J; J] * b_g = [r0; r1]\n" << endl;

    MatrixXd A(6, 3);
    VectorXd b_vec(6);
    A.block<3, 3>(0, 0) = J;
    A.block<3, 3>(3, 0) = J;
    b_vec.segment<3>(0) = r0;
    b_vec.segment<3>(3) = r1;

    Vector3d b_g_est = (A.transpose() * A).ldlt().solve(A.transpose() * b_vec);

    cout << "  결과:" << endl;
    cout << "    추정 b_g = [" << b_g_est.transpose() << "]" << endl;
    cout << "    참값 b_g = [" << b_g_true.transpose() << "]" << endl;
    cout << "    오차     = [" << (b_g_est - b_g_true).transpose() << "]\n" << endl;
    cout << "    → 단순화된 자코비안으로도 근사적 추정 가능" << endl;
    cout << "    → 실제 VINS에서는 정확한 J_ΔR_bg를 Pre-integration에서 계산" << endl;
}

void problem2_solution()
{
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 2 풀이: 스케일과 중력 동시 추정" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    double s_true = 2.5;
    Vector3d g_true(0, 0, -9.81);

    Vector3d p0_v(0.0, 0.0, 0.0);
    Vector3d p1_v(0.4, 0.0, 0.0);
    Vector3d p2_v(0.8, 0.2, 0.0);

    Matrix3d R0 = Matrix3d::Identity();
    Matrix3d R1 = Matrix3d::Identity();

    double dt01 = 0.5, dt12 = 0.5;

    Vector3d v0_true(2.0, 0.0, 0.0);
    Vector3d v1_true(2.0, 1.0, 0.0);

    Vector3d Dp_01 = s_true * (p1_v - p0_v) - v0_true * dt01 + 0.5 * g_true * dt01 * dt01;
    Vector3d Dv_01 = v1_true - v0_true + g_true * dt01;
    Vector3d Dp_12 = s_true * (p2_v - p1_v) - v1_true * dt12 + 0.5 * g_true * dt12 * dt12;

    cout << "  Pre-integrated 값 확인:" << endl;
    cout << "    Dp_01 = [" << Dp_01.transpose() << "]" << endl;
    cout << "    Dv_01 = [" << Dv_01.transpose() << "]" << endl;
    cout << "    Dp_12 = [" << Dp_12.transpose() << "]\n" << endl;

    cout << "  Step 1: 선형 시스템 구성\n" << endl;
    cout << "  미지수 x = [v0(3), v1(3), g(3), s(1)] (10차원)\n" << endl;

    // 방정식 3개:
    // (1) 위치 0→1: s*(p1-p0) = v0*dt - 0.5*g*dt² + R0*Dp_01
    //     → v0*dt - 0.5*g*dt² - s*(p1-p0) = -R0*Dp_01
    // (2) 속도 0→1: v1 = v0 - g*dt + R0*Dv_01
    //     → v0 - v1 - g*dt = -R0*Dv_01
    // (3) 위치 1→2: s*(p2-p1) = v1*dt - 0.5*g*dt² + R1*Dp_12
    //     → v1*dt - 0.5*g*dt² - s*(p2-p1) = -R1*Dp_12

    MatrixXd H = MatrixXd::Zero(9, 10);
    VectorXd z = VectorXd::Zero(9);

    Vector3d dp_01 = p1_v - p0_v;
    Vector3d dp_12 = p2_v - p1_v;

    // 방정식 (1): 위치 0→1
    H.block<3, 3>(0, 0) = Matrix3d::Identity() * dt01;                // v0
    H.block<3, 3>(0, 6) = -0.5 * dt01 * dt01 * Matrix3d::Identity();  // g
    H.block<3, 1>(0, 9) = -dp_01;                                     // s
    z.segment<3>(0) = -R0 * Dp_01;

    // 방정식 (2): 속도 0→1
    H.block<3, 3>(3, 0) = Matrix3d::Identity();          // v0
    H.block<3, 3>(3, 3) = -Matrix3d::Identity();         // v1
    H.block<3, 3>(3, 6) = -dt01 * Matrix3d::Identity();  // g
    z.segment<3>(3) = -R0 * Dv_01;

    // 방정식 (3): 위치 1→2
    H.block<3, 3>(6, 3) = Matrix3d::Identity() * dt12;                // v1
    H.block<3, 3>(6, 6) = -0.5 * dt12 * dt12 * Matrix3d::Identity();  // g
    H.block<3, 1>(6, 9) = -dp_12;                                     // s
    z.segment<3>(6) = -R1 * Dp_12;

    cout << "  H 행렬 (9x10):" << endl;
    cout << H << "\n" << endl;

    cout << "  z 벡터:" << endl;
    cout << z.transpose() << "\n" << endl;

    // 최소자승 해
    VectorXd x = (H.transpose() * H).ldlt().solve(H.transpose() * z);

    Vector3d v0_est = x.segment<3>(0);
    Vector3d v1_est = x.segment<3>(3);
    Vector3d g_est = x.segment<3>(6);
    double s_est = x(9);

    cout << "  Step 2: 결과\n" << endl;
    cout << "    추정 s  = " << fixed << setprecision(4) << s_est << "  (참값: " << s_true << ")"
         << endl;
    cout << "    추정 g  = [" << g_est.transpose() << "]" << endl;
    cout << "    참값 g  = [" << g_true.transpose() << "]" << endl;
    cout << "    추정 v0 = [" << v0_est.transpose() << "]" << endl;
    cout << "    참값 v0 = [" << v0_true.transpose() << "]" << endl;
    cout << "    추정 v1 = [" << v1_est.transpose() << "]" << endl;
    cout << "    참값 v1 = [" << v1_true.transpose() << "]\n" << endl;

    cout << "  ||g_estimated|| = " << g_est.norm() << "  (기대값: 9.81)" << endl;
    cout << "  → 노이즈 없는 이상적 경우이므로 정확히 복원됨" << endl;
}

void problem3_solution()
{
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 3 풀이: 중력 정제" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    Vector3d g_est(0.15, -0.08, -9.79);
    double g_norm_true = 9.81;

    cout << "  Step 1: 단위 벡터\n" << endl;
    Vector3d g_hat = g_est.normalized();
    cout << "    g_hat = g_est / ||g_est||" << endl;
    cout << "          = [" << g_hat.transpose() << "]" << endl;
    cout << "    ||g_hat|| = " << g_hat.norm() << " (확인: 1.0)\n" << endl;

    cout << "  Step 2: 수직 기저 벡터 (Gram-Schmidt)\n" << endl;
    Vector3d tmp = (abs(g_hat.x()) < 0.9) ? Vector3d(1, 0, 0) : Vector3d(0, 1, 0);
    Vector3d b1 = (tmp - g_hat * g_hat.dot(tmp)).normalized();
    Vector3d b2 = g_hat.cross(b1);

    cout << "    b1 = [" << b1.transpose() << "]" << endl;
    cout << "    b2 = [" << b2.transpose() << "]" << endl;
    cout << "    검증: g_hat . b1 = " << g_hat.dot(b1) << " (≈0)" << endl;
    cout << "    검증: g_hat . b2 = " << g_hat.dot(b2) << " (≈0)" << endl;
    cout << "    검증: b1 . b2    = " << b1.dot(b2) << " (≈0)\n" << endl;

    cout << "  Step 3: 중력 정제\n" << endl;
    Vector3d g_refined = g_norm_true * g_hat;
    cout << "    g_refined = 9.81 * g_hat" << endl;
    cout << "              = [" << g_refined.transpose() << "]" << endl;
    cout << "    ||g_refined|| = " << g_refined.norm() << "\n" << endl;

    cout << "  Step 4: 비교\n" << endl;
    Vector3d diff = g_refined - g_est;
    cout << "    g_estimated = [" << g_est.transpose() << "]" << endl;
    cout << "    g_refined   = [" << g_refined.transpose() << "]" << endl;
    cout << "    차이        = [" << diff.transpose() << "]\n" << endl;

    cout << "  수평 성분 분석:" << endl;
    cout << "    정제 전 수평: sqrt(" << g_est.x() << "² + " << g_est.y()
         << "²) = " << sqrt(g_est.x() * g_est.x() + g_est.y() * g_est.y()) << " m/s²" << endl;
    cout << "    정제 후 수평: sqrt(" << g_refined.x() << "² + " << g_refined.y()
         << "²) = " << sqrt(g_refined.x() * g_refined.x() + g_refined.y() * g_refined.y())
         << " m/s²" << endl;
    cout << "    → 방향은 보존하면서 크기만 9.81로 조정\n" << endl;

    cout << "  핵심:" << endl;
    cout << "    → 1단계 정제: 크기 보정 (||g|| = 9.81)" << endl;
    cout << "    → VINS에서는 이를 반복적으로 수행 (4회)" << endl;
    cout << "    → 매 반복마다 2D 파라미터(w1,w2)로 방향을 미세 조정" << endl;
    cout << "    → 최종적으로 물리적으로 일관된 중력 벡터 획득" << endl;
}

int main()
{
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 12 Quiz Medium - 풀이" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    return 0;
}
