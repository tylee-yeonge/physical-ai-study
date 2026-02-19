/**
 * Phase 3 Week 5 - Mini VO 직접 구현
 *
 * mini_vo.h의 함수들을 직접 구현해보세요.
 *
 * ┌──────┬──────────────────────────┬────────┬──────────────┐
 * │ Step │ 함수                     │ 난이도  │ 검증 방법     │
 * ├──────┼──────────────────────────┼────────┼──────────────┤
 * │  1   │ detect_features          │ 쉬움   │ ./my_basic   │
 * │  2   │ track_features           │ 쉬움   │ ./my_basic   │
 * │  3   │ initialize_2d2d          │ 핵심   │ ./my_basic   │
 * │  4   │ track_pnp                │ 핵심   │ ./my_basic   │
 * │  5   │ triangulate_new_points   │ 어려움  │ ./my_basic   │
 * │  6   │ compute_trajectory_error │ 쉬움   │ ./my_basic   │
 * │  7   │ demo (전체 통합)          │ 도전   │ ./my_basic   │
 * └──────┴──────────────────────────┴────────┴──────────────┘
 */
#include "mini_vo.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

// generate_synthetic_sequence — 테스트용 (그대로 제공)
void MiniVOBasic::generate_synthetic_sequence(
    const cv::Mat& K, int num_frames,
    std::vector<cv::Mat>& images,
    std::vector<cv::Mat>& gt_R,
    std::vector<cv::Mat>& gt_t)
{
    double fx = K.at<double>(0, 0);
    double fy = K.at<double>(1, 1);
    double cx = K.at<double>(0, 2);
    double cy = K.at<double>(1, 2);
    int width = static_cast<int>(cx * 2);
    int height = static_cast<int>(cy * 2);

    std::vector<cv::Point3f> world_pts;
    srand(42);
    for (int i = 0; i < 500; i++)
    {
        float x = (rand() % 200 - 100) / 10.0f;
        float y = (rand() % 100 - 50) / 10.0f;
        float z = 3.0f + (rand() % 500) / 10.0f;
        world_pts.push_back(cv::Point3f(x, y, z));
    }

    for (int i = 0; i < num_frames; i++)
    {
        double progress = i * 0.5;
        double yaw = i * 0.005;

        cv::Mat t = (cv::Mat_<double>(3, 1) <<
            progress * std::sin(yaw), 0.0, progress * std::cos(yaw));

        cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
        R.at<double>(0, 0) = std::cos(yaw);
        R.at<double>(0, 2) = std::sin(yaw);
        R.at<double>(2, 0) = -std::sin(yaw);
        R.at<double>(2, 2) = std::cos(yaw);

        gt_R.push_back(R.clone());
        gt_t.push_back(t.clone());

        cv::Mat image = cv::Mat::zeros(height, width, CV_8UC1);
        for (const auto& pt : world_pts)
        {
            cv::Mat Pw = (cv::Mat_<double>(3, 1) << pt.x, pt.y, pt.z);
            cv::Mat Pc = R * (Pw - t);
            double Zc = Pc.at<double>(2);
            if (Zc > 0.5)
            {
                int u = static_cast<int>(fx * Pc.at<double>(0) / Zc + cx);
                int v = static_cast<int>(fy * Pc.at<double>(1) / Zc + cy);
                if (u >= 4 && u < width - 4 && v >= 4 && v < height - 4)
                {
                    cv::circle(image, cv::Point(u, v), 4,
                               cv::Scalar(160 + rand() % 95), -1);
                }
            }
        }

        cv::Mat noise(image.size(), CV_8UC1);
        cv::randn(noise, 0, 2);
        image += noise;
        images.push_back(image);
    }
}

// [Step 1] detect_features — GFTT 특징점 검출
// 힌트: cv::goodFeaturesToTrack(frame, corners, max_corners, 0.01, 10)
// 기대값: 합성 이미지에서 50~300개 검출
std::vector<cv::Point2f> MiniVOBasic::detect_features(
    const cv::Mat& frame, int max_corners)
{
    // TODO: goodFeaturesToTrack으로 코너 검출 → 반환
    return {};
}

// [Step 2] track_features — Optical Flow 추적
// 힌트: cv::calcOpticalFlowPyrLK(prev, curr, prev_pts, curr_pts, status, err)
// 기대값: 대부분의 점이 status[i] = 1
void MiniVOBasic::track_features(
    const cv::Mat& prev, const cv::Mat& curr,
    const std::vector<cv::Point2f>& prev_pts,
    std::vector<cv::Point2f>& curr_pts,
    std::vector<uchar>& status)
{
    // TODO: LK Optical Flow 호출
}

