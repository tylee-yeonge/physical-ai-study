/**
 * Phase 4 Week 13 - Camera-IMU 외부 캘리브레이션 기초 퀴즈 풀이
 */

#include <iostream>
#include <cmath>

void problem1_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: Extrinsic의 정의" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (C) 카메라와 IMU 사이의 상대 회전과 병진\n" << std::endl;

    std::cout << "  Extrinsic = 외부 파라미터:" << std::endl;
    std::cout << "    R_ci: IMU → Camera 회전 (3x3)" << std::endl;
    std::cout << "    t_ci: IMU → Camera 병진 (3x1)\n" << std::endl;
    std::cout << "    p_camera = R_ci * p_imu + t_ci\n" << std::endl;

    std::cout << "  각 선택지:" << std::endl;
    std::cout << "    (A) 초점거리, 왜곡 = Intrinsic (내부 파라미터)" << std::endl;
    std::cout << "    (B) 바이어스, 노이즈 = IMU 캘리브레이션 파라미터" << std::endl;
    std::cout << "    (C) 상대 R, t = Extrinsic (외부 파라미터) ✓" << std::endl;
    std::cout << "    (D) 절대 위치 = Localization 문제" << std::endl;
}

void problem2_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 회전 vs 병진 오차" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) 중력 벡터 변환에 직접 영향을 주기 때문\n" << std::endl;

    std::cout << "  메커니즘:" << std::endl;
    std::cout << "    1. 가속도 변환: a_cam = R_ci * a_imu" << std::endl;
    std::cout << "       → R_ci에 오차가 있으면 a_cam이 틀림\n" << std::endl;
    std::cout << "    2. 중력(9.81 m/s²)이 포함되어 있으므로:" << std::endl;
    std::cout << "       → 작은 회전 오차 × 큰 중력 = 큰 가속도 오차\n" << std::endl;

    double err1 = std::sin(1.0 * M_PI / 180.0) * 9.81;
    double err3 = std::sin(3.0 * M_PI / 180.0) * 9.81;
    std::cout << "    수치 예시:" << std::endl;
    std::cout << "      1도 오차: sin(1°) × 9.81 = " << err1 << " m/s²" << std::endl;
    std::cout << "      3도 오차: sin(3°) × 9.81 = " << err3 << " m/s²\n" << std::endl;

    std::cout << "  병진 오차와 비교:" << std::endl;
    std::cout << "    1cm 병진 오차: 레버암 효과만 (ω × (ω × Δt))" << std::endl;
    std::cout << "    → 일반적인 운동에서는 회전 오차보다 영향이 작음" << std::endl;
}

void problem3_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 시간 오프셋" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) IMU 데이터를 잘못된 시점에 적분\n" << std::endl;

    std::cout << "  설명:" << std::endl;
    std::cout << "    Camera와 IMU의 시계가 td만큼 차이" << std::endl;
    std::cout << "    → Camera가 t에 촬영한 이미지에 대해" << std::endl;
    std::cout << "    → IMU 데이터를 t+td 시점 것을 사용하게 됨\n" << std::endl;

    std::cout << "  수치 예시:" << std::endl;
    std::cout << "    td = 10ms, omega = 1 rad/s:" << std::endl;
    std::cout << "    회전 오차 = 1 × 0.01 = 0.01 rad = 0.57도" << std::endl;
    std::cout << "    중력 누출 = sin(0.01) × 9.81 = 0.098 m/s²\n" << std::endl;

    std::cout << "  → 각 프레임마다 이 오차 발생" << std::endl;
    std::cout << "  → 고속 회전일수록 영향 커짐" << std::endl;
    std::cout << "  → VINS는 td를 온라인으로 추정하여 보정" << std::endl;
}

void problem4_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 풀이: 핸드-아이 캘리브레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) 모든 축에 대한 다양한 회전\n" << std::endl;

    std::cout << "  이유:" << std::endl;
    std::cout << "    AX = XB에서 X (3 DoF 회전)를 유일하게 결정하려면:" << std::endl;
    std::cout << "    → 3축 모두에 대한 정보가 필요\n" << std::endl;
    std::cout << "    → 한 축으로만 회전하면 X의 그 축 성분만 관측 가능" << std::endl;
    std::cout << "    → 나머지 2축은 결정 불가 (degenerate)\n" << std::endl;

    std::cout << "  각 선택지:" << std::endl;
    std::cout << "    (A) 많은 프레임: 도움되지만 다양성이 더 중요" << std::endl;
    std::cout << "    (B) 다양한 회전: 관측성 확보의 핵심! ✓" << std::endl;
    std::cout << "    (C) 직선 운동: 회전 정보 부족 → 실패" << std::endl;
    std::cout << "    (D) 정지: 상대 회전 = I → 정보 없음\n" << std::endl;

    std::cout << "  실용적 팁:" << std::endl;
    std::cout << "    → Kalibr 사용 시 '8자' 운동 추천" << std::endl;
    std::cout << "    → 모든 축 회전 + 다양한 속도" << std::endl;
    std::cout << "    → 60-120초 정도 수집" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 13 Quiz Easy - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
