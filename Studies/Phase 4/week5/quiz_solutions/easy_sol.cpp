/**
 * Phase 4 Week 5 - ESKF 기초 퀴즈 풀이
 */

#include <iostream>
#include <cmath>

void problem1_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: Nominal State vs Error State" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (A), (C)\n" << std::endl;

    std::cout << "  (A) 맞음: Nominal은 IMU 데이터만으로 비선형 적분" << std::endl;
    std::cout << "     → 자코비안 불필요, 칼만 필터 미사용" << std::endl;
    std::cout << "     → p̄ += v̄·dt + 0.5·R(q̄)·a·dt²" << std::endl;
    std::cout << "     → q̄ *= δq(ω·dt)\n" << std::endl;

    std::cout << "  (B) 틀림: Error State는 '작은 오차'만 추정" << std::endl;
    std::cout << "     → δx = [δp, δv, δθ, δb_a, δb_g]" << std::endl;
    std::cout << "     → 항상 0 근처 (Reset 덕분)\n" << std::endl;

    std::cout << "  (C) 맞음: δθ = [δθ_x, δθ_y, δθ_z] ∈ R³" << std::endl;
    std::cout << "     → 쿼터니언(4D)이 아닌 3D 벡터!" << std::endl;
    std::cout << "     → 3자유도를 3파라미터로 정확히 표현\n" << std::endl;

    std::cout << "  (D) 틀림: IMU만으로는 드리프트 발생" << std::endl;
    std::cout << "     → Nominal은 대략적 추정, Error 보정이 필수" << std::endl;
}

void problem2_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: Over-parameterization 해결" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (D) 위 모두 맞음\n" << std::endl;

    std::cout << "  (A) 차원 축소:" << std::endl;
    std::cout << "     EKF: q = [w,x,y,z] → 4파라미터, 하지만 3자유도" << std::endl;
    std::cout << "     ESKF: δθ = [δθ_x,δθ_y,δθ_z] → 3파라미터, 3자유도 ✓\n" << std::endl;

    std::cout << "  (B) 제약 불필요:" << std::endl;
    std::cout << "     EKF: |q|=1 제약 필요 → 업데이트 후 정규화" << std::endl;
    std::cout << "     ESKF: δθ는 자유로운 R³ 벡터 → 제약 없음 ✓\n" << std::endl;

    std::cout << "  (C) 공분산 정상화:" << std::endl;
    std::cout << "     EKF: P가 4×4인데 실제 rank 3 → singular 문제" << std::endl;
    std::cout << "     ESKF: P가 3×3이고 full rank → 수치적으로 안정 ✓" << std::endl;
}

void problem3_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: Reset의 중요성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (D) 위 모두 맞음\n" << std::endl;

    std::cout << "  Reset이 없으면 발생하는 연쇄 문제:\n" << std::endl;

    std::cout << "  Step 1: δx가 점점 커짐" << std::endl;
    std::cout << "    → 업데이트마다 Error가 누적" << std::endl;
    std::cout << "    → δθ가 0.001 → 0.01 → 0.1 → ...커짐\n" << std::endl;

    std::cout << "  Step 2: 작은 각도 근사 깨짐" << std::endl;
    std::cout << "    → sin(0.001) ≈ 0.001 (오차 0.00001%)" << std::endl;
    std::cout << "    → sin(0.1) ≈ 0.1? (오차 0.17%)" << std::endl;
    std::cout << "    → sin(1.0) ≈ 1.0? (오차 16%!)\n" << std::endl;

    std::cout << "  Step 3: EKF와 같은 문제 발생" << std::endl;
    std::cout << "    → 큰 값에서 선형화 = 부정확" << std::endl;
    std::cout << "    → ESKF의 장점이 사라짐" << std::endl;
    std::cout << "\n  결론: Reset은 ESKF의 핵심! 항상 δx ≈ 0을 유지해야 함" << std::endl;
}

void problem4_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 풀이: ESKF 3단계 순서" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (A) (나) → (다) → (가)\n" << std::endl;

    std::cout << "  올바른 순서:" << std::endl;
    std::cout << "  Stage 1 (나): Nominal 예측 (IMU 적분, 200Hz)" << std::endl;
    std::cout << "    → p̄ += v̄·dt + 0.5·R·a·dt²" << std::endl;
    std::cout << "    → q̄ *= δq(ω·dt)" << std::endl;
    std::cout << "    → 비선형 적분, 자코비안 불필요\n" << std::endl;

    std::cout << "  Stage 2 (다): Error 공분산 전파 (IMU와 동시)" << std::endl;
    std::cout << "    → P = F_δ · P · F_δᵀ + Q" << std::endl;
    std::cout << "    → F_δ는 δx≈0에서 계산 → 매우 정확\n" << std::endl;

    std::cout << "  Stage 3 (가): Error 업데이트 + Reset (측정 시, 20Hz)" << std::endl;
    std::cout << "    → δx = K·(z - h(x̄))" << std::endl;
    std::cout << "    → x̄ += δx, δx ← 0\n" << std::endl;

    std::cout << "  실제 동작:" << std::endl;
    std::cout << "    IMU 올 때: Stage 1 + Stage 2 (200Hz)" << std::endl;
    std::cout << "    Vision 올 때: Stage 3 (20Hz)" << std::endl;
    std::cout << "    → 10번 예측마다 1번 업데이트" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 5 Quiz Easy - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
