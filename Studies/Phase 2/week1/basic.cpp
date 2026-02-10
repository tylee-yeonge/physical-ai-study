#include "basic.h"
#include <iostream>
#include <cmath>

PinholeProjection::PinholeProjection(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t)
    : K_(K.clone()), R_(R.clone()), t_(t.clone()) {}

cv::Point2d PinholeProjection::project(const cv::Point3d& P_world) const {
    // Step 1: 월드 → 카메라 좌표 변환
    // Pc = R * Pw + t
    cv::Mat Pw = (cv::Mat_<double>(3, 1) << P_world.x, P_world.y, P_world.z);
    cv::Mat Pc = R_ * Pw + t_;

    double Xc = Pc.at<double>(0);
    double Yc = Pc.at<double>(1);
    double Zc = Pc.at<double>(2);

    // Zc가 0 이하면 카메라 뒤에 있음 → 투영 불가
    if (Zc <= 0) {
        return cv::Point2d(-1, -1);
    }

    // Step 2: 원근 투영 (정규화 좌표)
    double x_norm = Xc / Zc;
    double y_norm = Yc / Zc;

    // Step 3: 정규화 → 픽셀 좌표
    double fx = K_.at<double>(0, 0);
    double fy = K_.at<double>(1, 1);
    double cx = K_.at<double>(0, 2);
    double cy = K_.at<double>(1, 2);

    double u = fx * x_norm + cx;
    double v = fy * y_norm + cy;

    return cv::Point2d(u, v);
}

std::vector<cv::Point2d> PinholeProjection::projectMultiple(
    const std::vector<cv::Point3d>& points_3d) const {

    std::vector<cv::Point2d> pixels;
    pixels.reserve(points_3d.size());

    for (const auto& pt : points_3d) {
        pixels.push_back(project(pt));
    }

    return pixels;
}

cv::Vec3d PinholeProjection::backProject(const cv::Point2d& pixel) const {
    double fx = K_.at<double>(0, 0);
    double fy = K_.at<double>(1, 1);
    double cx = K_.at<double>(0, 2);
    double cy = K_.at<double>(1, 2);

    // 정규화 좌표 계산
    double x_norm = (pixel.x - cx) / fx;
    double y_norm = (pixel.y - cy) / fy;

    // 광선 방향 (카메라 좌표계, Zc=1 가정)
    cv::Vec3d ray(x_norm, y_norm, 1.0);

    // 단위 벡터로 정규화
    double norm = cv::norm(ray);
    return ray / norm;
}

cv::Size2d PinholeProjection::computeFOV(const cv::Size& imageSize) const {
    double fx = K_.at<double>(0, 0);
    double fy = K_.at<double>(1, 1);

    // FOV = 2 * arctan(image_size / (2 * focal_length))
    double fov_h = 2.0 * std::atan2(imageSize.width, 2.0 * fx) * 180.0 / CV_PI;
    double fov_v = 2.0 * std::atan2(imageSize.height, 2.0 * fy) * 180.0 / CV_PI;

    return cv::Size2d(fov_h, fov_v);
}

double PinholeProjection::reprojectionError(const cv::Point3d& P_world,
                                           const cv::Point2d& observed_pixel) const {
    cv::Point2d projected = project(P_world);

    if (projected.x < 0) return -1.0;  // 투영 불가

    double dx = projected.x - observed_pixel.x;
    double dy = projected.y - observed_pixel.y;

    return std::sqrt(dx * dx + dy * dy);
}

bool PinholeProjection::isInImage(const cv::Point2d& pixel, const cv::Size& imageSize) {
    return pixel.x >= 0 && pixel.x < imageSize.width &&
           pixel.y >= 0 && pixel.y < imageSize.height;
}

