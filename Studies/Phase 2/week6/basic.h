#ifndef TRIANGULATION_BASIC_H
#define TRIANGULATION_BASIC_H

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <vector>
#include <string>

/**
 * @brief 삼각측량 (Triangulation) 기본 클래스
 *
 * Week 6: 삼각측량과 3D 복원
 * - 2D 대응점에서 3D 점 복원
 * - Linear Triangulation (DLT)
 * - 재투영 오차 계산
 * - Stereo Depth 추정
 */
class TriangulationBasic
{
   public:
    /**
     * @brief 삼각측량으로 3D 점 복원
     * @param pt1 첫 번째 이미지의 2D 점
     * @param pt2 두 번째 이미지의 2D 점
     * @param P1 첫 번째 카메라의 투영 행렬 [R|t]
     * @param P2 두 번째 카메라의 투영 행렬 [R|t]
     * @param point3d 복원된 3D 점 (출력)
     * @return 성공 여부
     */
    static bool triangulatePoint(const cv::Point2f& pt1, const cv::Point2f& pt2, const cv::Mat& P1,
                                 const cv::Mat& P2, cv::Point3f& point3d);

    /**
     * @brief 여러 점 삼각측량
     * @param points1 첫 번째 이미지의 2D 점들
     * @param points2 두 번째 이미지의 2D 점들
     * @param P1 첫 번째 카메라의 투영 행렬
     * @param P2 두 번째 카메라의 투영 행렬
     * @param points3d 복원된 3D 점들 (출력)
     */
    static void triangulatePoints(const std::vector<cv::Point2f>& points1,
                                  const std::vector<cv::Point2f>& points2, const cv::Mat& P1,
                                  const cv::Mat& P2, std::vector<cv::Point3f>& points3d);

    /**
     * @brief 재투영 오차 계산
     * @param point3d 3D 점
     * @param point2d 실제 관측된 2D 점
     * @param P 카메라 투영 행렬
     * @return 재투영 오차 (픽셀)
     */
    static double reprojectionError(const cv::Point3f& point3d, const cv::Point2f& point2d,
                                    const cv::Mat& P);

    /**
     * @brief 평균 재투영 오차 계산
     * @param points3d 3D 점들
     * @param points2d 2D 관측점들
     * @param P 카메라 투영 행렬
     * @return 평균 재투영 오차
     */
    static double averageReprojectionError(const std::vector<cv::Point3f>& points3d,
                                           const std::vector<cv::Point2f>& points2d,
                                           const cv::Mat& P);

    /**
     * @brief Stereo 카메라로 depth 계산
     * @param disparity 시차 (픽셀)
     * @param baseline 베이스라인 (m)
     * @param focal_length 초점거리 (픽셀)
     * @return 깊이 (m)
     */
    static double disparityToDepth(double disparity, double baseline, double focal_length);

    /**
     * @brief 점이 카메라 앞에 있는지 확인 (Cheirality Check)
     * @param point3d 3D 점
     * @param R 카메라 회전
     * @param t 카메라 이동
     * @return 카메라 앞에 있으면 true
     */
    static bool isInFrontOfCamera(const cv::Point3f& point3d, const cv::Mat& R, const cv::Mat& t);

    /**
     * @brief 삼각측량 품질 평가
     * @param points3d 복원된 3D 점들
     * @param points1 첫 번째 이미지 점들
     * @param points2 두 번째 이미지 점들
     * @param P1 첫 번째 카메라 투영 행렬
     * @param P2 두 번째 카메라 투영 행렬
     */
    static void evaluateTriangulation(const std::vector<cv::Point3f>& points3d,
                                      const std::vector<cv::Point2f>& points1,
                                      const std::vector<cv::Point2f>& points2, const cv::Mat& P1,
                                      const cv::Mat& P2);

    /**
     * @brief 3D 점군 시각화 (간단한 통계)
     * @param points3d 3D 점들
     */
    static void visualize3DPoints(const std::vector<cv::Point3f>& points3d);

    /**
     * @brief 전체 파이프라인 데모
     * @param K 카메라 내부 파라미터
     * @param R 상대 회전
     * @param t 상대 이동
     */
    static void demoPipeline(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t);
};

#endif  // TRIANGULATION_BASIC_H
