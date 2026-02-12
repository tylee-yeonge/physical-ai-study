/**
 * Phase 4 Week 3 - 칼만 필터 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

void problem1_kalman_gain_computation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 칼만 게인 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "상황: 2D 상태 [위치, 속도]" << std::endl;
    std::cout << "P = [[4, 0], [0, 2]], H = [1, 0], R = [1]" << std::endl;
    std::cout << "칼만 게인 K를 구하시오.\n" << std::endl;

    Eigen::Matrix2d P;
    P << 4, 0, 0, 2;

    Eigen::Matrix<double, 1, 2> H;
    H << 1, 0;

    Eigen::Matrix<double, 1, 1> R;
    R << 1;

    Eigen::Matrix<double, 1, 1> S = H * P * H.transpose() + R;
    Eigen::Vector2d K = P * H.transpose() * S.inverse();

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "   S = H·P·Hᵀ + R = " << S(0, 0) << std::endl;
    std::cout << "   K = P·Hᵀ·S⁻¹ = [" << K(0) << ", " << K(1) << "]ᵀ" << std::endl;
    std::cout << "   → 위치: K=0.8 (측정 크게 반영)" << std::endl;
    std::cout << "   → 속도: K=0.0 (측정에 속도 정보 없음)" << std::endl;
}

void problem2_update_step()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 업데이트 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "예측: x̂⁻ = [10.0, 2.0]ᵀ (위치 10m, 속도 2m/s)" << std::endl;
    std::cout << "측정: z = 12.0 (위치 12m)" << std::endl;
    std::cout << "K = [0.8, 0.0]ᵀ (문제 1 결과)" << std::endl;
    std::cout << "업데이트 후 상태를 구하시오.\n" << std::endl;

    Eigen::Vector2d x_pred(10.0, 2.0);
    double z = 12.0;
    Eigen::Matrix<double, 1, 2> H;
    H << 1, 0;
    Eigen::Vector2d K(0.8, 0.0);

    double innovation = z - H * x_pred;
    Eigen::Vector2d x_updated = x_pred + K * innovation;

    std::cout << "💡 풀이:" << std::endl;
    std::cout << "   잔차(innovation) = z - H·x̂⁻ = " << innovation << std::endl;
    std::cout << "   x̂ = x̂⁻ + K·잔차" << std::endl;
    std::cout << "   x̂ = [" << x_updated(0) << ", " << x_updated(1) << "]ᵀ" << std::endl;
    std::cout << "   → 위치: 10 + 0.8×2 = 11.6m (측정쪽으로 이동)" << std::endl;
    std::cout << "   → 속도: 2.0 + 0×2 = 2.0m/s (변화 없음)" << std::endl;
}

void problem3_convergence()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 칼만 필터 수렴" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "1D 칼만 필터에서 K와 P의 수렴을 관찰\n" << std::endl;

    // 1D 간단 버전
    double P = 100.0;  // 초기 불확실성 (매우 큼)
    double Q = 0.01;   // 프로세스 노이즈
    double R = 1.0;    // 측정 노이즈

    std::cout << "  Step |   P(예측) |   K    |  P(업데이트)" << std::endl;
    std::cout << "  -----|----------|--------|------------" << std::endl;

    for (int i = 0; i < 15; i++)
    {
        // 예측
        double P_pred = P + Q;

        // 칼만 게인
        double K = P_pred / (P_pred + R);

        // 업데이트
        P = (1 - K) * P_pred;

        if (i < 5 || i >= 12)
        {
            printf("  %4d |  %6.3f  | %.4f | %6.4f\n", i + 1, P_pred, K, P);
        }
        else if (i == 5)
        {
            std::cout << "   ... |   ...    |  ...   |  ..." << std::endl;
        }
    }

    std::cout << "\n💡 관찰:" << std::endl;
    std::cout << "   → K가 빠르게 수렴 (초기 ~1.0 → 안정값)" << std::endl;
    std::cout << "   → P가 빠르게 수렴 (초기 100 → 안정값)" << std::endl;
    std::cout << "   → 정상 상태(steady-state)에서 K, P는 상수" << std::endl;
    std::cout << "   → 안정값은 Q와 R의 비율에 의존" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 3 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_kalman_gain_computation();
    problem2_update_step();
    problem3_convergence();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
