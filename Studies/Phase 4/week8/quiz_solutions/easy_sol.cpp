/**
 * Phase 4 Week 8 - Factor Graph 기초 퀴즈 풀이
 */

#include <iostream>
#include <cmath>

void problem1_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: Factor Graph 구성요소" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (1) 변수=[가,나,다], 팩터=[A,B,C]\n" << std::endl;

    std::cout << "  변수 노드 (추정할 것):" << std::endl;
    std::cout << "    (가) 포즈 {R, p, v}: 로봇의 위치/자세/속도" << std::endl;
    std::cout << "    (나) 3D 랜드마크: 환경의 특징점 위치" << std::endl;
    std::cout << "    (다) 바이어스 {b_a, b_g}: IMU 센서 오차\n" << std::endl;

    std::cout << "  팩터 노드 (제약 조건 = 측정에서 유도):" << std::endl;
    std::cout << "    (A) 재투영 오차: 3D 점이 이미지에 투영된 위치와 실제 관측의 차이"
              << std::endl;
    std::cout << "    (B) Pre-integrated measurement: IMU로 측정한 상대 운동" << std::endl;
    std::cout << "    (C) Random Walk 제약: 바이어스가 천천히 변한다는 사전 지식\n" << std::endl;

    std::cout << "  핵심 구분:" << std::endl;
    std::cout << "    변수 = '무엇을 추정할 것인가'" << std::endl;
    std::cout << "    팩터 = '어떤 측정/제약이 있는가'" << std::endl;
}

void problem2_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: IMU Factor 잔차" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (C) 0\n" << std::endl;

    std::cout << "  유도:" << std::endl;
    std::cout << "    상태 복원: p_j = p_i + v_i·Δt + 0.5·g·Δt² + R_i·Δp_ij" << std::endl;
    std::cout << "    양변 정리: R_i^T·(p_j - p_i - v_i·Δt - 0.5·g·Δt²) = Δp_ij\n" << std::endl;

    std::cout << "    잔차 정의: r = R_i^T·(p_j - p_i - v_i·Δt - 0.5·g·Δt²) - Δp_ij" << std::endl;
    std::cout << "            = Δp_ij - Δp_ij = 0 ✓\n" << std::endl;

    std::cout << "  의미:" << std::endl;
    std::cout << "    상태가 완벽하면 → 복원 공식이 정확히 성립" << std::endl;
    std::cout << "    → 잔차 = 0" << std::endl;
    std::cout << "    → 최적화 비용 = 0 (최소!)" << std::endl;
    std::cout << "\n    상태에 오차가 있으면 → 잔차 ≠ 0 → 최적화가 줄여줌" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 정보 행렬의 역할" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (A) IMU (Σ^{-1}이 더 크므로)\n" << std::endl;

    std::cout << "  비용 함수: E = r^T · Σ^{-1} · r\n" << std::endl;
    std::cout << "  IMU:    Σ = 0.01 → Σ^{-1} = 100" << std::endl;
    std::cout << "  Vision: Σ = 4.0  → Σ^{-1} = 0.25\n" << std::endl;

    std::cout << "  같은 크기의 잔차에 대해:" << std::endl;
    std::cout << "    IMU 비용:    r² × 100 = 큰 페널티" << std::endl;
    std::cout << "    Vision 비용: r² × 0.25 = 작은 페널티\n" << std::endl;

    std::cout << "  → 최적화는 IMU 잔차를 더 줄이려 함" << std::endl;
    std::cout << "  → 정확한 센서에 더 의존!\n" << std::endl;

    std::cout << "  칼만 필터와의 유사성:" << std::endl;
    std::cout << "    K = P / (P + R) ← 작은 R(정확) → 큰 K(더 신뢰)" << std::endl;
    std::cout << "    → 같은 원리를 Factor Graph로 일반화한 것!" << std::endl;
}

void problem4_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 풀이: Sliding Window" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (A) 계산량 제한 + 과거 정보를 Prior로 변환\n" << std::endl;

    std::cout << "  Sliding Window가 필요한 이유:" << std::endl;
    std::cout << "    1. 키프레임이 계속 추가됨" << std::endl;
    std::cout << "    2. 최적화 비용 ∝ O(n³) (Schur Complement)" << std::endl;
    std::cout << "    3. 실시간 불가능!\n" << std::endl;

    std::cout << "  Marginalization:" << std::endl;
    std::cout << "    1. 오래된 키프레임을 그래프에서 '제거'" << std::endl;
    std::cout << "    2. 하지만 그 키프레임의 '정보'는 보존" << std::endl;
    std::cout << "    3. Prior Factor로 변환하여 최신 키프레임에 연결\n" << std::endl;

    std::cout << "  (B)가 틀린 이유:" << std::endl;
    std::cout << "    과거 데이터를 '완전 삭제'하면 정보 손실!" << std::endl;
    std::cout << "    Marginalization은 정보를 '압축'하는 것 (삭제 아님)" << std::endl;

    std::cout << "\n  VINS-Mono 예시:" << std::endl;
    std::cout << "    Window = 10 키프레임" << std::endl;
    std::cout << "    11번째 키프레임 추가 시 → 가장 오래된 것 Marginalize" << std::endl;
    std::cout << "    → 항상 10개만 최적화 → 일정한 계산 시간" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 8 Quiz Easy - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
