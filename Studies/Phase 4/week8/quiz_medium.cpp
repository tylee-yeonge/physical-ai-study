/**
 * Phase 4 Week 8 - Factor Graph 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
    Eigen::Matrix3d m;
    m <<    0, -v.z(),  v.y(),
         v.z(),     0, -v.x(),
        -v.y(),  v.x(),     0;
    return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& omega) {
    double angle = omega.norm();
    if (angle < 1e-10) return Eigen::Matrix3d::Identity();
    Eigen::Vector3d axis = omega / angle;
    Eigen::Matrix3d K = skew(axis);
    return Eigen::Matrix3d::Identity()
         + std::sin(angle) * K + (1.0 - std::cos(angle)) * K * K;
}

Eigen::Vector3d logSO3(const Eigen::Matrix3d& R) {
    double cos_a = (R.trace() - 1.0) / 2.0;
    cos_a = std::max(-1.0, std::min(1.0, cos_a));
    double angle = std::acos(cos_a);
    if (angle < 1e-10) return Eigen::Vector3d::Zero();
    Eigen::Matrix3d log_R = angle / (2.0 * std::sin(angle)) * (R - R.transpose());
    return Eigen::Vector3d(log_R(2,1), log_R(0,2), log_R(1,0));
}

void problem1_imu_residual_computation() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: IMU Factor 잔차 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "주어진 값:" << std::endl;
    std::cout << "  R_i = I, p_i = [0,0,0], v_i = [1,0,0]" << std::endl;
    std::cout << "  R_j = Exp([0,0,0.1]), p_j = [1,0,-4.905], v_j = [1,0,-9.81]" << std::endl;
    std::cout << "  Δt = 1.0s, g = [0,0,-9.81]\n" << std::endl;
    std::cout << "  ΔR = I^T · R_j, Δv = [0,0,0], Δp = [0,0,0]" << std::endl;
    std::cout << "  (등속 직선 운동 + z축 회전)\n" << std::endl;
    std::cout << "IMU 잔차를 계산하시오.\n" << std::endl;

    Eigen::Matrix3d R_i = Eigen::Matrix3d::Identity();
    Eigen::Vector3d p_i(0, 0, 0);
    Eigen::Vector3d v_i(1, 0, 0);
    Eigen::Vector3d gravity(0, 0, -9.81);
    double dt = 1.0;

    Eigen::Matrix3d R_j = expSO3(Eigen::Vector3d(0, 0, 0.1));
    Eigen::Vector3d v_j(1, 0, -9.81);
    Eigen::Vector3d p_j = p_i + v_i * dt + 0.5 * gravity * dt * dt;

    // Pre-integrated (참값에서 역산)
    Eigen::Matrix3d delta_R = R_i.transpose() * R_j;
    Eigen::Vector3d delta_v = R_i.transpose() * (v_j - v_i - gravity * dt);
    Eigen::Vector3d delta_p = R_i.transpose() *
        (p_j - p_i - v_i * dt - 0.5 * gravity * dt * dt);

    // 잔차 계산
    Eigen::Vector3d r_rot = logSO3(delta_R.transpose() * R_i.transpose() * R_j);
    Eigen::Vector3d r_vel = R_i.transpose() * (v_j - v_i - gravity * dt) - delta_v;
    Eigen::Vector3d r_pos = R_i.transpose() *
        (p_j - p_i - v_i * dt - 0.5 * gravity * dt * dt) - delta_p;

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  r_rotation = " << r_rot.transpose() << std::endl;
    std::cout << "  r_velocity = " << r_vel.transpose() << std::endl;
    std::cout << "  r_position = " << r_pos.transpose() << std::endl;
    std::cout << "  ||r_total|| = " << std::sqrt(r_rot.squaredNorm() + r_vel.squaredNorm() + r_pos.squaredNorm()) << std::endl;
    std::cout << "\n  → 참값이므로 잔차 ≈ 0!" << std::endl;

    // 오차를 주면?
    Eigen::Vector3d p_j_err = p_j + Eigen::Vector3d(0.3, 0.1, 0);
    Eigen::Vector3d r_pos_err = R_i.transpose() *
        (p_j_err - p_i - v_i * dt - 0.5 * gravity * dt * dt) - delta_p;
    std::cout << "\n  p_j에 [0.3, 0.1, 0] 오차 추가:" << std::endl;
    std::cout << "  r_position = " << r_pos_err.transpose() << std::endl;
    std::cout << "  → 잔차가 오차만큼 발생!" << std::endl;
}

void problem2_visual_residual_computation() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Visual Factor 잔차 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "카메라 파라미터: fx=500, fy=500, cx=320, cy=240" << std::endl;
    std::cout << "카메라 포즈: R=I, p=[0,0,0] (원점)" << std::endl;
    std::cout << "3D 점: P_w = [2, 1, 5]" << std::endl;
    std::cout << "측정: z = (520, 340)\n" << std::endl;
    std::cout << "재투영 오차를 계산하시오.\n" << std::endl;

    double fx = 500, fy = 500, cx = 320, cy = 240;
    Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
    Eigen::Vector3d p(0, 0, 0);
    Eigen::Vector3d P_w(2, 1, 5);
    Eigen::Vector2d z_meas(520, 340);

    // 카메라 좌표
    Eigen::Vector3d P_c = R.transpose() * (P_w - p);

    // 투영
    double u = fx * P_c(0) / P_c(2) + cx;
    double v = fy * P_c(1) / P_c(2) + cy;

    Eigen::Vector2d r = z_meas - Eigen::Vector2d(u, v);

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  P_c = R^T(P_w - p) = " << P_c.transpose() << std::endl;
    std::cout << "  u = 500×2/5 + 320 = " << u << std::endl;
    std::cout << "  v = 500×1/5 + 240 = " << v << std::endl;
    std::cout << "  투영: (" << u << ", " << v << ")" << std::endl;
    std::cout << "  측정: (520, 340)" << std::endl;
    std::cout << "  잔차: " << r.transpose() << " 픽셀" << std::endl;
    std::cout << "  ||r|| = " << r.norm() << " 픽셀" << std::endl;
    std::cout << "\n  → 참값이면 잔차 = 0, 여기서는 " << r.norm() << " 픽셀 오차" << std::endl;
}

void problem3_mahalanobis_distance() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 마하라노비스 거리 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "두 Factor의 잔차와 공분산:\n" << std::endl;
    std::cout << "  Factor A: r = [0.1, 0.1], Σ = diag(0.01, 0.01)" << std::endl;
    std::cout << "  Factor B: r = [1.0, 1.0], Σ = diag(4.0, 4.0)\n" << std::endl;
    std::cout << "어떤 Factor의 마하라노비스 거리가 더 큰가?\n" << std::endl;

    // Factor A
    Eigen::Vector2d r_a(0.1, 0.1);
    Eigen::Matrix2d Sigma_a = Eigen::Matrix2d::Identity() * 0.01;
    double d_a = r_a.transpose() * Sigma_a.inverse() * r_a;

    // Factor B
    Eigen::Vector2d r_b(1.0, 1.0);
    Eigen::Matrix2d Sigma_b = Eigen::Matrix2d::Identity() * 4.0;
    double d_b = r_b.transpose() * Sigma_b.inverse() * r_b;

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  Factor A: r^T·Σ^{-1}·r = [0.1,0.1]·diag(100,100)·[0.1,0.1]^T" << std::endl;
    std::cout << "          = " << d_a << std::endl;
    std::cout << "\n  Factor B: r^T·Σ^{-1}·r = [1,1]·diag(0.25,0.25)·[1,1]^T" << std::endl;
    std::cout << "          = " << d_b << "\n" << std::endl;

    std::cout << "  Factor A > Factor B: " << (d_a > d_b ? "맞음" : "틀림") << std::endl;
    std::cout << "\n  해석:" << std::endl;
    std::cout << "    Factor A: 잔차는 작지만 센서가 매우 정확" << std::endl;
    std::cout << "    → 0.1의 오차도 '큰' 것 (기대 오차 0.1 대비)" << std::endl;
    std::cout << "    Factor B: 잔차는 크지만 센서가 부정확" << std::endl;
    std::cout << "    → 1.0의 오차는 '작은' 것 (기대 오차 2.0 대비)" << std::endl;
    std::cout << "\n  → 마하라노비스 거리는 '기대 대비 얼마나 벗어났나'를 측정!" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 8 Quiz - Medium (Factor Graph 계산)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_imu_residual_computation();
    problem2_visual_residual_computation();
    problem3_mahalanobis_distance();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
