/**
 * Phase 4 Week 5 - ESKF 중급 퀴즈 풀이
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

void problem1_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: Error State 차원 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "  Full State x = [p, v, q, b_a, b_g]:" << std::endl;
    std::cout << "    p: 3D (위치)" << std::endl;
    std::cout << "    v: 3D (속도)" << std::endl;
    std::cout << "    q: 4D (쿼터니언, 제약: |q|=1)" << std::endl;
    std::cout << "    b_a: 3D (가속도 바이어스)" << std::endl;
    std::cout << "    b_g: 3D (자이로 바이어스)" << std::endl;
    std::cout << "    총: 3+3+4+3+3 = 16 파라미터\n" << std::endl;

    std::cout << "  Error State δx = [δp, δv, δθ, δb_a, δb_g]:" << std::endl;
    std::cout << "    δp: 3D" << std::endl;
    std::cout << "    δv: 3D" << std::endl;
    std::cout << "    δθ: 3D (쿼터니언 4D → 회전 벡터 3D!)" << std::endl;
    std::cout << "    δb_a: 3D" << std::endl;
    std::cout << "    δb_g: 3D" << std::endl;
    std::cout << "    총: 3+3+3+3+3 = 15차원\n" << std::endl;

    std::cout << "  공분산 크기 비교:" << std::endl;
    std::cout << "    EKF: P ∈ R^{16×16} = 256 원소" << std::endl;
    std::cout << "      → 하지만 실제 자유도 15 → rank deficiency!" << std::endl;
    std::cout << "    ESKF: P ∈ R^{15×15} = 225 원소" << std::endl;
    std::cout << "      → 정확히 15자유도 → full rank ✓\n" << std::endl;

    std::cout << "  핵심: 쿼터니언(4D) → δθ(3D)로 1차원 줄어듦" << std::endl;
    std::cout << "  → Over-parameterization 해결 + 수치적 안정성 확보" << std::endl;
}

void problem2_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 선형화 오차 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // EKF 선형화 오차
    double theta0 = M_PI / 4.0;
    double delta = 0.1;

    double ekf_actual = std::sin(theta0 + delta);
    double ekf_approx = std::sin(theta0) + std::cos(theta0) * delta;
    double ekf_error = std::abs(ekf_actual - ekf_approx);

    std::cout << "  EKF: sin(θ)를 θ₀=π/4에서 선형화" << std::endl;
    std::cout << "  ───────────────────────────" << std::endl;
    std::cout << "    테일러 전개: sin(θ₀+Δ) ≈ sin(θ₀) + cos(θ₀)·Δ" << std::endl;
    std::cout << "    실제: sin(π/4 + 0.1) = " << ekf_actual << std::endl;
    std::cout << "    근사: sin(π/4) + cos(π/4)·0.1 = "
              << std::sin(theta0) << " + " << std::cos(theta0) << "·0.1 = " << ekf_approx << std::endl;
    std::cout << "    오차: " << ekf_error << std::endl;

    // ESKF 선형화 오차
    double dtheta = 0.01;
    double eskf_actual = std::sin(dtheta);
    double eskf_approx = dtheta;
    double eskf_error = std::abs(eskf_actual - eskf_approx);

    std::cout << "\n  ESKF: sin(δθ)를 δθ≈0에서 선형화" << std::endl;
    std::cout << "  ───────────────────────────" << std::endl;
    std::cout << "    작은 각도 근사: sin(δθ) ≈ δθ" << std::endl;
    std::cout << "    실제: sin(0.01) = " << eskf_actual << std::endl;
    std::cout << "    근사: 0.01" << std::endl;
    std::cout << "    오차: " << eskf_error << std::endl;

    std::cout << "\n  비교:" << std::endl;
    std::cout << "    EKF 오차 / ESKF 오차 = " << ekf_error / eskf_error << "배" << std::endl;
    std::cout << "    → ESKF가 수백~수천 배 더 정확!" << std::endl;

    // 다양한 각도에서 비교
    std::cout << "\n  다양한 θ에서 sin(θ)≈θ 근사 오차:" << std::endl;
    std::cout << "    θ(rad)     | 실제 sin(θ) | 근사값 θ | 오차(%)" << std::endl;
    std::cout << "    -----------|-------------|---------|--------" << std::endl;
    double angles[] = {0.001, 0.01, 0.1, 0.5, 1.0, 1.5};
    for (double a : angles) {
        double err_pct = std::abs(std::sin(a) - a) / std::sin(a) * 100.0;
        printf("    %10.3f | %11.6f | %7.3f | %6.3f%%\n", a, std::sin(a), a, err_pct);
    }
    std::cout << "\n  → δθ < 0.01 이면 오차 < 0.002% (매우 정확!)" << std::endl;
    std::cout << "  → Reset으로 항상 δθ ≈ 0 유지 = 항상 정확한 근사" << std::endl;
}

void problem3_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: ESKF 업데이트 + Reset" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Eigen::Vector2d x_nom(10.0, 2.0);
    Eigen::Matrix2d P;
    P << 0.5, 0.0,
         0.0, 0.3;
    double z_gps = 10.8;
    double R = 1.0;
    Eigen::Matrix<double, 1, 2> H;
    H << 1.0, 0.0;

    std::cout << "  초기 상태:" << std::endl;
    std::cout << "    Nominal: p̄=" << x_nom(0) << ", v̄=" << x_nom(1) << std::endl;
    std::cout << "    Error: δx = [0, 0] (Reset 후)" << std::endl;
    std::cout << "    P = [[" << P(0,0) << ", " << P(0,1) << "], ["
              << P(1,0) << ", " << P(1,1) << "]]" << std::endl;
    std::cout << "    측정: z_gps = " << z_gps << "\n" << std::endl;

    // Step 1: 잔차
    double y = z_gps - x_nom(0);
    std::cout << "  Step 1: 잔차 계산" << std::endl;
    std::cout << "    y = z - h(x̄) = z - p̄ = " << z_gps << " - " << x_nom(0) << " = " << y << "\n" << std::endl;

    // Step 2: 잔차 공분산
    double S = (H * P * H.transpose())(0,0) + R;
    std::cout << "  Step 2: 잔차 공분산" << std::endl;
    std::cout << "    S = H·P·Hᵀ + R = " << P(0,0) << " + " << R << " = " << S << "\n" << std::endl;

    // Step 3: 칼만 게인
    Eigen::Vector2d K = P * H.transpose() / S;
    std::cout << "  Step 3: 칼만 게인" << std::endl;
    std::cout << "    K = P·Hᵀ·S⁻¹" << std::endl;
    std::cout << "    K = [" << P(0,0) << "/" << S << ", " << P(1,0) << "/" << S << "]ᵀ" << std::endl;
    std::cout << "    K = [" << K(0) << ", " << K(1) << "]ᵀ\n" << std::endl;

    // Step 4: Error 업데이트
    Eigen::Vector2d dx = K * y;
    std::cout << "  Step 4: Error State 업데이트" << std::endl;
    std::cout << "    δx = K·y = [" << K(0) << ", " << K(1) << "]ᵀ × " << y << std::endl;
    std::cout << "    δp = " << dx(0) << std::endl;
    std::cout << "    δv = " << dx(1) << "\n" << std::endl;

    // Step 5: 공분산 업데이트
    Eigen::Matrix2d P_new = (Eigen::Matrix2d::Identity() - K * H) * P;
    std::cout << "  Step 5: 공분산 업데이트" << std::endl;
    std::cout << "    P⁺ = (I - K·H)·P" << std::endl;
    std::cout << "    P⁺ = \n" << P_new << "\n" << std::endl;

    // Step 6: Reset
    Eigen::Vector2d x_nom_new = x_nom + dx;
    std::cout << "  Step 6: Reset (Error → Nominal)" << std::endl;
    std::cout << "    p̄_new = p̄ + δp = " << x_nom(0) << " + " << dx(0) << " = " << x_nom_new(0) << std::endl;
    std::cout << "    v̄_new = v̄ + δv = " << x_nom(1) << " + " << dx(1) << " = " << x_nom_new(1) << std::endl;
    std::cout << "    δx ← [0, 0] (초기화!)\n" << std::endl;

    std::cout << "  해석:" << std::endl;
    std::cout << "    GPS(10.8)가 Nominal(10.0)보다 0.8m 앞" << std::endl;
    std::cout << "    → 위치를 +" << dx(0) << "m 보정 (측정 신뢰도 반영)" << std::endl;
    std::cout << "    → 속도도 +" << dx(1) << "m/s 보정 (상관관계는 없어 0)" << std::endl;
    std::cout << "    K(0)=" << K(0) << " → P/(P+R) = 0.5/1.5 ≈ 0.33" << std::endl;
    std::cout << "    → 1/3은 GPS 신뢰, 2/3은 Nominal 신뢰" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 5 Quiz Medium - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
