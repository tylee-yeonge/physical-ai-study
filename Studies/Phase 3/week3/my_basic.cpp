/**
 * Phase 3 Week 3 - 3D-2D 모션 추정 (PnP) 직접 구현
 *
 * motion_3d2d.h의 Motion3D2D 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make my_basic
 * 실행: ./my_basic (각 Step 구현 후 실행하여 확인)
 * 전체 테스트: cd build && cmake .. && make test_my_basic && ./test_my_basic
 *
 * ┌──────┬──────────────────────────┬────────┬────────────────┐
 * │ Step │ 함수                     │ 난이도  │ 검증 방법        │
 * ├──────┼──────────────────────────┼────────┼────────────────┤
 * │  1   │ solvePnP                 │ 쉬움   │ ./my_basic     │
 * │  2   │ solvePnPRansac           │ 쉬움   │ ./my_basic     │
 * │  3   │ computeReprojectionError │ 핵심   │ ./my_basic     │
 * │  4   │ projectPoints            │ 쉬움   │ ./my_basic     │
 * │  5   │ simulateTracking         │ 어려움  │ ./my_basic     │
 * │  6   │ demo                     │ 종합   │ ./my_basic     │
 * └──────┴──────────────────────────┴────────┴────────────────┘
 *
 * 💡 각 Step 구현 후 ./my_basic을 실행하면 해당 단계의 테스트 결과를 확인할 수 있습니다.
 */
#include "motion_3d2d.h"
#include <iostream>
#include <iomanip>

bool Motion3D2D::solvePnP(const std::vector<cv::Point3f>& points3d,
                          const std::vector<cv::Point2f>& points2d, const cv::Mat& K, cv::Mat& rvec,
                          cv::Mat& tvec)
{
    // [Step 1] EPnP 알고리즘으로 포즈 추정
    // 힌트: cv::solvePnP(pts3d, pts2d, K, Mat(), rvec, tvec, false, SOLVEPNP_EPNP)
    // 참고: basic.cpp의 solvePnP()
    // 기대값: 최소 4개 점 필요, 성공 시 true
    constexpr int kMinPoints = 4;

    if (static_cast<int>(points3d.size()) < kMinPoints ||
        static_cast<int>(points2d.size()) < kMinPoints)
    {
        return false;
    }

    return cv::solvePnP(points3d, points2d, K, cv::Mat(), rvec, tvec, false, cv::SOLVEPNP_EPNP);
}

int Motion3D2D::solvePnPRansac(const std::vector<cv::Point3f>& points3d,
                               const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                               cv::Mat& rvec, cv::Mat& tvec, std::vector<int>& inliers)
{
    // [Step 2] PnP + RANSAC으로 Outlier 제거
    // 힌트: cv::solvePnPRansac(pts3d, pts2d, K, Mat(), rvec, tvec, false, 100, 8.0, 0.99, inliers)
    // 참고: basic.cpp의 solvePnPRansac()
    // 기대값: inlier 비율 > 70%
    constexpr int kMinPoints = 4;

    if (static_cast<int>(points3d.size()) < kMinPoints ||
        static_cast<int>(points2d.size()) < kMinPoints)
    {
        return 0;
    }

    cv::solvePnPRansac(points3d, points2d, K, cv::Mat(), rvec, tvec,
                       false, 100, 8.0, 0.99, inliers);

    return inliers.size();
}

double Motion3D2D::computeReprojectionError(const std::vector<cv::Point3f>& points3d,
                                            const std::vector<cv::Point2f>& points2d,
                                            const cv::Mat& K, const cv::Mat& rvec,
                                            const cv::Mat& tvec)
{
    // [Step 3] 재투영 오차 = RMS(||관측 - 투영||) (핵심!)
    // 힌트: projectPoints() → 유클리드 거리 → RMS
    // 참고: basic.cpp의 computeReprojectionError()
    // 기대값: 정확한 포즈면 < 1px
    std::vector<cv::Point2f> projected;
    projectPoints(points3d, K, rvec, tvec, projected);

    double sum_error = 0.0;
    for (size_t i = 0; i < points2d.size(); i++)
    {
        double dx = projected[i].x - points2d[i].x;
        double dy = projected[i].y - points2d[i].y;
        sum_error += dx * dx + dy * dy;
    }

    return std::sqrt(sum_error / points2d.size());
}

