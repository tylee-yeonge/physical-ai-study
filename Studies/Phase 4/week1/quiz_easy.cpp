/**
 * Phase 4 Week 1 - IMU 센서 이해 기초 퀴즈
 */

#include <iostream>

void problem1_accel_stationary()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 정지 상태의 가속도계" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 가속도계가 테이블 위에 정지해 있을 때," << std::endl;
    std::cout << "      z축(위 방향) 출력값은 대략 얼마인가?\n" << std::endl;
    std::cout << "  a) 0 m/s²" << std::endl;
    std::cout << "  b) ~9.81 m/s²" << std::endl;
    std::cout << "  c) ~-9.81 m/s²" << std::endl;
    std::cout << "  d) 알 수 없음" << std::endl;
    std::cout << "  답: _____\n" << std::endl;
}

void problem2_gyro_bias()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 자이로 바이어스 영향" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 자이로 바이어스가 0.01 rad/s일 때," << std::endl;
    std::cout << "      10초 후 누적 각도 오차는?\n" << std::endl;
    std::cout << "  a) 0.001 rad (0.057°)" << std::endl;
    std::cout << "  b) 0.01 rad (0.57°)" << std::endl;
    std::cout << "  c) 0.1 rad (5.7°)" << std::endl;
    std::cout << "  d) 1.0 rad (57.3°)" << std::endl;
    std::cout << "  답: _____\n" << std::endl;
}

void problem3_imu_frequency()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: IMU vs Vision 주파수" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 일반적인 IMU와 카메라의 데이터 주파수 비교로" << std::endl;
    std::cout << "      올바른 것은?\n" << std::endl;
    std::cout << "  a) IMU 30Hz, Camera 200Hz" << std::endl;
    std::cout << "  b) IMU 200Hz, Camera 200Hz" << std::endl;
    std::cout << "  c) IMU 200Hz, Camera 30Hz" << std::endl;
    std::cout << "  d) IMU 30Hz, Camera 30Hz" << std::endl;
    std::cout << "  답: _____\n" << std::endl;
}

void problem4_scale_recovery()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 스케일 복구" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 모노 카메라 SLAM에서 실제 스케일(미터 단위)을" << std::endl;
    std::cout << "      복구하기 위해 IMU가 제공하는 정보는?\n" << std::endl;
    std::cout << "  a) 픽셀 좌표" << std::endl;
    std::cout << "  b) 절대 가속도 (실제 m/s² 단위)" << std::endl;
    std::cout << "  c) 색상 정보" << std::endl;
    std::cout << "  d) 깊이 정보" << std::endl;
    std::cout << "  답: _____\n" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 4 Week 1 Quiz - Easy" << std::endl;
    std::cout << "IMU 센서 이해" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_accel_stationary();
    problem2_gyro_bias();
    problem3_imu_frequency();
    problem4_scale_recovery();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
