/**
 * Quiz Easy - Week 11: VIO 초기화 문제
 */

#include <iostream>

void problem1_initialization_unknowns() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 초기화 미지수" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1. VIO 초기화 시 추정해야 할 미지수가 아닌 것은?\n" << std::endl;
    std::cout << "   a) 스케일 (scale)\n";
    std::cout << "   b) 중력 방향 (gravity direction)\n";
    std::cout << "   c) 카메라 초점 거리 (focal length)\n";
    std::cout << "   d) 자이로 바이어스 (gyro bias)\n" << std::endl;
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "   💡 힌트: 카메라 내부 파라미터는 사전 캘리브레이션으로 이미 알고 있습니다.\n" << std::endl;
}

void problem2_scale_ambiguity() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 스케일 모호성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q2. Vision-only SfM으로 복원한 궤적에서 스케일을 알 수 없는 근본적인\n";
    std::cout << "    이유는?\n" << std::endl;
    std::cout << "   a) 카메라 해상도가 너무 낮아서\n";
    std::cout << "   b) 단안 카메라는 깊이 정보가 없어 크기와 거리를 구분할 수 없으므로\n";
    std::cout << "   c) IMU가 없어서\n";
    std::cout << "   d) GPS 신호가 없어서\n" << std::endl;
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "   💡 힌트: 10cm 상자를 가까이서 본 것과 10m 벽을 멀리서 본 것의\n";
    std::cout << "            이미지가 같을 수 있습니다.\n" << std::endl;
}

void problem3_gravity_estimation() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 중력 방향 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q3. VIO 초기화에서 중력 벡터의 자유도(DoF)는 몇인가?\n" << std::endl;
    std::cout << "   a) 1 (크기만 추정)\n";
    std::cout << "   b) 2 (방향만 추정, 크기 9.81 m/s²는 알고 있음)\n";
    std::cout << "   c) 3 (x, y, z 성분 모두 추정)\n";
    std::cout << "   d) 6 (위치와 방향 모두 추정)\n" << std::endl;
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "   💡 힌트: ||g|| = 9.81 m/s²는 상수입니다.\n";
    std::cout << "            단위 구 위의 한 점으로 표현할 수 있습니다.\n" << std::endl;
}

void problem4_initialization_failure() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 초기화 실패 원인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q4. VIO 초기화가 실패하기 쉬운 상황은?\n" << std::endl;
    std::cout << "   a) 다양한 방향으로 빠르게 움직일 때\n";
    std::cout << "   b) 텍스처가 풍부한 환경에서 움직일 때\n";
    std::cout << "   c) 카메라를 정지 상태로 두고 있을 때\n";
    std::cout << "   d) 충분한 수의 특징점이 추적될 때\n" << std::endl;
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "   💡 힌트: 삼각측량을 위해서는 시차(parallax)가 필요합니다.\n";
    std::cout << "            정지 상태에서 시차는 얼마인가요?\n" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 11 Quiz - Easy" << std::endl;
    std::cout << "VIO 초기화 문제" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_initialization_unknowns();
    problem2_scale_ambiguity();
    problem3_gravity_estimation();
    problem4_initialization_failure();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