void Motion3D2D::projectPoints(const std::vector<cv::Point3f>& points3d, const cv::Mat& K,
                               const cv::Mat& rvec, const cv::Mat& tvec,
                               std::vector<cv::Point2f>& projected)
{
    // [Step 4] 3D → 2D 투영
    // 힌트: cv::projectPoints(pts3d, rvec, tvec, K, Mat(), projected)
    // 참고: basic.cpp의 projectPoints()
    cv::projectPoints(points3d, rvec, tvec, K, cv::Mat(), projected);
}

void Motion3D2D::simulateTracking(const std::vector<cv::Point3f>& map_points, const cv::Mat& K,
                                  int num_frames)
{
    // [Step 5] VO 추적 시뮬레이션 (어려움)
    // 힌트: 매 프레임 GT포즈→투영→노이즈→PnP→오차 확인
    // 참고: basic.cpp의 simulateTracking()
    // 기대값: inlier ratio > 70%, reproj error < 3px
    constexpr double kInlierThreshold = 0.3;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "VO 추적 시뮬레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "초기 맵: " << map_points.size() << "개 3D 점\n" << std::endl;

    for (int frame = 0; frame < num_frames; frame++)
    {
        cv::Mat rvec_gt = (cv::Mat_<double>(3, 1) << 0.01 * frame, 0.005 * frame, 0.0);
        cv::Mat tvec_gt = (cv::Mat_<double>(3, 1) << 0.1 * frame, 0.02 * frame, 0.0);

        std::vector<cv::Point2f> observations;
        projectPoints(map_points, K, rvec_gt, tvec_gt, observations);

        for (auto& pt : observations)
        {
            pt.x += (rand() % 20 - 10) / 10.0;
            pt.y += (rand() % 20 - 10) / 10.0;
        }

        cv::Mat rvec_est, tvec_est;
        std::vector<int> inliers;
        int inlier_count = solvePnPRansac(map_points, observations, K, rvec_est, tvec_est, inliers);

        double inlier_ratio = static_cast<double>(inlier_count) / map_points.size();
        double reproj_error =
            computeReprojectionError(map_points, observations, K, rvec_est, tvec_est);

        std::cout << "Frame " << frame << ":" << std::endl;
        std::cout << "   Inliers: " << inlier_count << "/" << map_points.size() << " ("
                  << std::fixed << std::setprecision(1) << inlier_ratio * 100 << "%)" << std::endl;
        std::cout << "   Reproj error: " << std::setprecision(2) << reproj_error << " px"
                  << std::endl;

        if (inlier_ratio < kInlierThreshold)
        {
            std::cout << "   ⚠️  추적 실패! 재초기화 필요\n" << std::endl;
            break;
        }
        std::cout << std::endl;
    }
}

