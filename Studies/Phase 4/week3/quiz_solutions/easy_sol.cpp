/**
 * Phase 4 Week 3 - 칼만 필터 기초 퀴즈 풀이
 */

#include <iostream>

void problem1_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: 예측-업데이트 사이클" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "예측 (Prediction):" << std::endl;
    std::cout << "  - 모델(물리 법칙)으로 다음 상태 추정" << std::endl;
    std::cout << "  - x̂⁻ = F·x̂ (상태 예측)" << std::endl;
    std::cout << "  - P⁻ = F·P·Fᵀ + Q (공분산 예측)" << std::endl;
    std::cout << "  - 불확실성(P)이 증가! (Q가 더해짐)\n" << std::endl;

    std::cout << "업데이트 (Update):" << std::endl;
    std::cout << "  - 측정값으로 예측을 보정" << std::endl;
    std::cout << "  - K = P⁻·Hᵀ·(H·P⁻·Hᵀ+R)⁻¹ (칼만 게인)" << std::endl;
    std::cout << "  - x̂ = x̂⁻ + K·(z - H·x̂⁻) (상태 보정)" << std::endl;
    std::cout << "  - P = (I-K·H)·P⁻ (공분산 감소)" << std::endl;
    std::cout << "  - 불확실성(P)이 감소!\n" << std::endl;

    std::cout << "핵심: 예측 → P 증가, 업데이트 → P 감소" << std::endl;
    std::cout << "  VIO에서: IMU → 예측(빠르게), Vision → 업데이트(느리게)" << std::endl;
}

void problem2_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 칼만 게인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "K ≈ 1 일 때:" << std::endl;
    std::cout << "  K = P/(P+R) 에서 P >> R" << std::endl;
    std::cout << "  → 예측의 불확실성(P)이 매우 큼" << std::endl;
    std::cout << "  → 측정 노이즈(R)가 상대적으로 작음" << std::endl;
    std::cout << "  → 결론: 측정값을 거의 100% 신뢰\n" << std::endl;

    std::cout << "반대로 K ≈ 0 일 때:" << std::endl;
    std::cout << "  P << R" << std::endl;
    std::cout << "  → 예측이 매우 확실" << std::endl;
    std::cout << "  → 측정 노이즈가 큼" << std::endl;
    std::cout << "  → 결론: 예측값을 거의 100% 신뢰" << std::endl;
}

void problem3_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 공분산 변화" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "P⁻ = F·P·Fᵀ + Q" << std::endl;
    std::cout << "  → 매 예측 단계마다 Q가 누적" << std::endl;
    std::cout << "  → P가 단조 증가\n" << std::endl;

    std::cout << "VIO 관점:" << std::endl;
    std::cout << "  - IMU만 사용 = 예측만 반복" << std::endl;
    std::cout << "  - P(불확실성)가 계속 커짐" << std::endl;
    std::cout << "  - = IMU drift!" << std::endl;
    std::cout << "  - Vision 업데이트로 P 감소 = drift 보정" << std::endl;
}

void problem4_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 풀이: 가중 평균" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double pred = 20.0, pred_var = 9.0;
    double meas = 22.0, meas_var = 1.0;

    double K = pred_var / (pred_var + meas_var);
    double result = pred + K * (meas - pred);
    double result_var = (1 - K) * pred_var;

    std::cout << "  K = " << pred_var << " / (" << pred_var << " + " << meas_var << ") = " << K << std::endl;
    std::cout << "  결과 = " << pred << " + " << K << " × (" << meas << " - " << pred << ") = " << result << std::endl;
    std::cout << "  결과 분산 = (1 - " << K << ") × " << pred_var << " = " << result_var << "\n" << std::endl;

    std::cout << "  해석:" << std::endl;
    std::cout << "  - 측정(σ²=1)이 예측(σ²=9)보다 9배 확실" << std::endl;
    std::cout << "  - 결과 21.8은 측정(22.0)에 매우 가까움" << std::endl;
    std::cout << "  - 결과 분산 0.9 < 두 입력(9, 1) 어느 것보다 작음!" << std::endl;
    std::cout << "  → 정보를 합치면 항상 더 정확해짐" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 3 Quiz Easy - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
