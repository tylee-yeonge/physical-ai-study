/**
 * Phase 4 Week 6 - Pre-integration 필요성 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>
#include <vector>
#include <cmath>
#include <chrono>

void problem1_naive_integration() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 단순 적분 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "문제: 간단한 1D IMU 적분을 구현하세요." << std::endl;
    std::cout << "가속도 a = 1.0 m/s^2가 일정할 때, 2초 후 위치와 속도는?\n" << std::endl;

    // 시뮬레이션 파라미터
    double dt = 0.005;      // 200Hz
    double duration = 2.0;  // 2초
    double accel = 1.0;     // 일정한 가속도

    double position = 0.0;
    double velocity = 0.0;
    int steps = static_cast<int>(duration / dt);

    for (int i = 0; i < steps; ++i) {
        position += velocity * dt + 0.5 * accel * dt * dt;
        velocity += accel * dt;
    }

    std::cout << "💡 답:" << std::endl;
    std::cout << "   이론값: p = 0.5 * a * t^2 = " << 0.5 * accel * duration * duration << " m" << std::endl;
    std::cout << "   적분값: p = " << position << " m" << std::endl;
    std::cout << "   이론값: v = a * t = " << accel * duration << " m/s" << std::endl;
    std::cout << "   적분값: v = " << velocity << " m/s" << std::endl;
    std::cout << "   오차: " << std::abs(position - 0.5 * accel * duration * duration) << " m" << std::endl;
}

void problem2_rotation_integration() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 회전 적분 (쿼터니언)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "문제: z축 주위로 omega = 0.5 rad/s 회전." << std::endl;
    std::cout << "2초 후 총 회전 각도는?\n" << std::endl;

    double dt = 0.005;      // 200Hz
    double duration = 2.0;
    Eigen::Vector3d omega(0.0, 0.0, 0.5);  // z축 회전

    Eigen::Quaterniond q = Eigen::Quaterniond::Identity();
    int steps = static_cast<int>(duration / dt);

    for (int i = 0; i < steps; ++i) {
        double angle = omega.norm() * dt;
        Eigen::Vector3d axis = omega.normalized();
        Eigen::Quaterniond dq(Eigen::AngleAxisd(angle, axis));
        q = (q * dq).normalized();
    }

    // 회전 각도 추출
    Eigen::AngleAxisd aa(q);
    double total_angle_deg = aa.angle() * 180.0 / M_PI;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   이론: omega * t = " << omega.z() * duration << " rad = "
              << omega.z() * duration * 180.0 / M_PI << " deg" << std::endl;
    std::cout << "   적분: " << aa.angle() << " rad = "
              << total_angle_deg << " deg" << std::endl;
    std::cout << "   회전축: " << aa.axis().transpose() << std::endl;
    std::cout << "   쿼터니언: [" << q.w() << ", " << q.x() << ", "
              << q.y() << ", " << q.z() << "]" << std::endl;
}

void problem3_absolute_vs_relative() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 절대 좌표 vs 상대 좌표" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "문제: 같은 IMU 데이터를 다른 출발 포즈에서 적분했을 때" << std::endl;
    std::cout << "절대 좌표와 상대 좌표의 차이를 확인하세요.\n" << std::endl;

    // 간단한 IMU: 일정 가속도 (x방향 1 m/s^2)
    Eigen::Vector3d acc_body(1.0, 0.0, 9.81);  // 중력 포함
    Eigen::Vector3d gravity(0.0, 0.0, -9.81);
    double dt = 0.01;
    int steps = 100;  // 1초
    double total_time = steps * dt;

    // 출발 포즈 1
    Eigen::Vector3d p1(0.0, 0.0, 0.0);
    Eigen::Vector3d v1(0.0, 0.0, 0.0);
    Eigen::Matrix3d R1 = Eigen::Matrix3d::Identity();

    // 출발 포즈 2 (다른 위치)
    Eigen::Vector3d p2(10.0, 5.0, 2.0);
    Eigen::Vector3d v2(1.0, 0.5, 0.0);
    Eigen::Matrix3d R2 = Eigen::Matrix3d::Identity();

    // 적분
    Eigen::Vector3d pos1 = p1, vel1 = v1;
    Eigen::Vector3d pos2 = p2, vel2 = v2;

    for (int i = 0; i < steps; ++i) {
        Eigen::Vector3d a1 = R1 * (acc_body - Eigen::Vector3d(0,0,9.81)) + gravity;
        pos1 += vel1 * dt + 0.5 * a1 * dt * dt;
        vel1 += a1 * dt;

        Eigen::Vector3d a2 = R2 * (acc_body - Eigen::Vector3d(0,0,9.81)) + gravity;
        pos2 += vel2 * dt + 0.5 * a2 * dt * dt;
        vel2 += a2 * dt;
    }

    // 절대 좌표: 다름
    std::cout << "  [절대 좌표]" << std::endl;
    std::cout << "  포즈1 결과: " << pos1.transpose() << std::endl;
    std::cout << "  포즈2 결과: " << pos2.transpose() << std::endl;
    std::cout << "  차이: " << (pos1 - pos2).norm() << " m\n" << std::endl;

    // 상대 좌표 (Pre-integration 스타일)
    Eigen::Vector3d dp1 = R1.transpose() * (pos1 - p1 - v1 * total_time
                          - 0.5 * gravity * total_time * total_time);
    Eigen::Vector3d dp2 = R2.transpose() * (pos2 - p2 - v2 * total_time
                          - 0.5 * gravity * total_time * total_time);

    std::cout << "  [상대 좌표 (Pre-integration)]" << std::endl;
    std::cout << "  포즈1 상대: " << dp1.transpose() << std::endl;
    std::cout << "  포즈2 상대: " << dp2.transpose() << std::endl;
    std::cout << "  차이: " << (dp1 - dp2).norm() << " m" << std::endl;

    std::cout << "\n💡 핵심: 상대 좌표(Pre-integration)는 출발 포즈에 무관!" << std::endl;
    std::cout << "   → 최적화로 p_i가 바뀌어도 Δp 재계산 불필요" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 6 Quiz - Medium" << std::endl;
    std::cout << "Pre-integration 필요성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_naive_integration();
    problem2_rotation_integration();
    problem3_absolute_vs_relative();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
