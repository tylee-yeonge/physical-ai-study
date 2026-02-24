/**
 * Quiz Medium - Week 12: Monocular 스케일 모호성
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. λ 스케일링 증명 — 투영 방정식에서 스케일이 소거되는 것을 코드로 확인
 *   2. 스케일 드리프트 누적 — 노이즈가 있는 스케일 추정의 누적 오차 시뮬레이션
 *   3. Sim(3) vs SE(3) — 스케일 자유도를 포함한 변환군의 차이
 *
 * 투영에서 스케일이 소거되는 원리:
 *
 *   x_proj = K · (R · X + t)  (3D → 동차 좌표)
 *   u = x_proj(0) / x_proj(2)  (동차 → 픽셀)
 *
 *   X → λX, t → λt로 바꾸면:
 *   K · (R · λX + λt) = λ · K · (R · X + t)
 *   → 동차 좌표의 λ배 → 나눗셈에서 소거!
 *
 * Sim(3) — 스케일을 포함한 유사 변환:
 *
 *   SE(3): [R  t]  (6 DoF: 회전 3 + 이동 3)
 *          [0  1]
 *
 *   Sim(3): [sR  t]  (7 DoF: 회전 3 + 이동 3 + 스케일 1)
 *           [0   1]
 *
 *   ★ ORB-SLAM2 Loop Closure에서 Sim(3) 정합 사용
 *     → 루프 양쪽의 스케일 차이를 보정
 *
 * 난이도: ★★☆ (수치 계산, Eigen 활용)
 * 선수 지식: quiz_easy (스케일 모호성 기본), Week 2 (Essential Matrix)
 */

#include <iostream>
#include <cmath>
#include <random>
#include <Eigen/Dense>

using namespace Eigen;

/**
 * 문제 1: λ 스케일링 증명
 *
 * 투영 방정식: x = K · (R · X + t)
 *   동차 좌표 [x, y, z]^T → 픽셀 좌표 (x/z, y/z)
 *
 * X → λX (동시에 t → λt) 치환:
 *   K · (R · λX + λt) = λ · K · (R · X + t) = λ · [x, y, z]^T
 *   → 픽셀 좌표: (λx/λz, λy/λz) = (x/z, y/z) — 동일!
 *
 * ★ 이것이 단안 카메라에서 절대 크기를 알 수 없는 수학적 근거
 *   코드로 λ = 2, 5, 10에 대해 실제로 같은 픽셀에 투영되는지 확인
 *
 * TODO:
 *   1. 카메라 내부 파라미터 K, 포즈 [R|t], 3D 점 X 정의
 *   2. X를 투영하여 픽셀 좌표 (u1, v1) 계산
 *   3. λ=2, λ=5, λ=10 으로 스케일한 점 λX를 투영
 *   4. (u1, v1)과 동일한지 확인
 */
void problem1_lambda_scaling()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Q1. λ 스케일링 증명" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "3D 점 X와 λX가 같은 픽셀에 투영됨을 확인하세요.\n" << std::endl;

    // 카메라 내부 파라미터
    Matrix3d K;
    K << 500, 0, 320, 0, 500, 240, 0, 0, 1;

    // 카메라 포즈 (단위 회전, 이동 없음)
    Matrix3d R = Matrix3d::Identity();
    Vector3d t(0, 0, 0);

    // 3D 점
    Vector3d X(1.0, 2.0, 5.0);

    // ─── TODO 시작 ───

    // 1. X를 투영하여 (u1, v1) 계산
    //    x_proj = K * (R * X + t) 후 동차 좌표 → 픽셀 좌표 변환
    //    u = x_proj(0) / x_proj(2), v = x_proj(1) / x_proj(2)

    // 2. λ=2로 스케일한 점 λX를 투영 (이동도 λ배: t_scaled = λ*t)
    //    투영 결과 (u2, v2)를 계산하세요

    // 3. 결과 비교: (u1,v1)과 (u2,v2)가 동일한지 출력

    // ─── TODO 끝 ───

    std::cout << "TODO: 위 코드의 주석을 해제하고 완성하세요.\n" << std::endl;
    std::cout << "기대 결과: 모든 λ에 대해 동일한 (u, v) 출력\n" << std::endl;
}

/**
 * 문제 2: 스케일 드리프트 누적 시뮬레이션
 *
 * Monocular VO에서 매 프레임 스케일 추정:
 *   실제: ||t_k|| = 1.0m (매 프레임 1m 이동)
 *   추정: ||t_k|| = s_k = 1.0 + ε_k (ε_k ~ N(0, σ²))
 *
 * n프레임 후 추정 위치:
 *   p_est = Σ s_k · Δt = Σ (1 + ε_k) · Δt
 *   오차 = |p_est - p_gt| = |Σ ε_k| · ||Δt||
 *
 * 기대 오차 (Random Walk):
 *   E[|Σ ε_k|] ≈ σ · √n
 *   σ=0.01, n=100 → 기대 오차 ≈ 0.1m (1% 노이즈)
 *   σ=0.05, n=100 → 기대 오차 ≈ 0.5m (5% 노이즈)
 *
 * ★ 실제 VO에서는 회전 오차도 누적되어 더 심각
 *   → 이 시뮬레이션은 이동 스케일만의 최소한의 오차
 *
 * TODO:
 *   1. 실제 이동: 매 프레임 [1, 0, 0] (1m 전진)
 *   2. 추정 이동: scale * [1, 0, 0] (scale = 1 + noise)
 *   3. 100프레임 누적 후 위치 비교
 *   4. noise_std = 0.01 (1%), 0.02 (2%), 0.05 (5%) 각각 시도
 */
