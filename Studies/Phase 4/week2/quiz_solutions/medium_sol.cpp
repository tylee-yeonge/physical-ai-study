/**
 * Phase 4 Week 2 - IMU 노이즈 모델 중급 퀴즈 풀이
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <random>
#include <iomanip>

using namespace Eigen;
using namespace std;

void problem1_solution()
{
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 1 풀이: 노이즈 전파 시뮬레이션" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    double sigma_a = 0.1;
    double dt = 0.01;
    double T = 5.0;
    int N = T / dt;

    // 이론값
    double sigma_p_theory = sigma_a * sqrt(dt) * pow(T, 1.5) / sqrt(3.0);

    cout << "  이론값:" << endl;
    cout << "    σ_p = σ_a × √dt × T^{3/2} / √3" << endl;
    cout << "        = " << sigma_a << " × " << sqrt(dt) << " × " << pow(T, 1.5) << " / "
         << sqrt(3.0) << endl;
    cout << "        = " << sigma_p_theory << " m\n" << endl;

    // 시뮬레이션 (여러 번 반복하여 RMS 계산)
    int trials = 1000;
    double sum_sq = 0;

    default_random_engine gen(42);
    normal_distribution<double> noise(0, sigma_a);

    for (int trial = 0; trial < trials; trial++)
    {
        double v = 0, p = 0;
        for (int i = 0; i < N; i++)
        {
            double a = noise(gen);
            v += a * dt;
            p += v * dt;
        }
        sum_sq += p * p;
    }

    double sigma_p_sim = sqrt(sum_sq / trials);

    cout << "  시뮬레이션 (" << trials << "회):" << endl;
    cout << "    σ_p (RMS) = " << sigma_p_sim << " m" << endl;
    cout << "    이론값    = " << sigma_p_theory << " m" << endl;
    cout << "    비율      = " << sigma_p_sim / sigma_p_theory << "\n" << endl;

    cout << "  핵심: White Noise → 위치 불확실성 ∝ t^{3/2}" << endl;
    cout << "    5초 후 약 " << fixed << setprecision(3) << sigma_p_theory << "m (1σ)" << endl;
}

void problem2_solution()
{
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 2 풀이: 바이어스 vs 노이즈 성장률" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    double sigma_a = 0.08;  // m/s²/√Hz
    double b_a = 0.02;      // m/s²
    double dt = 0.005;

    cout << "  노이즈 위치: σ_p = σ_a × √dt × t^{3/2} / √3" << endl;
    cout << "  바이어스 위치: p_b = 0.5 × b_a × t²\n" << endl;

    cout << "  시간(s) │ 노이즈 σ_p(m) │ 바이어스 p(m) │ 더 큰 쪽" << endl;
    cout << "  ────────┼──────────────┼──────────────┼─────────" << endl;

    double crossover = 0;
    for (double t : {1.0, 5.0, 10.0, 30.0})
    {
        double sigma_p = sigma_a * sqrt(dt) * pow(t, 1.5) / sqrt(3.0);
        double p_bias = 0.5 * b_a * t * t;
        const char* which = (sigma_p > p_bias) ? "노이즈" : "바이어스";

        printf("    %4.0f   │    %8.4f    │    %8.4f    │  %s\n", t, sigma_p, p_bias, which);
    }

    // 교차점: σ_a √dt t^{3/2}/√3 = 0.5 b_a t²
    // t^{1/2} = 2 σ_a √dt / (√3 b_a)
    // t = [2 σ_a √dt / (√3 b_a)]²
    double t_cross = pow(2 * sigma_a * sqrt(dt) / (sqrt(3.0) * b_a), 2);

    cout << "\n  교차점: t ≈ " << t_cross << "초" << endl;
    cout << "    → " << t_cross << "초 이후부터 바이어스가 지배적" << endl;
    cout << "    → 장시간 적분에서 바이어스 추정이 핵심인 이유!\n" << endl;

    cout << "  성장률 요약:" << endl;
    cout << "    White Noise → 위치: ∝ t^{3/2}" << endl;
    cout << "    Bias        → 위치: ∝ t²" << endl;
    cout << "    → 바이어스가 항상 이김 (충분한 시간 후)" << endl;
}

void problem3_solution()
{
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 3 풀이: 연속-이산 노이즈 변환" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    double sigma_a_c = 0.04;   // m/s²/√Hz
    double sigma_g_c = 0.005;  // rad/s/√Hz

    cout << "  (1) 200Hz (dt = 0.005s):\n" << endl;
    double dt_200 = 0.005;
    double sigma_a_200 = sigma_a_c / sqrt(dt_200);
    double sigma_g_200 = sigma_g_c / sqrt(dt_200);

    printf("    σ_a_discrete = %.4f / √%.4f = %.4f m/s²\n", sigma_a_c, dt_200, sigma_a_200);
    printf("    σ_g_discrete = %.5f / √%.4f = %.5f rad/s\n\n", sigma_g_c, dt_200, sigma_g_200);

    cout << "  (2) 400Hz (dt = 0.0025s):\n" << endl;
    double dt_400 = 0.0025;
    double sigma_a_400 = sigma_a_c / sqrt(dt_400);
    double sigma_g_400 = sigma_g_c / sqrt(dt_400);

    printf("    σ_a_discrete = %.4f / √%.5f = %.4f m/s²\n", sigma_a_c, dt_400, sigma_a_400);
    printf("    σ_g_discrete = %.5f / √%.5f = %.5f rad/s\n\n", sigma_g_c, dt_400, sigma_g_400);

    cout << "  비교:" << endl;
    cout << "    200Hz: σ_a = " << sigma_a_200 << " m/s²" << endl;
    cout << "    400Hz: σ_a = " << sigma_a_400 << " m/s²" << endl;
    cout << "    → 주파수 ↑ → 이산 노이즈 ↑ (√2배)\n" << endl;

    cout << "  (3) 적분 후 최종 오차는?\n" << endl;
    double T = 5.0;
    double sigma_p_200 = sigma_a_c * sqrt(dt_200) * pow(T, 1.5) / sqrt(3.0);
    double sigma_p_400 = sigma_a_c * sqrt(dt_400) * pow(T, 1.5) / sqrt(3.0);

    cout << "    5초 적분 후 위치 오차:" << endl;
    cout << "    200Hz: σ_p = " << sigma_p_200 << " m" << endl;
    cout << "    400Hz: σ_p = " << sigma_p_400 << " m" << endl;
    cout << "    → 주파수 ↑ → 적분 후 오차 ↓ (√2배 감소)\n" << endl;

    cout << "  핵심 교훈:" << endl;
    cout << "    → 이산 노이즈가 커져도 적분 후 오차는 줄어듦" << endl;
    cout << "    → 높은 IMU 주파수가 도움되는 이유!" << endl;
    cout << "    → σ_p ∝ √dt → dt가 절반이면 오차 1/√2" << endl;
}

int main()
{
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 2 Quiz Medium - 풀이" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    return 0;
}
