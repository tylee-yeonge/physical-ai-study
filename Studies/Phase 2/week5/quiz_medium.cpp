/**
 * Phase 2 Week 5 - 에피폴라 기하학 중급 퀴즈
 *
 * 다루는 개념:
 *   - 에피폴라 제약 수치 검증 (p2^T · E · p1 ≈ 0)
 *   - Fundamental Matrix 추정 (findFundamentalMat)
 *   - E → R, t 분해 (SVD + W 행렬)
 *   - 8-Point Algorithm 직접 구현 (Hartley 정규화 포함)
 *   - 노이즈가 F 추정에 미치는 영향 분석
 *
 * 선수 지식: week5 quiz_easy (E, F 개념), week1 (투영, K 행렬)
 */

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <cmath>

// 에피폴라 제약 수치 검증 — 실제 대응점으로 p2^T · E · p1 ≈ 0 확인
//
// 에피폴라 제약: p2^T · E · p1 = 0 (이론적으로 정확히 0)
// 실제로는 노이즈 때문에 정확히 0이 아니라 작은 값.
// 이 값의 크기가 곧 "이 매칭이 얼마나 정확한가"의 척도.
//
// SLAM에서 RANSAC을 수행할 때, 이 오차가 임계값보다 크면 outlier로 판정.
void problem1_implement_constraint()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 제약 검증 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // TODO: Essential Matrix 주어짐
    cv::Mat E = cv::Mat::eye(3, 3, CV_64F);
    cv::Point2f p1(0.5, 0.3);
    cv::Point2f p2(0.6, 0.35);

    // p2^T · E · p1 = 1×3 · 3×3 · 3×1 = 스칼라
    //   p1, p2를 동차 좌표 [x, y, 1]^T 벡터로 변환하여 행렬곱 수행
    cv::Mat pt1 = (cv::Mat_<double>(3, 1) << p1.x, p1.y, 1.0);
    cv::Mat pt2 = (cv::Mat_<double>(3, 1) << p2.x, p2.y, 1.0);

    // pt2.t(): 1×3, E: 3×3, pt1: 3×1 → 결과: 1×1 스칼라
    cv::Mat result = pt2.t() * E * pt1;
    double error = std::abs(result.at<double>(0));

    std::cout << "에피폴라 제약 오차: " << error << std::endl;
    std::cout << "(0에 가까울수록 정확한 대응)" << std::endl;
}

// Fundamental Matrix 추정 — 대응점에서 F를 구하는 실전
//
// findFundamentalMat: OpenCV가 제공하는 F 추정 함수
//   - FM_8POINT: 8-Point Algorithm (모든 점 사용, outlier에 약함)
//   - FM_RANSAC: RANSAC 기반 (outlier에 강함, 실전에서 사용)
//
// F를 구하면 에피폴라 선을 그릴 수 있다:
//   l2 = F · p1  → 이미지 2의 에피폴라 선 (ax + by + c = 0)
//   l1 = F^T · p2 → 이미지 1의 에피폴라 선
void problem2_estimate_F()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: F Matrix 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 최소 8개 대응점 (8-Point Algorithm 사용)
    std::vector<cv::Point2f> pts1, pts2;
    for (int i = 0; i < 10; i++)
    {
        pts1.push_back(cv::Point2f(100 + i * 30, 150 + i * 20));
        pts2.push_back(cv::Point2f(110 + i * 32, 160 + i * 21));
    }

    // FM_8POINT: SVD 기반, RANSAC 없이 모든 점 사용
    cv::Mat F = cv::findFundamentalMat(pts1, pts2, cv::FM_8POINT);

    std::cout << "Fundamental Matrix:\n" << F << std::endl;
    std::cout << "\n💡 이제 에피폴라 선을 그릴 수 있습니다!" << std::endl;
}

// E → R, t 복원 — SVD 분해로 포즈 추출하는 수학적 과정
//
// E = U · diag(1, 1, 0) · V^T 에서:
//
//   W = [0 -1  0]      (90도 회전 행렬)
//       [1  0  0]
//       [0  0  1]
//
//   R = U · W · V^T    (또는 U · W^T · V^T)
//   t = U의 마지막 열   (또는 -U의 마지막 열)
//
// → 4가지 (R, t) 조합 중 Cheirality Check로 1개 선택
//
// OpenCV에서는 cv::recoverPose()가 이 전 과정을 자동 수행:
//   E 분해 → 4해 생성 → 삼각측량 → Z > 0 검증 → 최적 해 반환
void problem3_e_to_pose()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: E → R, t 복원" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "E를 SVD 분해하여 R, t 추출:\n" << std::endl;
    std::cout << "E = U * diag(1, 1, 0) * V^T" << std::endl;
    std::cout << "R = U * W * V^T  (또는 W^T)" << std::endl;
    std::cout << "t = U의 마지막 열\n" << std::endl;

    std::cout << "W = [0 -1  0]" << std::endl;
    std::cout << "    [1  0  0]" << std::endl;
    std::cout << "    [0  0  1]" << std::endl;

    std::cout << "\n💡 OpenCV: cv::recoverPose()가 자동 수행" << std::endl;
}

