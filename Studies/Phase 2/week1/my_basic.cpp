/**
 * Phase 2 Week 1 - 핀홀 카메라 모델 직접 구현
 *
 * basic.h의 PinholeProjection 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make test_my_basic
 * 테스트: ./test_my_basic
 */
#include "basic.h"
#include <cmath>
#include <iostream>

PinholeProjection::PinholeProjection(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t) {
    // TODO: K, R, t를 멤버 변수에 저장 (clone 사용)
}

cv::Point2d PinholeProjection::project(const cv::Point3d& P_world) const {
    // TODO: 3D 월드 좌표 → 2D 픽셀 좌표 투영
    // Step 1: Pc = R * Pw + t (월드 → 카메라)
    // Step 2: 정규화 좌표 (Xc/Zc, Yc/Zc)
    // Step 3: 픽셀 좌표 (u = fx * x_norm + cx, v = fy * y_norm + cy)
    // Zc <= 0이면 카메라 뒤 → (-1, -1) 반환
    return cv::Point2d(-1, -1);
}

std::vector<cv::Point2d> PinholeProjection::projectMultiple(
    const std::vector<cv::Point3d>& points_3d) const {
    // TODO: 여러 점을 한번에 투영
    return {};
}

cv::Vec3d PinholeProjection::backProject(const cv::Point2d& pixel) const {
    // TODO: 2D 픽셀 → 3D 광선 방향 (정규화된 단위 벡터)
    // x_norm = (u - cx) / fx, y_norm = (v - cy) / fy
    // ray = normalize([x_norm, y_norm, 1])
    return cv::Vec3d(0, 0, 1);
}

cv::Size2d PinholeProjection::computeFOV(const cv::Size& imageSize) const {
    // TODO: 수평/수직 시야각 계산
    // FOV = 2 * atan(image_size / (2 * focal_length))
    return cv::Size2d(0, 0);
}

double PinholeProjection::reprojectionError(const cv::Point3d& P_world,
                                           const cv::Point2d& observed_pixel) const {
    // TODO: 재투영 오차 = ||project(P) - observed||
    return -1.0;
}

bool PinholeProjection::isInImage(const cv::Point2d& pixel, const cv::Size& imageSize) {
    // TODO: 픽셀이 이미지 범위 안에 있는지 확인
    return false;
}

#ifndef MY_BASIC_LIB_ONLY
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 핀홀 카메라 투영 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 내부 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) <<
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t = cv::Mat::zeros(3, 1, CV_64F);

    PinholeProjection camera(K, R, t);

    // 투영 테스트
    cv::Point3d test_pt(0, 0, 5);
    cv::Point2d pixel = camera.project(test_pt);
    std::cout << "투영: (0,0,5) -> (" << pixel.x << ", " << pixel.y << ")" << std::endl;

    // 역투영 테스트
    cv::Vec3d ray = camera.backProject(cv::Point2d(400, 300));
    std::cout << "역투영: (400,300) -> [" << ray[0] << ", " << ray[1] << ", " << ray[2] << "]" << std::endl;

    // FOV 테스트
    cv::Size2d fov = camera.computeFOV(cv::Size(800, 600));
    std::cout << "FOV: " << fov.width << " x " << fov.height << std::endl;

    // 재투영 오차
    double err = camera.reprojectionError(test_pt, pixel);
    std::cout << "재투영 오차: " << err << " px" << std::endl;

    std::cout << "\n구현 완료 후 테스트: ./test_my_basic" << std::endl;
    return 0;
}
#endif
