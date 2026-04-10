/**
 * @file basic.cpp
 * @brief Week 3: Multi-view 기하 + Stereo Rectification 데모
 *
 * 파이프라인:
 *   1. 합성 스테레오 쌍 생성 (또는 샘플 이미지 로드)
 *   2. 특징점 매칭 → Fundamental Matrix 계산
 *   3. 에피폴라 선 시각화
 *   4. Stereo Rectification → rectified 쌍
 *   5. Disparity 계산 (StereoBM)
 *   6. Disparity → Depth 변환
 *   7. 결과 output/ 에 저장
 *
 * Perception 맥락:
 *   이 rectified pair 가 HITNet / CRE-Stereo 같은
 *   Stereo Depth 모델의 입력이 된다.
 */

#include "basic.h"
#include <filesystem>
#include <iostream>

// ─── 헬퍼 함수 구현 ───

void save_output(const std::string& name, const cv::Mat& image)
{
    std::filesystem::create_directories("output");
    std::string path = "output/" + name + ".png";
    cv::imwrite(path, image);
    std::cout << "   저장: " << path << std::endl;
}

// 합성 3D 씬으로 스테레오 쌍을 렌더링
void generate_synthetic_stereo(
    const cv::Size& image_size,
    cv::Mat& K1, cv::Mat& K2,
    cv::Mat& dist1, cv::Mat& dist2,
    cv::Mat& R, cv::Mat& T,
    cv::Mat& left, cv::Mat& right)
{
    // 공통 초점거리
    double fx = 500.0, fy = 500.0;
    double cx = image_size.width / 2.0;
    double cy = image_size.height / 2.0;

    K1 = (cv::Mat_<double>(3, 3) << fx, 0, cx, 0, fy, cy, 0, 0, 1);
    K2 = K1.clone();
    dist1 = cv::Mat::zeros(5, 1, CV_64F);
    dist2 = cv::Mat::zeros(5, 1, CV_64F);

    // 스테레오 baseline 0.12m (수평 이동만)
    double baseline = 0.12;
    R = cv::Mat::eye(3, 3, CV_64F);
    T = (cv::Mat_<double>(3, 1) << baseline, 0, 0);

    // 3D 점 생성 (깊이 3~10m 의 랜덤 점)
    std::vector<cv::Point3d> points_3d;
    cv::RNG rng(42);
    for (int i = 0; i < 200; i++)
    {
        double x = rng.uniform(-3.0, 3.0);
        double y = rng.uniform(-2.0, 2.0);
        double z = rng.uniform(3.0, 10.0);
        points_3d.emplace_back(x, y, z);
    }

    // 이미지 생성
    left = cv::Mat::zeros(image_size, CV_8UC1);
    right = cv::Mat::zeros(image_size, CV_8UC1);

    for (const auto& p : points_3d)
    {
        // 왼쪽 카메라 (원점)
        if (p.z > 0)
        {
            double u1 = fx * p.x / p.z + cx;
            double v1 = fy * p.y / p.z + cy;
            if (u1 >= 0 && u1 < image_size.width && v1 >= 0 && v1 < image_size.height)
            {
                cv::circle(left, cv::Point(static_cast<int>(u1), static_cast<int>(v1)),
                           3, cv::Scalar(255), -1);
            }
        }

        // 오른쪽 카메라 (baseline 만큼 오른쪽 이동)
        double x2 = p.x - baseline;
        if (p.z > 0)
        {
            double u2 = fx * x2 / p.z + cx;
            double v2 = fy * p.y / p.z + cy;
            if (u2 >= 0 && u2 < image_size.width && v2 >= 0 && v2 < image_size.height)
            {
                cv::circle(right, cv::Point(static_cast<int>(u2), static_cast<int>(v2)),
                           3, cv::Scalar(255), -1);
            }
        }
    }
}

