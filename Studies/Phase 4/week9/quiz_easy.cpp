/**
 * Phase 4 Week 9 - Pre-integration 심화 기초 퀴즈
 */

#include <iostream>
#include <cmath>

void problem1_why_covariance()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 공분산이 필요한 이유" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Pre-integration 공분산 Σ가 없으면 어떤 문제가 생기나요?\n" << std::endl;
    std::cout << "  (A) 적분 자체가 불가능" << std::endl;
    std::cout << "  (B) Factor Graph에서 IMU Factor의 가중치를 정할 수 없음" << std::endl;
    std::cout << "  (C) 바이어스 보정이 불가능" << std::endl;
    std::cout << "  (D) Visual Factor에 영향\n" << std::endl;

    std::cout << "💡 힌트: 비용 함수 E = r^T · Σ^{-1} · r 에서 Σ의 역할" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem2_covariance_growth()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 공분산 증가 패턴" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "적분 시간이 늘어날 때, 어떤 불확실성이 가장 빠르게 증가하나요?\n" << std::endl;
    std::cout << "  (A) 회전 σ_φ (∝ √t)" << std::endl;
    std::cout << "  (B) 속도 σ_v (∝ √t)" << std::endl;
    std::cout << "  (C) 위치 σ_p (∝ t^{3/2})" << std::endl;
    std::cout << "  (D) 모두 같은 속도\n" << std::endl;

    std::cout << "💡 힌트: 위치 = 속도의 적분 = 가속도의 이중 적분" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem3_noise_parameters()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: IMU 노이즈 파라미터" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "VINS config에서 IMU 파라미터:\n" << std::endl;
    std::cout << "  acc_n: 가속도 noise density" << std::endl;
    std::cout << "  gyr_n: 자이로 noise density" << std::endl;
    std::cout << "  acc_w: 가속도 bias random walk" << std::endl;
    std::cout << "  gyr_w: 자이로 bias random walk\n" << std::endl;
    std::cout << "질문: Pre-integration 공분산 Q에 사용되는 파라미터는?\n" << std::endl;
    std::cout << "  (A) acc_n, gyr_n (white noise)" << std::endl;
    std::cout << "  (B) acc_w, gyr_w (random walk)" << std::endl;
    std::cout << "  (C) 모두 사용" << std::endl;
    std::cout << "  (D) 사용하지 않음\n" << std::endl;

    std::cout << "💡 힌트: 적분 노이즈는 white noise, 바이어스 변화는 random walk" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem4_vins_dimension()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: VINS의 15D 공분산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "기본 Pre-integration은 9D 공분산이지만," << std::endl;
    std::cout << "VINS는 15×15 공분산을 사용합니다.\n" << std::endl;
    std::cout << "추가된 6차원은 무엇인가요?\n" << std::endl;
    std::cout << "  (A) 중력 방향 (3D) + 스케일 (3D)" << std::endl;
    std::cout << "  (B) 가속도 바이어스 δb_a (3D) + 자이로 바이어스 δb_g (3D)" << std::endl;
    std::cout << "  (C) 카메라 외부 파라미터 (6D)" << std::endl;
    std::cout << "  (D) 랜드마크 좌표 (6D)\n" << std::endl;

    std::cout << "💡 힌트: 바이어스도 시간에 따라 변하는 불확실성이 있음" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 9 Quiz - Easy (Pre-integration 심화 개념)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_why_covariance();
    problem2_covariance_growth();
    problem3_noise_parameters();
    problem4_vins_dimension();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
