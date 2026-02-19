/**
 * Phase 3 Week 2 - 2D-2D 모션 추정 직접 구현
 *
 * motion_2d2d.h의 Motion2D2D 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make my_basic
 * 실행: ./my_basic (각 Step 구현 후 실행하여 확인)
 * 전체 테스트: cd build && cmake .. && make test_my_basic && ./test_my_basic
 *
 * ┌──────┬────────────────────┬────────┬────────────────┐
 * │ Step │ 함수               │ 난이도  │ 검증 방법        │
 * ├──────┼────────────────────┼────────┼────────────────┤
 * │  1   │ estimateEssential  │ 쉬움   │ ./my_basic     │
 * │  2   │ recoverPose        │ 쉬움   │ ./my_basic     │
 * │  3   │ normalizePoints    │ 핵심   │ ./my_basic     │
 * │  4   │ triangulate        │ 어려움  │ ./my_basic     │
 * │  5   │ pipeline           │ 종합   │ ./my_basic     │
 * └──────┴────────────────────┴────────┴────────────────┘
 *
 * 💡 각 Step 구현 후 ./my_basic을 실행하면 해당 단계의 테스트 결과를 확인할 수 있습니다.
 */
#include "motion_2d2d.h"
#include <iostream>

int Motion2D2D::estimateEssential(const std::vector<cv::Point2f>& points1,
                                  const std::vector<cv::Point2f>& points2, cv::Mat& E,
                                  std::vector<uchar>& inliers)
{
    // [Step 1] Essential Matrix 추정 (RANSAC)
    // 힌트: cv::findEssentialMat(pts1, pts2, I, RANSAC, 0.999, 1.0, inliers)
    // 참고: basic.cpp의 estimateEssential()
    // 기대값: inlier 비율 > 80%
    E = cv::findEssentialMat(points1, points2,
                             cv::Mat::eye(3, 3, CV_64F),
                             cv::RANSAC, 0.999, 1.0, inliers);

    int inlier_count = 0;
    for (uchar status : inliers)
    {
        if (status)
            inlier_count++;
    }
    return inlier_count;
}

int Motion2D2D::recoverPose(const cv::Mat& E, const std::vector<cv::Point2f>& points1,
                            const std::vector<cv::Point2f>& points2, cv::Mat& R, cv::Mat& t,
                            std::vector<uchar>& inliers)
{
    // [Step 2] E에서 R, t 복원 (Cheirality check 자동)
    // 힌트: cv::recoverPose(E, pts1, pts2, I, R, t, inliers)
    // 참고: basic.cpp의 recoverPose()
    // 기대값: R은 작은 회전, ||t|| = 1 (정규화됨)
    int good_points =
        cv::recoverPose(E, points1, points2, cv::Mat::eye(3, 3, CV_64F), R, t, inliers);
    return good_points;
}

void Motion2D2D::normalizePoints(const cv::Mat& K, const std::vector<cv::Point2f>& points,
                                 std::vector<cv::Point2f>& normalized)
{
    // [Step 3] 픽셀 좌표 → 정규화 좌표 (핵심!)
    // 공식: x_norm = (u - cx) / fx, y_norm = (v - cy) / fy
    // 참고: basic.cpp의 normalizePoints(), ./basic 출력의 "정규화" 부분
    // 기대값: 중심 (400,300) → (0,0)
    normalized.clear();

    double fx = K.at<double>(0, 0);
    double fy = K.at<double>(1, 1);
    double cx = K.at<double>(0, 2);
    double cy = K.at<double>(1, 2);

    for (const auto& pt : points)
    {
        cv::Point2f p_norm;
        p_norm.x = (pt.x - cx) / fx;
        p_norm.y = (pt.y - cy) / fy;
        normalized.push_back(p_norm);
    }
}

void Motion2D2D::triangulate(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t,
                             const std::vector<cv::Point2f>& points1,
                             const std::vector<cv::Point2f>& points2,
                             std::vector<cv::Point3f>& points3d)
{
    // [Step 4] 삼각측량으로 3D 점 복원
    // 힌트: P1 = K*[I|0], P2 = K*[R|t], cv::triangulatePoints()
    // 참고: basic.cpp의 triangulate()
    // 기대값: depth > 0인 점들만 출력

    // 투영 행렬 P1 = K * [I | 0]
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));

    // 투영 행렬 P2 = K * [R | t]
    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R.copyTo(RT(cv::Rect(0, 0, 3, 3)));
    t.copyTo(RT(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT;

    // 삼각측량
    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, points1, points2, points4D);

    // 동차 좌표 → 3D
    points3d.clear();
    for (int i = 0; i < points4D.cols; i++)
    {
        float w = points4D.at<float>(3, i);
        if (std::abs(w) > 1e-6)
        {
            cv::Point3f pt;
            pt.x = points4D.at<float>(0, i) / w;
            pt.y = points4D.at<float>(1, i) / w;
            pt.z = points4D.at<float>(2, i) / w;

            if (pt.z > 0)
            {
                points3d.push_back(pt);
            }
        }
    }
}

