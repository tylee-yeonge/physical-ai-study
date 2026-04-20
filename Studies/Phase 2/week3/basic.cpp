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
        // 주의: 아래 -c/b 계산은 b가 0에 가까우면 (수직에 가까운 에피폴라 선)
        //       0 나눗셈 또는 큰 오차가 발생할 수 있다. 일반적인 stereo 세팅에서는
        //       에피폴라 선이 거의 수평이라 문제되지 않지만, 엄밀하게 하려면
        //       |a| > |b|일 때 x에 대해 푸는 분기가 필요하다.
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

// initUndistortRectifyMap 을 직접 구현한 헬퍼.
//
// Rectified 이미지의 각 (u_dst, v_dst) 픽셀이 원본 이미지의 어느
// (u_src, v_src) 에서 와야 하는지를 미리 계산해서 테이블(map_x, map_y)
// 에 저장한다. (Backward mapping)
//
// 왜 backward 인가?
//   Forward (원본 -> rectified) 로 쏘면 dst 에 정수 픽셀이 고르게
//   안 맞아 구멍이 생긴다. Backward 는 dst 픽셀마다 src 에서 하나씩
//   꺼내오므로 빈 곳이 없다.
//
// 계산 순서 (각 dst 픽셀마다):
//   [1] P_rect 역투영 : (u_dst, v_dst) -> rectified normalized (x_r, y_r)
//   [2] R_rect^T 회전 : rectified 방향벡터 -> 원본 카메라 좌표계
//   [3] 렌즈 왜곡 적용 : 이상적 normalized -> 왜곡된 normalized
//   [4] K 적용        : 왜곡된 normalized -> 원본 픽셀 (u_src, v_src)
static void build_rectify_map(
    const cv::Mat& K,       // 원본 카메라 내부 행렬 (3x3)
    const cv::Mat& dist,    // 왜곡 계수 [k1, k2, p1, p2, k3]
    const cv::Mat& R_rect,  // 원본 -> rectified 좌표계 회전 (3x3)
    const cv::Mat& P_rect,  // rectified 투영 행렬 (3x4)
    const cv::Size& size,
    cv::Mat& map_x,
    cv::Mat& map_y)
{
    map_x.create(size, CV_32FC1);
    map_y.create(size, CV_32FC1);

    // rectified 쪽 내부 파라미터 (P_rect 안에 박혀 있음)
    const double fx_r = P_rect.at<double>(0, 0);
    const double fy_r = P_rect.at<double>(1, 1);
    const double cx_r = P_rect.at<double>(0, 2);
    const double cy_r = P_rect.at<double>(1, 2);

    // 원본 카메라 내부 파라미터
    const double fx = K.at<double>(0, 0);
    const double fy = K.at<double>(1, 1);
    const double cx = K.at<double>(0, 2);
    const double cy = K.at<double>(1, 2);

    // 왜곡 계수 (Brown-Conrady: radial k1,k2,k3 + tangential p1,p2)
    const double k1 = dist.at<double>(0);
    const double k2 = dist.at<double>(1);
    const double p1 = dist.at<double>(2);
    const double p2 = dist.at<double>(3);
    const double k3 = dist.rows >= 5 ? dist.at<double>(4) : 0.0;

    // R_rect 는 원본 -> rectified. 우리는 그 반대가 필요하므로 전치(=역)
    const cv::Mat R_inv = R_rect.t();

    for (int v_dst = 0; v_dst < size.height; v_dst++)
    {
        for (int u_dst = 0; u_dst < size.width; u_dst++)
        {
            // [1] rectified 픽셀 -> rectified normalized 좌표
            //     P_rect 의 역투영 : 픽셀에서 중심(cx',cy') 빼고 초점거리로 나눔
            const double x_r = (u_dst - cx_r) / fx_r;
            const double y_r = (v_dst - cy_r) / fy_r;

            // [2] rectified 방향벡터 (x_r, y_r, 1) 을 원본 카메라 좌표계로 되돌림
            //     = R_rect^T 곱하기
            const double Xc = R_inv.at<double>(0, 0) * x_r
                            + R_inv.at<double>(0, 1) * y_r
                            + R_inv.at<double>(0, 2);
            const double Yc = R_inv.at<double>(1, 0) * x_r
                            + R_inv.at<double>(1, 1) * y_r
                            + R_inv.at<double>(1, 2);
            const double Zc = R_inv.at<double>(2, 0) * x_r
                            + R_inv.at<double>(2, 1) * y_r
                            + R_inv.at<double>(2, 2);

            // 원본 카메라의 normalized 평면 좌표 (Z 로 나눔)
            const double x = Xc / Zc;
            const double y = Yc / Zc;

            // [3] 렌즈 왜곡 적용 (Brown-Conrady)
            //     원본 이미지는 왜곡된 상태로 찍혔으므로, "이상적" normalized
            //     좌표를 왜곡된 좌표로 되돌려야 원본 픽셀 위치를 찾을 수 있다.
            //     x_d = x*(1 + k1*r^2 + k2*r^4 + k3*r^6) + 2*p1*x*y + p2*(r^2 + 2*x^2)
            //     y_d = y*(1 + k1*r^2 + k2*r^4 + k3*r^6) + p1*(r^2 + 2*y^2) + 2*p2*x*y
            const double r2 = x * x + y * y;
            const double radial = 1.0 + k1 * r2 + k2 * r2 * r2 + k3 * r2 * r2 * r2;
            const double x_d = x * radial + 2.0 * p1 * x * y + p2 * (r2 + 2.0 * x * x);
            const double y_d = y * radial + p1 * (r2 + 2.0 * y * y) + 2.0 * p2 * x * y;

            // [4] 왜곡된 normalized -> 원본 픽셀 좌표 (K 적용)
            const double u_src = fx * x_d + cx;
            const double v_src = fy * y_d + cy;

            // 테이블에 저장. remap 이 이 float 좌표로 쌍선형 보간.
            map_x.at<float>(v_dst, u_dst) = static_cast<float>(u_src);
            map_y.at<float>(v_dst, u_dst) = static_cast<float>(v_src);
        }
    }
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
    // [1] Rectification 변환 규칙 계산 (cv 에 맡김)
    //
    // 내부적으로 Bouguet 알고리즘으로 두 카메라 사이 회전 R 을
    // "반씩" 분배해 양쪽 이미지의 왜곡을 최소화한다. 이 수학은
    // 복잡하므로 cv 에 맡긴다.
    //
    // 출력의 의미:
    //   R1, R2 : 각 카메라 좌표계 -> rectified 좌표계로 보내는 3x3 회전
    //   P1, P2 : rectified 상태에서의 3x4 투영 행렬
    //     P1 = [ fx'  0   cx'   0      ]
    //          [  0  fy'  cy'   0      ]
    //          [  0   0    1    0      ]
    //     P2 = [ fx'  0   cx'  -fx'*B  ]   <- (0,3) 에 baseline 인코딩됨
    //          [  0  fy'  cy'   0      ]
    //          [  0   0    1    0      ]
    //   Q      : disparity -> 3D 복원 4x4 행렬
    //            [X Y Z W]^T = Q * [u v disp 1]^T, (X/W, Y/W, Z/W) 가 3D 점
    //            cv::reprojectImageTo3D(disparity, Q) 로 한 번에 쓸 수도 있다.
    cv::Mat R1, R2, P1, P2;
    cv::stereoRectify(K1, dist1, K2, dist2, image_size, R, T,
                      R1, R2, P1, P2, Q,
                      cv::CALIB_ZERO_DISPARITY, 0, image_size);

    // [2] 픽셀 매핑 테이블 생성 (직접 구현)
    //
    // 원래 cv 버전:
    //   cv::initUndistortRectifyMap(K1, dist1, R1, P1, image_size, CV_32FC1, map1x, map1y);
    //   cv::initUndistortRectifyMap(K2, dist2, R2, P2, image_size, CV_32FC1, map2x, map2y);
    //
    // 학습을 위해 build_rectify_map 으로 풀어서 구현한다.
    // 각 rectified 픽셀이 원본 이미지의 어느 위치에서 왔는지를 테이블에
    // 저장. 자세한 4단계는 build_rectify_map 내부 주석 참고.
    cv::Mat map1x, map1y, map2x, map2y;
    build_rectify_map(K1, dist1, R1, P1, image_size, map1x, map1y);
    build_rectify_map(K2, dist2, R2, P2, image_size, map2x, map2y);

    // [3] Remap 적용 (cv 에 맡김)
    //
    // 테이블(map*x, map*y) 을 따라 원본 이미지에서 픽셀을 꺼내
    // 쌍선형 보간으로 rectified 이미지를 생성. 보간 자체는 SIMD 최적화가
    // 되어 있으므로 cv 에 맡긴다.
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
