#include "mini_vo.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════
// 구현부
// ═══════════════════════════════════════════════════════════════

std::vector<cv::Point2f> MiniVOBasic::detect_features(
    const cv::Mat& frame, int max_corners)
{
    std::vector<cv::Point2f> corners;
    cv::goodFeaturesToTrack(
        frame, corners,
        max_corners,
        0.01,   // qualityLevel
        10      // minDistance
    );
    return corners;
}

void MiniVOBasic::track_features(
    const cv::Mat& prev, const cv::Mat& curr,
    const std::vector<cv::Point2f>& prev_pts,
    std::vector<cv::Point2f>& curr_pts,
    std::vector<uchar>& status)
{
    std::vector<float> err;
    cv::calcOpticalFlowPyrLK(prev, curr, prev_pts, curr_pts, status, err);
}

bool MiniVOBasic::initialize_2d2d(
    const std::vector<cv::Point2f>& pts1,
    const std::vector<cv::Point2f>& pts2,
    const cv::Mat& K,
    cv::Mat& R, cv::Mat& t,
    std::vector<cv::Point3f>& map_points,
    std::vector<cv::Point2f>& map_keypoints)
{
    const int kMinInliers = 20;

    // Essential Matrix
    cv::Mat inlier_mask;
    cv::Mat E = cv::findEssentialMat(pts1, pts2, K, cv::RANSAC, 0.999, 1.0, inlier_mask);

    // R, t 복원
    int good = cv::recoverPose(E, pts1, pts2, K, R, t, inlier_mask);
    if (good < kMinInliers)
    {
        return false;
    }

    // 삼각측량: P1 = K[I|0], P2 = K[R|t]
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));

    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R.copyTo(RT(cv::Rect(0, 0, 3, 3)));
    t.copyTo(RT(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT;

    // inlier만 삼각측량
    std::vector<cv::Point2f> inlier_pts1, inlier_pts2;
    for (size_t i = 0; i < pts1.size(); i++)
    {
        if (inlier_mask.at<uchar>(i))
        {
            inlier_pts1.push_back(pts1[i]);
            inlier_pts2.push_back(pts2[i]);
        }
    }

    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, inlier_pts1, inlier_pts2, points4D);

    // 3D 점 추출 (depth > 0) + 대응 2D 키포인트
    map_points.clear();
    map_keypoints.clear();
    for (int i = 0; i < points4D.cols; i++)
    {
        float w = points4D.at<float>(3, i);
        if (std::abs(w) < 1e-6)
            continue;
        float x = points4D.at<float>(0, i) / w;
        float y = points4D.at<float>(1, i) / w;
        float z = points4D.at<float>(2, i) / w;
        if (z > 0.1 && z < 200.0)
        {
            map_points.push_back(cv::Point3f(x, y, z));
            map_keypoints.push_back(inlier_pts2[i]);
        }
    }

    return map_points.size() >= 10;
}

int MiniVOBasic::track_pnp(
    const std::vector<cv::Point3f>& pts3d,
    const std::vector<cv::Point2f>& pts2d,
    const cv::Mat& K,
    cv::Mat& R, cv::Mat& t,
    std::vector<int>& inliers)
{
    if (pts3d.size() < 4)
        return 0;

    cv::Mat rvec, tvec;
    bool ok = cv::solvePnPRansac(
        pts3d, pts2d, K, cv::noArray(),
        rvec, tvec,
        false, 100, 8.0, 0.99,
        inliers);

    if (!ok || inliers.empty())
        return 0;

    cv::Rodrigues(rvec, R);
    t = tvec.clone();
    return static_cast<int>(inliers.size());
}

