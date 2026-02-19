/**
 * Phase 2 Week 7 - PnP 직접 구현
 *
 * basic.h의 PnPBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make my_basic
 * 실행: ./my_basic (각 Step 구현 후 실행하여 확인)
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │                    구현 순서 가이드                            │
 * ├──────┬──────────────────────┬────────┬───────────────────────┤
 * │ Step │ 함수                  │ 난이도 │ 검증 방법              │
 * ├──────┼──────────────────────┼────────┼───────────────────────┤
 * │  1   │ rodrigues            │ 쉬움   │ ./my_basic            │
 * │  2   │ solvePnP             │ 쉬움   │ ./my_basic            │
 * │  3   │ solvePnPRansac       │ 보통   │ ./my_basic            │
 * │  4   │ evaluatePose         │ 보통   │ ./my_basic            │
 * │  5   │ visualizePnP         │ 쉬움   │ ./my_basic            │
 * │  6   │ compareMethods       │ 보통   │ ./my_basic            │
 * │  7   │ demoVisualOdometry   │ 어려움 │ ./my_basic            │
 * │  8   │ demoPipeline         │ 어려움 │ ./my_basic            │
 * └──────┴──────────────────────┴────────┴───────────────────────┘
 *
 * 💡 각 Step 구현 후 ./my_basic을 실행하면 해당 단계의 ✅/❌를 확인할 수 있습니다.
 */
#include "basic.h"
#include <iostream>
#include <iomanip>
#include <cmath>

void PnPBasic::rodrigues(const cv::Mat& rvec, cv::Mat& R)
{
    // [Step 1] 회전 벡터 → 회전 행렬 (가장 먼저 구현!)
    // cv::Rodrigues(rvec, R)
    // 참고: basic.cpp의 rodrigues()
    // 기대값: R(3×3), det(R)≈1
}

bool PnPBasic::solvePnP(const std::vector<cv::Point3f>& points3d,
                        const std::vector<cv::Point2f>& points2d, const cv::Mat& K, cv::Mat& rvec,
                        cv::Mat& tvec, int method)
{
    // [Step 2] PnP로 카메라 포즈 추정
    // 1) points3d.size() < 4이면 false
    // 2) distCoeffs = cv::Mat::zeros(4, 1, CV_64F)
    // 3) cv::solvePnP(points3d, points2d, K, distCoeffs, rvec, tvec, false, method)
    // 참고: basic.cpp의 solvePnP()
    // 기대값: rvec(3×1), tvec(3×1), GT와 오차 < 0.01
    return false;
}

int PnPBasic::solvePnPRansac(const std::vector<cv::Point3f>& points3d,
                             const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                             cv::Mat& rvec, cv::Mat& tvec, std::vector<int>& inliers,
                             double reprojection_error)
{
    // [Step 3] RANSAC PnP (outlier 제거)
    // 1) cv::solvePnPRansac(points3d, points2d, K, distCoeffs,
    //                        rvec, tvec, false, 100, reprojection_error, 0.99, inliers_mask)
    // 2) inliers_mask에서 inlier 인덱스 추출
    // 참고: basic.cpp의 solvePnPRansac()
    // 기대값: inlier 수 < 전체 수 (outlier 있을 때)
    return 0;
}

double PnPBasic::evaluatePose(const std::vector<cv::Point3f>& points3d,
                              const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                              const cv::Mat& rvec, const cv::Mat& tvec)
{
    // [Step 4] 재투영 오차로 포즈 품질 평가
    // 1) cv::projectPoints(points3d, rvec, tvec, K, distCoeffs, projected)
    // 2) 각 점의 유클리드 거리 합 / 점 개수
    // 참고: basic.cpp의 evaluatePose()
    // 기대값: 노이즈 없으면 ~0
    return -1.0;
}

void PnPBasic::visualizePnP(const cv::Mat& img, const std::vector<cv::Point3f>& points3d,
                            const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                            const cv::Mat& rvec, const cv::Mat& tvec, cv::Mat& output)
{
    // [Step 5] 관측점(녹색) + 투영점(빨강) + 연결선 시각화
    // 1) cv::projectPoints로 투영
    // 2) cv::circle + cv::line
    // 참고: basic.cpp의 visualizePnP()
    // 기대값: output.empty() == false
}

void PnPBasic::compareMethods()
{
    // [Step 6] 2D-2D (Essential) vs 3D-2D (PnP) vs 3D-3D (ICP) 비교
    // 참고: basic.cpp의 compareMethods()
}

void PnPBasic::demoVisualOdometry(const cv::Mat& K)
{
    // [Step 7] Visual Odometry 시뮬레이션
    // 10프레임 동안 카메라 이동, 3D 랜드마크 추적
    // 각 프레임에서 PnP로 포즈 추정 → GT와 비교
    // 참고: basic.cpp의 demoVisualOdometry()
}

