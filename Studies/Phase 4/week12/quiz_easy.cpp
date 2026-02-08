/**
 * Phase 4 Week 12 - VIO 초기화 과정 기초 퀴즈
 */

#include <iostream>
#include <cmath>

void problem1_sfm_scale() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Vision-only SfM의 한계" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "단안 카메라로 SfM을 수행하면 up-to-scale 궤적을 얻습니다." << std::endl;
    std::cout << "이때 스케일을 알 수 없는 근본적인 이유는?\n" << std::endl;
    std::cout << "  (A) 카메라 해상도가 부족해서" << std::endl;
    std::cout << "  (B) Essential Matrix에서 t가 정규화되기 때문" << std::endl;
    std::cout << "  (C) 특징점 매칭 오차 때문" << std::endl;
    std::cout << "  (D) 렌즈 왜곡 때문\n" << std::endl;

    std::cout << "  힌트: E = [t]_x · R에서 E는 스케일에 무관합니다." << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem2_bias_estimation_order() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 바이어스 추정 순서" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "VINS-Mono 초기화에서 자이로 바이어스를 먼저 추정하는 이유는?\n" << std::endl;
    std::cout << "  (A) 자이로가 가속도계보다 더 정확해서" << std::endl;
    std::cout << "  (B) 회전 비교가 스케일과 무관하기 때문" << std::endl;
    std::cout << "  (C) 자이로 바이어스가 더 크기 때문" << std::endl;
    std::cout << "  (D) 하드웨어 제약 때문\n" << std::endl;

    std::cout << "  힌트: Vision R과 IMU ΔR을 비교할 때 스케일이 필요한가?" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem3_gravity_constraint() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 중력 제약 조건" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "선형 Alignment 후 중력 벡터가 g = [0.2, -0.1, -9.75]로 추정되었습니다." << std::endl;
    std::cout << "이 결과를 어떻게 검증하고 정제하는가?\n" << std::endl;
    std::cout << "  (A) 그대로 사용 (충분히 정확)" << std::endl;
    std::cout << "  (B) ||g|| = 9.81 제약을 적용하여 방향만 보정" << std::endl;
    std::cout << "  (C) z축 성분만 -9.81로 교체" << std::endl;
    std::cout << "  (D) g = [0, 0, -9.81]로 고정\n" << std::endl;

    double g_norm = std::sqrt(0.2*0.2 + 0.1*0.1 + 9.75*9.75);
    std::cout << "  현재 ||g|| = " << g_norm << " (참값: 9.81)" << std::endl;
    std::cout << "  힌트: 중력의 크기는 알지만 방향은 미지수" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem4_initialization_failure() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 초기화 실패 조건" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "다음 중 VIO 초기화가 실패할 가능성이 가장 높은 상황은?\n" << std::endl;
    std::cout << "  (A) 빠른 회전 운동" << std::endl;
    std::cout << "  (B) 순수 병진 운동 (회전 없음)" << std::endl;
    std::cout << "  (C) 정지 상태 (움직임 없음)" << std::endl;
    std::cout << "  (D) 다양한 축의 회전 + 병진\n" << std::endl;

    std::cout << "  힌트: 스케일 추정에는 '가속도 변화'가 필요합니다." << std::endl;
    std::cout << "  중력과 가속도를 분리하려면?" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 12 Quiz - Easy (VIO 초기화 과정)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_sfm_scale();
    problem2_bias_estimation_order();
    problem3_gravity_constraint();
    problem4_initialization_failure();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
