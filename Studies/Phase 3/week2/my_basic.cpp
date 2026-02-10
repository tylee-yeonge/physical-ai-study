/**
 * Phase 3 Week 2 - 2D-2D 모션 추정 직접 구현
 *
 * motion_2d2d.h의 Motion2D2D 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make test_my_basic
 * 테스트: ./test_my_basic
 */
#include "motion_2d2d.h"
#include <iostream>

int Motion2D2D::estimateEssential(const std::vector<cv::Point2f>& points1,
                                   const std::vector<cv::Point2f>& points2,
                                   cv::Mat& E,
                                   std::vector<uchar>& inliers) {
    // TODO: cv::findEssentialMat 사용
    return 0;
}

int Motion2D2D::recoverPose(const cv::Mat& E,
                             const std::vector<cv::Point2f>& points1,
                             const std::vector<cv::Point2f>& points2,
                             cv::Mat& R,
                             cv::Mat& t,
                             std::vector<uchar>& inliers) {
    // TODO: cv::recoverPose 사용
    return 0;
}

void Motion2D2D::normalizePoints(const cv::Mat& K,
                                  const std::vector<cv::Point2f>& points,
                                  std::vector<cv::Point2f>& normalized) {
    // TODO: 픽셀 좌표 → 정규화 좌표
    // x_norm = (u - cx) / fx, y_norm = (v - cy) / fy
}

void Motion2D2D::triangulate(const cv::Mat& K,
                              const cv::Mat& R,
                              const cv::Mat& t,
                              const std::vector<cv::Point2f>& points1,
                              const std::vector<cv::Point2f>& points2,
                              std::vector<cv::Point3f>& points3d) {
    // TODO: cv::triangulatePoints 사용
}

bool Motion2D2D::pipeline(const cv::Mat& K,
                           const std::vector<cv::Point2f>& points1,
                           const std::vector<cv::Point2f>& points2,
                           cv::Mat& R,
                           cv::Mat& t,
                           std::vector<cv::Point3f>& points3d) {
    // TODO: 전체 파이프라인 (E 추정 → 포즈 복원 → 삼각측량)
    return false;
}

void Motion2D2D::demo() {
    // TODO: 전체 데모
}

#ifndef MY_BASIC_LIB_ONLY
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 2D-2D 모션 추정 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 포인트 (정규화 좌표)
    std::vector<cv::Point2f> pts1 = {{0.0f, 0.0f}, {0.5f, 0.0f}, {0.0f, 0.5f},
                                      {-0.5f, 0.0f}, {0.0f, -0.5f},
                                      {0.3f, 0.3f}, {-0.3f, 0.3f}, {0.1f, -0.2f}};
    std::vector<cv::Point2f> pts2 = {{0.1f, 0.0f}, {0.6f, 0.0f}, {0.1f, 0.5f},
                                      {-0.4f, 0.0f}, {0.1f, -0.5f},
                                      {0.4f, 0.3f}, {-0.2f, 0.3f}, {0.2f, -0.2f}};

    // Essential Matrix 추정
    cv::Mat E;
    std::vector<uchar> inliers;
    int n = Motion2D2D::estimateEssential(pts1, pts2, E, inliers);
    std::cout << "인라이어: " << n << "/" << pts1.size() << std::endl;

    // 포즈 복원
    cv::Mat R, t;
    Motion2D2D::recoverPose(E, pts1, pts2, R, t, inliers);
    std::cout << "포즈 복원 완료" << std::endl;

    std::cout << "\n구현 완료 후 테스트: ./test_my_basic" << std::endl;
    return 0;
}
#endif
