/**
 * Quiz Solutions - Medium
 * Week 12: Monocular 스케일 모호성 (Eigen 활용)
 */

#include <iostream>
#include <cmath>
#include <random>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

/**
 * Q1 풀이: λ 스케일링 증명
 */
void solution1_lambda_scaling() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Q1 풀이: λ 스케일링 증명" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    // 카메라 내부 파라미터
    Matrix3d K;
    K << 500, 0, 320,
         0, 500, 240,
         0,   0,   1;

    // 카메라 포즈 (단위 회전, 이동 없음)
    Matrix3d R = Matrix3d::Identity();
    Vector3d t(0, 0, 0);

    // 3D 점
    Vector3d X(1.0, 2.0, 5.0);

    cout << "카메라 K:\n" << K << "\n" << endl;
    cout << "3D 점 X = " << X.transpose() << "\n" << endl;

    // 원본 투영
    Vector3d x_proj = K * (R * X + t);
    double u1 = x_proj(0) / x_proj(2);
    double v1 = x_proj(1) / x_proj(2);
    cout << "원본 X   → 투영: (" << u1 << ", " << v1 << ")" << endl;

    // 다양한 λ로 스케일
    for (double lambda : {2.0, 5.0, 10.0, 0.1}) {
        Vector3d X_scaled = lambda * X;
        // 핵심: 이동도 λ배 스케일 (t=0이므로 여기서는 상관없음)
        // 일반적인 경우: t_scaled = lambda * t
        Vector3d x_proj_scaled = K * (R * X_scaled + lambda * t);
        double u2 = x_proj_scaled(0) / x_proj_scaled(2);
        double v2 = x_proj_scaled(1) / x_proj_scaled(2);

        cout << "λ=" << lambda << " → X_s = " << X_scaled.transpose()
             << " → 투영: (" << u2 << ", " << v2 << ")";

        if (abs(u1 - u2) < 1e-10 && abs(v1 - v2) < 1e-10) {
            cout << "  [동일!]";
        }
        cout << endl;
    }

    cout << "\n결론: 어떤 λ를 곱해도 투영 결과는 동일!" << endl;
    cout << "  → 단안 카메라는 X와 λX를 구분할 수 없음" << endl;
    cout << "  → 이것이 스케일 모호성의 근본 원인\n" << endl;
}

/**
 * Q2 풀이: 스케일 드리프트 누적
 */
void solution2_drift_accumulation() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Q2 풀이: 스케일 드리프트 누적" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    const int n_frames = 100;
    Vector3d true_delta(1.0, 0.0, 0.0);  // 매 프레임 1m 전진

    cout << "설정: " << n_frames << "프레임, 매 프레임 1m 전진" << endl;
    cout << "GT 최종 위치: " << n_frames * 1.0 << "m\n" << endl;

    cout << "노이즈(%) | 추정 위치(m) | 절대 오차(m) | 상대 오차(%)" << endl;
    cout << "----------|-------------|-------------|------------" << endl;

    for (double noise_std : {0.001, 0.005, 0.01, 0.02, 0.05}) {
        default_random_engine gen(42);
        normal_distribution<double> noise(1.0, noise_std);

        Vector3d gt_position = Vector3d::Zero();
        Vector3d est_position = Vector3d::Zero();

        for (int i = 0; i < n_frames; i++) {
            gt_position += true_delta;

            double scale = noise(gen);
            est_position += scale * true_delta;
        }

        double error = (gt_position - est_position).norm();
        double relative_error = error / gt_position.norm() * 100.0;

        printf("  %5.1f   |   %7.2f   |    %6.3f   |    %5.2f\n",
               noise_std * 100, est_position.x(), error, relative_error);
    }

    cout << "\n분석:" << endl;
    cout << "  - 1% 노이즈: ~1m 오차 (100m 중)" << endl;
    cout << "  - 5% 노이즈: ~5m 오차 (100m 중)" << endl;
    cout << "  - 노이즈가 sqrt(n)에 비례하여 누적 (랜덤 워크)" << endl;
    cout << "  - 바이어스가 있으면 n에 비례하여 누적 (더 심각!)\n" << endl;

    // 바이어스가 있는 경우 추가 분석
    cout << "참고: 바이어스가 있는 경우 (평균 = 0.99)" << endl;
    {
        default_random_engine gen(42);
        normal_distribution<double> noise(0.99, 0.01);  // 평균 0.99 (1% 바이어스)

        Vector3d gt_position = Vector3d::Zero();
        Vector3d est_position = Vector3d::Zero();

        for (int i = 0; i < n_frames; i++) {
            gt_position += true_delta;
            double scale = noise(gen);
            est_position += scale * true_delta;
        }

        double error = (gt_position - est_position).norm();
        printf("  바이어스 1%%: 추정 %.2fm, 오차 %.3fm\n",
               est_position.x(), error);
        cout << "  → 바이어스가 있으면 오차가 n에 비례하여 훨씬 심각!\n" << endl;
    }
}

