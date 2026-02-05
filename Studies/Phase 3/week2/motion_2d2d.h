#ifndef MOTION_2D2D_H
#define MOTION_2D2D_H

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <vector>

/**
 * @brief 2D-2D 모션 추정 클래스
 * 
 * Essential Matrix 기반 상대 포즈 추정
 */
class Motion2D2D {
public:
    /**
     * @brief Essential Matrix 추정
     * @param points1 첫 번째 이미지의 정규화 좌표
     * @param points2 두 번째 이미지의 정규화 좌표
     * @param E Essential Matrix (출력)
     * @param inliers Inlier 마스크 (출력)
     * @return Inlier 개수
     */
    static int estimateEssential(const std::vector<cv::Point2f>& points1,
                                 const std::vector<cv::Point2f>& points2,
                                 cv::Mat& E,
                                 std::vector<uchar>& inliers);
    
    /**
     * @brief E에서 R, t 복원
     * @param E Essential Matrix
     * @param points1 첫 번째 이미지 점들
     * @param points2 두 번째 이미지 점들
     * @param R 회전 (출력)
     * @param t 이동 (출력)
     * @param inliers Inlier 마스크 (출력)
     * @return Cheirality check 통과한 점 개수
     */
    static int recoverPose(const cv::Mat& E,
                          const std::vector<cv::Point2f>& points1,
                          const std::vector<cv::Point2f>& points2,
                          cv::Mat& R,
                          cv::Mat& t,
                          std::vector<uchar>& inliers);
    
    /**
     * @brief 픽셀 → 정규화 좌표 변환
     * @param K 카메라 내부 파라미터
     * @param points 픽셀 좌표
     * @param normalized 정규화 좌표 (출력)
     */
    static void normalizePoints(const cv::Mat& K,
                                const std::vector<cv::Point2f>& points,
                                std::vector<cv::Point2f>& normalized);
    
    /**
     * @brief 초기 3D 맵 생성 (삼각측량)
     * @param K 카메라 내부 파라미터
     * @param R 회전
     * @param t 이동
     * @param points1 첫 번째 이미지 점들
     * @param points2 두 번째 이미지 점들
     * @param points3d 복원된 3D 점들 (출력)
     */
    static void triangulate(const cv::Mat& K,
                           const cv::Mat& R,
                           const cv::Mat& t,
                           const std::vector<cv::Point2f>& points1,
                           const std::vector<cv::Point2f>& points2,
                           std::vector<cv::Point3f>& points3d);
    
    /**
     * @brief 2D-2D 전체 파이프라인
     * @param K 카메라 내부 파라미터
     * @param points1 첫 번째 이미지 점들
     * @param points2 두 번째 이미지 점들
     * @param R 회전 (출력)
     * @param t 이동 (출력)
     * @param points3d 3D 점들 (출력)
     * @return 성공 여부
     */
    static bool pipeline(const cv::Mat& K,
                        const std::vector<cv::Point2f>& points1,
                        const std::vector<cv::Point2f>& points2,
                        cv::Mat& R,
                        cv::Mat& t,
                        std::vector<cv::Point3f>& points3d);
    
    /**
     * @brief 전체 데모
     */
    static void demo();
};

#endif // MOTION_2D2D_H
