#ifndef EPIPOLAR_GEOMETRY_BASIC_H
#define EPIPOLAR_GEOMETRY_BASIC_H

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <vector>
#include <string>

/**
 * @brief 에피폴라 기하학 기본 클래스
 *
 * Week 5: 에피폴라 기하학 (Epipolar Geometry)
 * - Essential Matrix와 Fundamental Matrix
 * - 에피폴라 제약 (Epipolar Constraint)
 * - 포즈 복원 (Pose Recovery)
 */
class EpipolarGeometryBasic
{
   public:
    /**
     * @brief Essential Matrix 추정
     * @param points1 첫 번째 이미지의 특징점 (정규화 좌표)
     * @param points2 두 번째 이미지의 특징점 (정규화 좌표)
     * @param essential Essential Matrix (출력)
     * @param method 추정 방법 (cv::RANSAC 등)
     * @return inlier 수
     */
    static int estimateEssential(const std::vector<cv::Point2f>& points1,
                                 const std::vector<cv::Point2f>& points2, cv::Mat& essential,
                                 int method = cv::RANSAC);

    /**
     * @brief Fundamental Matrix 추정
     * @param points1 첫 번째 이미지의 특징점 (픽셀 좌표)
     * @param points2 두 번째 이미지의 특징점 (픽셀 좌표)
     * @param fundamental Fundamental Matrix (출력)
     * @param method 추정 방법
     * @return inlier 수
     */
    static int estimateFundamental(const std::vector<cv::Point2f>& points1,
                                   const std::vector<cv::Point2f>& points2, cv::Mat& fundamental,
                                   int method = cv::FM_RANSAC);

    /**
     * @brief Essential Matrix에서 R, t 복원
     * @param essential Essential Matrix
     * @param points1 첫 번째 이미지의 특징점
     * @param points2 두 번째 이미지의 특징점
     * @param K 카메라 행렬
     * @param R 회전 행렬 (출력)
     * @param t 이동 벡터 (출력)
     * @return 성공 여부
     */
    static bool recoverPose(const cv::Mat& essential, const std::vector<cv::Point2f>& points1,
                            const std::vector<cv::Point2f>& points2, const cv::Mat& K, cv::Mat& R,
                            cv::Mat& t);

    /**
     * @brief 에피폴라 선 계산
     * @param point 한 이미지의 점
     * @param fundamental Fundamental Matrix
     * @param for_image2 true면 image2의 선, false면 image1의 선
     * @return 에피폴라 선 (ax + by + c = 0)
     */
    static cv::Vec3f computeEpipolarLine(const cv::Point2f& point, const cv::Mat& fundamental,
                                         bool for_image2 = true);

    /**
     * @brief 에피폴라 제약 검증
     * @param point1 첫 번째 이미지의 점
     * @param point2 두 번째 이미지의 점
     * @param essential_or_fundamental E 또는 F 행렬
     * @return 제약 오차 (0에 가까울수록 정확)
     */
    static double verifyEpipolarConstraint(const cv::Point2f& point1, const cv::Point2f& point2,
                                           const cv::Mat& essential_or_fundamental);

    /**
     * @brief 에피폴라 선 시각화
     * @param img1 첫 번째 이미지
     * @param img2 두 번째 이미지
     * @param points1 첫 번째 이미지의 점들
     * @param points2 두 번째 이미지의 점들
     * @param fundamental Fundamental Matrix
     * @param output 출력 이미지
     */
    static void visualizeEpipolarLines(const cv::Mat& img1, const cv::Mat& img2,
                                       const std::vector<cv::Point2f>& points1,
                                       const std::vector<cv::Point2f>& points2,
                                       const cv::Mat& fundamental, cv::Mat& output);

    /**
     * @brief E와 F의 관계 검증
     * @param K 카메라 행렬
     * @param essential Essential Matrix
     * @param fundamental Fundamental Matrix
     * @return 차이 (F = K'^-T * E * K^-1 관계 확인)
     */
    static double verifyEF_Relationship(const cv::Mat& K, const cv::Mat& essential,
                                        const cv::Mat& fundamental);

    /**
     * @brief 전체 파이프라인 데모
     * @param img1 첫 번째 이미지
     * @param img2 두 번째 이미지
     * @param K 카메라 행렬
     */
    static void demoPipeline(const cv::Mat& img1, const cv::Mat& img2, const cv::Mat& K);
};

#endif  // EPIPOLAR_GEOMETRY_BASIC_H