/**
 * Q3 풀이: Sim(3) vs SE(3) 비교
 */
void solution3_sim3_vs_se3() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Q3 풀이: Sim(3) vs SE(3) 비교" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    // 회전: Z축 30도
    double angle = M_PI / 6.0;
    Matrix3d R;
    R << cos(angle), -sin(angle), 0,
         sin(angle),  cos(angle), 0,
         0,           0,          1;

    Vector3d t(1.0, 0.5, 0.0);
    Vector3d X(2.0, 3.0, 4.0);

    cout << "회전: Z축 30도" << endl;
    cout << "이동: " << t.transpose() << endl;
    cout << "3D 점: " << X.transpose() << "\n" << endl;

    // SE(3) 변환
    Vector3d X_se3 = R * X + t;
    cout << "SE(3) 변환 결과: " << X_se3.transpose() << "\n" << endl;

    // Sim(3) 변환 (다양한 스케일)
    cout << "Sim(3) 변환 결과:" << endl;
    cout << "  s   | 변환 결과             | SE(3)과의 차이" << endl;
    cout << "------|----------------------|------------------" << endl;

    for (double s : {0.5, 1.0, 2.0, 3.0}) {
        Vector3d X_sim3 = s * R * X + t;
        Vector3d diff = X_se3 - X_sim3;

        printf("  %.1f | (%6.2f, %6.2f, %6.2f) | (%6.2f, %6.2f, %6.2f)\n",
               s, X_sim3(0), X_sim3(1), X_sim3(2),
               diff(0), diff(1), diff(2));
    }

    cout << "\n분석:" << endl;
    cout << "  1. s=1일 때 SE(3)와 Sim(3)는 완전히 동일" << endl;
    cout << "     → SE(3)는 Sim(3)의 특수 경우 (s=1)" << endl;
    cout << endl;
    cout << "  2. s가 변하면 회전된 좌표가 스케일됨" << endl;
    cout << "     → 이동(t)은 스케일 영향 없음" << endl;
    cout << "     → 3D 구조의 크기만 변화" << endl;
    cout << endl;
    cout << "  3. Loop Closure에서 Sim(3)가 필요한 이유:" << endl;
    cout << "     → 단안 SLAM에서 맵의 스케일이 드리프트" << endl;
    cout << "     → 같은 장소를 재방문했을 때 스케일이 다름" << endl;
    cout << "     → SE(3)로는 정렬 불가 (스케일 자유도 없음)" << endl;
    cout << "     → Sim(3)로 스케일 + 포즈 동시 정렬" << endl;

    // 4x4 행렬 표현
    cout << "\n4x4 행렬 표현:" << endl;

    cout << "\nSE(3):" << endl;
    Matrix4d T_se3 = Matrix4d::Identity();
    T_se3.block<3,3>(0,0) = R;
    T_se3.block<3,1>(0,3) = t;
    cout << T_se3 << endl;

    double s = 2.0;
    cout << "\nSim(3) (s=2):" << endl;
    Matrix4d T_sim3 = Matrix4d::Identity();
    T_sim3.block<3,3>(0,0) = s * R;
    T_sim3.block<3,1>(0,3) = t;
    cout << T_sim3 << endl;

    cout << "\n자유도 비교:" << endl;
    cout << "  SE(3):  6 DoF (회전 3 + 이동 3)" << endl;
    cout << "  Sim(3): 7 DoF (회전 3 + 이동 3 + 스케일 1)" << endl;

    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 12 Quiz Solutions (Medium)" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    solution1_lambda_scaling();
    solution2_drift_accumulation();
    solution3_sim3_vs_se3();

    return 0;
}
