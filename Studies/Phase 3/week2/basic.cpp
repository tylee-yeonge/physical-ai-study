#include "motion_2d2d.h"
#include <iostream>
#include <iomanip>

int Motion2D2D::estimateEssential(const std::vector<cv::Point2f>& points1,
                                  const std::vector<cv::Point2f>& points2, cv::Mat& E,
                                  std::vector<uchar>& inliers)
{
    // Essential Matrix 추정 (정규화 좌표 사용)
    E = cv::findEssentialMat(points1, points2,
                             cv::Mat::eye(3, 3, CV_64F),  // 정규화 좌표이므로 Identity
                             cv::RANSAC,
                             0.999,  // 신뢰도
                             1.0,    // 임계값
                             inliers);

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
    // E에서 R, t 복원 (Cheirality check 자동)
    int good_points =
        cv::recoverPose(E, points1, points2, cv::Mat::eye(3, 3, CV_64F), R, t, inliers);

    return good_points;
}

void Motion2D2D::normalizePoints(const cv::Mat& K, const std::vector<cv::Point2f>& points,
                                 std::vector<cv::Point2f>& normalized)
{
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
    // 투영 행렬
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));

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

            // 깊이 체크 (양수)
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
    // Step 1: 정규화
    std::vector<cv::Point2f> pts1_norm, pts2_norm;
    normalizePoints(K, points1, pts1_norm);
    normalizePoints(K, points2, pts2_norm);

    // Step 2: Essential Matrix 추정
    cv::Mat E;
    std::vector<uchar> inliers_e;
    int inlier_count = estimateEssential(pts1_norm, pts2_norm, E, inliers_e);

    if (inlier_count < 8)
    {
        std::cout << "❌ Inlier 부족: " << inlier_count << std::endl;
        return false;
    }

    // Step 3: R, t 복원
    std::vector<uchar> inliers_r;
    int good_points = recoverPose(E, pts1_norm, pts2_norm, R, t, inliers_r);

    // Step 4: 삼각측량
    triangulate(K, R, t, points1, points2, points3d);

    return points3d.size() >= 10;
}

