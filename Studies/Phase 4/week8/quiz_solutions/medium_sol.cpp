/**
 * Phase 4 Week 8 - Factor Graph 중급 퀴즈 풀이
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

Eigen::Matrix3d skew(const Eigen::Vector3d& v)
{
    Eigen::Matrix3d m;
    m << 0, -v.z(), v.y(), v.z(), 0, -v.x(), -v.y(), v.x(), 0;
    return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& omega)
{
    double angle = omega.norm();
    if (angle < 1e-10)
        return Eigen::Matrix3d::Identity();
    Eigen::Vector3d axis = omega / angle;
    Eigen::Matrix3d K = skew(axis);
    return Eigen::Matrix3d::Identity() + std::sin(angle) * K + (1.0 - std::cos(angle)) * K * K;
}

Eigen::Vector3d logSO3(const Eigen::Matrix3d& R)
{
    double cos_a = (R.trace() - 1.0) / 2.0;
    cos_a = std::max(-1.0, std::min(1.0, cos_a));
    double angle = std::acos(cos_a);
    if (angle < 1e-10)
        return Eigen::Vector3d::Zero();
    Eigen::Matrix3d log_R = angle / (2.0 * std::sin(angle)) * (R - R.transpose());
    return Eigen::Vector3d(log_R(2, 1), log_R(0, 2), log_R(1, 0));
}

void problem1_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: IMU Factor 잔차 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Eigen::Matrix3d R_i = Eigen::Matrix3d::Identity();
    Eigen::Vector3d p_i(0, 0, 0), v_i(1, 0, 0);
    Eigen::Vector3d gravity(0, 0, -9.81);
    double dt = 1.0;

    Eigen::Matrix3d R_j = expSO3(Eigen::Vector3d(0, 0, 0.1));
    Eigen::Vector3d v_j(1, 0, -9.81);
    Eigen::Vector3d p_j = p_i + v_i * dt + 0.5 * gravity * dt * dt;

    // Pre-integrated (참값)
    Eigen::Matrix3d delta_R = R_i.transpose() * R_j;
    Eigen::Vector3d delta_v = R_i.transpose() * (v_j - v_i - gravity * dt);
    Eigen::Vector3d delta_p = R_i.transpose() * (p_j - p_i - v_i * dt - 0.5 * gravity * dt * dt);

    std::cout << "  Step 1: Pre-integrated measurements" << std::endl;
    std::cout << "    ΔR = R_i^T · R_j" << std::endl;
    std::cout << "    log(ΔR) = " << logSO3(delta_R).transpose() << std::endl;
    std::cout << "    Δv = R_i^T · (v_j - v_i - g·Δt) = " << delta_v.transpose() << std::endl;
    std::cout << "    Δp = R_i^T · (p_j - p_i - v_i·Δt - 0.5·g·Δt²) = " << delta_p.transpose()
              << "\n"
              << std::endl;

    // 잔차 (참값이면 0)
    Eigen::Vector3d r_rot = logSO3(delta_R.transpose() * R_i.transpose() * R_j);
    Eigen::Vector3d r_vel = R_i.transpose() * (v_j - v_i - gravity * dt) - delta_v;
    Eigen::Vector3d r_pos =
        R_i.transpose() * (p_j - p_i - v_i * dt - 0.5 * gravity * dt * dt) - delta_p;

    std::cout << "  Step 2: 잔차 계산 (참값)" << std::endl;
    std::cout << "    r_rotation = " << r_rot.transpose() << std::endl;
    std::cout << "    r_velocity = " << r_vel.transpose() << std::endl;
    std::cout << "    r_position = " << r_pos.transpose() << std::endl;
    std::cout << "    → 모두 0! (참값이므로)\n" << std::endl;

    // 오차 추가
    Eigen::Vector3d p_j_err = p_j + Eigen::Vector3d(0.3, 0.1, 0);
    Eigen::Vector3d r_pos_err =
        R_i.transpose() * (p_j_err - p_i - v_i * dt - 0.5 * gravity * dt * dt) - delta_p;

    std::cout << "  Step 3: 위치에 [0.3, 0.1, 0] 오차 추가" << std::endl;
    std::cout << "    r_position = " << r_pos_err.transpose() << std::endl;
    std::cout << "    ||r_pos|| = " << r_pos_err.norm() << "\n" << std::endl;
    std::cout << "    → 잔차 = 오차와 동일! (R_i=I이므로)" << std::endl;
    std::cout << "    → 최적화가 이 잔차를 줄이도록 p_j를 수정" << std::endl;
}

void problem2_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: Visual Factor 잔차 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double fx = 500, fy = 500, cx = 320, cy = 240;
    Eigen::Vector3d P_w(2, 1, 5);
    Eigen::Vector2d z_meas(520, 340);

    std::cout << "  Step 1: 카메라 좌표 변환" << std::endl;
    std::cout << "    P_c = R^T(P_w - p) = I·([2,1,5] - [0,0,0]) = [2, 1, 5]\n" << std::endl;

    double u = fx * 2.0 / 5.0 + cx;
    double v = fy * 1.0 / 5.0 + cy;

    std::cout << "  Step 2: 투영" << std::endl;
    std::cout << "    u = fx·X/Z + cx = 500·2/5 + 320 = " << u << std::endl;
    std::cout << "    v = fy·Y/Z + cy = 500·1/5 + 240 = " << v << "\n" << std::endl;

    Eigen::Vector2d z_proj(u, v);
    Eigen::Vector2d r = z_meas - z_proj;

    std::cout << "  Step 3: 잔차" << std::endl;
    std::cout << "    r = z_meas - z_proj = (" << z_meas(0) << "," << z_meas(1) << ") - (" << u
              << "," << v << ")" << std::endl;
    std::cout << "    r = " << r.transpose() << " 픽셀" << std::endl;
    std::cout << "    ||r|| = " << r.norm() << " 픽셀\n" << std::endl;

    std::cout << "  해석:" << std::endl;
    std::cout << "    투영: (" << u << ", " << v << ") = 참값" << std::endl;
    std::cout << "    측정: (520, 340) = 참값과 동일!" << std::endl;
    std::cout << "    → 잔차 = 0 → 포즈와 3D 점이 정확\n" << std::endl;

    std::cout << "    만약 포즈가 틀리면:" << std::endl;
    std::cout << "    → 투영 위치가 달라짐 → 잔차 ≠ 0" << std::endl;
    std::cout << "    → 최적화가 포즈 또는 3D 점을 수정" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 마하라노비스 거리 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Eigen::Vector2d r_a(0.1, 0.1);
    Eigen::Matrix2d S_a = Eigen::Matrix2d::Identity() * 0.01;
    double d_a = r_a.transpose() * S_a.inverse() * r_a;

    std::cout << "  Factor A:" << std::endl;
    std::cout << "    r = [0.1, 0.1], Σ = 0.01·I" << std::endl;
    std::cout << "    Σ^{-1} = 100·I" << std::endl;
    std::cout << "    d = r^T·Σ^{-1}·r = (0.01+0.01)×100 = " << d_a << "\n" << std::endl;

    Eigen::Vector2d r_b(1.0, 1.0);
    Eigen::Matrix2d S_b = Eigen::Matrix2d::Identity() * 4.0;
    double d_b = r_b.transpose() * S_b.inverse() * r_b;

    std::cout << "  Factor B:" << std::endl;
    std::cout << "    r = [1.0, 1.0], Σ = 4.0·I" << std::endl;
    std::cout << "    Σ^{-1} = 0.25·I" << std::endl;
    std::cout << "    d = r^T·Σ^{-1}·r = (1+1)×0.25 = " << d_b << "\n" << std::endl;

    std::cout << "  비교: Factor A (" << d_a << ") vs Factor B (" << d_b << ")" << std::endl;
    std::cout << "  → Factor A의 마하라노비스 거리가 더 큼!\n" << std::endl;

    std::cout << "  직관적 이해:" << std::endl;
    std::cout << "    Factor A: σ=0.1인 센서에서 0.14(=√(0.1²+0.1²)) 오차" << std::endl;
    std::cout << "    → 기대 오차의 1.4배 → '꽤 큰' 오차" << std::endl;
    std::cout << "    Factor B: σ=2.0인 센서에서 1.41(=√(1²+1²)) 오차" << std::endl;
    std::cout << "    → 기대 오차의 0.7배 → '작은' 오차\n" << std::endl;

    std::cout << "  최적화에서의 의미:" << std::endl;
    std::cout << "    Factor A의 비용(2.0)이 Factor B(0.5)보다 큼" << std::endl;
    std::cout << "    → 최적화는 Factor A의 잔차를 더 줄이려 함" << std::endl;
    std::cout << "    → 정확한 센서의 측정을 더 잘 만족시킴!" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 8 Quiz Medium - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
