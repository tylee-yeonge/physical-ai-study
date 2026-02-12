/**
 * Phase 4 Week 7 - Pre-integration 수식 중급 퀴즈 풀이
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
    double cos_angle = (R.trace() - 1.0) / 2.0;
    cos_angle = std::max(-1.0, std::min(1.0, cos_angle));
    double angle = std::acos(cos_angle);
    if (angle < 1e-10)
        return Eigen::Vector3d::Zero();
    Eigen::Matrix3d log_R = angle / (2.0 * std::sin(angle)) * (R - R.transpose());
    return Eigen::Vector3d(log_R(2, 1), log_R(0, 2), log_R(1, 0));
}

void problem1_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: 단일 스텝 Pre-integration" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Eigen::Vector3d a_m(0, 0, 9.81);
    Eigen::Vector3d w_m(0, 0, 0.1);
    double dt = 0.005;

    Eigen::Vector3d acc = a_m;   // b_a = 0
    Eigen::Vector3d gyro = w_m;  // b_g = 0

    Eigen::Matrix3d dR = Eigen::Matrix3d::Identity();
    Eigen::Vector3d dv = Eigen::Vector3d::Zero();
    Eigen::Vector3d dp = Eigen::Vector3d::Zero();

    std::cout << "  초기: ΔR=I, Δv=[0,0,0], Δp=[0,0,0]\n" << std::endl;

    // Step 1: Δp 업데이트
    dp += dv * dt + 0.5 * dR * acc * dt * dt;
    std::cout << "  Step 1: Δp 업데이트" << std::endl;
    std::cout << "    Δp += [0,0,0]·0.005 + 0.5·I·[0,0,9.81]·0.000025" << std::endl;
    std::cout << "    Δp = " << dp.transpose() << std::endl;
    std::cout << "    (z 방향: 0.5 × 9.81 × 0.000025 = " << 0.5 * 9.81 * 0.000025 << ")\n"
              << std::endl;

    // Step 2: Δv 업데이트
    dv += dR * acc * dt;
    std::cout << "  Step 2: Δv 업데이트" << std::endl;
    std::cout << "    Δv += I·[0,0,9.81]·0.005" << std::endl;
    std::cout << "    Δv = " << dv.transpose() << std::endl;
    std::cout << "    (z 방향: 9.81 × 0.005 = " << 9.81 * 0.005 << ")\n" << std::endl;

    // Step 3: ΔR 업데이트
    dR = dR * expSO3(gyro * dt);
    std::cout << "  Step 3: ΔR 업데이트" << std::endl;
    std::cout << "    ΔR *= Exp([0,0,0.1]·0.005) = Exp([0,0,0.0005])" << std::endl;
    std::cout << "    logSO3(ΔR) = " << logSO3(dR).transpose() << std::endl;
    std::cout << "    (z축 0.0005 rad ≈ 0.029도 회전)\n" << std::endl;

    std::cout << "  요약:" << std::endl;
    std::cout << "    Δp = " << dp.transpose() << " (매우 작은 z 변위)" << std::endl;
    std::cout << "    Δv = " << dv.transpose() << " (z 속도 축적)" << std::endl;
    std::cout << "    ΔR: z축 0.0005 rad 회전" << std::endl;
}

void problem2_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 다중 스텝 Pre-integration" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double dt = 0.005;
    int steps = 100;
    Eigen::Vector3d a_meas(0, 0, 9.81);

    Eigen::Matrix3d dR = Eigen::Matrix3d::Identity();
    Eigen::Vector3d dv = Eigen::Vector3d::Zero();
    Eigen::Vector3d dp = Eigen::Vector3d::Zero();

    for (int i = 0; i < steps; i++)
    {
        dp += dv * dt + 0.5 * dR * a_meas * dt * dt;
        dv += dR * a_meas * dt;
    }

    std::cout << "  100스텝 적분 결과:" << std::endl;
    std::cout << "  Δp = " << dp.transpose() << std::endl;
    std::cout << "  Δv = " << dv.transpose() << "\n" << std::endl;

    // 상태 복원
    Eigen::Vector3d g(0, 0, -9.81);
    double total_t = steps * dt;  // 0.5s
    Eigen::Vector3d v_i(1, 0, 0);
    Eigen::Vector3d p_i(0, 0, 0);

    Eigen::Vector3d p_j = p_i + v_i * total_t + 0.5 * g * total_t * total_t + dp;

    std::cout << "  상태 복원:" << std::endl;
    std::cout << "    v_i·Δt = [1,0,0]·0.5 = [0.5, 0, 0]" << std::endl;
    std::cout << "    0.5·g·Δt² = 0.5·[0,0,-9.81]·0.25 = [0, 0, " << 0.5 * (-9.81) * 0.25 << "]"
              << std::endl;
    std::cout << "    R_i·Δp = " << dp.transpose() << "\n" << std::endl;

    std::cout << "    p_j = " << p_j.transpose() << "\n" << std::endl;

    std::cout << "  핵심 관찰:" << std::endl;
    std::cout << "    Δp의 z성분 = " << dp(2) << std::endl;
    std::cout << "    0.5·g·Δt²의 z성분 = " << 0.5 * (-9.81) * total_t * total_t << std::endl;
    std::cout << "    합 = " << dp(2) + 0.5 * (-9.81) * total_t * total_t << " ≈ 0!" << std::endl;
    std::cout << "    → 중력이 Pre-integration과 복원 공식에서 정확히 상쇄!" << std::endl;
    std::cout << "    → x 방향: v_i·Δt = 0.5m (등속 운동 정확 반영)" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 바이어스 보정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double dt = 0.005;
    int N = 100;

    Eigen::Matrix3d J_v_ba = Eigen::Matrix3d::Zero();
    for (int i = 0; i < N; i++)
    {
        J_v_ba -= Eigen::Matrix3d::Identity() * dt;
    }

    std::cout << "  J_v_ba 계산:" << std::endl;
    std::cout << "    매 스텝: J_v_ba -= ΔR·dt ≈ -I·dt" << std::endl;
    std::cout << "    100스텝: J_v_ba = -I·100·0.005 = -0.5·I" << std::endl;
    std::cout << "    J_v_ba =\n" << J_v_ba << "\n" << std::endl;

    Eigen::Vector3d db_a(0.1, 0, 0);
    Eigen::Vector3d correction = J_v_ba * db_a;

    std::cout << "  바이어스 보정:" << std::endl;
    std::cout << "    δb_a = [0.1, 0, 0]" << std::endl;
    std::cout << "    Δv' = Δv + J_v_ba · δb_a" << std::endl;
    std::cout << "    보정량 = " << correction.transpose() << "\n" << std::endl;

    std::cout << "  물리적 의미:" << std::endl;
    std::cout << "    바이어스 0.1 m/s^2 → 측정 가속도가 0.1 높았음" << std::endl;
    std::cout << "    0.5초 적분 × 0.1 m/s^2 = 0.05 m/s 과대 추정" << std::endl;
    std::cout << "    → Δv_x를 -0.05 보정\n" << std::endl;

    std::cout << "  장점:" << std::endl;
    std::cout << "    재적분: 100번의 행렬 연산 필요" << std::endl;
    std::cout << "    자코비안 보정: 행렬·벡터 곱 1번!" << std::endl;
    std::cout << "    → 최적화에서 바이어스가 바뀔 때마다 즉시 보정 가능" << std::endl;
    std::cout << "    → 단, 바이어스 변화가 크면 재적분 필요 (1차 근사 한계)" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7 Quiz Medium - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
