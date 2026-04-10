/**
 * @file basic.cpp
 * @brief Week 4: 삼각측량 + PnP 데모
 *
 * 파이프라인:
 *   1. 합성 3D 박스 생성
 *   2. 두 가상 카메라에서 투영
 *   3. 삼각측량으로 3D 점 복원
 *   4. 원본 vs 복원 비교 (삼각측량 오차)
 *   5. PnP 로 카메라 포즈 역추정
 *   6. 재투영 오차 시각화
 *
 * Perception 맥락:
 *   - Monocular 3D Detection (FCOS3D, SMOKE) 이 예측한 3D 박스를
 *     2D 에 재투영해 검증할 때 동일한 연산 사용
 *   - nuScenes Multi-view 에서 여러 카메라 관측 → 삼각측량으로 3D 위치 복원
 */

#include "basic.h"
#include <filesystem>
#include <iostream>
#include <numeric>
#include <cmath>

// ─── 헬퍼 함수 구현 ───

void save_output(const std::string& name, const cv::Mat& image)
{
    std::filesystem::create_directories("output");
    std::string path = "output/" + name + ".png";
    cv::imwrite(path, image);
    std::cout << "   저장: " << path << std::endl;
}

std::vector<cv::Point3d> generate_3d_box(
    const cv::Point3d& center,
    const cv::Vec3d& size)
{
    double hw = size[0] / 2.0;  // half width
    double hh = size[1] / 2.0;  // half height
    double hd = size[2] / 2.0;  // half depth

    // 8개 코너 (자동차 바운딩 박스 형태)
    return {
        {center.x - hw, center.y - hh, center.z - hd},  // 0: 앞 좌하
        {center.x + hw, center.y - hh, center.z - hd},  // 1: 앞 우하
        {center.x + hw, center.y + hh, center.z - hd},  // 2: 앞 우상
        {center.x - hw, center.y + hh, center.z - hd},  // 3: 앞 좌상
        {center.x - hw, center.y - hh, center.z + hd},  // 4: 뒤 좌하
        {center.x + hw, center.y - hh, center.z + hd},  // 5: 뒤 우하
        {center.x + hw, center.y + hh, center.z + hd},  // 6: 뒤 우상
        {center.x - hw, center.y + hh, center.z + hd},  // 7: 뒤 좌상
    };
}

std::vector<cv::Point2d> project_points(
    const std::vector<cv::Point3d>& points_3d,
    const cv::Mat& K,
    const cv::Mat& R,
    const cv::Mat& t)
{
    std::vector<cv::Point2d> result;
    double fx = K.at<double>(0, 0), fy = K.at<double>(1, 1);
    double cx = K.at<double>(0, 2), cy = K.at<double>(1, 2);

    for (const auto& p : points_3d)
    {
        cv::Mat pw = (cv::Mat_<double>(3, 1) << p.x, p.y, p.z);
        cv::Mat pc = R * pw + t;

        double Xc = pc.at<double>(0);
        double Yc = pc.at<double>(1);
        double Zc = pc.at<double>(2);

        if (Zc > 0)
        {
            double u = fx * Xc / Zc + cx;
            double v = fy * Yc / Zc + cy;
            result.emplace_back(u, v);
        }
        else
        {
            result.emplace_back(-1, -1);
        }
    }
    return result;
}

std::vector<cv::Point3d> triangulate(
    const std::vector<cv::Point2d>& pts1,
    const std::vector<cv::Point2d>& pts2,
    const cv::Mat& P1,
    const cv::Mat& P2)
{
    // OpenCV triangulatePoints 는 Point2f 를 요구
    std::vector<cv::Point2f> pts1f(pts1.begin(), pts1.end());
    std::vector<cv::Point2f> pts2f(pts2.begin(), pts2.end());

    cv::Mat points_4d;
    cv::triangulatePoints(P1, P2, pts1f, pts2f, points_4d);

    // 동차 좌표 → 3D
    std::vector<cv::Point3d> result;
    for (int i = 0; i < points_4d.cols; i++)
    {
        double w = points_4d.at<float>(3, i);
        double x = points_4d.at<float>(0, i) / w;
        double y = points_4d.at<float>(1, i) / w;
        double z = points_4d.at<float>(2, i) / w;
        result.emplace_back(x, y, z);
    }
    return result;
}

