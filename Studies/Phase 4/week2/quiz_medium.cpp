/**
 * Phase 4 Week 2 - IMU 노이즈 모델 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <random>
#include <iomanip>

using namespace Eigen;
using namespace std;

void problem1_noise_propagation() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 1: 노이즈 전파 시뮬레이션" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "1D 가속도를 적분하여 위치를 구합니다.\n" << endl;
    cout << "  실제 가속도: a_true = 0 (정지 상태)" << endl;
    cout << "  측정 노이즈: σ_a = 0.1 m/s² (white noise)" << endl;
    cout << "  dt = 0.01s, T = 5초\n" << endl;

    cout << "과제: 시뮬레이션으로 5초 후 위치 오차를 구하세요.\n" << endl;
    cout << "  이론적 위치 불확실성: σ_p = σ_a · √dt · T^{3/2} / √3\n" << endl;

    double sigma_a = 0.1;
    double dt = 0.01;
    double T = 5.0;

    double sigma_p_theory = sigma_a * sqrt(dt) * pow(T, 1.5) / sqrt(3.0);
    cout << "  이론값 σ_p = " << sigma_a << " × " << sqrt(dt) << " × "
         << pow(T, 1.5) << " / " << sqrt(3.0) << " = _____ m\n" << endl;

    // TODO: 학생이 구현
    // default_random_engine gen(42);
    // normal_distribution<double> noise(0, sigma_a);
    // double v = 0, p = 0;
    // for (int i = 0; i < (int)(T/dt); i++) {
    //     double a = noise(gen);
    //     v += a * dt;
    //     p += v * dt;
    // }

    cout << "  시뮬레이션 결과: p = _____ m" << endl;
    cout << "  이론값:          σ_p = _____ m\n" << endl;
    cout << "  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

void problem2_bias_vs_noise_growth() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 2: 바이어스 vs 노이즈 성장률" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "White Noise와 Bias가 각각 적분될 때의 성장률:\n" << endl;
    cout << "  가속도계:" << endl;
    cout << "    White Noise σ_a → 속도: σ_a·√(dt·N), 위치: ∝ t^{3/2}" << endl;
    cout << "    Bias b_a → 속도: b_a·t, 위치: 0.5·b_a·t²\n" << endl;

    cout << "  자이로:" << endl;
    cout << "    White Noise σ_g → 각도: σ_g·√(dt·N) ∝ √t" << endl;
    cout << "    Bias b_g → 각도: b_g·t\n" << endl;

    cout << "과제: 아래 파라미터에서 각 시간의 오차를 계산하세요.\n" << endl;
    cout << "  σ_a = 0.08 m/s²/√Hz, b_a = 0.02 m/s²" << endl;
    cout << "  dt = 0.005s (200Hz IMU)\n" << endl;

    double sigma_a = 0.08;
    double b_a = 0.02;
    double dt = 0.005;

    cout << "  시간(s) │ 노이즈 위치 σ_p(m) │ 바이어스 위치(m) │ 어느 쪽이 큰가?" << endl;
    cout << "  ────────┼───────────────────┼────────────────┼────────────────" << endl;

    // TODO: 학생이 구현
    for (double t : {1.0, 5.0, 10.0, 30.0}) {
        // double sigma_p = sigma_a * sqrt(dt) * pow(t, 1.5) / sqrt(3.0);
        // double p_bias = 0.5 * b_a * t * t;
        cout << "    " << setw(4) << fixed << setprecision(0) << t
             << "    │     ___________    │   ___________   │   ___________" << endl;
    }

    cout << "\n  어느 시점에서 바이어스가 노이즈를 역전하는가?" << endl;
    cout << "  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

void problem3_discrete_noise() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 3: 연속-이산 노이즈 변환" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "연속 시간 noise density와 이산 시간 노이즈의 관계:\n" << endl;
    cout << "  σ_discrete = σ_continuous / √dt\n" << endl;
    cout << "데이터시트에 다음 값이 주어졌습니다:\n" << endl;
    cout << "  가속도계 noise density: 0.04 m/s²/√Hz" << endl;
    cout << "  자이로 noise density:   0.005 rad/s/√Hz" << endl;
    cout << "  IMU 주파수: 200Hz (dt = 0.005s)\n" << endl;

    cout << "과제:" << endl;
    cout << "  1. 각 이산 노이즈 표준편차를 구하세요." << endl;
    cout << "     σ_a_discrete = 0.04 / √0.005 = _____ m/s²" << endl;
    cout << "     σ_g_discrete = 0.005 / √0.005 = _____ rad/s\n" << endl;

    cout << "  2. IMU 주파수가 400Hz로 바뀌면?" << endl;
    cout << "     dt = 0.0025s" << endl;
    cout << "     σ_a_discrete = 0.04 / √0.0025 = _____ m/s²" << endl;
    cout << "     σ_g_discrete = 0.005 / √0.0025 = _____ rad/s\n" << endl;

    cout << "  3. 주파수가 높아지면 이산 노이즈는 커지는가 작아지는가?" << endl;
    cout << "     적분 후 최종 오차는 어떻게 되는가?\n" << endl;

    cout << "  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 2 Quiz - Medium (IMU 노이즈 모델)" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    problem1_noise_propagation();
    problem2_bias_vs_noise_growth();
    problem3_discrete_noise();

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