bool Motion2D2D::pipeline(const cv::Mat& K, const std::vector<cv::Point2f>& points1,
                          const std::vector<cv::Point2f>& points2, cv::Mat& R, cv::Mat& t,
                          std::vector<cv::Point3f>& points3d)
{
    // [Step 5] 전체 파이프라인 (정규화 → E 추정 → 포즈 복원 → 삼각측량)
    // 힌트: Step 1~4를 순서대로 호출
    // 참고: basic.cpp의 pipeline()
    // 기대값: 3D 점 >= 10개이면 성공
    constexpr int kMinInliers = 8;
    constexpr int kMinPoints3d = 10;

    std::vector<cv::Point2f> pts1_norm, pts2_norm;
    normalizePoints(K, points1, pts1_norm);
    normalizePoints(K, points2, pts2_norm);

    cv::Mat E;
    std::vector<uchar> inliers_e;
    int inlier_count = estimateEssential(pts1_norm, pts2_norm, E, inliers_e);

    if (inlier_count < kMinInliers)
    {
        std::cout << "❌ Inlier 부족: " << inlier_count << std::endl;
        return false;
    }

    std::vector<uchar> inliers_r;
    recoverPose(E, pts1_norm, pts2_norm, R, t, inliers_r);

    triangulate(K, R, t, points1, points2, points3d);

    return static_cast<int>(points3d.size()) >= kMinPoints3d;
}