void PnPBasic::demoPipeline(const cv::Mat& K)
{
    // [Step 8] 전체 파이프라인 (Step 1~7 조합)
    // 1) 3D 점 생성 → 2) GT 포즈 → 3) 노이즈 관측
    // 4) PnP → 5) RANSAC PnP → 6) 비교
    // 참고: basic.cpp의 demoPipeline()
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] PnP - 단계별 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);

    // GT 포즈
    cv::Mat rvec_gt = (cv::Mat_<double>(3, 1) << 0.1, 0.05, 0.0);
    cv::Mat tvec_gt = (cv::Mat_<double>(3, 1) << 0.5, 0.2, 0.1);

    // 3D 점 + 2D 관측 생성
    std::vector<cv::Point3f> pts3d;
    for (int i = 0; i < 20; i++)
    {
        pts3d.push_back(cv::Point3f(-2.0f + (rand() % 40) / 10.0f,
                                    -1.5f + (rand() % 30) / 10.0f,
                                    2.0f + (rand() % 60) / 10.0f));
    }

    std::vector<cv::Point2f> pts2d;
    cv::projectPoints(pts3d, rvec_gt, tvec_gt, K, cv::Mat::zeros(4, 1, CV_64F), pts2d);

    // ── Step 1: rodrigues ────────────────────────
    std::cout << "Step 1: rodrigues" << std::endl;
    cv::Mat R;
    PnPBasic::rodrigues(rvec_gt, R);
    if (!R.empty() && R.rows == 3 && R.cols == 3)
    {
        double det = cv::determinant(R);
        std::cout << "   R: 3×3, det=" << std::fixed << std::setprecision(4) << det
                  << (std::abs(det - 1.0) < 0.01 ? " ✅" : " ❌ 기대: ~1.0") << std::endl;
    }
    else
    {
        std::cout << "   R 비어있음 ❌" << std::endl;
    }

    // ── Step 2: solvePnP ─────────────────────────
    std::cout << "\nStep 2: solvePnP" << std::endl;
    cv::Mat rvec_est, tvec_est;
    bool pnp_ok = PnPBasic::solvePnP(pts3d, pts2d, K, rvec_est, tvec_est);
    std::cout << "   PnP: " << (pnp_ok ? "성공" : "실패") << (pnp_ok ? " ✅" : " ❌") << std::endl;
    if (pnp_ok)
    {
        double t_err = cv::norm(tvec_gt - tvec_est);
        std::cout << "   t 오차: " << std::setprecision(4) << t_err
                  << (t_err < 0.01 ? " ✅ (< 0.01)" : " ❌ 기대: < 0.01") << std::endl;
    }

    // ── Step 3: solvePnPRansac ───────────────────
    std::cout << "\nStep 3: solvePnPRansac" << std::endl;
    // outlier 추가
    std::vector<cv::Point2f> pts2d_noisy = pts2d;
    for (int i = 0; i < 3; i++)
    {
        pts2d_noisy[i].x += 50;
        pts2d_noisy[i].y += 50;
    }
    cv::Mat rvec_r, tvec_r;
    std::vector<int> inliers;
    int n_inliers = PnPBasic::solvePnPRansac(pts3d, pts2d_noisy, K, rvec_r, tvec_r, inliers, 5.0);
    std::cout << "   Inliers: " << n_inliers << "/" << pts3d.size()
              << (n_inliers > 0 && n_inliers <= static_cast<int>(pts3d.size()) ? " ✅" : " ❌")
              << std::endl;

    // ── Step 4: evaluatePose ─────────────────────
    std::cout << "\nStep 4: evaluatePose" << std::endl;
    if (pnp_ok)
    {
        double err = PnPBasic::evaluatePose(pts3d, pts2d, K, rvec_est, tvec_est);
        std::cout << "   재투영 오차: " << std::setprecision(4) << err
                  << (err >= 0 && err < 1.0 ? " px ✅" : " px ❌ 기대: < 1.0") << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  Step 2 먼저 구현 필요" << std::endl;
    }

    // ── Step 5: visualizePnP ─────────────────────
    std::cout << "\nStep 5: visualizePnP" << std::endl;
    if (pnp_ok)
    {
        cv::Mat img = cv::Mat::zeros(600, 800, CV_8UC1);
        cv::Mat vis;
        PnPBasic::visualizePnP(img, pts3d, pts2d, K, rvec_est, tvec_est, vis);
        std::cout << "   출력 이미지: "
                  << (vis.empty() ? "비어있음 ❌" : std::to_string(vis.cols) + "×"
                                                          + std::to_string(vis.rows) + " ✅")
                  << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  Step 2 먼저 구현 필요" << std::endl;
    }

    // ── Step 6: compareMethods ───────────────────
    std::cout << "\nStep 6: compareMethods" << std::endl;
    PnPBasic::compareMethods();
    std::cout << "   (위 출력 확인)" << std::endl;

    // ── Step 7: demoVisualOdometry ───────────────
    std::cout << "\nStep 7: demoVisualOdometry" << std::endl;
    PnPBasic::demoVisualOdometry(K);
    std::cout << "   (위 출력 확인)" << std::endl;

    // ── Step 8: demoPipeline ─────────────────────
    std::cout << "\nStep 8: demoPipeline" << std::endl;
    PnPBasic::demoPipeline(K);
    std::cout << "   (위 출력 확인)" << std::endl;

    // ── 요약 ────────────────────────────────────
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  모든 Step ✅ 확인 후 → quiz_easy.cpp, quiz_medium.cpp" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