void Motion2D2D::demo()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "2D-2D 모션 추정 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 💡 교육 블록: Essential vs Fundamental
    std::cout << "💡 [Essential vs Fundamental]" << std::endl;
    std::cout << "   Essential (E): 정규화 좌표, K 필요, p'^T E p = 0" << std::endl;
    std::cout << "   Fundamental (F): 픽셀 좌표, K 불필요, F = K₂⁻ᵀ E K₁⁻¹" << std::endl;
    std::cout << "   💡 이 비교가 quiz_easy 문제 1!\n" << std::endl;

    // 💡 교육 블록: 5-Point Algorithm
    std::cout << "💡 [5-Point Algorithm]" << std::endl;
    std::cout << "   E의 자유도 = 5 (회전 3 + 이동 방향 2)" << std::endl;
    std::cout << "   → 최소 5개 대응점 필요 (8-Point는 과결정)" << std::endl;
    std::cout << "   💡 이 개념이 quiz_easy 문제 2!\n" << std::endl;

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    std::cout << "카메라 내부 파라미터 K:\n" << K << "\n" << std::endl;

    // Ground truth 포즈
    cv::Mat R_gt = (cv::Mat_<double>(3, 3) << 0.9998, -0.0175, 0.0000, 0.0175, 0.9998, 0.0000,
                    0.0000, 0.0000, 1.0000);  // 약 1도 회전

    cv::Mat t_gt = (cv::Mat_<double>(3, 1) << 1.0, 0.0, 0.0);

    std::cout << "Ground Truth 포즈:" << std::endl;
    std::cout << "   t = " << t_gt.t() << "\n" << std::endl;

    // 3D 점 생성
    std::vector<cv::Point3f> pts3d_world;
    for (int i = 0; i < 50; i++)
    {
        pts3d_world.push_back(cv::Point3f(-2.0 + (rand() % 40) / 10.0, -1.5 + (rand() % 30) / 10.0,
                                          3.0 + (rand() % 40) / 10.0));
    }

    // 투영 → 2D 관측
    std::vector<cv::Point2f> points1, points2;

    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));

    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R_gt.copyTo(RT(cv::Rect(0, 0, 3, 3)));
    t_gt.copyTo(RT(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT;

    for (const auto& pt3d : pts3d_world)
    {
        cv::Mat pt = (cv::Mat_<double>(4, 1) << pt3d.x, pt3d.y, pt3d.z, 1.0);

        cv::Mat proj1 = P1 * pt;
        points1.push_back(cv::Point2f(proj1.at<double>(0) / proj1.at<double>(2),
                                      proj1.at<double>(1) / proj1.at<double>(2)));

        cv::Mat proj2 = P2 * pt;
        points2.push_back(cv::Point2f(proj2.at<double>(0) / proj2.at<double>(2),
                                      proj2.at<double>(1) / proj2.at<double>(2)));
    }

    // 노이즈 추가
    for (auto& pt : points2)
    {
        pt.x += (rand() % 20 - 10) / 10.0;
        pt.y += (rand() % 20 - 10) / 10.0;
    }

    std::cout << "생성된 대응점: " << points1.size() << "개\n" << std::endl;

    // 파이프라인 실행
    cv::Mat R_est, t_est;
    std::vector<cv::Point3f> points3d_est;

    bool success = pipeline(K, points1, points2, R_est, t_est, points3d_est);

    // 💡 교육 블록: Cheirality Check
    std::cout << "\n💡 [Cheirality Check]" << std::endl;
    std::cout << "   E 분해 → 4가지 (R,t) 해" << std::endl;
    std::cout << "   3D 점이 두 카메라 앞(depth > 0)인 해 선택" << std::endl;
    std::cout << "   💡 이 과정이 quiz_easy 문제 3!\n" << std::endl;

    if (success)
    {
        std::cout << "✅ 추정 성공!" << std::endl;
        std::cout << "   추정 t = " << t_est.t() << std::endl;
        std::cout << "   복원된 3D 점: " << points3d_est.size() << "개" << std::endl;

        double t_error = cv::norm(t_gt - t_est);
        std::cout << "   이동 오차: " << std::fixed << std::setprecision(4) << t_error << "\n"
                  << std::endl;

        std::cout << "⚠️  주의: 스케일 모호성!" << std::endl;
        std::cout << "   - GT: ||t|| = " << cv::norm(t_gt) << std::endl;
        std::cout << "   - Est: ||t|| = " << cv::norm(t_est) << std::endl;
        std::cout << "   - 정규화되어 둘 다 1.0" << std::endl;
        std::cout << "   💡 이 현상이 quiz_easy 문제 4!\n" << std::endl;
    }

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 2: 2D-2D 모션 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Motion2D2D::demo();

    std::cout << "\n💡 핵심 내용:" << std::endl;
    std::cout << "   - Essential Matrix로 2D-2D 모션 추정" << std::endl;
    std::cout << "   - R, t 복원 (Cheirality check)" << std::endl;
    std::cout << "   - 삼각측량으로 초기 3D 맵" << std::endl;
    std::cout << "   - ⚠️ 스케일 모호성 존재!\n" << std::endl;

    std::cout << "💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md에서 E/F 이론 읽기 → quiz_easy 문제 1~2" << std::endl;
    std::cout << "   2. my_basic.cpp Step 1~3 구현 → quiz_easy 문제 3~4" << std::endl;
    std::cout << "   3. my_basic.cpp Step 4~5 구현 → quiz_medium 문제 1~3" << std::endl;
    std::cout << "   4. PRACTICE.md에서 추가 실습\n" << std::endl;

    std::cout << "다음: Week 3 - 3D-2D 모션 추정 (PnP)\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