void Motion2D2D::demo()
{
    // [Step 5에서 사용] 전체 데모는 basic.cpp 참고
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 2D-2D 모션 추정 - 단계별 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 공통 설정
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // ── Step 1: estimateEssential ──
    std::cout << "Step 1: estimateEssential" << std::endl;
    // 정규화 좌표로 테스트 (x방향 이동)
    std::vector<cv::Point2f> pts1_n = {{0.0f, 0.0f},   {0.5f, 0.0f}, {0.0f, 0.5f},
                                       {-0.5f, 0.0f},  {0.0f, -0.5f}, {0.3f, 0.3f},
                                       {-0.3f, 0.3f},  {0.1f, -0.2f}};
    std::vector<cv::Point2f> pts2_n = {{0.05f, 0.0f},  {0.55f, 0.0f}, {0.05f, 0.5f},
                                       {-0.45f, 0.0f}, {0.05f, -0.5f}, {0.35f, 0.3f},
                                       {-0.25f, 0.3f}, {0.15f, -0.2f}};

    cv::Mat E;
    std::vector<uchar> inliers;
    int n_inliers = Motion2D2D::estimateEssential(pts1_n, pts2_n, E, inliers);
    std::cout << "   Inliers: " << n_inliers << "/" << pts1_n.size() << std::endl;
    bool e_ok = (n_inliers >= 5 && !E.empty());
    std::cout << "   " << (e_ok ? "✅ E 추정 성공" : "❌ E 추정 실패") << std::endl;

    // ── Step 2: recoverPose ──
    std::cout << "\nStep 2: recoverPose" << std::endl;
    cv::Mat R, t;
    std::vector<uchar> inliers_r;
    int good = Motion2D2D::recoverPose(E, pts1_n, pts2_n, R, t, inliers_r);
    std::cout << "   Cheirality 통과: " << good << "개" << std::endl;

    double t_norm = cv::norm(t);
    std::cout << "   ||t|| = " << t_norm << std::endl;
    bool t_ok = (std::abs(t_norm - 1.0) < 0.01);
    std::cout << "   " << (t_ok ? "✅ ||t|| ≈ 1 (정규화됨)" : "❌ 기대: ||t|| ≈ 1") << std::endl;

    // ── Step 3: normalizePoints ──
    std::cout << "\nStep 3: normalizePoints" << std::endl;
    std::vector<cv::Point2f> pixels = {{400.0f, 300.0f}, {700.0f, 300.0f}, {400.0f, 600.0f}};
    std::vector<cv::Point2f> norm;
    Motion2D2D::normalizePoints(K, pixels, norm);

    if (norm.size() == 3)
    {
        std::cout << "   (400,300) → (" << norm[0].x << ", " << norm[0].y << ")"
                  << (std::abs(norm[0].x) < 0.001 ? " ✅ 중심→(0,0)" : " ❌ 기대: (0,0)")
                  << std::endl;
        std::cout << "   (700,300) → (" << norm[1].x << ", " << norm[1].y << ")"
                  << (std::abs(norm[1].x - 0.5) < 0.001 ? " ✅" : " ❌ 기대: (0.5,0)")
                  << std::endl;
    }
    else
    {
        std::cout << "   ❌ 출력 크기 오류: " << norm.size() << " (기대: 3)" << std::endl;
    }

    // ── Step 4: triangulate ──
    std::cout << "\nStep 4: triangulate" << std::endl;
    // 단순 전방 이동 시나리오 (카메라2 = x방향 1m 이동)
    cv::Mat R_id = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t_right = (cv::Mat_<double>(3, 1) << 1.0, 0.0, 0.0);

    // 3D 점 (0, 0, 5)을 두 카메라에서 관측
    std::vector<cv::Point2f> obs1 = {{400.0f, 300.0f}};  // 중심
    std::vector<cv::Point2f> obs2 = {
        {static_cast<float>(600.0 * (-1.0 / 5.0) + 400.0), 300.0f}};  // x 이동 후

    std::vector<cv::Point3f> pts3d;
    Motion2D2D::triangulate(K, R_id, t_right, obs1, obs2, pts3d);
    std::cout << "   복원된 3D 점: " << pts3d.size() << "개" << std::endl;
    if (!pts3d.empty())
    {
        std::cout << "   위치: (" << pts3d[0].x << ", " << pts3d[0].y << ", " << pts3d[0].z << ")"
                  << std::endl;
        bool z_ok = (pts3d[0].z > 0);
        std::cout << "   " << (z_ok ? "✅ depth > 0" : "❌ 기대: depth > 0") << std::endl;
    }

    // ── Step 5: pipeline ──
    std::cout << "\nStep 5: pipeline" << std::endl;
    // 50개 3D 점 → 두 카메라에서 투영 → 파이프라인
    cv::Mat R_gt = (cv::Mat_<double>(3, 3) << 0.9998, -0.0175, 0.0,
                    0.0175, 0.9998, 0.0,
                    0.0, 0.0, 1.0);
    cv::Mat t_gt = (cv::Mat_<double>(3, 1) << 1.0, 0.0, 0.0);

    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));
    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R_gt.copyTo(RT(cv::Rect(0, 0, 3, 3)));
    t_gt.copyTo(RT(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT;

    std::vector<cv::Point2f> p1, p2;
    srand(42);
    for (int i = 0; i < 50; i++)
    {
        cv::Point3f pt3d(-2.0f + (rand() % 40) / 10.0f,
                         -1.5f + (rand() % 30) / 10.0f,
                         3.0f + (rand() % 40) / 10.0f);
        cv::Mat pw = (cv::Mat_<double>(4, 1) << pt3d.x, pt3d.y, pt3d.z, 1.0);
        cv::Mat proj1 = P1 * pw;
        cv::Mat proj2 = P2 * pw;
        p1.push_back(cv::Point2f(proj1.at<double>(0) / proj1.at<double>(2),
                                 proj1.at<double>(1) / proj1.at<double>(2)));
        p2.push_back(cv::Point2f(proj2.at<double>(0) / proj2.at<double>(2),
                                 proj2.at<double>(1) / proj2.at<double>(2)));
    }

    cv::Mat R_est, t_est;
    std::vector<cv::Point3f> pts3d_est;
    bool ok = Motion2D2D::pipeline(K, p1, p2, R_est, t_est, pts3d_est);
    std::cout << "   파이프라인: " << (ok ? "✅ 성공" : "❌ 실패") << std::endl;
    if (ok)
    {
        std::cout << "   복원 3D 점: " << pts3d_est.size() << "개" << std::endl;
        std::cout << "   ||t_est|| = " << cv::norm(t_est)
                  << " (항상 1.0 = 스케일 모호성)" << std::endl;
    }

    // ── 요약 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  💡 핵심 확인 사항:" << std::endl;
    std::cout << "   - Step 2: ||t|| = 1 → 스케일 모호성의 기원!" << std::endl;
    std::cout << "   - Step 3: 정규화 = K⁻¹ 적용 (E 추정의 전제)" << std::endl;
    std::cout << "   - Step 4: 삼각측량 = 두 시점의 교차점" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