cv::Mat draw_epipolar_lines(
    const cv::Mat& left, const cv::Mat& right,
    const std::vector<cv::Point2f>& pts1,
    const std::vector<cv::Point2f>& pts2,
    const std::string& output_path)
{
    // Fundamental Matrix 계산 (RANSAC)
    cv::Mat F = cv::findFundamentalMat(pts1, pts2, cv::FM_RANSAC, 3.0, 0.99);

    // 에피폴라 선 계산
    std::vector<cv::Vec3f> lines1, lines2;
    cv::computeCorrespondEpilines(pts2, 2, F, lines1);  // 오른쪽 점 → 왼쪽 에피폴라 선
    cv::computeCorrespondEpilines(pts1, 1, F, lines2);  // 왼쪽 점 → 오른쪽 에피폴라 선

    // 시각화
    cv::Mat vis_left, vis_right;
    cv::cvtColor(left, vis_left, cv::COLOR_GRAY2BGR);
    cv::cvtColor(right, vis_right, cv::COLOR_GRAY2BGR);

    cv::RNG rng(0);
    int draw_count = std::min(static_cast<int>(pts1.size()), 15);
    for (int i = 0; i < draw_count; i++)
    {
        cv::Scalar color(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));

        // 왼쪽 이미지에 에피폴라 선 + 점
        double a = lines1[i][0], b = lines1[i][1], c = lines1[i][2];
        cv::Point pt1(0, static_cast<int>(-c / b));
        cv::Point pt2(vis_left.cols, static_cast<int>(-(c + a * vis_left.cols) / b));
        cv::line(vis_left, pt1, pt2, color, 1);
        cv::circle(vis_left, pts1[i], 5, color, -1);

        // 오른쪽 이미지에 에피폴라 선 + 점
        a = lines2[i][0]; b = lines2[i][1]; c = lines2[i][2];
        pt1 = cv::Point(0, static_cast<int>(-c / b));
        pt2 = cv::Point(vis_right.cols, static_cast<int>(-(c + a * vis_right.cols) / b));
        cv::line(vis_right, pt1, pt2, color, 1);
        cv::circle(vis_right, pts2[i], 5, color, -1);
    }

    cv::Mat result;
    cv::hconcat(vis_left, vis_right, result);

    if (!output_path.empty())
    {
        save_output(output_path, result);
    }

    return F;
}

void stereo_rectify(
    const cv::Mat& left, const cv::Mat& right,
    const cv::Mat& K1, const cv::Mat& K2,
    const cv::Mat& dist1, const cv::Mat& dist2,
    const cv::Mat& R, const cv::Mat& T,
    const cv::Size& image_size,
    cv::Mat& rect_left, cv::Mat& rect_right,
    cv::Mat& Q)
{
    // Rectification 변환 행렬 계산
    cv::Mat R1, R2, P1, P2;
    cv::stereoRectify(K1, dist1, K2, dist2, image_size, R, T,
                      R1, R2, P1, P2, Q,
                      cv::CALIB_ZERO_DISPARITY, 0, image_size);

    // Undistort + Rectify 맵 생성
    cv::Mat map1x, map1y, map2x, map2y;
    cv::initUndistortRectifyMap(K1, dist1, R1, P1, image_size, CV_32FC1, map1x, map1y);
    cv::initUndistortRectifyMap(K2, dist2, R2, P2, image_size, CV_32FC1, map2x, map2y);

    // Remap 적용
    cv::remap(left, rect_left, map1x, map1y, cv::INTER_LINEAR);
    cv::remap(right, rect_right, map2x, map2y, cv::INTER_LINEAR);
}

cv::Mat compute_disparity(const cv::Mat& rect_left, const cv::Mat& rect_right)
{
    // 그레이스케일 변환 (이미 그레이면 그대로)
    cv::Mat gray_left, gray_right;
    if (rect_left.channels() == 3)
        cv::cvtColor(rect_left, gray_left, cv::COLOR_BGR2GRAY);
    else
        gray_left = rect_left;

    if (rect_right.channels() == 3)
        cv::cvtColor(rect_right, gray_right, cv::COLOR_BGR2GRAY);
    else
        gray_right = rect_right;

    // StereoBM 설정
    int num_disparities = 64;   // 16의 배수
    int block_size = 15;         // 홀수

    auto stereo = cv::StereoBM::create(num_disparities, block_size);
    cv::Mat disparity_raw;
    stereo->compute(gray_left, gray_right, disparity_raw);

    // StereoBM 출력은 16배 스케일된 정수 → float 변환
    cv::Mat disparity;
    disparity_raw.convertTo(disparity, CV_32F, 1.0 / 16.0);

    return disparity;
}