/**
 * @brief 8-Point Algorithm 직접 구현 (Hartley 정규화 포함)
 *
 * 대응점에서 A 행렬(N×9) 구성 → SVD → rank-2 강제로 F를 추정한다.
 *
 * 알고리즘 단계:
 *   1. Hartley 정규화: 점 중심=0, 평균 거리=sqrt(2)
 *      - 수치 안정성 확보 (픽셀 좌표는 수백 단위라 행렬 조건수가 나쁨)
 *      - T = [s 0 -s*cx; 0 s -s*cy; 0 0 1]  (s = sqrt(2)/mean_dist)
 *   2. A 행렬 구성: 각 대응점 (u1',v1') ↔ (u2',v2')에서
 *      A[i] = [u2'·u1', u2'·v1', u2', v2'·u1', v2'·v1', v2', u1', v1', 1]
 *   3. SVD(A): V의 마지막 열 → f 벡터 → 3×3 reshape → F̃
 *   4. Rank-2 강제: SVD(F̃)에서 가장 작은 특이값을 0으로 설정
 *   5. 역정규화: F = T2^T · F_norm · T1
 *
 * OpenCV findFundamentalMat 결과와 비교하여 구현을 검증한다.
 */
void problem4_eight_point_algorithm()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 8-Point 직접 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 내부 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    // 3D 점 생성 (카메라 앞)
    std::vector<cv::Point3d> pts_3d;
    cv::RNG rng(42);
    for (int i = 0; i < 20; i++)
    {
        pts_3d.push_back(cv::Point3d(
            rng.uniform(-1.0, 1.0),
            rng.uniform(-1.0, 1.0),
            rng.uniform(3.0, 7.0)));
    }

    // 카메라 2: 5도 Y축 회전 + 오른쪽 이동
    double angle = 5.0 * CV_PI / 180.0;
    cv::Mat R2 = (cv::Mat_<double>(3, 3) <<
        cos(angle), 0, sin(angle),
        0, 1, 0,
        -sin(angle), 0, cos(angle));
    cv::Mat t2 = (cv::Mat_<double>(3, 1) << 0.1, 0.0, 0.0);

    // 두 카메라에서 투영 (p = K · [R|t] · P)
    std::vector<cv::Point2f> pts1, pts2;
    for (const auto& P : pts_3d)
    {
        // 카메라 1 (원점): P_cam = P, p = K · P_cam
        cv::Mat p1_3d = (cv::Mat_<double>(3, 1) << P.x, P.y, P.z);
        cv::Mat proj1 = K * p1_3d;
        pts1.push_back(cv::Point2f(
            proj1.at<double>(0) / proj1.at<double>(2),
            proj1.at<double>(1) / proj1.at<double>(2)));

        // 카메라 2: P_cam2 = R2 · P + t2, p = K · P_cam2
        cv::Mat p2_3d = R2 * p1_3d + t2;
        cv::Mat proj2 = K * p2_3d;
        pts2.push_back(cv::Point2f(
            proj2.at<double>(0) / proj2.at<double>(2),
            proj2.at<double>(1) / proj2.at<double>(2)));
    }

    int n = static_cast<int>(pts1.size());

    // TODO 1: Hartley 정규화
    // 각 점 집합의 중심을 (0,0)으로, 평균 거리를 sqrt(2)로 정규화
    //   (1) 중심 계산: cx = mean(u), cy = mean(v)
    //   (2) 평균 거리 계산: mean(sqrt((u-cx)^2 + (v-cy)^2))
    //   (3) 스케일: s = sqrt(2) / mean_dist
    //   (4) T = [s 0 -s*cx; 0 s -s*cy; 0 0 1]
    //   (5) 정규화된 좌표: p_norm = T * [u, v, 1]^T

    // TODO 2: A 행렬 (N×9) 구성
    // 정규화된 좌표 (u1',v1') ↔ (u2',v2') 사용
    // A[i] = [u2'*u1', u2'*v1', u2', v2'*u1', v2'*v1', v2', u1', v1', 1]
    //
    // 유도: p2'^T · F · p1' = 0 을 전개하면 위 형태가 됨
    //   [u2' v2' 1] · F · [u1' v1' 1]^T = 0
    //   → f 벡터(F의 9개 원소)에 대한 선형 방정식

    // TODO 3: SVD로 f 벡터 → F_norm (3×3) 재구성
    // A = U·S·V^T, V의 마지막 열(가장 작은 특이값) = f
    // f를 3×3으로 reshape

    // TODO 4: Rank-2 강제 (F의 필수 성질)
    // F̃ = U·diag(s1, s2, s3)·V^T 에서 s3=0으로 설정
    // ★ rank(F) = 2여야 에피폴라 선들이 한 점(에피폴)에서 교차

    // TODO 5: 역정규화: F = T2^T · F_norm · T1
    // 정규화를 풀어서 원래 픽셀 좌표계의 F로 복원

    // OpenCV 결과와 비교
    cv::Mat F_opencv = cv::findFundamentalMat(pts1, pts2, cv::FM_8POINT);
    std::cout << "OpenCV F:\n" << F_opencv << "\n" << std::endl;

    std::cout << "TODO: 직접 구현한 F와 비교하세요" << std::endl;
    std::cout << "   차이가 작으면 (< 1e-6) 성공!" << std::endl;
}