std::vector<cv::Point3f> MiniVOBasic::triangulate_new_points(
    const std::vector<cv::Point2f>& pts1,
    const std::vector<cv::Point2f>& pts2,
    const cv::Mat& K,
    const cv::Mat& R1, const cv::Mat& t1,
    const cv::Mat& R2, const cv::Mat& t2)
{
    // P = K[R|t]
    cv::Mat RT1 = cv::Mat::zeros(3, 4, CV_64F);
    R1.copyTo(RT1(cv::Rect(0, 0, 3, 3)));
    t1.copyTo(RT1(cv::Rect(3, 0, 1, 3)));
    cv::Mat P1 = K * RT1;

    cv::Mat RT2 = cv::Mat::zeros(3, 4, CV_64F);
    R2.copyTo(RT2(cv::Rect(0, 0, 3, 3)));
    t2.copyTo(RT2(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT2;

    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, points4D);

    std::vector<cv::Point3f> result;
    for (int i = 0; i < points4D.cols; i++)
    {
        float w = points4D.at<float>(3, i);
        if (std::abs(w) < 1e-6)
            continue;
        float x = points4D.at<float>(0, i) / w;
        float y = points4D.at<float>(1, i) / w;
        float z = points4D.at<float>(2, i) / w;
        if (z > 0.1 && z < 200.0)
        {
            result.push_back(cv::Point3f(x, y, z));
        }
    }
    return result;
}

double MiniVOBasic::compute_trajectory_error(
    const std::vector<cv::Mat>& est_t,
    const std::vector<cv::Mat>& gt_t)
{
    int n = std::min(est_t.size(), gt_t.size());
    if (n == 0)
        return -1.0;

    double sum_sq = 0.0;
    for (int i = 0; i < n; i++)
    {
        sum_sq += cv::norm(est_t[i] - gt_t[i], cv::NORM_L2SQR);
    }
    return std::sqrt(sum_sq / n);
}

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

    // 3D 월드 점: 카메라 전방에 분포
    // 카메라가 +Z 방향으로 전진하므로 점들은 Z > 0 영역에 배치
    std::vector<cv::Point3f> world_pts;
    srand(42);
    for (int i = 0; i < 500; i++)
    {
        float x = (rand() % 200 - 100) / 10.0f;   // [-10, 10]
        float y = (rand() % 100 - 50) / 10.0f;     // [-5, 5]
        float z = 3.0f + (rand() % 500) / 10.0f;   // [3, 53] — 전방 넓은 범위
        world_pts.push_back(cv::Point3f(x, y, z));
    }

    // 전진 궤적 (KITTI 스타일: +Z 방향 이동 + 약간의 회전)
    for (int i = 0; i < num_frames; i++)
    {
        double progress = i * 0.5;  // 프레임당 0.5m 전진
        double yaw = i * 0.005;     // 프레임당 약 0.3° 회전 (서서히 곡선)

        // 카메라 위치
        cv::Mat t = (cv::Mat_<double>(3, 1) <<
            progress * std::sin(yaw), 0.0, progress * std::cos(yaw));

        // Y축 회전 (미세 회전)
        cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
        R.at<double>(0, 0) = std::cos(yaw);
        R.at<double>(0, 2) = std::sin(yaw);
        R.at<double>(2, 0) = -std::sin(yaw);
        R.at<double>(2, 2) = std::cos(yaw);

        gt_R.push_back(R.clone());
        gt_t.push_back(t.clone());

        // 가상 이미지 생성 (3D → 2D 투영)
        // Pc = R * (Pw - t) — 전진 궤적에서 대부분 점이 카메라 앞에 위치
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

        // 약간의 노이즈
        cv::Mat noise(image.size(), CV_8UC1);
        cv::randn(noise, 0, 2);
        image += noise;
        images.push_back(image);
    }
}

// ═══════════════════════════════════════════════════════════════
// 교육 블록
// ═══════════════════════════════════════════════════════════════

// ── 블록 1: VO 파이프라인 구조 ──
static void demoVOPipeline()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 1] VO 파이프라인 = 초기화 + 추적" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_easy Q1 (VO 파이프라인 2단계) 연결
    std::cout << "VO 시스템 상태 전이:" << std::endl;
    std::cout << "  INITIALIZING ──[2D-2D 성공]──→ TRACKING" << std::endl;
    std::cout << "  TRACKING ──[inlier < 30%]──→ LOST" << std::endl;
    std::cout << "  LOST ──[재초기화]──→ INITIALIZING\n" << std::endl;

    // 💡 quiz_easy Q2 (초기화 vs 추적) 연결
    std::cout << "초기화 (Week 2): Essential Matrix" << std::endl;
    std::cout << "  입력: 2D-2D 대응 (첫 두 프레임)" << std::endl;
    std::cout << "  출력: R, t (||t||=1) + 초기 3D 맵" << std::endl;
    std::cout << "  조건: 시차 > 1.5 px, inlier > 50%\n" << std::endl;

    std::cout << "추적 (Week 3): PnP + RANSAC" << std::endl;
    std::cout << "  입력: 3D 맵 + 현재 2D 관측" << std::endl;
    std::cout << "  출력: 현재 포즈 (R, t)" << std::endl;
    std::cout << "  장점: 스케일 유지 (3D 정보 사용)\n" << std::endl;

    // 수치 예시: 합성 데이터로 초기화
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);
    std::vector<cv::Mat> images, gt_R, gt_t;
    MiniVOBasic::generate_synthetic_sequence(K, 5, images, gt_R, gt_t);

    // 프레임 0, 1로 초기화
    auto kp0 = MiniVOBasic::detect_features(images[0]);
    std::vector<cv::Point2f> kp1;
    std::vector<uchar> status;
    MiniVOBasic::track_features(images[0], images[1], kp0, kp1, status);

    // 유효한 매칭만 추출
    std::vector<cv::Point2f> matched0, matched1;
    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i])
        {
            matched0.push_back(kp0[i]);
            matched1.push_back(kp1[i]);
        }
    }

    cv::Mat R, t;
    std::vector<cv::Point3f> map_pts;
    std::vector<cv::Point2f> map_kp;
    bool ok = MiniVOBasic::initialize_2d2d(matched0, matched1, K, R, t, map_pts, map_kp);

    std::cout << "초기화 결과:" << std::endl;
    std::cout << "  특징점: " << kp0.size() << " → 매칭: " << matched0.size()
              << " → 맵: " << map_pts.size() << "개" << std::endl;
    std::cout << "  ||t|| = " << cv::norm(t) << " (정규화됨 = 스케일 모호)"
              << std::endl;
    std::cout << "  초기화 " << (ok ? "성공" : "실패") << std::endl;
}