cv::Mat disparity_to_depth(const cv::Mat& disparity, double focal_length, double baseline)
{
    // Z = fB / d (d > 0 인 픽셀만)
    cv::Mat depth = cv::Mat::zeros(disparity.size(), CV_32F);

    for (int y = 0; y < disparity.rows; y++)
    {
        for (int x = 0; x < disparity.cols; x++)
        {
            float d = disparity.at<float>(y, x);
            if (d > 1.0f)  // 유효한 disparity 만
            {
                depth.at<float>(y, x) = static_cast<float>(focal_length * baseline / d);
            }
        }
    }

    return depth;
}

// ─── main: 전체 파이프라인 데모 ───

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 3: Multi-view 기하 + Stereo Rectification" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Size image_size(640, 480);

    // ── Step 1: 합성 스테레오 쌍 생성 ──
    std::cout << "[Step 1] 합성 스테레오 쌍 생성" << std::endl;
    cv::Mat K1, K2, dist1, dist2, R, T;
    cv::Mat left, right;
    generate_synthetic_stereo(image_size, K1, K2, dist1, dist2, R, T, left, right);

    double baseline = T.at<double>(0, 0);
    double focal = K1.at<double>(0, 0);
    std::cout << "   K (fx=" << focal << ", cx=" << K1.at<double>(0, 2) << ")" << std::endl;
    std::cout << "   Baseline: " << baseline << " m" << std::endl;
    save_output("01_left", left);
    save_output("01_right", right);

    // ── Step 2: 특징점 매칭 ──
    std::cout << "\n[Step 2] 특징점 매칭" << std::endl;
    auto orb = cv::ORB::create(500);
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat desc1, desc2;
    orb->detectAndCompute(left, cv::noArray(), kp1, desc1);
    orb->detectAndCompute(right, cv::noArray(), kp2, desc2);

    auto matcher = cv::BFMatcher::create(cv::NORM_HAMMING);
    std::vector<cv::DMatch> matches;
    matcher->match(desc1, desc2, matches);

    // 상위 50개만 사용
    std::sort(matches.begin(), matches.end(),
              [](const cv::DMatch& a, const cv::DMatch& b) { return a.distance < b.distance; });
    int num_matches = std::min(static_cast<int>(matches.size()), 50);
    matches.resize(num_matches);

    std::vector<cv::Point2f> pts1, pts2;
    for (const auto& m : matches)
    {
        pts1.push_back(kp1[m.queryIdx].pt);
        pts2.push_back(kp2[m.trainIdx].pt);
    }
    std::cout << "   매칭된 점 수: " << pts1.size() << std::endl;

    // ── Step 3: Fundamental Matrix + 에피폴라 선 ──
    std::cout << "\n[Step 3] Fundamental Matrix + 에피폴라 선 시각화" << std::endl;
    if (pts1.size() >= 8)
    {
        cv::Mat F = draw_epipolar_lines(left, right, pts1, pts2, "02_epipolar_lines");
        std::cout << "   F 행렬:\n" << F << std::endl;
    }
    else
    {
        std::cout << "   ⚠️ 매칭 점이 부족하여 에피폴라 선 계산 스킵" << std::endl;
    }

    // ── Step 4: Stereo Rectification ──
    std::cout << "\n[Step 4] Stereo Rectification" << std::endl;
    cv::Mat rect_left, rect_right, Q;
    stereo_rectify(left, right, K1, K2, dist1, dist2, R, T, image_size,
                   rect_left, rect_right, Q);

    // rectified 쌍 나란히 저장 (수평선이 일치하는지 확인용)
    cv::Mat rect_pair;
    cv::Mat rect_left_bgr, rect_right_bgr;
    cv::cvtColor(rect_left, rect_left_bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(rect_right, rect_right_bgr, cv::COLOR_GRAY2BGR);
    cv::hconcat(rect_left_bgr, rect_right_bgr, rect_pair);

    // 수평 기준선 그리기
    for (int y = 0; y < rect_pair.rows; y += 30)
    {
        cv::line(rect_pair, cv::Point(0, y), cv::Point(rect_pair.cols, y),
                 cv::Scalar(0, 255, 0), 1);
    }
    save_output("03_rectified_pair", rect_pair);
    std::cout << "   Rectification 완료 — 수평선이 두 이미지를 가로질러 정렬되어야 함" << std::endl;
    std::cout << "   → 이 rectified 쌍이 Stereo Depth 모델(HITNet 등)의 입력" << std::endl;

    // ── Step 5: Disparity 계산 ──
    std::cout << "\n[Step 5] Disparity 계산 (StereoBM)" << std::endl;
    cv::Mat disparity = compute_disparity(rect_left, rect_right);

    // 시각화용 정규화
    cv::Mat disp_vis;
    cv::normalize(disparity, disp_vis, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::applyColorMap(disp_vis, disp_vis, cv::COLORMAP_JET);
    save_output("04_disparity", disp_vis);

    // 통계
    double min_d, max_d;
    cv::minMaxLoc(disparity, &min_d, &max_d);
    std::cout << "   Disparity 범위: " << min_d << " ~ " << max_d << " px" << std::endl;

    // ── Step 6: Depth 계산 ──
    std::cout << "\n[Step 6] Disparity → Depth (Z = fB/d)" << std::endl;
    cv::Mat depth = disparity_to_depth(disparity, focal, baseline);

    // 유효 depth 통계
    double min_z = 1e9, max_z = 0;
    int valid_count = 0;
    for (int y = 0; y < depth.rows; y++)
    {
        for (int x = 0; x < depth.cols; x++)
        {
            float z = depth.at<float>(y, x);
            if (z > 0 && z < 100)
            {
                min_z = std::min(min_z, static_cast<double>(z));
                max_z = std::max(max_z, static_cast<double>(z));
                valid_count++;
            }
        }
    }

    // Depth 시각화 (유효 범위만)
    cv::Mat depth_vis;
    cv::Mat depth_clipped;
    depth.copyTo(depth_clipped);
    depth_clipped.setTo(0, depth_clipped > 20.0);  // 20m 이상 클리핑
    cv::normalize(depth_clipped, depth_vis, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::applyColorMap(depth_vis, depth_vis, cv::COLORMAP_INFERNO);
    save_output("05_depth_map", depth_vis);

    std::cout << "   유효 depth 픽셀 수: " << valid_count << std::endl;
    if (valid_count > 0)
    {
        std::cout << "   Depth 범위: " << min_z << " ~ " << max_z << " m" << std::endl;
    }

    // ── 요약 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 파이프라인 완료!" << std::endl;
    std::cout << "   output/ 디렉토리에 다음 파일이 저장되었습니다:" << std::endl;
    std::cout << "   01_left.png / 01_right.png   — 원본 스테레오 쌍" << std::endl;
    std::cout << "   02_epipolar_lines.png        — 에피폴라 선 시각화" << std::endl;
    std::cout << "   03_rectified_pair.png        — Rectified 쌍 (수평선 정렬 확인)" << std::endl;
    std::cout << "   04_disparity.png             — Disparity map (JET 컬러맵)" << std::endl;
    std::cout << "   05_depth_map.png             — Depth map (INFERNO 컬러맵)" << std::endl;
    std::cout << "\n📌 Perception 맥락:" << std::endl;
    std::cout << "   이 파이프라인의 Step 4 결과(rectified pair)가" << std::endl;
    std::cout << "   HITNet / CRE-Stereo 같은 Stereo Depth 모델의 입력입니다." << std::endl;
    std::cout << "   Z = fB/d 공식으로 disparity → 미터 단위 depth 를 복원합니다." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