/**
 * @brief 노이즈/아웃라이어가 F 추정에 미치는 영향 분석
 *
 * 노이즈 수준을 0~5px로 변화시키며 F 추정 정확도를 측정한다.
 * 에피폴라 제약 |p2^T · F · p1| 의 평균 오차로 정량화한다.
 *
 * 관찰 포인트:
 *   - 노이즈 0px: 제약 오차 ≈ 0 (이론적으로 완벽)
 *   - 노이즈 1~2px: 오차 증가하지만 F는 여전히 의미 있음
 *   - 노이즈 5px: 오차 크게 증가 → RANSAC 필수
 *
 * SLAM에서 특징점 검출 오차가 보통 0.5~2px이므로,
 * 이 범위에서 F가 얼마나 안정적인지 이해하는 것이 중요하다.
 */
void problem5_noise_analysis()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: 노이즈 영향 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 내부 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    // 3D 점 생성
    std::vector<cv::Point3d> pts_3d;
    cv::RNG rng(42);
    for (int i = 0; i < 30; i++)
    {
        pts_3d.push_back(cv::Point3d(
            rng.uniform(-1.0, 1.0),
            rng.uniform(-1.0, 1.0),
            rng.uniform(3.0, 7.0)));
    }

    // 카메라 2
    double angle = 5.0 * CV_PI / 180.0;
    cv::Mat R2 = (cv::Mat_<double>(3, 3) <<
        cos(angle), 0, sin(angle),
        0, 1, 0,
        -sin(angle), 0, cos(angle));
    cv::Mat t2 = (cv::Mat_<double>(3, 1) << 0.1, 0.0, 0.0);

    // 깨끗한 대응점 생성
    std::vector<cv::Point2f> pts1_clean, pts2_clean;
    for (const auto& P : pts_3d)
    {
        cv::Mat p = (cv::Mat_<double>(3, 1) << P.x, P.y, P.z);
        cv::Mat proj1 = K * p;
        pts1_clean.push_back(cv::Point2f(
            proj1.at<double>(0) / proj1.at<double>(2),
            proj1.at<double>(1) / proj1.at<double>(2)));

        cv::Mat p2 = R2 * p + t2;
        cv::Mat proj2 = K * p2;
        pts2_clean.push_back(cv::Point2f(
            proj2.at<double>(0) / proj2.at<double>(2),
            proj2.at<double>(1) / proj2.at<double>(2)));
    }

    // 노이즈 수준별 F 추정 및 오차 측정
    std::vector<double> noise_levels = {0.0, 0.5, 1.0, 2.0, 5.0};

    std::cout << "노이즈 수준에 따른 에피폴라 제약 오차:" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Noise(px)  |  Mean |p2^T F p1|" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;

    for (double noise : noise_levels)
    {
        // TODO 1: pts1_clean, pts2_clean에 가우시안 노이즈 추가
        //   각 점의 (x, y)에 N(0, noise) 노이즈를 더함
        //   cv::RNG::gaussian(sigma)로 랜덤 값 생성

        // TODO 2: 노이즈 추가된 점으로 F 추정
        //   cv::findFundamentalMat(pts1_noisy, pts2_noisy, cv::FM_8POINT)

        // TODO 3: 깨끗한 원본 점으로 에피폴라 제약 오차 계산
        //   각 대응점에 대해: error_i = |p2^T · F · p1|
        //   평균 오차 = sum(error_i) / N
        //
        //   ★ 노이즈로 추정한 F를 "깨끗한" 점에 적용하여 측정하는 이유:
        //     노이즈 점으로 측정하면 overfitting 효과로 오차가 작아 보임

        std::cout << "   " << noise << " px  →  ??? (TODO)" << std::endl;
    }

    std::cout << "\n관찰:" << std::endl;
    std::cout << "   - 노이즈 증가 → 에피폴라 제약 오차 증가" << std::endl;
    std::cout << "   - 실제로는 RANSAC으로 아웃라이어 제거 필요" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 5 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_implement_constraint();
    problem2_estimate_F();
    problem3_e_to_pose();
    problem4_eight_point_algorithm();
    problem5_noise_analysis();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
