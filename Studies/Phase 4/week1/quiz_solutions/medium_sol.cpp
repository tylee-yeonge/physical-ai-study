/**
 * Phase 4 Week 1 - IMU 센서 이해 중급 퀴즈 정답
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 4 Week 1 Quiz Solutions (Medium)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 문제 1 풀이
    std::cout << "문제 1: Body Frame에서의 중력" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double angle = M_PI / 4.0;  // 45도
    Eigen::Matrix3d R_wb;
    R_wb << 1, 0, 0,
            0, cos(angle), -sin(angle),
            0, sin(angle), cos(angle);

    Eigen::Vector3d g_world(0, 0, -9.81);
    Eigen::Vector3d g_body = R_wb.transpose() * g_world;

    std::cout << "풀이:" << std::endl;
    std::cout << "  R_wb (x축 45도):" << std::endl;
    std::cout << R_wb << std::endl;
    std::cout << "\n  g_body = R_wb^T * g_world" << std::endl;
    std::cout << "        = R_wb^T * [0, 0, -9.81]^T" << std::endl;
    std::cout << "        = " << g_body.transpose() << std::endl;
    std::cout << "\n  해석:" << std::endl;
    std::cout << "  - y축: " << g_body.y() << " m/s² (45도 기울어서 중력 y성분 생김)" << std::endl;
    std::cout << "  - z축: " << g_body.z() << " m/s² (중력 z성분 감소)" << std::endl;
    std::cout << "  - |g_body| = " << g_body.norm() << " m/s² (크기는 보존됨!)\n" << std::endl;

    // 문제 2 풀이
    std::cout << "문제 2: 가속도계 측정값 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
    Eigen::Vector3d a_true(1, 0, 0);
    Eigen::Vector3d gravity(0, 0, -9.81);
    Eigen::Vector3d b_a(0.02, 0.01, 0.03);

    Eigen::Vector3d a_meas = R.transpose() * (a_true - gravity) + b_a;

    std::cout << "풀이:" << std::endl;
    std::cout << "  a_meas = R^T * (a_true - g) + b_a" << std::endl;
    std::cout << "        = I * ([1,0,0] - [0,0,-9.81]) + [0.02, 0.01, 0.03]" << std::endl;
    std::cout << "        = [1, 0, 9.81] + [0.02, 0.01, 0.03]" << std::endl;
    std::cout << "        = " << a_meas.transpose() << std::endl;
    std::cout << "\n  해석:" << std::endl;
    std::cout << "  - x축: 1.02 (실제 가속도 1 + 바이어스 0.02)" << std::endl;
    std::cout << "  - y축: 0.01 (바이어스만)" << std::endl;
    std::cout << "  - z축: 9.84 (중력 9.81 + 바이어스 0.03)" << std::endl;
    std::cout << "  → 정지 시에도 z축에 ~9.81이 나오는 이유!\n" << std::endl;

    // 문제 3 풀이
    std::cout << "문제 3: 바이어스에 의한 위치 드리프트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double bias = 0.05;   // m/s²
    double t = 10.0;      // 초
    double drift = 0.5 * bias * t * t;

    std::cout << "풀이:" << std::endl;
    std::cout << "  등가속도 운동 공식: s = 1/2 * a * t²" << std::endl;
    std::cout << "  바이어스를 일정한 가속도로 간주하면:" << std::endl;
    std::cout << "  drift = 0.5 * " << bias << " * " << t << "² = " << drift << " m" << std::endl;
    std::cout << "\n  해석:" << std::endl;
    std::cout << "  - 0.05 m/s²는 매우 작은 바이어스지만" << std::endl;
    std::cout << "  - 10초 후 2.5m의 위치 오차 발생!" << std::endl;
    std::cout << "  - 이중 적분(가속도→속도→위치)이기 때문에 t²에 비례" << std::endl;
    std::cout << "  - 60초 후: 0.5 * 0.05 * 3600 = 90m 오차!" << std::endl;
    std::cout << "  → 바이어스 추정 없이는 VIO 불가능\n" << std::endl;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
