/**
 * Phase 3 Week 3 - 3D-2D 모션 추정 (PnP) 직접 구현
 *
 * motion_3d2d.h의 Motion3D2D 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make test_my_basic
 * 테스트: ./test_my_basic
 */
#include "motion_3d2d.h"
#include <iostream>

bool Motion3D2D::solvePnP(const std::vector<cv::Point3f>& points3d,
                           const std::vector<cv::Point2f>& points2d,
                           const cv::Mat& K,
                           cv::Mat& rvec,
                           cv::Mat& tvec) {
    // TODO: cv::solvePnP (EPNP) 사용
    return false;
}

int Motion3D2D::solvePnPRansac(const std::vector<cv::Point3f>& points3d,
                                const std::vector<cv::Point2f>& points2d,
                                const cv::Mat& K,
                                cv::Mat& rvec,
                                cv::Mat& tvec,
                                std::vector<int>& inliers) {
    // TODO: cv::solvePnPRansac 사용
    return 0;
}

double Motion3D2D::computeReprojectionError(const std::vector<cv::Point3f>& points3d,
                                             const std::vector<cv::Point2f>& points2d,
                                             const cv::Mat& K,
                                             const cv::Mat& rvec,
                                             const cv::Mat& tvec) {
    // TODO: cv::projectPoints → RMS 오차 계산
    return -1.0;
}

void Motion3D2D::projectPoints(const std::vector<cv::Point3f>& points3d,
                                const cv::Mat& K,
                                const cv::Mat& rvec,
                                const cv::Mat& tvec,
                                std::vector<cv::Point2f>& projected) {
    // TODO: cv::projectPoints 사용
}

void Motion3D2D::simulateTracking(const std::vector<cv::Point3f>& map_points,
                                   const cv::Mat& K,
                                   int num_frames) {
    // TODO: VO 추적 시뮬레이션
}

void Motion3D2D::demo() {
    // TODO: 전체 데모
}

#ifndef MY_BASIC_LIB_ONLY
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 3D-2D 모션 추정 (PnP) 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    // 3D-2D 대응점
    std::vector<cv::Point3f> pts3d = {{0,0,5}, {1,0,5}, {0,1,5}, {1,1,5}};
    std::vector<cv::Point2f> pts2d;
    for (const auto& p : pts3d) {
        pts2d.push_back({500.0f * p.x / p.z + 320.0f, 500.0f * p.y / p.z + 240.0f});
    }

    cv::Mat rvec, tvec;
    bool ok = Motion3D2D::solvePnP(pts3d, pts2d, K, rvec, tvec);
    std::cout << "PnP: " << (ok ? "성공" : "실패") << std::endl;

    if (ok) {
        double err = Motion3D2D::computeReprojectionError(pts3d, pts2d, K, rvec, tvec);
        std::cout << "재투영 오차: " << err << " px" << std::endl;
    }

    std::cout << "\n구현 완료 후 테스트: ./test_my_basic" << std::endl;
    return 0;
}
#endif
