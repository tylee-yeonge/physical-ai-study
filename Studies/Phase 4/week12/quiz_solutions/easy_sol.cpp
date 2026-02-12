/**
 * Phase 4 Week 12 - VIO 초기화 과정 기초 퀴즈 풀이
 */

#include <iostream>
#include <cmath>

void problem1_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: Vision-only SfM의 한계" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) Essential Matrix에서 t가 정규화되기 때문\n" << std::endl;

    std::cout << "  설명:" << std::endl;
    std::cout << "    1. Essential Matrix: E = [t]_x · R" << std::endl;
    std::cout << "       → E와 k·E는 같은 에피폴라 제약을 만족" << std::endl;
    std::cout << "       → t의 크기(스케일)를 결정할 수 없음\n" << std::endl;
    std::cout << "    2. SVD로 E 분해 시 ||t|| = 1로 정규화" << std::endl;
    std::cout << "       → 이동 거리가 항상 1 (단위가 없는 값)\n" << std::endl;
    std::cout << "    3. 이로 인해 모든 3D 점도 up-to-scale" << std::endl;
    std::cout << "       → 실제 크기를 알 수 없음\n" << std::endl;
    std::cout << "    4. 이것이 단안 카메라의 근본적 한계" << std::endl;
    std::cout << "       → Stereo 카메라는 baseline을 아니까 해결됨" << std::endl;
    std::cout << "       → IMU도 가속도 → 실제 거리를 제공" << std::endl;
}

void problem2_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 바이어스 추정 순서" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) 회전 비교가 스케일과 무관하기 때문\n" << std::endl;

    std::cout << "  핵심 논리:" << std::endl;
    std::cout << "    1. 회전(R)은 스케일과 무관한 양" << std::endl;
    std::cout << "       → 물체가 크든 작든 회전 각도는 같음\n" << std::endl;
    std::cout << "    2. Vision의 R_ij와 IMU의 ΔR_ij를 직접 비교 가능" << std::endl;
    std::cout << "       → 스케일 s를 모르는 상태에서도 가능!\n" << std::endl;
    std::cout << "    3. b_g를 먼저 추정하면:" << std::endl;
    std::cout << "       → Pre-integration 보정 가능 (ΔR, Δv, Δp 모두)" << std::endl;
    std::cout << "       → 이후 스케일, 중력 추정이 더 정확\n" << std::endl;
    std::cout << "    4. 가속도계 바이어스 b_a는 중력과 결합됨" << std::endl;
    std::cout << "       → a_m = R^T(a - g) + b_a + n_a" << std::endl;
    std::cout << "       → g와 b_a를 분리하기 어려움" << std::endl;
    std::cout << "       → 나중에 비선형 최적화로 추정" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 중력 제약 조건" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) ||g|| = 9.81 제약을 적용하여 방향만 보정\n" << std::endl;

    double gx = 0.2, gy = -0.1, gz = -9.75;
    double g_norm = std::sqrt(gx * gx + gy * gy + gz * gz);

    std::cout << "  풀이:" << std::endl;
    std::cout << "    g_estimated = [0.2, -0.1, -9.75]" << std::endl;
    std::cout << "    ||g_estimated|| = " << g_norm << "\n" << std::endl;

    double gx_n = gx / g_norm, gy_n = gy / g_norm, gz_n = gz / g_norm;
    std::cout << "    단위 벡터: g_hat = g / ||g||" << std::endl;
    printf("    = [%.6f, %.6f, %.6f]\n\n", gx_n, gy_n, gz_n);

    double g_true = 9.81;
    printf("    정제: g_refined = 9.81 * g_hat\n");
    printf("    = [%.4f, %.4f, %.4f]\n\n", gx_n * g_true, gy_n * g_true, gz_n * g_true);

    std::cout << "  왜 (B)인가:" << std::endl;
    std::cout << "    (A) 틀림: 0.2 m/s² 수평 성분 → 큰 드리프트 유발" << std::endl;
    std::cout << "    (C) 틀림: 수평 성분이 완전히 0이 아닐 수 있음" << std::endl;
    std::cout << "           (센서가 기울어진 경우)" << std::endl;
    std::cout << "    (D) 틀림: 월드 프레임이 수평이 아닐 수 있음" << std::endl;
    std::cout << "    (B) 맞음: 방향은 추정값 유지, 크기만 물리 상수로 보정" << std::endl;
}

void problem4_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 풀이: 초기화 실패 조건" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (C) 정지 상태 (움직임 없음)\n" << std::endl;

    std::cout << "  각 선택지 분석:\n" << std::endl;
    std::cout << "  (A) 빠른 회전: 오히려 좋음" << std::endl;
    std::cout << "      → 다양한 시점 → 좋은 삼각측량" << std::endl;
    std::cout << "      → 자이로 바이어스 추정에 유리\n" << std::endl;

    std::cout << "  (B) 순수 병진: 부분적으로 가능" << std::endl;
    std::cout << "      → Essential Matrix 분해 가능" << std::endl;
    std::cout << "      → 단, 자이로 바이어스 관측성 제한\n" << std::endl;

    std::cout << "  (C) 정지 상태: 초기화 불가!" << std::endl;
    std::cout << "      → Vision: 동일한 영상 → SfM 실패" << std::endl;
    std::cout << "      → IMU: 중력만 측정 → 가속도 변화 없음" << std::endl;
    std::cout << "      → 스케일 추정 불가 (이동이 없으므로)" << std::endl;
    std::cout << "      → 중력-바이어스 분리 불가\n" << std::endl;

    std::cout << "  (D) 다양한 운동: 가장 이상적!" << std::endl;
    std::cout << "      → 모든 미지수에 대한 관측성(observability) 확보" << std::endl;
    std::cout << "      → 이것이 Kalibr에서도 다양한 움직임을 요구하는 이유" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 12 Quiz Easy - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
