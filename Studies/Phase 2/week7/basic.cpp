#include "basic.h"
#include <iostream>
#include <iomanip>
#include <cmath>

void PnPBasic::rodrigues(const cv::Mat& rvec, cv::Mat& R)
{
    cv::Rodrigues(rvec, R);
}

bool PnPBasic::solvePnP(const std::vector<cv::Point3f>& points3d,
                        const std::vector<cv::Point2f>& points2d, const cv::Mat& K, cv::Mat& rvec,
                        cv::Mat& tvec, int method)
{
    if (points3d.size() < 4 || points3d.size() != points2d.size())
    {
        return false;
    }

    // 왜곡 계수 (없음)
    cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);

    // PnP 풀기
    bool success = cv::solvePnP(points3d, points2d, K, distCoeffs, rvec, tvec, false, method);

    return success;
}

int PnPBasic::solvePnPRansac(const std::vector<cv::Point3f>& points3d,
                             const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                             cv::Mat& rvec, cv::Mat& tvec, std::vector<int>& inliers,
                             double reprojection_error)
{
    if (points3d.size() < 4 || points3d.size() != points2d.size())
    {
        return 0;
    }

    cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
    cv::Mat inliers_mask;

    // RANSAC PnP
    bool success =
        cv::solvePnPRansac(points3d, points2d, K, distCoeffs, rvec, tvec, false, 100,  // 100번 반복
                           reprojection_error, 0.99, inliers_mask);

    if (!success)
        return 0;

    // Inlier 인덱스 추출
    inliers.clear();
    for (int i = 0; i < inliers_mask.rows; i++)
    {
        if (inliers_mask.at<uchar>(i))
        {
            inliers.push_back(i);
        }
    }

    return inliers.size();
}

double PnPBasic::evaluatePose(const std::vector<cv::Point3f>& points3d,
                              const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                              const cv::Mat& rvec, const cv::Mat& tvec)
{
    // 3D 점들을 카메라로 투영
    std::vector<cv::Point2f> projected;
    cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
    cv::projectPoints(points3d, rvec, tvec, K, distCoeffs, projected);

    // 재투영 오차 계산
    double total_error = 0.0;
    for (size_t i = 0; i < points2d.size(); i++)
    {
        double dx = projected[i].x - points2d[i].x;
        double dy = projected[i].y - points2d[i].y;
        total_error += std::sqrt(dx * dx + dy * dy);
    }

    return total_error / points2d.size();
}

void PnPBasic::visualizePnP(const cv::Mat& img, const std::vector<cv::Point3f>& points3d,
                            const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                            const cv::Mat& rvec, const cv::Mat& tvec, cv::Mat& output)
{
    if (img.channels() == 1)
    {
        cv::cvtColor(img, output, cv::COLOR_GRAY2BGR);
    }
    else
    {
        output = img.clone();
    }

    // 재투영된 점들
    std::vector<cv::Point2f> projected;
    cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
    cv::projectPoints(points3d, rvec, tvec, K, distCoeffs, projected);

    // 시각화
    for (size_t i = 0; i < std::min(points2d.size(), (size_t)20); i++)
    {
        // 관측점 (녹색)
        cv::circle(output, points2d[i], 5, cv::Scalar(0, 255, 0), 2);

        // 투영점 (빨강)
        cv::circle(output, projected[i], 3, cv::Scalar(0, 0, 255), -1);

        // 연결선
        cv::line(output, points2d[i], projected[i], cv::Scalar(255, 0, 0), 1);
    }

    // 정보
    double error = evaluatePose(points3d, points2d, K, rvec, tvec);
    std::string info = "Reprojection Error: " + std::to_string(error).substr(0, 5) + " px";
    cv::putText(output, info, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8,
                cv::Scalar(0, 255, 0), 2);
}

void PnPBasic::compareMethods()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "포즈 추정 방법 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "1️⃣  2D-2D (Essential Matrix)" << std::endl;
    std::cout << "   입력: 두 이미지의 2D 대응점" << std::endl;
    std::cout << "   출력: 상대 포즈 (R, t) ← 스케일 모호성!" << std::endl;
    std::cout << "   사용: VO 초기화, 처음 두 프레임\n" << std::endl;

    std::cout << "2️⃣  3D-2D (PnP)" << std::endl;
    std::cout << "   입력: 3D 점 + 2D 관측" << std::endl;
    std::cout << "   출력: 절대 포즈 (R, t) ← 스케일 복원!" << std::endl;
    std::cout << "   사용: VO tracking, 매 프레임\n" << std::endl;

    std::cout << "3️⃣  3D-3D (ICP)" << std::endl;
    std::cout << "   입력: 두 3D 점군" << std::endl;
    std::cout << "   출력: 상대 변환 (R, t)" << std::endl;
    std::cout << "   사용: RGB-D SLAM, LiDAR\n" << std::endl;

    std::cout << "💡 SLAM에서의 전형적인 흐름:" << std::endl;
    std::cout << "   Frame 0-1: 2D-2D (Essential) → 초기화" << std::endl;
    std::cout << "   Frame 1-2: 3D-2D (PnP) → Tracking" << std::endl;
    std::cout << "   Frame 2-N: 계속 PnP..." << std::endl;
}