// ── 블록 2: 추적과 드리프트 ──
static void demoDriftAccumulation()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 2] 추적 + 드리프트 누적" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_easy Q3 (드리프트 원인), quiz_medium Q1 (inlier ratio) 연결
    std::cout << "드리프트 원인:" << std::endl;
    std::cout << "  매 프레임 작은 오차 → 누적 → 발산" << std::endl;
    std::cout << "  예: 프레임당 0.1% 오차 × 1000 프레임 = ~100% 오차\n" << std::endl;

    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);
    const int kNumFrames = 30;
    std::vector<cv::Mat> images, gt_R, gt_t;
    MiniVOBasic::generate_synthetic_sequence(K, kNumFrames, images, gt_R, gt_t);

    // 초기화
    auto kp0 = MiniVOBasic::detect_features(images[0]);
    std::vector<cv::Point2f> kp1;
    std::vector<uchar> status;
    MiniVOBasic::track_features(images[0], images[1], kp0, kp1, status);

    std::vector<cv::Point2f> matched0, matched1;
    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i])
        {
            matched0.push_back(kp0[i]);
            matched1.push_back(kp1[i]);
        }
    }

    cv::Mat R, t;
    std::vector<cv::Point3f> map_pts;
    std::vector<cv::Point2f> map_kp;
    bool init_ok = MiniVOBasic::initialize_2d2d(matched0, matched1, K, R, t, map_pts, map_kp);
    if (!init_ok || map_pts.empty())
    {
        std::cout << "  초기화 실패 — 합성 데이터 재생성 필요" << std::endl;
        return;
    }

    // 추적 루프
    std::vector<cv::Mat> est_t_list;
    est_t_list.push_back(cv::Mat::zeros(3, 1, CV_64F));  // 프레임 0
    est_t_list.push_back(t.clone());                       // 프레임 1

    // map_kp: map_pts에 대응하는 2D 키포인트 (인덱스 일치)
    std::vector<cv::Point2f> prev_2d = map_kp;
    std::vector<cv::Point3f> curr_map = map_pts;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "프레임별 추적:" << std::endl;

    for (int f = 2; f < kNumFrames; f++)
    {
        // Optical Flow
        std::vector<cv::Point2f> curr_kp;
        std::vector<uchar> flow_status;
        MiniVOBasic::track_features(images[f - 1], images[f], prev_2d, curr_kp, flow_status);

        // 유효한 대응만
        std::vector<cv::Point3f> valid_3d;
        std::vector<cv::Point2f> valid_2d;
        for (size_t i = 0; i < flow_status.size(); i++)
        {
            if (flow_status[i] && i < curr_map.size())
            {
                valid_3d.push_back(curr_map[i]);
                valid_2d.push_back(curr_kp[i]);
            }
        }

        // PnP
        cv::Mat R_new, t_new;
        std::vector<int> inliers;
        int n_inlier = MiniVOBasic::track_pnp(valid_3d, valid_2d, K, R_new, t_new, inliers);

        if (n_inlier > 0)
        {
            est_t_list.push_back(t_new.clone());

            double drift = cv::norm(t_new - gt_t[f]);
            double ratio = (double)n_inlier / valid_3d.size() * 100;

            if (f % 5 == 0 || f == kNumFrames - 1)
            {
                std::cout << "  Frame " << std::setw(2) << f
                          << ": inlier=" << n_inlier
                          << "/" << valid_3d.size()
                          << " (" << std::setw(4) << ratio << "%)"
                          << "  drift=" << drift << "m" << std::endl;
            }

            // 다음 프레임용 업데이트 (inlier만)
            curr_map.clear();
            prev_2d.clear();
            for (int idx : inliers)
            {
                curr_map.push_back(valid_3d[idx]);
                prev_2d.push_back(valid_2d[idx]);
            }
        }
        else
        {
            std::cout << "  Frame " << f << ": 추적 실패 (LOST)" << std::endl;
            break;
        }
    }

    // ATE 계산
    // est_t_list와 gt_t의 크기를 맞춤
    std::vector<cv::Mat> gt_subset(gt_t.begin(), gt_t.begin() + est_t_list.size());
    double ate = MiniVOBasic::compute_trajectory_error(est_t_list, gt_subset);
    double total_dist = cv::norm(gt_t.back());
    std::cout << "\n  ATE RMSE: " << ate << " m" << std::endl;
    if (total_dist > 0)
    {
        std::cout << "  드리프트 비율: " << (ate / total_dist * 100) << "%" << std::endl;
    }
}

