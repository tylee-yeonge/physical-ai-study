/**
 * Phase 4 Week 5 - ESKF 기초 퀴즈
 */

#include <iostream>
#include <cmath>

void problem1_nominal_vs_error() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Nominal State vs Error State" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "ESKF에서 Full State = Nominal ⊕ Error 입니다.\n" << std::endl;
    std::cout << "다음 중 올바른 설명을 모두 고르시오:\n" << std::endl;
    std::cout << "  (A) Nominal State는 IMU로 적분하며, 칼만 필터를 사용하지 않는다" << std::endl;
    std::cout << "  (B) Error State는 큰 값을 직접 추정한다" << std::endl;
    std::cout << "  (C) Error State의 회전 오차 δθ는 3차원 벡터이다" << std::endl;
    std::cout << "  (D) Nominal State만으로도 충분히 정확한 추정이 가능하다\n" << std::endl;

    std::cout << "💡 힌트: Nominal은 비선형 적분, Error는 칼만 필터" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem2_over_parameterization() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Over-parameterization 해결" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "EKF에서 쿼터니언 q = [w, x, y, z]로 회전을 표현할 때," << std::endl;
    std::cout << "ESKF에서는 δθ = [δθ_x, δθ_y, δθ_z]로 회전 오차를 표현합니다.\n" << std::endl;

    std::cout << "질문: 왜 ESKF가 Over-parameterization 문제를 해결하나요?\n" << std::endl;
    std::cout << "  (A) 쿼터니언 4D → δθ 3D로 차원이 줄어서" << std::endl;
    std::cout << "  (B) 3자유도를 3파라미터로 정확히 표현하므로 제약 불필요" << std::endl;
    std::cout << "  (C) 공분산이 4×4 → 3×3으로 줄어 rank deficiency 해결" << std::endl;
    std::cout << "  (D) 위 모두 맞음\n" << std::endl;

    std::cout << "💡 힌트: δq ≈ [1, δθ/2] (작은 각도 근사)" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem3_reset_importance() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Reset의 중요성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "ESKF 업데이트 후 Reset 단계에서 하는 일:\n" << std::endl;
    std::cout << "  1. Nominal에 Error를 반영: x̄ += δx" << std::endl;
    std::cout << "  2. Error를 0으로 초기화: δx ← 0\n" << std::endl;

    std::cout << "질문: Reset을 하지 않으면 어떤 문제가 발생하나요?\n" << std::endl;
    std::cout << "  (A) δx가 점점 커져서 작은 오차 가정이 깨짐" << std::endl;
    std::cout << "  (B) sin(δθ) ≈ δθ 근사가 부정확해짐" << std::endl;
    std::cout << "  (C) 결국 EKF와 같은 선형화 오차 문제 발생" << std::endl;
    std::cout << "  (D) 위 모두 맞음\n" << std::endl;

    std::cout << "💡 힌트: ESKF의 핵심 장점은 δx ≈ 0 근처에서의 정확한 선형화" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem4_eskf_stages() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: ESKF 3단계 순서" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "ESKF의 3단계를 올바른 순서로 나열하시오:\n" << std::endl;
    std::cout << "  (가) Error 업데이트 + Reset (측정이 올 때)" << std::endl;
    std::cout << "  (나) Nominal 예측 (IMU 적분)" << std::endl;
    std::cout << "  (다) Error 공분산 전파\n" << std::endl;
    std::cout << "  선택지:" << std::endl;
    std::cout << "  (A) (나) → (다) → (가)" << std::endl;
    std::cout << "  (B) (다) → (나) → (가)" << std::endl;
    std::cout << "  (C) (나) → (가) → (다)" << std::endl;
    std::cout << "  (D) (가) → (나) → (다)\n" << std::endl;

    std::cout << "💡 힌트: IMU가 200Hz, Vision이 20Hz이면 어떤 순서?" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 5 Quiz - Easy (ESKF 개념)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_nominal_vs_error();
    problem2_over_parameterization();
    problem3_reset_importance();
    problem4_eskf_stages();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
