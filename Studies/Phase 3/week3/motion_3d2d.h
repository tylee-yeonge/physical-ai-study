#ifndef MOTION_3D2D_H
#define MOTION_3D2D_H

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <vector>

/**
 * @brief 3D-2D 모션 추정 클래스 (PnP)
 */
class Motion3D2D
{
   public:
    /**
     * @brief EPnP 알고리즘
     * @param points3d 3D 점들 (월드 좌표)
     * @param points2d 2D 관측 (픽셀 좌표)
     * @param K 카메라 내부 파라미터
     * @param rvec 회전 벡터 (출력)
     * @param tvec 이동 벡터 (출력)
     * @return 성공 여부
     */
    static bool solvePnP(const std::vector<cv::Point3f>& points3d,
                         const std::vector<cv::Point2f>& points2d, const cv::Mat& K, cv::Mat& rvec,
                         cv::Mat& tvec);

    /**
     * @brief PnP + RANSAC
     * @param points3d 3D 점들
     * @param points2d 2D 관측
     * @param K 카메라 내부 파라미터
     * @param rvec 회전 벡터 (출력)
     * @param tvec 이동 벡터 (출력)
     * @param inliers Inlier 마스크 (출력)
     * @return Inlier 개수
     */
    static int solvePnPRansac(const std::vector<cv::Point3f>& points3d,
                              const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                              cv::Mat& rvec, cv::Mat& tvec, std::vector<int>& inliers);

    /**
     * @brief 재투영 오차 계산
     * @param points3d 3D 점들
     * @param points2d 2D 관측
     * @param K 카메라 내부 파라미터
     * @param rvec 회전 벡터
     * @param tvec 이동 벡터
     * @return RMS 재투영 오차 (픽셀)
     */
    static double computeReprojectionError(const std::vector<cv::Point3f>& points3d,
                                           const std::vector<cv::Point2f>& points2d,
                                           const cv::Mat& K, const cv::Mat& rvec,
                                           const cv::Mat& tvec);

    /**
     * @brief 3D 점 투영
     * @param points3d 3D 점들
     * @param K 카메라 내부 파라미터
     * @param rvec 회전 벡터
     * @param tvec 이동 벡터
     * @param projected 투영된 2D 점들 (출력)
     */
    static void projectPoints(const std::vector<cv::Point3f>& points3d, const cv::Mat& K,
                              const cv::Mat& rvec, const cv::Mat& tvec,
                              std::vector<cv::Point2f>& projected);

    /**
     * @brief VO 추적 시뮬레이션
     * @param map_points 초기 3D 맵
     * @param K 카메라 내부 파라미터
     * @param num_frames 프레임 수
     */
    static void simulateTracking(const std::vector<cv::Point3f>& map_points, const cv::Mat& K,
                                 int num_frames);

    /**
     * @brief 전체 데모
     */
    static void demo();
};

#endif  // MOTION_3D2D_H
