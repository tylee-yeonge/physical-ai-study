/**
 * Quiz Medium - Week 12: Monocular 스케일 모호성
 *
 * 3개의 중급 문제 (Eigen 활용)
 * 스케일 모호성과 드리프트를 코드로 직접 확인합니다.
 */

#include <iostream>
#include <cmath>
#include <random>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

/**
 * 문제 1: λ 스케일링 증명
 *
 * 3D 점 X와 λX가 같은 2D 픽셀에 투영됨을 확인하세요.
 *
 * TODO:
 *   1. 카메라 내부 파라미터 K, 포즈 [R|t], 3D 점 X 정의
 *   2. X를 투영하여 픽셀 좌표 (u1, v1) 계산
 *   3. λ=2, λ=5, λ=10 으로 스케일한 점 λX를 투영
 *   4. (u1, v1)과 동일한지 확인
 */
void problem1_lambda_scaling() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Q1. λ 스케일링 증명" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "3D 점 X와 λX가 같은 픽셀에 투영됨을 확인하세요.\n" << endl;

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

    // ─── TODO 시작 ───

    // 1. X를 투영하여 (u1, v1) 계산
    // Vector3d x_proj = K * (R * X + t);
    // double u1 = ???;
    // double v1 = ???;

    // 2. λ=2로 스케일한 점 투영
    // double lambda = 2.0;
    // Vector3d X_scaled = lambda * X;
    // 이동도 λ배: t_scaled = lambda * t
    // Vector3d x_proj2 = K * (R * X_scaled + lambda * t);
    // double u2 = ???;
    // double v2 = ???;

    // 3. 결과 비교 출력
    // cout << "원본: (" << u1 << ", " << v1 << ")" << endl;
    // cout << "λ=2:  (" << u2 << ", " << v2 << ")" << endl;

    // ─── TODO 끝 ───

    cout << "TODO: 위 코드의 주석을 해제하고 완성하세요.\n" << endl;
    cout << "기대 결과: 모든 λ에 대해 동일한 (u, v) 출력\n" << endl;
}

/**
 * 문제 2: 스케일 드리프트 누적 시뮬레이션
 *
 * 직선 궤적에서 매 프레임 스케일 노이즈를 추가하고
 * 100프레임 후 총 드리프트를 계산하세요.
 *
 * TODO:
 *   1. 실제 이동: 매 프레임 [1, 0, 0] (1m 전진)
 *   2. 추정 이동: scale * [1, 0, 0] (scale = 1 + noise)
 *   3. 100프레임 누적 후 위치 비교
 *   4. noise_std = 0.01 (1%), 0.02 (2%), 0.05 (5%) 각각 시도
 */
void problem2_drift_accumulation() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Q2. 스케일 드리프트 누적" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "직선 궤적 (100프레임, 매 프레임 1m 전진)에서\n";
    cout << "스케일 노이즈에 따른 최종 위치 오차를 계산하세요.\n" << endl;

    const int n_frames = 100;
    Vector3d true_delta(1.0, 0.0, 0.0);  // 매 프레임 1m 전진

    // ─── TODO 시작 ───

    // for (double noise_std : {0.01, 0.02, 0.05}) {
    //     default_random_engine gen(42);
    //     normal_distribution<double> noise(1.0, noise_std);
    //
    //     Vector3d gt_position = Vector3d::Zero();
    //     Vector3d est_position = Vector3d::Zero();
    //
    //     for (int i = 0; i < n_frames; i++) {
    //         gt_position += true_delta;
    //
    //         double scale = noise(gen);
    //         est_position += ??? * true_delta;  // 스케일 적용
    //     }
    //
    //     double error = (gt_position - est_position).norm();
    //     cout << "노이즈 " << noise_std * 100 << "%: ";
    //     cout << "GT = " << gt_position.x() << "m, ";
    //     cout << "추정 = " << est_position.x() << "m, ";
    //     cout << "오차 = " << error << "m" << endl;
    // }

    // ─── TODO 끝 ───

    cout << "TODO: 위 코드의 주석을 해제하고 완성하세요.\n" << endl;
    cout << "기대 결과: 노이즈가 커질수록 오차도 커짐\n" << endl;
}

/**
 * 문제 3: Sim(3) vs SE(3) 비교
 *
 * SE(3) 변환과 Sim(3) 변환의 차이를 코드로 확인하세요.
 *
 * TODO:
 *   1. SE(3) 변환 행렬 구성: [R t; 0 1] (4x4)
 *   2. Sim(3) 변환 행렬 구성: [sR t; 0 1] (4x4)
 *   3. 같은 3D 점에 두 변환을 적용
 *   4. 결과 비교 (스케일 s의 효과 관찰)
 */
void problem3_sim3_vs_se3() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Q3. Sim(3) vs SE(3) 변환 비교" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "SE(3)와 Sim(3)의 차이를 확인하세요.\n" << endl;
    cout << "SE(3):  T = [R  t]    (6 자유도)\n";
    cout << "            [0  1]\n" << endl;
    cout << "Sim(3): S = [sR t]    (7 자유도)\n";
    cout << "            [0  1]\n" << endl;

    // 회전: Z축 30도
    double angle = M_PI / 6.0;
    Matrix3d R;
    R << cos(angle), -sin(angle), 0,
         sin(angle),  cos(angle), 0,
         0,           0,          1;

    Vector3d t(1.0, 0.5, 0.0);

    // 3D 점
    Vector3d X(2.0, 3.0, 4.0);

    // ─── TODO 시작 ───

    // 1. SE(3) 변환 적용
    // Vector3d X_se3 = R * X + t;

    // 2. Sim(3) 변환 적용 (s = 0.5, 1.0, 2.0)
    // for (double s : {0.5, 1.0, 2.0}) {
    //     Vector3d X_sim3 = s * R * X + t;
    //
    //     cout << "s = " << s << endl;
    //     cout << "  SE(3):  " << X_se3.transpose() << endl;
    //     cout << "  Sim(3): " << X_sim3.transpose() << endl;
    //     cout << "  차이:   " << (X_se3 - X_sim3).transpose() << endl;
    //     cout << endl;
    // }

    // 3. 관찰 사항 기술:
    // - s=1일 때 SE(3)와 Sim(3)는 동일한가?
    // - s가 변하면 어떤 효과가 있는가?
    // - Loop Closure에서 왜 Sim(3)가 필요한가?

    // ─── TODO 끝 ───

    cout << "TODO: 위 코드의 주석을 해제하고 완성하세요.\n" << endl;
    cout << "기대 결과: s=1일 때 SE(3)=Sim(3), s!=1일 때 스케일 차이 관찰\n" << endl;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 12 Quiz - Medium" << endl;
    cout << "Monocular 스케일 모호성 (Eigen 활용)" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    problem1_lambda_scaling();
    problem2_drift_accumulation();
    problem3_sim3_vs_se3();

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
