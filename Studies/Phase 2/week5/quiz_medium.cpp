/**
 * Phase 2 Week 5 - 에피폴라 기하학 중급 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <cmath>

void problem1_implement_constraint()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 제약 검증 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // TODO: Essential Matrix 주어짐
    cv::Mat E = cv::Mat::eye(3, 3, CV_64F);
    cv::Point2f p1(0.5, 0.3);
    cv::Point2f p2(0.6, 0.35);

    // TODO: p2^T * E * p1 계산
    cv::Mat pt1 = (cv::Mat_<double>(3, 1) << p1.x, p1.y, 1.0);
    cv::Mat pt2 = (cv::Mat_<double>(3, 1) << p2.x, p2.y, 1.0);

    cv::Mat result = pt2.t() * E * pt1;
    double error = std::abs(result.at<double>(0));

    std::cout << "에피폴라 제약 오차: " << error << std::endl;
    std::cout << "(0에 가까울수록 정확한 대응)" << std::endl;
}

void problem2_estimate_F()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: F Matrix 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // TODO: 대응점 준비 (최소 8개)
    std::vector<cv::Point2f> pts1, pts2;
    for (int i = 0; i < 10; i++)
    {
        pts1.push_back(cv::Point2f(100 + i * 30, 150 + i * 20));
        pts2.push_back(cv::Point2f(110 + i * 32, 160 + i * 21));
    }

    // TODO: findFundamentalMat 사용
    cv::Mat F = cv::findFundamentalMat(pts1, pts2, cv::FM_8POINT);

    std::cout << "Fundamental Matrix:\n" << F << std::endl;
    std::cout << "\n💡 이제 에피폴라 선을 그릴 수 있습니다!" << std::endl;
}

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
 * @brief 8-point 알고리즘 직접 구현 (Hartley 정규화 포함)
 *
 * 대응점에서 A 행렬(Nx9) 구성 → SVD → rank-2 강제로 F를 추정한다.
 * Hartley 정규화: 점 중심=0, 평균 거리=sqrt(2) 로 수치 안정성 확보.
 * OpenCV findFundamentalMat 결과와 비교한다.
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

    // 두 카메라에서 투영
    std::vector<cv::Point2f> pts1, pts2;
    for (const auto& P : pts_3d)
    {
        // 카메라 1 (원점)
        cv::Mat p1_3d = (cv::Mat_<double>(3, 1) << P.x, P.y, P.z);
        cv::Mat proj1 = K * p1_3d;
        pts1.push_back(cv::Point2f(
            proj1.at<double>(0) / proj1.at<double>(2),
            proj1.at<double>(1) / proj1.at<double>(2)));

        // 카메라 2
        cv::Mat p2_3d = R2 * p1_3d + t2;
        cv::Mat proj2 = K * p2_3d;
        pts2.push_back(cv::Point2f(
            proj2.at<double>(0) / proj2.at<double>(2),
            proj2.at<double>(1) / proj2.at<double>(2)));
    }

    int n = static_cast<int>(pts1.size());

    // TODO 1: Hartley 정규화
    // 각 점 집합의 중심을 0으로, 평균 거리를 sqrt(2)로 정규화
    // T1, T2 정규화 변환 행렬 (3x3) 구하기

    // TODO 2: A 행렬 (Nx9) 구성
    // A[i] = [u2'*u1', u2'*v1', u2', v2'*u1', v2'*v1', v2', u1', v1', 1]
    // (정규화된 좌표 사용)

    // TODO 3: SVD로 f 벡터 → F_norm (3x3) 재구성

    // TODO 4: Rank-2 강제 (가장 작은 특이값을 0으로)

    // TODO 5: 역정규화: F = T2^T * F_norm * T1

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
 * 에피폴라 제약 |p2^T F p1| 평균 오차로 정량화한다.
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

    // TODO: 노이즈 수준별 F 추정 및 오차 측정
    std::vector<double> noise_levels = {0.0, 0.5, 1.0, 2.0, 5.0};

    std::cout << "노이즈 수준에 따른 에피폴라 제약 오차:" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Noise(px)  |  Mean |p2^T F p1|" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;

    for (double noise : noise_levels)
    {
        // TODO 1: pts1_clean, pts2_clean에 가우시안 노이즈 추가
        //         cv::RNG::gaussian(sigma) 사용

        // TODO 2: 노이즈 추가된 점으로 F 추정
        //         cv::findFundamentalMat(pts1_noisy, pts2_noisy, cv::FM_8POINT)

        // TODO 3: 깨끗한 원본 점으로 에피폴라 제약 오차 계산
        //         오차 = |p2^T * F * p1| 의 평균

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