void Motion3D2D::demo()
{
    // [Step 6에서 사용] 전체 데모는 basic.cpp 참고
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 3D-2D PnP - 단계별 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 공통 설정
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // 3D-2D 대응 생성 (원점 카메라에서 관측)
    std::vector<cv::Point3f> pts3d = {
        {0.0f, 0.0f, 5.0f}, {1.0f, 0.0f, 5.0f}, {0.0f, 1.0f, 5.0f},
        {1.0f, 1.0f, 5.0f}, {0.5f, 0.5f, 4.0f}, {-0.5f, 0.5f, 6.0f}};

    // 원점 카메라에서 투영
    std::vector<cv::Point2f> pts2d;
    cv::Mat rvec_zero = cv::Mat::zeros(3, 1, CV_64F);
    cv::Mat tvec_zero = cv::Mat::zeros(3, 1, CV_64F);
    cv::projectPoints(pts3d, rvec_zero, tvec_zero, K, cv::Mat(), pts2d);

    // ── Step 1: solvePnP ──
    std::cout << "Step 1: solvePnP" << std::endl;
    cv::Mat rvec, tvec;
    bool ok = Motion3D2D::solvePnP(pts3d, pts2d, K, rvec, tvec);
    std::cout << "   결과: " << (ok ? "✅ 성공" : "❌ 실패") << std::endl;
    if (ok)
    {
        double t_norm = cv::norm(tvec);
        std::cout << "   ||t|| = " << t_norm
                  << (t_norm < 0.1 ? " ✅ 원점 근처" : " ❌ 기대: ~0") << std::endl;
    }

    // ── Step 2: solvePnPRansac ──
    std::cout << "\nStep 2: solvePnPRansac" << std::endl;
    cv::Mat rvec_r, tvec_r;
    std::vector<int> inliers;
    int n_inliers = Motion3D2D::solvePnPRansac(pts3d, pts2d, K, rvec_r, tvec_r, inliers);
    double ratio = static_cast<double>(n_inliers) / pts3d.size();
    std::cout << "   Inliers: " << n_inliers << "/" << pts3d.size()
              << " (" << std::fixed << std::setprecision(0) << ratio * 100 << "%)" << std::endl;
    std::cout << "   " << (ratio > 0.7 ? "✅ Inlier ratio > 70%" : "❌ 기대: > 70%")
              << std::endl;

    // ── Step 3: computeReprojectionError ──
    std::cout << "\nStep 3: computeReprojectionError" << std::endl;
    double err = Motion3D2D::computeReprojectionError(pts3d, pts2d, K, rvec_r, tvec_r);
    std::cout << "   재투영 오차: " << std::setprecision(4) << err << " px" << std::endl;
    std::cout << "   " << (err < 1.0 ? "✅ < 1px 정확" : "❌ 기대: < 1px") << std::endl;

    // ── Step 4: projectPoints ──
    std::cout << "\nStep 4: projectPoints" << std::endl;
    std::vector<cv::Point2f> projected;
    Motion3D2D::projectPoints(pts3d, K, rvec_zero, tvec_zero, projected);
    std::cout << "   투영 점: " << projected.size() << "개"
              << (projected.size() == pts3d.size() ? " ✅" : " ❌") << std::endl;
    if (!projected.empty())
    {
        // (0,0,5) → (400,300) = 이미지 중심
        bool center_ok = (std::abs(projected[0].x - 400) < 1 && std::abs(projected[0].y - 300) < 1);
        std::cout << "   (0,0,5) → (" << (int)projected[0].x << "," << (int)projected[0].y << ")"
                  << (center_ok ? " ✅ 중심" : " ❌ 기대: (400,300)") << std::endl;
    }

    // ── Step 5: simulateTracking ──
    std::cout << "\nStep 5: simulateTracking" << std::endl;
    std::vector<cv::Point3f> map_points;
    srand(42);
    for (int i = 0; i < 50; i++)
    {
        map_points.push_back(cv::Point3f(-3.0f + (rand() % 60) / 10.0f,
                                         -2.0f + (rand() % 40) / 10.0f,
                                         3.0f + (rand() % 50) / 10.0f));
    }
    Motion3D2D::simulateTracking(map_points, K, 3);

    // ── 요약 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  💡 핵심 확인 사항:" << std::endl;
    std::cout << "   - Step 1: PnP = 절대 스케일 포즈 추정" << std::endl;
    std::cout << "   - Step 2: RANSAC = Outlier에 강건" << std::endl;
    std::cout << "   - Step 3: 재투영 오차 = 포즈 품질 지표" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
