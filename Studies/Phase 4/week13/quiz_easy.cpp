/**
 * Phase 4 Week 13 - Camera-IMU 외부 캘리브레이션 기초 퀴즈
 */

#include <iostream>
#include <cmath>

void problem1_extrinsic_definition()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Extrinsic의 정의" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Camera-IMU Extrinsic Calibration에서 추정하는 것은?\n" << std::endl;
    std::cout << "  (A) 카메라의 초점거리와 왜곡 계수" << std::endl;
    std::cout << "  (B) IMU의 바이어스와 노이즈 파라미터" << std::endl;
    std::cout << "  (C) 카메라와 IMU 사이의 상대 회전과 병진" << std::endl;
    std::cout << "  (D) 월드 프레임에서의 카메라 절대 위치\n" << std::endl;

    std::cout << "  힌트: Extrinsic = 두 센서 사이의 공간적 관계" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem2_rotation_vs_translation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 회전 vs 병진 오차" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Extrinsic에서 회전 오차가 병진 오차보다 더 치명적인 이유는?\n" << std::endl;
    std::cout << "  (A) 회전 행렬의 크기가 더 크기 때문" << std::endl;
    std::cout << "  (B) 중력 벡터 변환에 직접 영향을 주기 때문" << std::endl;
    std::cout << "  (C) 병진 오차는 자동으로 보정되기 때문" << std::endl;
    std::cout << "  (D) 회전이 추정하기 더 어렵기 때문\n" << std::endl;

    double error_deg = 1.0;
    double error_rad = error_deg * M_PI / 180.0;
    double gravity_leak = std::sin(error_rad) * 9.81;
    std::cout << "  힌트: 1도 회전 오차 → sin(1°) × 9.81 = " << gravity_leak << " m/s^2 가속도 오차"
              << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem3_time_offset()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 시간 오프셋" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Camera-IMU 시간 오프셋(td)을 무시하면 발생하는 문제는?\n" << std::endl;
    std::cout << "  (A) 이미지가 흐려짐" << std::endl;
    std::cout << "  (B) IMU 데이터를 잘못된 시점에 적분" << std::endl;
    std::cout << "  (C) 카메라 내부 파라미터가 변함" << std::endl;
    std::cout << "  (D) IMU 바이어스가 증가함\n" << std::endl;

    std::cout << "  힌트: td = 10ms, 각속도 1 rad/s일 때" << std::endl;
    std::cout << "  → 10ms 동안 회전 = 0.01 rad ≈ 0.57도" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem4_hand_eye()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 핸드-아이 캘리브레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "핸드-아이 캘리브레이션 문제 AX = XB에서\n";
    std::cout << "좋은 결과를 얻기 위해 필요한 데이터 조건은?\n" << std::endl;
    std::cout << "  (A) 가능한 많은 프레임 수" << std::endl;
    std::cout << "  (B) 모든 축에 대한 다양한 회전" << std::endl;
    std::cout << "  (C) 일정한 속도로 직선 운동" << std::endl;
    std::cout << "  (D) 정지 상태에서 오래 촬영\n" << std::endl;

    std::cout << "  힌트: 관측성(observability)을 확보하려면?" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 13 Quiz - Easy (외부 캘리브레이션)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_extrinsic_definition();
    problem2_rotation_vs_translation();
    problem3_time_offset();
    problem4_hand_eye();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
