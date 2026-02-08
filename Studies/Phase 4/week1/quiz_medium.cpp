/**
 * Phase 4 Week 1 - IMU 센서 이해 중급 퀴즈
 *
 * Eigen을 사용한 IMU 측정 모델 계산 문제
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

void problem1_gravity_in_body() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Body Frame에서의 중력" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "로봇이 x축 기준 45도 기울어져 있을 때," << std::endl;
    std::cout << "Body frame에서 느끼는 중력 벡터를 구하세요.\n" << std::endl;
    std::cout << "조건:" << std::endl;
    std::cout << "  - World 중력: g = [0, 0, -9.81]^T" << std::endl;
    std::cout << "  - 회전: x축 기준 45도 기울임\n" << std::endl;

    // TODO: 학생이 직접 계산
    // 힌트: g_body = R_wb^T * g_world
    // R_wb = Rx(45°) 회전 행렬

    double angle = M_PI / 4.0;  // 45도
    Eigen::Matrix3d R_wb;
    R_wb << 1, 0, 0,
            0, cos(angle), -sin(angle),
            0, sin(angle), cos(angle);

    Eigen::Vector3d g_world(0, 0, -9.81);

    std::cout << "💡 R_wb (x축 45도 회전):" << std::endl;
    std::cout << R_wb << std::endl;
    std::cout << "\n질문: g_body = R_wb^T * g_world = ?\n" << std::endl;
    std::cout << "답: _____\n" << std::endl;
}

void problem2_accel_measurement() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 가속도계 측정값 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "조건:" << std::endl;
    std::cout << "  - 실제 가속도 (world): a_true = [1, 0, 0]^T m/s²" << std::endl;
    std::cout << "  - 로봇 자세: 단위 행렬 (수평)" << std::endl;
    std::cout << "  - 중력: g = [0, 0, -9.81]^T" << std::endl;
    std::cout << "  - 바이어스: b_a = [0.02, 0.01, 0.03]^T" << std::endl;
    std::cout << "  - 노이즈: 무시 (= 0)\n" << std::endl;

    std::cout << "질문: 가속도계 출력 a_meas = R^T*(a_true - g) + b_a = ?\n" << std::endl;

    // 학생이 손으로 계산할 것
    // R = I (수평)
    // a_meas = I^T * ([1,0,0] - [0,0,-9.81]) + [0.02, 0.01, 0.03]
    //        = [1, 0, 9.81] + [0.02, 0.01, 0.03]
    //        = [1.02, 0.01, 9.84]

    std::cout << "답: _____\n" << std::endl;
}

void problem3_drift_estimation() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 바이어스에 의한 위치 드리프트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "조건:" << std::endl;
    std::cout << "  - 가속도계 바이어스: b_a = 0.05 m/s² (한 축)" << std::endl;
    std::cout << "  - 노이즈 무시, 정지 상태" << std::endl;
    std::cout << "  - 시간: 10초\n" << std::endl;

    std::cout << "질문: 바이어스만으로 인한 위치 드리프트 = ?" << std::endl;
    std::cout << "  힌트: 위치 = 1/2 * a * t²\n" << std::endl;

    // 답: 0.5 * 0.05 * 10^2 = 2.5 m

    std::cout << "답: _____\n" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 4 Week 1 Quiz - Medium" << std::endl;
    std::cout << "IMU 센서 이해 (Eigen 계산)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_gravity_in_body();
    problem2_accel_measurement();
    problem3_drift_estimation();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
