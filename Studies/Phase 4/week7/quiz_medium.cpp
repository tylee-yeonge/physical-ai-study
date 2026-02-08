/**
 * Phase 4 Week 7 - Pre-integration 수식 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

// SO(3) 유틸리티
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
         + std::sin(angle) * K
         + (1.0 - std::cos(angle)) * K * K;
}

Eigen::Vector3d logSO3(const Eigen::Matrix3d& R) {
    double cos_angle = (R.trace() - 1.0) / 2.0;
    cos_angle = std::max(-1.0, std::min(1.0, cos_angle));
    double angle = std::acos(cos_angle);
    if (angle < 1e-10) return Eigen::Vector3d::Zero();
    Eigen::Matrix3d log_R = angle / (2.0 * std::sin(angle)) * (R - R.transpose());
    return Eigen::Vector3d(log_R(2,1), log_R(0,2), log_R(1,0));
}

void problem1_single_step_preintegration() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 단일 스텝 Pre-integration" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "IMU 데이터 1개:" << std::endl;
    std::cout << "  a_m = [0, 0, 9.81] m/s^2 (정지, z축 중력)" << std::endl;
    std::cout << "  ω_m = [0, 0, 0.1] rad/s (z축 회전)" << std::endl;
    std::cout << "  b_a = [0, 0, 0], b_g = [0, 0, 0]" << std::endl;
    std::cout << "  dt = 0.005s (200Hz)\n" << std::endl;
    std::cout << "ΔR, Δv, Δp를 한 스텝 업데이트하시오.\n" << std::endl;

    Eigen::Vector3d a_m(0, 0, 9.81);
    Eigen::Vector3d w_m(0, 0, 0.1);
    Eigen::Vector3d b_a = Eigen::Vector3d::Zero();
    Eigen::Vector3d b_g = Eigen::Vector3d::Zero();
    double dt = 0.005;

    Eigen::Vector3d acc = a_m - b_a;
    Eigen::Vector3d gyro = w_m - b_g;

    // 초기값
    Eigen::Matrix3d dR = Eigen::Matrix3d::Identity();
    Eigen::Vector3d dv = Eigen::Vector3d::Zero();
    Eigen::Vector3d dp = Eigen::Vector3d::Zero();

    // 업데이트 (p → v → R 순서)
    dp += dv * dt + 0.5 * dR * acc * dt * dt;
    dv += dR * acc * dt;
    dR = dR * expSO3(gyro * dt);

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  acc = a_m - b_a = " << acc.transpose() << std::endl;
    std::cout << "  gyro = ω_m - b_g = " << gyro.transpose() << "\n" << std::endl;

    std::cout << "  Δp += Δv·dt + 0.5·ΔR·acc·dt²" << std::endl;
    std::cout << "     = [0,0,0]·0.005 + 0.5·I·[0,0,9.81]·0.005²" << std::endl;
    std::cout << "     = " << dp.transpose() << "\n" << std::endl;

    std::cout << "  Δv += ΔR·acc·dt" << std::endl;
    std::cout << "     = I·[0,0,9.81]·0.005" << std::endl;
    std::cout << "     = " << dv.transpose() << "\n" << std::endl;

    std::cout << "  ΔR *= Exp([0,0,0.1]·0.005)" << std::endl;
    std::cout << "  ΔR = Exp([0,0,0.0005])" << std::endl;
    Eigen::Vector3d dR_log = logSO3(dR);
    std::cout << "  logSO3(ΔR) = " << dR_log.transpose() << std::endl;
    std::cout << "\n  핵심: z축 0.0005 rad 회전, 가속도 적분으로 Δv, Δp 축적" << std::endl;
}

void problem2_multi_step_integration() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 다중 스텝 Pre-integration" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "등속 직선 운동 (x방향 1m/s, 0.5초)" << std::endl;
    std::cout << "중력 보상된 IMU: a_body = [0,0,9.81], ω = [0,0,0]" << std::endl;
    std::cout << "Pre-integration 후 Δp를 구하시오.\n" << std::endl;

    // 정지 상태에서의 등속 운동 → 가속도 = 0
    // 가속도계: R^T(0 - g) = [0,0,9.81] (중력의 반대)
    // 바이어스 제거 후: acc = [0,0,9.81]
    // Δv에 중력이 포함됨 → 복원 시 g·Δt로 상쇄

    // 하지만! Pre-integration에서 '실제 가속도'는 0
    // Δp는 '가속도에 의한 상대 변위'만 측정
    // v_i에 의한 이동은 복원 공식의 v_i·Δt 항이 담당

    double dt = 0.005;
    int steps = 100;  // 0.5초

    Eigen::Vector3d a_meas(0, 0, 9.81);  // 정지 상태 가속도계 출력
    Eigen::Vector3d w_meas(0, 0, 0);
    Eigen::Vector3d b_a = Eigen::Vector3d::Zero();
    Eigen::Vector3d b_g = Eigen::Vector3d::Zero();

    Eigen::Matrix3d dR = Eigen::Matrix3d::Identity();
    Eigen::Vector3d dv = Eigen::Vector3d::Zero();
    Eigen::Vector3d dp = Eigen::Vector3d::Zero();

    for (int i = 0; i < steps; i++) {
        Eigen::Vector3d acc = a_meas - b_a;
        Eigen::Vector3d gyro = w_meas - b_g;
        dp += dv * dt + 0.5 * dR * acc * dt * dt;
        dv += dR * acc * dt;
        dR = dR * expSO3(gyro * dt);
    }

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  100스텝 후:" << std::endl;
    std::cout << "  Δp = " << dp.transpose() << std::endl;
    std::cout << "  Δv = " << dv.transpose() << "\n" << std::endl;

    // 상태 복원
    Eigen::Vector3d p_i(0, 0, 0);
    Eigen::Vector3d v_i(1, 0, 0);
    Eigen::Matrix3d R_i = Eigen::Matrix3d::Identity();
    Eigen::Vector3d g(0, 0, -9.81);
    double total_t = steps * dt;

    Eigen::Vector3d p_j = p_i + v_i * total_t + 0.5 * g * total_t * total_t + R_i * dp;

    std::cout << "  상태 복원:" << std::endl;
    std::cout << "  p_j = p_i + v_i·Δt + 0.5·g·Δt² + R_i·Δp" << std::endl;
    std::cout << "       = [0,0,0] + [1,0,0]·0.5 + 0.5·[0,0,-9.81]·0.25 + " << dp.transpose() << std::endl;
    std::cout << "       = " << p_j.transpose() << std::endl;
    std::cout << "  실제: [0.5, 0, 0] (등속 0.5초)\n" << std::endl;

    std::cout << "  핵심: Δp의 z성분과 0.5·g·Δt²이 상쇄!" << std::endl;
    std::cout << "  → 중력 효과는 Pre-integration + 복원 공식에서 정확히 처리" << std::endl;
}

void problem3_bias_correction() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 바이어스 보정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Pre-integration이 b_a=[0,0,0]으로 계산되었는데," << std::endl;
    std::cout << "최적화 후 b_a=[0.1, 0, 0]으로 바뀌었습니다.\n" << std::endl;
    std::cout << "δb_a = [0.1, 0, 0]\n" << std::endl;
    std::cout << "J_v_ba (100스텝 후)를 단순화하여 계산하고," << std::endl;
    std::cout << "보정된 Δv'을 구하시오.\n" << std::endl;

    // J_v_ba 누적: J_v_ba -= ΔR · dt 매 스텝
    // ΔR ≈ I (작은 회전) → J_v_ba ≈ -I · N · dt = -I · 0.5
    double dt = 0.005;
    int N = 100;

    Eigen::Matrix3d J_v_ba = Eigen::Matrix3d::Zero();
    Eigen::Matrix3d dR = Eigen::Matrix3d::Identity();

    for (int i = 0; i < N; i++) {
        J_v_ba -= dR * dt;
        // dR stays Identity (no rotation)
    }

    Eigen::Vector3d db_a(0.1, 0, 0);
    Eigen::Vector3d dv_correction = J_v_ba * db_a;

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "  J_v_ba = Σ(-ΔR·dt) = -I·100·0.005 = -0.5·I" << std::endl;
    std::cout << "  J_v_ba =\n" << J_v_ba << "\n" << std::endl;

    std::cout << "  보정량: J_v_ba · δb_a = " << dv_correction.transpose() << std::endl;
    std::cout << "\n  의미: 바이어스가 +0.1이면 측정 가속도가 0.1만큼 높았던 것" << std::endl;
    std::cout << "  → 적분된 속도를 -0.05 보정 (0.5초 × 0.1 m/s²)" << std::endl;
    std::cout << "  → 재적분 없이 행렬 곱 한 번으로 보정 완료!" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7 Quiz - Medium (Pre-integration 계산)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_single_step_preintegration();
    problem2_multi_step_integration();
    problem3_bias_correction();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
