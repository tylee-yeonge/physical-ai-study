/**
 * Phase 2 Week 5 - 에피폴라 기하학 직접 구현
 *
 * basic.h의 EpipolarGeometryBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make test_my_basic
 * 테스트: ./test_my_basic
 */
#include "basic.h"
#include <iostream>

int EpipolarGeometryBasic::estimateEssential(const std::vector<cv::Point2f>& points1,
                                             const std::vector<cv::Point2f>& points2,
                                             cv::Mat& essential, int method)
{
    // TODO: cv::findEssentialMat 사용
    return 0;
}

int EpipolarGeometryBasic::estimateFundamental(const std::vector<cv::Point2f>& points1,
                                               const std::vector<cv::Point2f>& points2,
                                               cv::Mat& fundamental, int method)
{
    // TODO: cv::findFundamentalMat 사용
    return 0;
}

bool EpipolarGeometryBasic::recoverPose(const cv::Mat& essential,
                                        const std::vector<cv::Point2f>& points1,
                                        const std::vector<cv::Point2f>& points2, const cv::Mat& K,
                                        cv::Mat& R, cv::Mat& t)
{
    // TODO: cv::recoverPose 사용
    return false;
}

cv::Vec3f EpipolarGeometryBasic::computeEpipolarLine(const cv::Point2f& point,
                                                     const cv::Mat& fundamental, bool for_image2)
{
    // TODO: l = F * p (또는 l = F^T * p)
    return cv::Vec3f(0, 0, 0);
}

double EpipolarGeometryBasic::verifyEpipolarConstraint(const cv::Point2f& point1,
                                                       const cv::Point2f& point2,
                                                       const cv::Mat& essential_or_fundamental)
{
    // TODO: p2^T * E * p1 계산 (0에 가까울수록 정확)
    return -1.0;
}

void EpipolarGeometryBasic::visualizeEpipolarLines(const cv::Mat& img1, const cv::Mat& img2,
                                                   const std::vector<cv::Point2f>& points1,
                                                   const std::vector<cv::Point2f>& points2,
                                                   const cv::Mat& fundamental, cv::Mat& output)
{
    // TODO: 에피폴라 선 시각화
}

double EpipolarGeometryBasic::verifyEF_Relationship(const cv::Mat& K, const cv::Mat& essential,
                                                    const cv::Mat& fundamental)
{
    // TODO: F = K'^-T * E * K^-1 관계 검증
    return -1.0;
}

void EpipolarGeometryBasic::demoPipeline(const cv::Mat& img1, const cv::Mat& img2, const cv::Mat& K)
{
    // TODO: 전체 파이프라인
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 에피폴라 기하학 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 포인트 (정규화 좌표)
    std::vector<cv::Point2f> pts1 = {{0.0f, 0.0f},  {0.5f, 0.0f}, {0.0f, 0.5f},  {-0.5f, 0.0f},
                                     {0.0f, -0.5f}, {0.3f, 0.3f}, {-0.3f, 0.3f}, {0.1f, -0.2f}};
    std::vector<cv::Point2f> pts2 = {{0.1f, 0.0f},  {0.6f, 0.0f}, {0.1f, 0.5f},  {-0.4f, 0.0f},
                                     {0.1f, -0.5f}, {0.4f, 0.3f}, {-0.2f, 0.3f}, {0.2f, -0.2f}};

    // Essential Matrix 추정
    cv::Mat E;
    int n_inliers = EpipolarGeometryBasic::estimateEssential(pts1, pts2, E);
    std::cout << "Essential Matrix 인라이어: " << n_inliers << std::endl;

    // 포즈 복원
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);
    cv::Mat R, t;
    bool ok = EpipolarGeometryBasic::recoverPose(E, pts1, pts2, K, R, t);
    std::cout << "포즈 복원: " << (ok ? "성공" : "실패") << std::endl;

    std::cout << "\n구현 완료 후 테스트: ./test_my_basic" << std::endl;
    return 0;
}
#endif