// [Step 3] initialize_2d2d — Essential Matrix 초기화 (Week 2 통합, 핵심!)
// 힌트:
//   1. cv::findEssentialMat(pts1, pts2, K, RANSAC, 0.999, 1.0, mask)
//   2. cv::recoverPose(E, pts1, pts2, K, R, t, mask)
//   3. 삼각측량: P1 = K[I|0], P2 = K[R|t]
//   4. cv::triangulatePoints(P1, P2, inlier_pts1, inlier_pts2, points4D)
//   5. 동차 → 3D: (x,y,z) = (X/W, Y/W, Z/W),  z > 0.1 필터
//   6. map_keypoints에 유효한 3D 점에 대응하는 pts2 저장
// 참고: basic.cpp initialize_2d2d()
// 기대값: map_points.size() >= 10이면 true
bool MiniVOBasic::initialize_2d2d(
    const std::vector<cv::Point2f>& pts1,
    const std::vector<cv::Point2f>& pts2,
    const cv::Mat& K,
    cv::Mat& R, cv::Mat& t,
    std::vector<cv::Point3f>& map_points,
    std::vector<cv::Point2f>& map_keypoints)
{
    // TODO: 위 6단계 구현
    return false;
}

// [Step 4] track_pnp — PnP 추적 (Week 3 통합, 핵심!)
// 힌트:
//   1. pts3d.size() < 4이면 return 0
//   2. cv::solvePnPRansac(pts3d, pts2d, K, noArray(), rvec, tvec,
//                          false, 100, 8.0, 0.99, inliers)
//   3. cv::Rodrigues(rvec, R)
//   4. t = tvec.clone()
// 참고: basic.cpp track_pnp()
// 기대값: inlier > 0이면 성공
int MiniVOBasic::track_pnp(
    const std::vector<cv::Point3f>& pts3d,
    const std::vector<cv::Point2f>& pts2d,
    const cv::Mat& K,
    cv::Mat& R, cv::Mat& t,
    std::vector<int>& inliers)
{
    // TODO: PnP + RANSAC 구현
    return 0;
}

// [Step 5] triangulate_new_points — 새 3D 점 삼각측량
// 힌트:
//   1. P1 = K[R1|t1], P2 = K[R2|t2]
//   2. cv::triangulatePoints(P1, P2, pts1, pts2, points4D)
//   3. 동차 → 3D 변환 + depth 필터 (0.1 < z < 200)
// 참고: basic.cpp triangulate_new_points()
// 기대값: 유효한 3D 점 벡터 반환
std::vector<cv::Point3f> MiniVOBasic::triangulate_new_points(
    const std::vector<cv::Point2f>& pts1,
    const std::vector<cv::Point2f>& pts2,
    const cv::Mat& K,
    const cv::Mat& R1, const cv::Mat& t1,
    const cv::Mat& R2, const cv::Mat& t2)
{
    // TODO: 삼각측량 구현
    return {};
}

// [Step 6] compute_trajectory_error — ATE RMSE 계산
// 힌트: sqrt(Σ ||est_t[i] - gt_t[i]||² / n)
// 기대값: est == gt → 0,  드리프트 있으면 > 0
double MiniVOBasic::compute_trajectory_error(
    const std::vector<cv::Mat>& est_t,
    const std::vector<cv::Mat>& gt_t)
{
    // TODO: 거리의 제곱합 → 평균 → sqrt
    return -1.0;
}