void PnPBasic::demoVisualOdometry(const cv::Mat& K)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Visual Odometry 시뮬레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "시나리오: 카메라가 이동하면서 3D 랜드마크 추적\n" << std::endl;

    // 3D 랜드마크 (월드 좌표계)
    std::vector<cv::Point3f> landmarks;
    for (int i = 0; i < 50; i++)
    {
        landmarks.push_back(cv::Point3f(-5.0 + (rand() % 100) / 10.0,  // X: -5 ~ 5
                                        -3.0 + (rand() % 60) / 10.0,   // Y: -3 ~ 3
                                        5.0 + (rand() % 100) / 10.0    // Z: 5 ~ 15
                                        ));
    }

    std::cout << "랜드마크 개수: " << landmarks.size() << "개\n" << std::endl;

    // 카메라 궤적
    std::vector<cv::Mat> gt_poses;   // Ground truth
    std::vector<cv::Mat> est_poses;  // 추정값

    for (int frame = 0; frame < 10; frame++)
    {
        // Ground truth 포즈 (직선 이동)
        cv::Mat rvec_gt = (cv::Mat_<double>(3, 1) << 0.0, 0.0, 0.0);          // 회전 없음
        cv::Mat tvec_gt = (cv::Mat_<double>(3, 1) << frame * 0.1, 0.0, 0.0);  // X 방향 이동

        gt_poses.push_back(tvec_gt.clone());

        // 2D 관측 생성
        std::vector<cv::Point2f> observations;
        cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
        cv::projectPoints(landmarks, rvec_gt, tvec_gt, K, distCoeffs, observations);

        // 노이즈 추가 (픽셀 오차)
        for (auto& obs : observations)
        {
            obs.x += (rand() % 20 - 10) / 10.0;  // ±1 픽셀
            obs.y += (rand() % 20 - 10) / 10.0;
        }

        // PnP로 포즈 추정
        cv::Mat rvec_est, tvec_est;
        std::vector<int> inliers;
        int inlier_count =
            solvePnPRansac(landmarks, observations, K, rvec_est, tvec_est, inliers, 5.0);

        est_poses.push_back(tvec_est.clone());

        // 오차 계산
        double error = cv::norm(tvec_gt - tvec_est);

        std::cout << "Frame " << frame << ": "
                  << "GT=(" << tvec_gt.at<double>(0) << "), "
                  << "Est=(" << tvec_est.at<double>(0) << "), "
                  << "Error=" << std::fixed << std::setprecision(4) << error << " m, "
                  << "Inliers=" << inlier_count << "/" << landmarks.size() << std::endl;
    }

    std::cout << "\n✅ Visual Odometry 완료!" << std::endl;
}

