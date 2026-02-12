/**
 * Phase 4 Week 8 - Factor Graph 기초 퀴즈
 */

#include <iostream>
#include <cmath>

void problem1_factor_graph_components()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Factor Graph 구성요소" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Factor Graph에서 올바른 매칭을 고르시오:\n" << std::endl;
    std::cout << "  변수 노드:        팩터 노드:" << std::endl;
    std::cout << "  (가) 포즈         (A) 재투영 오차" << std::endl;
    std::cout << "  (나) 3D 랜드마크  (B) Pre-integrated measurement" << std::endl;
    std::cout << "  (다) 바이어스     (C) Random Walk 제약\n" << std::endl;
    std::cout << "  선택지:" << std::endl;
    std::cout << "  (1) 변수=[가,나,다], 팩터=[A,B,C]" << std::endl;
    std::cout << "  (2) 변수=[가,A], 팩터=[나,B,다,C]" << std::endl;
    std::cout << "  (3) 변수=[가,나], 팩터=[다,A,B,C]" << std::endl;
    std::cout << "  (4) 변수=[A,B,C], 팩터=[가,나,다]\n" << std::endl;

    std::cout << "💡 힌트: 변수 = 추정할 것, 팩터 = 제약 조건" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem2_imu_factor_residual()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: IMU Factor 잔차" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "IMU Factor의 잔차 r_Δp는:\n" << std::endl;
    std::cout << "  r_Δp = R_i^T · (p_j - p_i - v_i·Δt - 0.5·g·Δt²) - Δp_ij\n" << std::endl;
    std::cout << "질문: 상태 추정이 완벽하면 이 잔차값은?\n" << std::endl;
    std::cout << "  (A) 무한대" << std::endl;
    std::cout << "  (B) 1" << std::endl;
    std::cout << "  (C) 0" << std::endl;
    std::cout << "  (D) 알 수 없음\n" << std::endl;

    std::cout << "💡 힌트: 복원 공식 p_j = p_i + v_i·Δt + 0.5·g·Δt² + R_i·Δp_ij" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem3_information_matrix()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 정보 행렬의 역할" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "비용 함수: E = r^T · Σ^{-1} · r (마하라노비스 거리)\n" << std::endl;
    std::cout << "두 센서의 공분산:" << std::endl;
    std::cout << "  IMU:    σ² = 0.01 (정확)" << std::endl;
    std::cout << "  Vision: σ² = 4.0  (부정확)\n" << std::endl;
    std::cout << "질문: 최적화에서 어떤 센서에 더 의존하나요?\n" << std::endl;
    std::cout << "  (A) IMU (Σ^{-1}이 더 크므로)" << std::endl;
    std::cout << "  (B) Vision (Σ가 더 크므로)" << std::endl;
    std::cout << "  (C) 둘 다 같음" << std::endl;
    std::cout << "  (D) 잔차 크기에 따라 다름\n" << std::endl;

    std::cout << "💡 힌트: E = r^T · Σ^{-1} · r 에서 Σ^{-1}은 가중치" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem4_sliding_window()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Sliding Window" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "VIO에서 Sliding Window를 사용하는 이유와" << std::endl;
    std::cout << "Marginalization의 역할을 설명하시오.\n" << std::endl;
    std::cout << "  (A) 계산량 제한 + 과거 정보를 Prior로 변환" << std::endl;
    std::cout << "  (B) 메모리 절약 + 과거 데이터 완전 삭제" << std::endl;
    std::cout << "  (C) 정확도 향상 + 미래 예측" << std::endl;
    std::cout << "  (D) 루프 클로저 + 전역 최적화\n" << std::endl;

    std::cout << "💡 힌트: Marginalization은 '정보를 버리지 않고 압축'" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 8 Quiz - Easy (Factor Graph 개념)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_factor_graph_components();
    problem2_imu_factor_residual();
    problem3_information_matrix();
    problem4_sliding_window();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