#ifndef PINHOLE_LIB_ONLY
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  핀홀 카메라 투영 기본 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 내부 파라미터 K
    cv::Mat K = (cv::Mat_<double>(3, 3) <<
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);

    // 카메라 외부 파라미터 (단위 행렬 = 원점에서 정면)
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t = cv::Mat::zeros(3, 1, CV_64F);

    PinholeProjection camera(K, R, t);

    // 1. 기본 투영
    std::cout << "📐 카메라 파라미터:" << std::endl;
    std::cout << "   fx = " << camera.getFx() << ", fy = " << camera.getFy() << std::endl;
    std::cout << "   cx = " << camera.getCx() << ", cy = " << camera.getCy() << "\n" << std::endl;

    // 3D 점 정의 (큐브 꼭짓점)
    std::vector<cv::Point3d> cube_points = {
        {-1, -1, 5}, { 1, -1, 5}, { 1,  1, 5}, {-1,  1, 5},  // 앞면
        {-1, -1, 7}, { 1, -1, 7}, { 1,  1, 7}, {-1,  1, 7}   // 뒷면
    };

    std::cout << "📊 3D 큐브 투영 결과:" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    auto pixels = camera.projectMultiple(cube_points);
    cv::Size imageSize(800, 600);

    for (size_t i = 0; i < cube_points.size(); i++) {
        const auto& p3d = cube_points[i];
        const auto& p2d = pixels[i];
        bool visible = PinholeProjection::isInImage(p2d, imageSize);

        std::cout << "   (" << p3d.x << ", " << p3d.y << ", " << p3d.z << ")"
                  << " → (" << (int)p2d.x << ", " << (int)p2d.y << ")"
                  << (visible ? " ✅" : " ❌ 이미지 밖") << std::endl;
    }

    // 2. FOV 계산
    std::cout << "\n📐 시야각 (FOV):" << std::endl;
    cv::Size2d fov = camera.computeFOV(imageSize);
    std::cout << "   수평 FOV: " << fov.width << "°" << std::endl;
    std::cout << "   수직 FOV: " << fov.height << "°" << std::endl;

    // 3. 역투영
    std::cout << "\n🔄 역투영 테스트:" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    // 이미지 중심 역투영
    cv::Point2d center(camera.getCx(), camera.getCy());
    cv::Vec3d ray_center = camera.backProject(center);
    std::cout << "   중심 (" << center.x << ", " << center.y << ")"
              << " → ray [" << ray_center[0] << ", " << ray_center[1]
              << ", " << ray_center[2] << "]" << std::endl;

    // 모서리 역투영
    cv::Point2d corner(0, 0);
    cv::Vec3d ray_corner = camera.backProject(corner);
    std::cout << "   좌상단 (0, 0)"
              << " → ray [" << ray_corner[0] << ", " << ray_corner[1]
              << ", " << ray_corner[2] << "]" << std::endl;

    // 4. 재투영 오차
    std::cout << "\n📏 재투영 오차:" << std::endl;
    cv::Point3d test_point(2.0, 1.0, 5.0);
    cv::Point2d projected = camera.project(test_point);
    cv::Point2d noisy_obs(projected.x + 1.5, projected.y - 0.8);

    double error = camera.reprojectionError(test_point, noisy_obs);
    std::cout << "   투영: (" << projected.x << ", " << projected.y << ")" << std::endl;
    std::cout << "   관측: (" << noisy_obs.x << ", " << noisy_obs.y << ")" << std::endl;
    std::cout << "   오차: " << error << " 픽셀" << std::endl;

    // 5. 카메라 위치 변경
    std::cout << "\n📷 카메라 위치 변경 테스트:" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    cv::Mat t2 = (cv::Mat_<double>(3, 1) << 2.0, 0.0, 0.0);  // 오른쪽으로 2m 이동
    PinholeProjection camera2(K, R, t2);

    cv::Point3d P(0, 0, 5);
    cv::Point2d pix1 = camera.project(P);
    cv::Point2d pix2 = camera2.project(P);

    std::cout << "   점 (0,0,5):" << std::endl;
    std::cout << "   카메라1 (원점): (" << pix1.x << ", " << pix1.y << ")" << std::endl;
    std::cout << "   카메라2 (x+2m): (" << pix2.x << ", " << pix2.y << ")" << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  ✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "💡 다음 단계:" << std::endl;
    std::cout << "   1. quiz_easy.cpp로 기초 개념 확인" << std::endl;
    std::cout << "   2. quiz_medium.cpp로 실전 문제 풀이" << std::endl;
    std::cout << "   3. PRACTICE.md에서 심화 실습\n" << std::endl;

    return 0;
}
#endif // PINHOLE_LIB_ONLY