// ── 블록 3: 삼각측량과 맵 관리 ──
static void demoTriangulationAndMap()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 3] 삼각측량 + 맵 관리" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 💡 quiz_easy Q4 (맵 관리), quiz_medium Q2 (삼각측량 조건) 연결
    std::cout << "삼각측량 품질 조건:" << std::endl;
    std::cout << "  1. 충분한 시차 (> 1.5 px)" << std::endl;
    std::cout << "  2. Depth > 0 (카메라 앞)" << std::endl;
    std::cout << "  3. 재투영 오차 < 3 px\n" << std::endl;

    std::cout << "맵 관리 전략:" << std::endl;
    std::cout << "  추가: Keyframe에서만 새 점 생성" << std::endl;
    std::cout << "  제거: 나이 > 30 프레임, 관측 부족" << std::endl;
    std::cout << "  목표: 적절한 맵 크기 유지 (500-1000점)\n" << std::endl;

    // 수치 시연: 시차에 따른 삼각측량 정확도
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    std::cout << "시차 vs 삼각측량 정확도 (GT depth=5m):" << std::endl;

    cv::Mat R1 = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t1 = cv::Mat::zeros(3, 1, CV_64F);

    // baseline을 바꾸며 테스트
    double baselines[] = {0.01, 0.05, 0.1, 0.5, 1.0};
    for (double b : baselines)
    {
        cv::Mat R2 = cv::Mat::eye(3, 3, CV_64F);
        cv::Mat t2 = (cv::Mat_<double>(3, 1) << b, 0, 0);

        // GT 3D 점: (0, 0, 5)
        double gt_z = 5.0;
        cv::Point2f p1(320, 240);  // 이미지 중심
        cv::Point2f p2(static_cast<float>(320 + 500 * b / gt_z), 240);

        double parallax = std::abs(p2.x - p1.x);

        std::vector<cv::Point2f> pts1 = {p1};
        std::vector<cv::Point2f> pts2 = {p2};
        auto result = MiniVOBasic::triangulate_new_points(pts1, pts2, K, R1, t1, R2, t2);

        if (!result.empty())
        {
            double err = std::abs(result[0].z - gt_z);
            std::cout << "  baseline=" << std::setw(4) << b
                      << "m  parallax=" << std::setw(5) << parallax
                      << "px  depth_err=" << err << "m" << std::endl;
        }
        else
        {
            std::cout << "  baseline=" << b << "m  삼각측량 실패" << std::endl;
        }
    }
}

// ═══════════════════════════════════════════════════════════════
// 전체 데모 + main
// ═══════════════════════════════════════════════════════════════

void MiniVOBasic::demo()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Week 5: Mini VO — 완전한 VO 파이프라인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    std::cout << "\nWeek 2 (초기화) + Week 3 (추적) = 완전한 VO\n" << std::endl;

    demoVOPipeline();
    demoDriftAccumulation();
    demoTriangulationAndMap();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  정리" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "VO 한계:" << std::endl;
    std::cout << "  1. 드리프트 불가피 (오차 누적)" << std::endl;
    std::cout << "  2. 스케일 모호 (||t||=1)" << std::endl;
    std::cout << "  3. Loop Closure 없음\n" << std::endl;

    std::cout << "해결 방향:" << std::endl;
    std::cout << "  Week 6: Keyframe 관리" << std::endl;
    std::cout << "  Week 7-9: BA로 드리프트 감소" << std::endl;
    std::cout << "  Week 12-13: 스케일 복구" << std::endl;

    std::cout << "\n다음 단계:" << std::endl;
    std::cout << "  1. ./my_basic 으로 직접 구현 연습" << std::endl;
    std::cout << "  2. quiz_easy / quiz_medium 풀기" << std::endl;
}

int main()
{
    MiniVOBasic::demo();
    return 0;
}
