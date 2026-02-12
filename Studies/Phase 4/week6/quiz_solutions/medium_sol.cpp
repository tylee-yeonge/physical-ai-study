/**
 * Phase 4 Week 6 - Pre-integration 필요성 중급 퀴즈 정답
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 6 Medium Quiz - 정답 해설" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "문제 1: 단순 적분 구현" << std::endl;
    std::cout << "  핵심: p = v*dt + 0.5*a*dt^2 를 반복 적용" << std::endl;
    std::cout << "  이론: p = 0.5 * 1.0 * 4.0 = 2.0 m" << std::endl;
    std::cout << "  이론: v = 1.0 * 2.0 = 2.0 m/s" << std::endl;
    std::cout << "  적분 결과는 이론값과 거의 일치해야 합니다." << std::endl;
    std::cout << "  dt가 작을수록(주파수 높을수록) 더 정확합니다.\n" << std::endl;

    std::cout << "문제 2: 회전 적분 (쿼터니언)" << std::endl;
    std::cout << "  핵심: dq = AngleAxis(omega*dt, axis)로 미소 회전 생성" << std::endl;
    std::cout << "  이론: theta = 0.5 * 2.0 = 1.0 rad = 57.3 deg" << std::endl;
    std::cout << "  쿼터니언 적분: q = q * dq를 반복하면" << std::endl;
    std::cout << "  총 회전 각도가 이론값과 일치해야 합니다." << std::endl;
    std::cout << "  핵심: 쿼터니언은 반드시 normalize 필요!\n" << std::endl;

    std::cout << "문제 3: 절대 좌표 vs 상대 좌표" << std::endl;
    std::cout << "  핵심:" << std::endl;
    std::cout << "  절대 좌표: 출발점이 다르면 결과도 다름" << std::endl;
    std::cout << "  상대 좌표: 출발점이 달라도 상대 이동은 동일" << std::endl;
    std::cout << std::endl;
    std::cout << "  상대 좌표 계산 공식:" << std::endl;
    std::cout << "  Δp = R_i^T * (p_j - p_i - v_i*dt - 0.5*g*dt^2)" << std::endl;
    std::cout << std::endl;
    std::cout << "  이것이 Pre-integration의 핵심입니다!" << std::endl;
    std::cout << "  같은 IMU 데이터에서 나온 Δp는" << std::endl;
    std::cout << "  출발 포즈(p_i, v_i, R_i)가 무엇이든 동일합니다." << std::endl;

    // 검증
    std::cout << "\n  [수치 검증]" << std::endl;
    Eigen::Vector3d gravity(0, 0, -9.81);
    double dt = 0.01;
    int steps = 100;
    double T = steps * dt;
    Eigen::Vector3d acc_body(1.0, 0.0, 9.81);
    Eigen::Matrix3d R = Eigen::Matrix3d::Identity();

    // 포즈 1
    Eigen::Vector3d p1(0, 0, 0), v1(0, 0, 0);
    Eigen::Vector3d pos1 = p1, vel1 = v1;
    for (int i = 0; i < steps; ++i)
    {
        Eigen::Vector3d a = R * (acc_body - Eigen::Vector3d(0, 0, 9.81)) + gravity;
        pos1 += vel1 * dt + 0.5 * a * dt * dt;
        vel1 += a * dt;
    }

    // 포즈 2
    Eigen::Vector3d p2(10, 5, 2), v2(1, 0.5, 0);
    Eigen::Vector3d pos2 = p2, vel2 = v2;
    for (int i = 0; i < steps; ++i)
    {
        Eigen::Vector3d a = R * (acc_body - Eigen::Vector3d(0, 0, 9.81)) + gravity;
        pos2 += vel2 * dt + 0.5 * a * dt * dt;
        vel2 += a * dt;
    }

    Eigen::Vector3d dp1 = R.transpose() * (pos1 - p1 - v1 * T - 0.5 * gravity * T * T);
    Eigen::Vector3d dp2 = R.transpose() * (pos2 - p2 - v2 * T - 0.5 * gravity * T * T);

    std::cout << "  Δp (포즈1): " << dp1.transpose() << std::endl;
    std::cout << "  Δp (포즈2): " << dp2.transpose() << std::endl;
    std::cout << "  차이: " << (dp1 - dp2).norm() << " (거의 0)" << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "핵심 정리:" << std::endl;
    std::cout << "  1. 단순 적분은 이론값과 잘 맞지만 포즈 의존적" << std::endl;
    std::cout << "  2. 쿼터니언 적분 시 normalize 필수" << std::endl;
    std::cout << "  3. 상대 좌표 Δp는 출발 포즈에 무관 → Pre-integration!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
