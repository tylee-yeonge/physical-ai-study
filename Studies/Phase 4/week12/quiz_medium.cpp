/**
 * Phase 4 Week 12 - VIO 초기화 과정 중급 퀴즈
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

void problem1_gyro_bias_estimation() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 1: 자이로 바이어스 추정" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "3개의 키프레임에서 Vision 회전과 IMU Pre-integrated 회전이\n";
    cout << "주어졌습니다. 자이로 바이어스를 추정하세요.\n" << endl;

    // Vision 회전 (월드 기준)
    Matrix3d R0 = Matrix3d::Identity();
    Matrix3d R1 = expSO3(Vector3d(0.1, 0.05, 0.02));
    Matrix3d R2 = expSO3(Vector3d(0.2, 0.08, 0.05));

    // Pre-integrated 회전 (b_g = 0으로 계산됨)
    // 실제 b_g = [0.003, -0.002, 0.001] 존재
    Vector3d b_g_true(0.003, -0.002, 0.001);
    double dt = 0.5;

    // ΔR_01 = expSO3((omega_true + b_g) * dt)에서 b_g=0으로 적분한 결과
    // 즉, 바이어스로 인해 약간 틀린 ΔR
    Matrix3d DR_01 = R0.transpose() * R1 * expSO3(-b_g_true * dt);
    Matrix3d DR_12 = R1.transpose() * R2 * expSO3(-b_g_true * dt);

    cout << "  Vision 회전:" << endl;
    cout << "    R0 = I (단위행렬)" << endl;
    cout << "    R1 = ExpSO3([0.1, 0.05, 0.02])" << endl;
    cout << "    R2 = ExpSO3([0.2, 0.08, 0.05])\n" << endl;

    cout << "  Pre-integrated 회전 (b_g=0으로 계산):" << endl;
    cout << "    DR_01:" << endl;
    cout << DR_01 << "\n" << endl;
    cout << "    DR_12:" << endl;
    cout << DR_12 << "\n" << endl;

    cout << "과제:" << endl;
    cout << "  Vision에서의 상대 회전: R_ij = R_i^T · R_j" << endl;
    cout << "  잔차: r_k = Log(DR_ij^T · R_ij)\n" << endl;
    cout << "  1차 근사: ΔR(b_g) ≈ ΔR(0) · Exp(J · δb_g)" << endl;
    cout << "  여기서 J ≈ -I * dt (단순화)\n" << endl;

    // TODO: 학생이 구현
    // Vector3d r0 = logSO3(DR_01.transpose() * R0.transpose() * R1);
    // Vector3d r1 = logSO3(DR_12.transpose() * R1.transpose() * R2);
    // J = -I * dt
    // [J; J] * b_g = [r0; r1]

    cout << "  추정된 b_g = [_____, _____, _____] rad/s" << endl;
    cout << "  (참값: [0.003, -0.002, 0.001])\n" << endl;
    cout << "  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

void problem2_scale_gravity_estimation() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 2: 스케일과 중력 동시 추정" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "3개의 키프레임에서 아래 데이터가 주어졌습니다.\n" << endl;

    // 실제 스케일 s = 2.5
    // 실제 중력 g = [0, 0, -9.81]
    double s_true = 2.5;
    Vector3d g_true(0, 0, -9.81);

    // Vision 위치 (up-to-scale, s로 나눈 값)
    Vector3d p0_v(0.0, 0.0, 0.0);
    Vector3d p1_v(0.4, 0.0, 0.0);    // 실제: 1.0m → /2.5 = 0.4
    Vector3d p2_v(0.8, 0.2, 0.0);    // 실제: 2.0, 0.5 → /2.5

    // Vision 회전 (정확하다고 가정)
    Matrix3d R0 = Matrix3d::Identity();
    Matrix3d R1 = Matrix3d::Identity();
    Matrix3d R2 = Matrix3d::Identity();

    // 시간 간격
    double dt01 = 0.5;
    double dt12 = 0.5;

    // Pre-integrated 값 (b_g 보정 후)
    // 실제 값에서 역산
    Vector3d v0_true(2.0, 0.0, 0.0);
    Vector3d v1_true(2.0, 1.0, 0.0);

    Vector3d Dp_01 = s_true*(p1_v - p0_v) - v0_true*dt01 + 0.5*g_true*dt01*dt01;
    Vector3d Dv_01 = v1_true - v0_true + g_true * dt01;
    Vector3d Dp_12 = s_true*(p2_v - p1_v) - v1_true*dt12 + 0.5*g_true*dt12*dt12;

    cout << "  Vision 위치 (up-to-scale):" << endl;
    cout << "    p0 = [" << p0_v.transpose() << "]" << endl;
    cout << "    p1 = [" << p1_v.transpose() << "]" << endl;
    cout << "    p2 = [" << p2_v.transpose() << "]\n" << endl;

    cout << "  R0 = R1 = R2 = I (단순화)\n" << endl;

    cout << "  Pre-integrated:" << endl;
    cout << "    Dp_01 = [" << Dp_01.transpose() << "]" << endl;
    cout << "    Dv_01 = [" << Dv_01.transpose() << "]" << endl;
    cout << "    Dp_12 = [" << Dp_12.transpose() << "]\n" << endl;

    cout << "  dt_01 = dt_12 = " << dt01 << "s\n" << endl;

    cout << "과제: 선형 시스템 Hx = z를 구성하여 풀이하세요.\n" << endl;
    cout << "  미지수: x = [v0(3), v1(3), g(3), s(1)]  (10차원)\n" << endl;
    cout << "  위치 방정식: s*(p_j - p_i) = v_i*dt - 0.5*g*dt² + Ri*Dp_ij" << endl;
    cout << "  속도 방정식: v_j = v_i - g*dt + Ri*Dv_ij\n" << endl;

    // TODO: 학생이 구현
    // H(9x10) * x(10) = z(9)

    cout << "  추정 결과:" << endl;
    cout << "    s = _____  (참값: 2.5)" << endl;
    cout << "    g = [_____, _____, _____]  (참값: [0, 0, -9.81])" << endl;
    cout << "    v0 = [_____, _____, _____]  (참값: [2.0, 0, 0])" << endl;
    cout << "\n  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

void problem3_gravity_refinement() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 3: 중력 정제" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "선형 Alignment에서 다음과 같은 중력 추정값을 얻었습니다:\n" << endl;
    Vector3d g_est(0.15, -0.08, -9.79);
    double g_norm_est = g_est.norm();
    double g_norm_true = 9.81;

    cout << "  g_estimated = [" << g_est.transpose() << "]" << endl;
    cout << "  ||g_estimated|| = " << g_norm_est << endl;
    cout << "  ||g_true|| = " << g_norm_true << "\n" << endl;

    cout << "과제:" << endl;
    cout << "  1. g_hat = g_estimated / ||g_estimated|| 를 구하세요\n" << endl;
    cout << "  2. g_hat에 수직인 기저 벡터 b1, b2를 구하세요" << endl;
    cout << "     (Gram-Schmidt 사용)\n" << endl;
    cout << "  3. g_refined = 9.81 * g_hat 을 구하세요\n" << endl;
    cout << "  4. g_estimated와 g_refined의 차이를 구하세요" << endl;
    cout << "     (수평 성분은 어떻게 변하는가?)\n" << endl;

    // TODO: 학생이 구현
    // Vector3d g_hat = g_est.normalized();
    // Vector3d g_refined = g_norm_true * g_hat;

    cout << "  g_hat     = [_____, _____, _____]" << endl;
    cout << "  g_refined = [_____, _____, _____]" << endl;
    cout << "  ||g_refined|| = _____\n" << endl;

    cout << "  추가: 이 정제 과정이 수평 성분을 어떻게 바꾸는지 관찰하세요." << endl;
    cout << "\n  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 12 Quiz - Medium" << endl;
    cout << "VIO 초기화 과정" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    problem1_gyro_bias_estimation();
    problem2_scale_gravity_estimation();
    problem3_gravity_refinement();

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
