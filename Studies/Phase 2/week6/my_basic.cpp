/**
 * Phase 2 Week 6 - 삼각측량 직접 구현
 *
 * basic.h의 TriangulationBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make test_my_basic
 * 테스트: ./test_my_basic
 */
#include "basic.h"
#include <iostream>
#include <cmath>

bool TriangulationBasic::triangulatePoint(const cv::Point2f& pt1, const cv::Point2f& pt2,
                                          const cv::Mat& P1, const cv::Mat& P2,
                                          cv::Point3f& point3d)
{
    // TODO: DLT (Direct Linear Transform) 삼각측량
    // cv::triangulatePoints 사용 가능
    return false;
}

void TriangulationBasic::triangulatePoints(const std::vector<cv::Point2f>& points1,
                                           const std::vector<cv::Point2f>& points2,
                                           const cv::Mat& P1, const cv::Mat& P2,
                                           std::vector<cv::Point3f>& points3d)
{
    // TODO: 여러 점 삼각측량
}

double TriangulationBasic::reprojectionError(const cv::Point3f& point3d, const cv::Point2f& point2d,
                                             const cv::Mat& P)
{
    // TODO: 3D점을 P로 투영 → 관측값과 비교
    return -1.0;
}

double TriangulationBasic::averageReprojectionError(const std::vector<cv::Point3f>& points3d,
                                                    const std::vector<cv::Point2f>& points2d,
                                                    const cv::Mat& P)
{
    // TODO: 평균 재투영 오차
    return -1.0;
}

double TriangulationBasic::disparityToDepth(double disparity, double baseline, double focal_length)
{
    // TODO: depth = baseline * focal_length / disparity
    return 0.0;
}

bool TriangulationBasic::isInFrontOfCamera(const cv::Point3f& point3d, const cv::Mat& R,
                                           const cv::Mat& t)
{
    // TODO: Cheirality Check - Pc = R * Pw + t, Zc > 0?
    return false;
}

void TriangulationBasic::evaluateTriangulation(const std::vector<cv::Point3f>& points3d,
                                               const std::vector<cv::Point2f>& points1,
                                               const std::vector<cv::Point2f>& points2,
                                               const cv::Mat& P1, const cv::Mat& P2)
{
    // TODO: 삼각측량 품질 평가
}

void TriangulationBasic::visualize3DPoints(const std::vector<cv::Point3f>& points3d)
{
    // TODO: 3D 점군 통계 출력
}

void TriangulationBasic::demoPipeline(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t)
{
    // TODO: 전체 파이프라인
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 삼각측량 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 프로젝션 행렬 설정
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);
    cv::Mat P1 = K * cv::Mat::eye(3, 4, CV_64F);
    cv::Mat Rt2 = cv::Mat::eye(3, 4, CV_64F);
    Rt2.at<double>(0, 3) = 1.0;  // 베이스라인 1m
    cv::Mat P2 = K * Rt2;

    // 삼각측량 테스트
    cv::Point2f pt1(320, 240), pt2(310, 240);
    cv::Point3f pt3d;
    bool ok = TriangulationBasic::triangulatePoint(pt1, pt2, P1, P2, pt3d);
    std::cout << "삼각측량: (" << pt3d.x << ", " << pt3d.y << ", " << pt3d.z << ") "
              << (ok ? "성공" : "실패") << std::endl;

    // Disparity -> Depth
    double depth = TriangulationBasic::disparityToDepth(10.0, 1.0, 500.0);
    std::cout << "Disparity 10px -> Depth: " << depth << "m" << std::endl;

    std::cout << "\n구현 완료 후 테스트: ./test_my_basic" << std::endl;
    return 0;
}
#endif