// demo는 Step 7에서 직접 main에 작성

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] Mini VO 파이프라인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    // 합성 데이터 생성
    const int kNumFrames = 20;
    std::vector<cv::Mat> images, gt_R, gt_t;
    MiniVOBasic::generate_synthetic_sequence(K, kNumFrames, images, gt_R, gt_t);
    std::cout << "합성 데이터: " << kNumFrames << " 프레임 생성\n" << std::endl;

    // ── Step 1 테스트: detect_features ──
    std::cout << "[Step 1] detect_features" << std::endl;
    auto kp0 = MiniVOBasic::detect_features(images[0]);
    bool step1 = (kp0.size() > 10);
    std::cout << "  특징점: " << kp0.size()
              << (step1 ? " ✅" : " ❌ (기대: > 10)") << "\n" << std::endl;

    // ── Step 2 테스트: track_features ──
    std::cout << "[Step 2] track_features" << std::endl;
    std::vector<cv::Point2f> kp1;
    std::vector<uchar> status;
    MiniVOBasic::track_features(images[0], images[1], kp0, kp1, status);
    int tracked = 0;
    for (uchar s : status)
    {
        if (s)
            tracked++;
    }
    bool step2 = (tracked > 10);
    std::cout << "  추적: " << tracked << "/" << kp0.size()
              << (step2 ? " ✅" : " ❌ (기대: > 10)") << "\n" << std::endl;

    // 유효한 매칭 추출
    std::vector<cv::Point2f> matched0, matched1;
    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i])
        {
            matched0.push_back(kp0[i]);
            matched1.push_back(kp1[i]);
        }
    }

    // ── Step 3 테스트: initialize_2d2d ──
    std::cout << "[Step 3] initialize_2d2d" << std::endl;
    cv::Mat R, t;
    std::vector<cv::Point3f> map_pts;
    std::vector<cv::Point2f> map_kp;
    bool step3 = MiniVOBasic::initialize_2d2d(matched0, matched1, K, R, t, map_pts, map_kp);
    std::cout << "  맵: " << map_pts.size() << "개, ||t||=" << cv::norm(t)
              << (step3 ? " ✅" : " ❌ (기대: map >= 10)") << "\n" << std::endl;

    // ── Step 4 테스트: track_pnp ──
    std::cout << "[Step 4] track_pnp" << std::endl;
    bool step4 = false;
    if (step3 && !map_pts.empty())
    {
        // 프레임 2에서 추적 (map_kp 사용: map_pts와 인덱스 대응)
        std::vector<cv::Point2f> kp2;
        std::vector<uchar> status2;
        MiniVOBasic::track_features(images[1], images[2], map_kp, kp2, status2);

        std::vector<cv::Point3f> valid_3d;
        std::vector<cv::Point2f> valid_2d;
        for (size_t i = 0; i < status2.size(); i++)
        {
            if (status2[i])
            {
                valid_3d.push_back(map_pts[i]);
                valid_2d.push_back(kp2[i]);
            }
        }

        cv::Mat R2, t2;
        std::vector<int> inliers;
        int n = MiniVOBasic::track_pnp(valid_3d, valid_2d, K, R2, t2, inliers);
        step4 = (n > 0);
        std::cout << "  inlier: " << n << "/" << valid_3d.size()
                  << (step4 ? " ✅" : " ❌ (기대: inlier > 0)") << "\n" << std::endl;
    }
    else
    {
        std::cout << "  건너뜀 (Step 3 미완)\n" << std::endl;
    }

    // ── Step 5 테스트: triangulate_new_points ──
    std::cout << "[Step 5] triangulate_new_points" << std::endl;
    cv::Mat R1_eye = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t1_zero = cv::Mat::zeros(3, 1, CV_64F);
    cv::Mat R2_test = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t2_test = (cv::Mat_<double>(3, 1) << 0.5, 0, 0);

    // 간단한 점 쌍으로 테스트
    std::vector<cv::Point2f> test_p1 = {cv::Point2f(320, 240)};
    std::vector<cv::Point2f> test_p2 = {cv::Point2f(330, 240)};
    auto tri_pts = MiniVOBasic::triangulate_new_points(
        test_p1, test_p2, K, R1_eye, t1_zero, R2_test, t2_test);
    bool step5 = (!tri_pts.empty() && tri_pts[0].z > 0);
    if (!tri_pts.empty())
    {
        std::cout << "  결과: (" << tri_pts[0].x << ", "
                  << tri_pts[0].y << ", " << tri_pts[0].z << ")"
                  << (step5 ? " ✅" : " ❌") << "\n" << std::endl;
    }
    else
    {
        std::cout << "  결과: 없음 ❌\n" << std::endl;
    }

    // ── Step 6 테스트: compute_trajectory_error ──
    std::cout << "[Step 6] compute_trajectory_error" << std::endl;
    cv::Mat t_a = (cv::Mat_<double>(3, 1) << 1.0, 0, 0);
    cv::Mat t_b = (cv::Mat_<double>(3, 1) << 1.1, 0.05, 0);
    std::vector<cv::Mat> est_list = {t_a};
    std::vector<cv::Mat> gt_list = {t_a};
    double ate_zero = MiniVOBasic::compute_trajectory_error(est_list, gt_list);
    est_list = {t_b};
    double ate_diff = MiniVOBasic::compute_trajectory_error(est_list, gt_list);
    bool step6 = (ate_zero < 1e-10 && ate_diff > 0);
    std::cout << "  동일 궤적 ATE: " << ate_zero
              << (ate_zero < 1e-10 ? " ✅" : " ❌") << std::endl;
    std::cout << "  다른 궤적 ATE: " << ate_diff
              << (ate_diff > 0 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── 종합 결과 ──
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  종합: Step1 " << (step1 ? "✅" : "❌")
              << " Step2 " << (step2 ? "✅" : "❌")
              << " Step3 " << (step3 ? "✅" : "❌")
              << " Step4 " << (step4 ? "✅" : "❌")
              << " Step5 " << (step5 ? "✅" : "❌")
              << " Step6 " << (step6 ? "✅" : "❌") << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "\n[Step 7] 도전: 전체 VO 루프 직접 작성!" << std::endl;
    std::cout << "  초기화 → 추적 루프 → ATE 출력" << std::endl;
    std::cout << "  basic.cpp demo()를 참고하세요.\n" << std::endl;

    return 0;
}
#endif