std::vector<double> compute_reprojection_errors(
    const std::vector<cv::Point3d>& points_3d,
    const std::vector<cv::Point2d>& points_2d,
    const cv::Mat& K,
    const cv::Mat& rvec,
    const cv::Mat& tvec)
{
    std::vector<cv::Point2f> projected;
    cv::projectPoints(points_3d, rvec, tvec, K, cv::noArray(), projected);

    std::vector<double> errors;
    for (size_t i = 0; i < points_3d.size(); i++)
    {
        double dx = projected[i].x - points_2d[i].x;
        double dy = projected[i].y - points_2d[i].y;
        errors.push_back(std::sqrt(dx * dx + dy * dy));
    }
    return errors;
}

// ─── main: 전체 파이프라인 데모 ───

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 4: 삼각측량 + PnP (Perception 3D 맥락)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ── Step 1: 합성 3D 박스 생성 ──
    std::cout << "[Step 1] 합성 3D 박스 생성" << std::endl;
    // 자동차 크기 (폭 1.8m, 높이 1.5m, 길이 4.5m), 카메라 앞 8m
    auto box = generate_3d_box(cv::Point3d(1.0, 0.0, 8.0), cv::Vec3d(1.8, 1.5, 4.5));
    std::cout << "   박스 중심: (1.0, 0.0, 8.0) m" << std::endl;
    std::cout << "   박스 크기: 1.8 x 1.5 x 4.5 m (W x H x D)" << std::endl;
    std::cout << "   코너 수: " << box.size() << std::endl;

    // ── Step 2: 두 가상 카메라 설정 + 투영 ──
    std::cout << "\n[Step 2] 두 가상 카메라에서 투영" << std::endl;
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    // 카메라 1: 원점
    cv::Mat R1 = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t1 = cv::Mat::zeros(3, 1, CV_64F);

    // 카메라 2: 오른쪽으로 0.5m 이동
    cv::Mat R2 = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t2 = (cv::Mat_<double>(3, 1) << 0.5, 0, 0);

    auto pixels1 = project_points(box, K, R1, t1);
    auto pixels2 = project_points(box, K, R2, t2);

    std::cout << "   카메라 1 (원점): 8개 코너 투영 완료" << std::endl;
    std::cout << "   카메라 2 (x+0.5m): 8개 코너 투영 완료" << std::endl;

    // 투영 시각화
    cv::Mat vis1 = cv::Mat::zeros(480, 640, CV_8UC3);
    cv::Mat vis2 = cv::Mat::zeros(480, 640, CV_8UC3);

    // 3D 박스 모서리 연결 (앞면 0-1-2-3, 뒷면 4-5-6-7, 앞뒤 연결)
    int edges[][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
    for (auto& e : edges)
    {
        if (pixels1[e[0]].x >= 0 && pixels1[e[1]].x >= 0)
            cv::line(vis1, cv::Point(pixels1[e[0]]), cv::Point(pixels1[e[1]]),
                     cv::Scalar(0, 255, 0), 2);
        if (pixels2[e[0]].x >= 0 && pixels2[e[1]].x >= 0)
            cv::line(vis2, cv::Point(pixels2[e[0]]), cv::Point(pixels2[e[1]]),
                     cv::Scalar(0, 255, 0), 2);
    }

    cv::Mat vis_pair;
    cv::hconcat(vis1, vis2, vis_pair);
    save_output("01_3d_box_projections", vis_pair);

    // ── Step 3: 삼각측량으로 3D 복원 ──
    std::cout << "\n[Step 3] 삼각측량으로 3D 점 복원" << std::endl;

    // 투영 행렬 P = K · [R|t]
    cv::Mat Rt1, Rt2;
    cv::hconcat(R1, t1, Rt1);
    cv::hconcat(R2, t2, Rt2);
    cv::Mat P1 = K * Rt1;
    cv::Mat P2 = K * Rt2;
    P1.convertTo(P1, CV_64F);
    P2.convertTo(P2, CV_64F);

    auto recovered = triangulate(pixels1, pixels2, P1, P2);

    std::cout << "\n   원본 vs 복원 비교:" << std::endl;
    std::cout << "   " << std::string(60, '-') << std::endl;
    double total_error = 0;
    for (size_t i = 0; i < box.size(); i++)
    {
        double err = cv::norm(cv::Point3d(recovered[i].x - box[i].x,
                                           recovered[i].y - box[i].y,
                                           recovered[i].z - box[i].z));
        total_error += err;
        std::cout << "   코너 " << i << ": 원본=(" << box[i].x << ", " << box[i].y << ", " << box[i].z
                  << ") 복원=(" << recovered[i].x << ", " << recovered[i].y << ", " << recovered[i].z
                  << ") 오차=" << err << " m" << std::endl;
    }
    std::cout << "   평균 3D 오차: " << total_error / box.size() << " m" << std::endl;

    // ── Step 4: PnP 로 카메라 포즈 역추정 ──
    std::cout << "\n[Step 4] PnP 로 카메라 2 의 포즈 역추정" << std::endl;

    // 3D-2D 대응: 원본 3D 점 + 카메라 2 의 투영
    std::vector<cv::Point3f> obj_pts;
    std::vector<cv::Point2f> img_pts;
    for (size_t i = 0; i < box.size(); i++)
    {
        if (pixels2[i].x >= 0)
        {
            obj_pts.emplace_back(static_cast<float>(box[i].x),
                                  static_cast<float>(box[i].y),
                                  static_cast<float>(box[i].z));
            img_pts.emplace_back(static_cast<float>(pixels2[i].x),
                                  static_cast<float>(pixels2[i].y));
        }
    }

    cv::Mat rvec, tvec;
    bool success = cv::solvePnP(obj_pts, img_pts, K, cv::noArray(), rvec, tvec);

    if (success)
    {
        std::cout << "   추정된 t: [" << tvec.at<double>(0) << ", "
                  << tvec.at<double>(1) << ", " << tvec.at<double>(2) << "]" << std::endl;
        std::cout << "   실제 t:   [" << t2.at<double>(0) << ", "
                  << t2.at<double>(1) << ", " << t2.at<double>(2) << "]" << std::endl;

        double t_error = cv::norm(tvec - t2);
        std::cout << "   이동 벡터 오차: " << t_error << " m" << std::endl;
    }

    // ── Step 5: 재투영 오차 ──
    std::cout << "\n[Step 5] 재투영 오차 시각화" << std::endl;

    std::vector<cv::Point3d> obj_pts_d(obj_pts.begin(), obj_pts.end());
    std::vector<cv::Point2d> img_pts_d(img_pts.begin(), img_pts.end());
    auto errors = compute_reprojection_errors(obj_pts_d, img_pts_d, K, rvec, tvec);

    double mean_error = std::accumulate(errors.begin(), errors.end(), 0.0) / errors.size();
    double max_error = *std::max_element(errors.begin(), errors.end());

    std::cout << "   재투영 오차:" << std::endl;
    std::cout << "     평균: " << mean_error << " px" << std::endl;
    std::cout << "     최대: " << max_error << " px" << std::endl;

    // 재투영 시각화 (원본 관측 vs 재투영)
    cv::Mat vis_reproj = cv::Mat::zeros(480, 640, CV_8UC3);
    std::vector<cv::Point2f> reproj_pts;
    cv::projectPoints(obj_pts, rvec, tvec, K, cv::noArray(), reproj_pts);

    for (size_t i = 0; i < img_pts.size(); i++)
    {
        cv::circle(vis_reproj, img_pts[i], 6, cv::Scalar(0, 255, 0), 2);      // 관측 (녹색)
        cv::circle(vis_reproj, reproj_pts[i], 4, cv::Scalar(0, 0, 255), -1);  // 재투영 (적색)
        cv::line(vis_reproj, img_pts[i], reproj_pts[i], cv::Scalar(255, 0, 0), 1);
    }
    save_output("02_reprojection_error", vis_reproj);

    // ── 요약 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 파이프라인 완료!" << std::endl;
    std::cout << "   output/ 디렉토리:" << std::endl;
    std::cout << "   01_3d_box_projections.png  — 두 카메라에서의 3D 박스 투영" << std::endl;
    std::cout << "   02_reprojection_error.png  — 관측(녹) vs 재투영(적) 비교" << std::endl;
    std::cout << "\n📌 Perception 맥락:" << std::endl;
    std::cout << "   이 연산이 Monocular 3D Detection (FCOS3D, SMOKE) 의" << std::endl;
    std::cout << "   3D 박스 재투영 검증과 동일합니다." << std::endl;
    std::cout << "   재투영 오차는 3D Detection 의 평가 기준이기도 합니다." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