void PnPBasic::demoPipeline(const cv::Mat& K)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "PnP 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Step 1: 3D 점 준비
    std::cout << "1️⃣  3D 랜드마크 생성..." << std::endl;

    std::vector<cv::Point3f> points3d;
    for (int i = 0; i < 30; i++)
    {
        points3d.push_back(cv::Point3f(-2.0 + (rand() % 40) / 10.0, -1.5 + (rand() % 30) / 10.0,
                                       2.0 + (rand() % 60) / 10.0));
    }

    std::cout << "   랜드마크: " << points3d.size() << "개\n" << std::endl;

    // Step 2: Ground truth 포즈
    std::cout << "2️⃣  Ground Truth 포즈 설정..." << std::endl;

    cv::Mat rvec_gt = (cv::Mat_<double>(3, 1) << 0.1, 0.05, 0.0);  // 약간 회전
    cv::Mat tvec_gt = (cv::Mat_<double>(3, 1) << 0.5, 0.2, 0.1);   // 이동

    cv::Mat R_gt;
    rodrigues(rvec_gt, R_gt);

    std::cout << "   회전 벡터:\n" << rvec_gt << std::endl;
    std::cout << "   이동 벡터:\n" << tvec_gt << "\n" << std::endl;

    // Step 3: 2D 관측 생성
    std::cout << "3️⃣  2D 관측 생성 (노이즈 추가)..." << std::endl;

    std::vector<cv::Point2f> points2d;
    cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
    cv::projectPoints(points3d, rvec_gt, tvec_gt, K, distCoeffs, points2d);

    // 노이즈 + Outlier
    for (size_t i = 0; i < points2d.size(); i++)
    {
        points2d[i].x += (rand() % 20 - 10) / 10.0;  // ±1 픽셀
        points2d[i].y += (rand() % 20 - 10) / 10.0;

        // 10% outlier
        if (rand() % 100 < 10)
        {
            points2d[i].x += 50;  // 큰 오차
        }
    }

    std::cout << "   관측점: " << points2d.size() << "개\n" << std::endl;

    // Step 4: PnP 추정 (RANSAC 없이)
    std::cout << "4️⃣  PnP 추정 (RANSAC 없음)..." << std::endl;

    cv::Mat rvec_simple, tvec_simple;
    solvePnP(points3d, points2d, K, rvec_simple, tvec_simple, cv::SOLVEPNP_ITERATIVE);

    double error_simple = evaluatePose(points3d, points2d, K, rvec_simple, tvec_simple);
    std::cout << "   재투영 오차: " << std::fixed << std::setprecision(3) << error_simple
              << " 픽셀\n"
              << std::endl;

    // Step 5: PnP 추정 (RANSAC)
    std::cout << "5️⃣  PnP 추정 (RANSAC)..." << std::endl;

    cv::Mat rvec_ransac, tvec_ransac;
    std::vector<int> inliers;
    int inlier_count =
        solvePnPRansac(points3d, points2d, K, rvec_ransac, tvec_ransac, inliers, 5.0);

    double error_ransac = evaluatePose(points3d, points2d, K, rvec_ransac, tvec_ransac);

    std::cout << "   Inliers: " << inlier_count << " / " << points3d.size() << std::endl;
    std::cout << "   재투영 오차: " << error_ransac << " 픽셀\n" << std::endl;

    // Step 6: 정확도 비교
    std::cout << "6️⃣  Ground Truth와 비교..." << std::endl;

    double t_error_simple = cv::norm(tvec_gt - tvec_simple);
    double t_error_ransac = cv::norm(tvec_gt - tvec_ransac);

    std::cout << "   Simple PnP 이동 오차: " << t_error_simple << " m" << std::endl;
    std::cout << "   RANSAC PnP 이동 오차: " << t_error_ransac << " m" << std::endl;

    std::cout << "\n💡 관찰:" << std::endl;
    std::cout << "   - RANSAC이 outlier를 제거하여 더 정확" << std::endl;
    std::cout << "   - 실제 SLAM에서는 항상 RANSAC 사용" << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 파이프라인 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

// 메인 함수
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  PnP 기본 데모 (Week 7)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    std::cout << "카메라 파라미터 K:\n" << K << "\n" << std::endl;

    // 💡 [교육] PnP란?
    std::cout << "💡 [교육] PnP란? (quiz 문제 1에서 사용!)" << std::endl;
    std::cout << "   입력: N개의 3D 점(맵) + 2D 관측(이미지) + K" << std::endl;
    std::cout << "   출력: 카메라 포즈 (R, t) ← 6 DoF" << std::endl;
    std::cout << "   최소 3점 필요 (P3P), 실제로는 RANSAC + 수십 개 사용\n" << std::endl;

    // 💡 [교육] Rodrigues 변환
    std::cout << "💡 [교육] Rodrigues 변환:" << std::endl;
    std::cout << "   rvec (3×1 회전 벡터) ↔ R (3×3 회전 행렬)" << std::endl;
    std::cout << "   |rvec| = 회전 각도 (라디안)" << std::endl;
    std::cout << "   rvec/|rvec| = 회전 축 (단위 벡터)" << std::endl;
    std::cout << "   OpenCV: cv::Rodrigues(rvec, R)\n" << std::endl;

    // 💡 [교육] PnP vs Essential
    std::cout << "💡 [교육] PnP vs Essential (quiz 문제 2에서 사용!):" << std::endl;
    std::cout << "   2D-2D (Essential): 스케일 모호성 O → VO 초기화용" << std::endl;
    std::cout << "   3D-2D (PnP): 절대 스케일 복원! → VO tracking용" << std::endl;
    std::cout << "   💡 SLAM: Frame 0-1은 E, Frame 1~N은 PnP\n" << std::endl;

    // 전체 파이프라인
    PnPBasic::demoPipeline(K);

    // Visual Odometry 시뮬레이션
    PnPBasic::demoVisualOdometry(K);

    // 방법 비교
    PnPBasic::compareMethods();

    std::cout << "\n💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md 이론 읽기 (PnP, DLT, RANSAC)" << std::endl;
    std::cout << "   2. quiz_easy.cpp — PnP 정의, VO 흐름, DLT 개념" << std::endl;
    std::cout << "   3. my_basic.cpp — Step 1~8 순서대로 직접 구현" << std::endl;
    std::cout << "   4. quiz_medium.cpp — PnP 구현, RANSAC 횟수, DLT 구현" << std::endl;
    std::cout << "   5. PRACTICE.md — PnP/삼각측량 실습\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
