/**
 * Phase 4 Week 3 - 칼만 필터 중급 퀴즈 풀이
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

void problem1_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: 칼만 게인 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Eigen::Matrix2d P;
    P << 4, 0, 0, 2;

    Eigen::Matrix<double, 1, 2> H;
    H << 1, 0;

    Eigen::Matrix<double, 1, 1> R;
    R << 1;

    std::cout << "Step 1: S = H·P·Hᵀ + R" << std::endl;
    Eigen::Matrix<double, 1, 1> S = H * P * H.transpose() + R;
    std::cout << "  H·P = [1,0]·[[4,0],[0,2]] = [4, 0]" << std::endl;
    std::cout << "  H·P·Hᵀ = [4,0]·[1,0]ᵀ = 4" << std::endl;
    std::cout << "  S = 4 + 1 = " << S(0,0) << "\n" << std::endl;

    std::cout << "Step 2: K = P·Hᵀ·S⁻¹" << std::endl;
    Eigen::Vector2d K = P * H.transpose() * S.inverse();
    std::cout << "  P·Hᵀ = [[4,0],[0,2]]·[1,0]ᵀ = [4, 0]ᵀ" << std::endl;
    std::cout << "  K = [4, 0]ᵀ / 5 = [" << K(0) << ", " << K(1) << "]ᵀ\n" << std::endl;

    std::cout << "해석:" << std::endl;
    std::cout << "  K(위치) = 0.8: 측정으로 위치를 80% 보정" << std::endl;
    std::cout << "  K(속도) = 0.0: 측정에 속도 정보 없으므로 보정 없음" << std::endl;
    std::cout << "  → H가 [1,0]이므로 위치만 관측, 속도는 간접 추정" << std::endl;
}

void problem2_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 업데이트 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Eigen::Vector2d x_pred(10.0, 2.0);
    double z = 12.0;
    Eigen::Matrix<double, 1, 2> H;
    H << 1, 0;
    Eigen::Vector2d K(0.8, 0.0);

    double innovation = z - (H * x_pred)(0);
    std::cout << "Step 1: 잔차(innovation)" << std::endl;
    std::cout << "  y = z - H·x̂⁻ = 12.0 - [1,0]·[10, 2]ᵀ = " << innovation << "\n" << std::endl;

    Eigen::Vector2d x_updated = x_pred + K * innovation;
    std::cout << "Step 2: 상태 업데이트" << std::endl;
    std::cout << "  x̂ = x̂⁻ + K·y" << std::endl;
    std::cout << "  위치: 10.0 + 0.8 × 2.0 = " << x_updated(0) << std::endl;
    std::cout << "  속도: 2.0 + 0.0 × 2.0 = " << x_updated(1) << "\n" << std::endl;

    // 공분산 업데이트도 계산
    Eigen::Matrix2d P_pred;
    P_pred << 4, 0, 0, 2;
    Eigen::Matrix2d I = Eigen::Matrix2d::Identity();
    Eigen::Matrix2d P_updated = (I - K * H) * P_pred;

    std::cout << "Step 3: 공분산 업데이트" << std::endl;
    std::cout << "  P = (I - K·H)·P⁻" << std::endl;
    std::cout << "  P(0,0) = " << P_updated(0,0) << " (4.0에서 감소!)" << std::endl;
    std::cout << "  P(1,1) = " << P_updated(1,1) << " (변화 없음)\n" << std::endl;

    std::cout << "핵심: 위치 불확실성이 4.0 → 0.8로 감소" << std::endl;
    std::cout << "  → 측정값 하나로 5배나 확실해짐!" << std::endl;
}

void problem3_solution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: 칼만 필터 수렴" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double P = 100.0;
    double Q = 0.01;
    double R = 1.0;

    std::cout << "  초기 P = 100.0 (매우 불확실)" << std::endl;
    std::cout << "  Q = 0.01 (프로세스 노이즈)" << std::endl;
    std::cout << "  R = 1.0 (측정 노이즈)\n" << std::endl;

    for (int i = 0; i < 20; i++) {
        double P_pred = P + Q;
        double K = P_pred / (P_pred + R);
        P = (1 - K) * P_pred;

        if (i < 5 || i >= 17) {
            printf("  Step %2d: P_pred=%7.4f, K=%6.4f, P=%7.4f\n", i+1, P_pred, K, P);
        } else if (i == 5) {
            std::cout << "  ...  (수렴 중)" << std::endl;
        }
    }

    std::cout << "\n관찰:" << std::endl;
    std::cout << "  1. K: 0.99 → ~0.01 로 빠르게 감소" << std::endl;
    std::cout << "  2. P: 100 → ~0.01 로 빠르게 감소" << std::endl;
    std::cout << "  3. 정상상태에서 K와 P는 Q, R에만 의존" << std::endl;
    std::cout << "\n정상상태 공식 (Ricatti 방정식 해):" << std::endl;
    double P_ss = (-R + std::sqrt(R*R + 4*R*Q)) / 2;
    double K_ss = P_ss / (P_ss + R);
    std::cout << "  P_ss = (-R + √(R² + 4RQ)) / 2 = " << P_ss << std::endl;
    std::cout << "  K_ss = P_ss / (P_ss + R) = " << K_ss << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 3 Quiz Medium - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
