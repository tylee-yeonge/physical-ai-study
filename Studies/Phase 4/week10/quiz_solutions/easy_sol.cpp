/**
 * Phase 4 Week 10 - IMU 적분 기초 퀴즈 풀이
 */

#include <iostream>
#include <cmath>

void problem1_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: 드리프트의 원인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (C) 자이로 바이어스 → 중력 누출\n" << std::endl;

    std::cout << "  메커니즘:" << std::endl;
    std::cout << "    1. 자이로 바이어스 b_g → 회전 오차 축적" << std::endl;
    std::cout << "       δθ = b_g × t (시간에 비례)\n" << std::endl;
    std::cout << "    2. 회전 오차 → 중력 방향 틀어짐" << std::endl;
    std::cout << "       중력 벡터가 수평 방향으로 '누출'\n" << std::endl;
    std::cout << "    3. 가짜 수평 가속도 발생" << std::endl;
    std::cout << "       a_fake ≈ sin(δθ) × 9.81 ≈ δθ × 9.81\n" << std::endl;
    std::cout << "    4. 이중 적분 → 위치 드리프트" << std::endl;
    std::cout << "       p_drift ∝ b_g × g × t³ / 6\n" << std::endl;

    std::cout << "  수치 예시:" << std::endl;
    std::cout << "    b_g = 0.01 rad/s → 1초 후 δθ = 0.01 rad" << std::endl;
    double fake_a = 0.01 * 9.81;
    std::cout << "    가짜 가속도: " << fake_a << " m/s²" << std::endl;
    std::cout << "    → 이것이 (A)가속도 노이즈 0.1보다 작지만," << std::endl;
    std::cout << "    → 바이어스이므로 확정적으로 누적 → 더 치명적!" << std::endl;
}

void problem2_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 중력 보상" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (D) a_world = R · (a_m - b_a) + g\n" << std::endl;

    std::cout << "  단계별 풀이:\n" << std::endl;
    std::cout << "  1. 바이어스 제거: a_m - b_a" << std::endl;
    std::cout << "     → 센서 고유의 오프셋 제거\n" << std::endl;
    std::cout << "  2. 좌표 변환: R · (a_m - b_a)" << std::endl;
    std::cout << "     → 바디 프레임 → 월드 프레임\n" << std::endl;
    std::cout << "  3. 중력 보상: + g" << std::endl;
    std::cout << "     → 가속도계가 측정한 중력 성분 제거" << std::endl;
    std::cout << "     → g = [0, 0, -9.81]\n" << std::endl;

    std::cout << "  검증 (정지 상태):" << std::endl;
    std::cout << "    a_m = [0, 0, 9.81], R = I, b_a = 0" << std::endl;
    std::cout << "    a_world = I·[0,0,9.81] + [0,0,-9.81] = [0,0,0] ✓" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 드리프트 크기 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) 2.5 m\n" << std::endl;

    double b_a = 0.05;
    double t = 10.0;
    double p_drift = 0.5 * b_a * t * t;

    std::cout << "  바이어스에 의한 위치 드리프트:" << std::endl;
    std::cout << "    p = 0.5 · b_a · t²" << std::endl;
    std::cout << "    = 0.5 × " << b_a << " × " << t * t << std::endl;
    std::cout << "    = " << p_drift << " m\n" << std::endl;

    std::cout << "  유도:" << std::endl;
    std::cout << "    v(t) = ∫b_a dt = b_a·t (등가속도)" << std::endl;
    std::cout << "    p(t) = ∫v dt = 0.5·b_a·t² (이차 함수)\n" << std::endl;

    std::cout << "  시간별:" << std::endl;
    for (double ti : {1.0, 5.0, 10.0, 30.0})
    {
        printf("    t=%4.0fs: p = %.2f m\n", ti, 0.5 * b_a * ti * ti);
    }
    std::cout << "  → 시간의 제곱에 비례해서 빠르게 발산!" << std::endl;
}

void problem4_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 풀이: VIO 동기" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) Vision으로 주기적 보정이 필수\n" << std::endl;

    std::cout << "  (A) 틀림: 비싼 IMU도 결국 드리프트" << std::endl;
    std::cout << "    → 항법급 IMU: 느리지만 여전히 발산" << std::endl;
    std::cout << "    → MEMS vs 항법급: 속도 차이만 (본질은 같음)\n" << std::endl;

    std::cout << "  (B) 맞음: Vision이 '절대적 기준'을 제공" << std::endl;
    std::cout << "    → 특징점 매칭 → 포즈 보정" << std::endl;
    std::cout << "    → 주기적 보정으로 드리프트 억제" << std::endl;
    std::cout << "    → 이것이 VIO (Visual-Inertial Odometry)!\n" << std::endl;

    std::cout << "  (C) 틀림: 미분하면 노이즈 증폭" << std::endl;
    std::cout << "    → 적분은 노이즈를 평활화하는 장점도 있음\n" << std::endl;

    std::cout << "  (D) 틀림: 바이어스는 현실에서 항상 존재" << std::endl;
    std::cout << "    → 무시하면 이차 함수 드리프트 발생" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 10 Quiz Easy - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
