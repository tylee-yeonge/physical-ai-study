/**
 * Phase 4 Week 5 - ESKF 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

void problem1_error_state_dimension()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Error State 차원 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Full State: x = [p, v, q, b_a, b_g]" << std::endl;
    std::cout << "  p: 위치 (3D)" << std::endl;
    std::cout << "  v: 속도 (3D)" << std::endl;
    std::cout << "  q: 쿼터니언 (4D, 하지만 3자유도)" << std::endl;
    std::cout << "  b_a: 가속도 바이어스 (3D)" << std::endl;
    std::cout << "  b_g: 자이로 바이어스 (3D)\n" << std::endl;

    std::cout << "질문 1: Full State의 파라미터 수는?" << std::endl;
    std::cout << "질문 2: Error State δx = [δp, δv, δθ, δb_a, δb_g]의 차원은?" << std::endl;
    std::cout << "질문 3: EKF의 공분산 P 크기 vs ESKF의 공분산 P 크기는?\n" << std::endl;

    // 정답 계산
    int full_params = 3 + 3 + 4 + 3 + 3;  // 16
    int error_dim = 3 + 3 + 3 + 3 + 3;    // 15

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "   Full State 파라미터: 3+3+4+3+3 = " << full_params << std::endl;
    std::cout << "   Error State 차원: 3+3+3+3+3 = " << error_dim << std::endl;
    std::cout << "   (쿼터니언 4D → δθ 3D로 1 줄어듦!)" << std::endl;
    std::cout << "\n   EKF P: " << full_params << "×" << full_params << " = "
              << full_params * full_params << " 원소" << std::endl;
    std::cout << "   ESKF P: " << error_dim << "×" << error_dim << " = " << error_dim * error_dim
              << " 원소" << std::endl;
    std::cout << "   → ESKF가 더 작고 rank deficiency 없음!" << std::endl;
}

void problem2_linearization_error()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 선형화 오차 비교 (EKF vs ESKF)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "f(θ) = sin(θ)를 선형화할 때," << std::endl;
    std::cout << "EKF는 θ₀에서, ESKF는 δθ≈0 에서 선형화합니다.\n" << std::endl;
    std::cout << "θ₀ = π/4 (45도), δθ = 0.01 rad에서 각각의 선형화 오차를 비교하시오.\n"
              << std::endl;

    // EKF: sin(θ₀ + Δ) ≈ sin(θ₀) + cos(θ₀)·Δ
    double theta0 = M_PI / 4.0;
    double delta = 0.1;  // 10도 정도의 변화

    double ekf_actual = std::sin(theta0 + delta);
    double ekf_approx = std::sin(theta0) + std::cos(theta0) * delta;
    double ekf_error = std::abs(ekf_actual - ekf_approx);

    // ESKF: sin(δθ) ≈ δθ (δθ ≈ 0 근처)
    double dtheta = 0.01;
    double eskf_actual = std::sin(dtheta);
    double eskf_approx = dtheta;
    double eskf_error = std::abs(eskf_actual - eskf_approx);

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "\n   EKF (θ₀=π/4에서 Δ=0.1 변화):" << std::endl;
    std::cout << "   sin(θ₀+Δ) 실제값: " << ekf_actual << std::endl;
    std::cout << "   선형 근사: sin(θ₀) + cos(θ₀)·Δ = " << ekf_approx << std::endl;
    std::cout << "   오차: " << ekf_error << std::endl;

    std::cout << "\n   ESKF (δθ=0.01 근처):" << std::endl;
    std::cout << "   sin(δθ) 실제값: " << eskf_actual << std::endl;
    std::cout << "   선형 근사: δθ = " << eskf_approx << std::endl;
    std::cout << "   오차: " << eskf_error << std::endl;

    std::cout << "\n   오차 비율: " << ekf_error / eskf_error << "배!" << std::endl;
    std::cout << "   → ESKF가 훨씬 정확한 선형화!" << std::endl;
}

void problem3_eskf_update_reset()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: ESKF 업데이트 + Reset 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "1D ESKF에서:" << std::endl;
    std::cout << "  Nominal: p̄ = 10.0, v̄ = 2.0" << std::endl;
    std::cout << "  Error P = [[0.5, 0], [0, 0.3]]" << std::endl;
    std::cout << "  측정: z_gps = 10.8 (위치만 관측)" << std::endl;
    std::cout << "  R = 1.0\n" << std::endl;
    std::cout << "업데이트 후 δx와 Reset 후 Nominal을 구하시오.\n" << std::endl;

    Eigen::Vector2d x_nom(10.0, 2.0);
    Eigen::Matrix2d P;
    P << 0.5, 0.0, 0.0, 0.3;
    double z_gps = 10.8;
    double R = 1.0;
    Eigen::Matrix<double, 1, 2> H;
    H << 1.0, 0.0;

    // 잔차
    double y = z_gps - x_nom(0);

    // 잔차 공분산
    double S = (H * P * H.transpose())(0, 0) + R;

    // 칼만 게인
    Eigen::Vector2d K = P * H.transpose() / S;

    // Error State 업데이트
    Eigen::Vector2d dx = K * y;

    // 공분산 업데이트
    Eigen::Matrix2d P_new = (Eigen::Matrix2d::Identity() - K * H) * P;

    // Reset
    Eigen::Vector2d x_nom_new = x_nom + dx;

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "   잔차: y = z - p̄ = " << z_gps << " - " << x_nom(0) << " = " << y << std::endl;
    std::cout << "   S = H·P·Hᵀ + R = " << (H * P * H.transpose())(0, 0) << " + " << R << " = " << S
              << std::endl;
    std::cout << "   K = P·Hᵀ/S = [" << K(0) << ", " << K(1) << "]ᵀ" << std::endl;
    std::cout << "\n   Error State:" << std::endl;
    std::cout << "   δp = K(0)·y = " << K(0) << "×" << y << " = " << dx(0) << std::endl;
    std::cout << "   δv = K(1)·y = " << K(1) << "×" << y << " = " << dx(1) << std::endl;
    std::cout << "\n   Reset 후 Nominal:" << std::endl;
    std::cout << "   p̄_new = p̄ + δp = " << x_nom(0) << " + " << dx(0) << " = " << x_nom_new(0)
              << std::endl;
    std::cout << "   v̄_new = v̄ + δv = " << x_nom(1) << " + " << dx(1) << " = " << x_nom_new(1)
              << std::endl;
    std::cout << "\n   업데이트 후 P:\n" << P_new << std::endl;
    std::cout << "   → GPS가 p̄보다 0.8m 더 크므로 Nominal이 양의 방향으로 보정됨" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 5 Quiz - Medium (ESKF 계산)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_error_state_dimension();
    problem2_linearization_error();
    problem3_eskf_update_reset();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