void problem2_drift_accumulation()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Q2. 스케일 드리프트 누적" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "직선 궤적 (100프레임, 매 프레임 1m 전진)에서\n";
    std::cout << "스케일 노이즈에 따른 최종 위치 오차를 계산하세요.\n" << std::endl;

    const int n_frames = 100;
    Vector3d true_delta(1.0, 0.0, 0.0);  // 매 프레임 1m 전진

    // ─── TODO 시작 ───

    // 각 noise_std(0.01, 0.02, 0.05)에 대해:
    //   1. normal_distribution(평균=1.0, 표준편차=noise_std)으로 스케일 생성
    //   2. n_frames만큼 반복하며:
    //      - gt_position에 true_delta 누적
    //      - est_position에 scale * true_delta 누적 (scale = 노이즈 샘플)
    //   3. 최종 오차 = (gt_position - est_position).norm()
    //   4. 각 노이즈 수준별 GT 위치, 추정 위치, 오차를 출력

    // ─── TODO 끝 ───

    std::cout << "TODO: 위 코드의 주석을 해제하고 완성하세요.\n" << std::endl;
    std::cout << "기대 결과: 노이즈가 커질수록 오차도 커짐\n" << std::endl;
}

/**
 * 문제 3: Sim(3) vs SE(3) 비교
 *
 * SE(3) (Special Euclidean):
 *   T · X = R · X + t     (6 DoF)
 *   거리와 각도를 보존 → 강체 변환
 *
 * Sim(3) (Similarity):
 *   S · X = s · R · X + t  (7 DoF)
 *   각도만 보존, 거리는 s배 → 유사 변환
 *
 * Monocular SLAM에서 Sim(3)가 필요한 이유:
 *   Loop Closure 시 루프 양 끝의 맵 스케일이 다를 수 있음
 *   SE(3)로는 스케일 차이를 보정할 수 없음
 *   Sim(3)의 s 파라미터로 스케일 차이를 흡수
 *
 * ★ ORB-SLAM2:
 *   Monocular → Sim(3) Loop Closure
 *   Stereo/RGB-D → SE(3) Loop Closure (스케일이 이미 결정됨)
 *
 * TODO:
 *   1. SE(3) 변환 행렬 구성: [R t; 0 1] (4x4)
 *   2. Sim(3) 변환 행렬 구성: [sR t; 0 1] (4x4)
 *   3. 같은 3D 점에 두 변환을 적용
 *   4. 결과 비교 (스케일 s의 효과 관찰)
 */
void problem3_sim3_vs_se3()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Q3. Sim(3) vs SE(3) 변환 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "SE(3)와 Sim(3)의 차이를 확인하세요.\n" << std::endl;
    std::cout << "SE(3):  T = [R  t]    (6 자유도)\n";
    std::cout << "            [0  1]\n" << std::endl;
    std::cout << "Sim(3): S = [sR t]    (7 자유도)\n";
    std::cout << "            [0  1]\n" << std::endl;

    // 회전: Z축 30도
    double angle = M_PI / 6.0;
    Matrix3d R;
    R << cos(angle), -sin(angle), 0, sin(angle), cos(angle), 0, 0, 0, 1;

    Vector3d t(1.0, 0.5, 0.0);

    // 3D 점
    Vector3d X(2.0, 3.0, 4.0);

    // ─── TODO 시작 ───

    // 1. SE(3) 변환 적용: X_se3 = R * X + t

    // 2. Sim(3) 변환 적용 (s = 0.5, 1.0, 2.0 각각)
    //    X_sim3 = s * R * X + t
    //    SE(3) 결과와 Sim(3) 결과, 차이를 출력하세요

    // 3. 관찰 사항 기술:
    // - s=1일 때 SE(3)와 Sim(3)는 동일한가?
    // - s가 변하면 어떤 효과가 있는가?
    // - Loop Closure에서 왜 Sim(3)가 필요한가?

    // ─── TODO 끝 ───

    std::cout << "TODO: 위 코드의 주석을 해제하고 완성하세요.\n" << std::endl;
    std::cout << "기대 결과: s=1일 때 SE(3)=Sim(3), s!=1일 때 스케일 차이 관찰\n" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 12 Quiz - Medium" << std::endl;
    std::cout << "Monocular 스케일 모호성 (Eigen 활용)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_lambda_scaling();
    problem2_drift_accumulation();
    problem3_sim3_vs_se3();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
