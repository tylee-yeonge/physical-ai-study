/**
 * Phase 4 Week 10 - IMU 적분 기초 퀴즈
 */

#include <iostream>
#include <cmath>

void problem1_drift_causes() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 드리프트의 원인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "IMU 적분에서 가장 치명적인 드리프트 원인은?\n" << std::endl;
    std::cout << "  (A) 가속도 노이즈 (white noise)" << std::endl;
    std::cout << "  (B) 가속도 바이어스" << std::endl;
    std::cout << "  (C) 자이로 바이어스 → 중력 누출" << std::endl;
    std::cout << "  (D) 이산화 오차\n" << std::endl;

    std::cout << "💡 힌트: 0.01 rad 회전 오차 × 9.81 m/s² = ?" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem2_gravity_compensation() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 중력 보상" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "가속도계 출력: a_m = [0, 0, 9.81] m/s² (정지 상태)" << std::endl;
    std::cout << "월드 프레임 가속도를 구하는 올바른 공식은?\n" << std::endl;
    std::cout << "  (A) a_world = a_m" << std::endl;
    std::cout << "  (B) a_world = R · a_m" << std::endl;
    std::cout << "  (C) a_world = R · a_m + g" << std::endl;
    std::cout << "  (D) a_world = R · (a_m - b_a) + g\n" << std::endl;

    std::cout << "💡 힌트: 바이어스 제거 → 좌표 변환 → 중력 보상" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem3_drift_magnitude() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 드리프트 크기 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "가속도 바이어스 b_a = 0.05 m/s²가 있을 때," << std::endl;
    std::cout << "10초 후 위치 드리프트는 약 얼마?\n" << std::endl;
    std::cout << "  (A) 0.5 m (= 0.05 × 10)" << std::endl;
    std::cout << "  (B) 2.5 m (= 0.5 × 0.05 × 10²)" << std::endl;
    std::cout << "  (C) 0.25 m (= 0.05 × 5)" << std::endl;
    std::cout << "  (D) 5.0 m (= 0.05 × 100)\n" << std::endl;

    std::cout << "💡 힌트: p_drift = 0.5 · b_a · t²" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem4_vio_motivation() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: VIO 동기" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "IMU 드리프트 실험에서 얻는 가장 중요한 교훈은?\n" << std::endl;
    std::cout << "  (A) IMU를 더 비싼 것으로 바꾸면 해결" << std::endl;
    std::cout << "  (B) Vision으로 주기적 보정이 필수" << std::endl;
    std::cout << "  (C) 적분 대신 미분을 사용하면 해결" << std::endl;
    std::cout << "  (D) 바이어스를 0으로 가정하면 됨\n" << std::endl;

    std::cout << "💡 힌트: 어떤 IMU든 결국 드리프트는 발생" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 10 Quiz - Easy (IMU 적분 개념)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_drift_causes();
    problem2_gravity_compensation();
    problem3_drift_magnitude();
    problem4_vio_motivation();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
