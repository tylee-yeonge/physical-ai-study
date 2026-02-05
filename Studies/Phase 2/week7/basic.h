#ifndef PNP_BASIC_H
#define PNP_BASIC_H

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <vector>
#include <string>

/**
 * @brief PnP (Perspective-n-Point) 기본 클래스
 * 
 * Week 7: PnP - 3D-2D 대응으로 카메라 포즈 추정
 * - P3P (최소 3점)
 * - EPnP (효율적인 n점)
 * - RANSAC과 결합
 * - Visual Odometry에서 활용
 */
class PnPBasic {
public:
    /**
     * @brief PnP로 카메라 포즈 추정
     * @param points3d 3D 점들 (월드 좌표계)
     * @param points2d 2D 관측점들 (이미지 좌표)
     * @param K 카메라 내부 파라미터
     * @param rvec 회전 벡터 (출력)
     * @param tvec 이동 벡터 (출력)
     * @param method 추정 방법 (cv::SOLVEPNP_ITERATIVE, EPNP 등)
     * @return 성공 여부
     */
    static bool solvePnP(const std::vector<cv::Point3f>& points3d,
                        const std::vector<cv::Point2f>& points2d,
                        const cv::Mat& K,
                        cv::Mat& rvec,
                        cv::Mat& tvec,
                        int method = cv::SOLVEPNP_ITERATIVE);
    
    /**
     * @brief RANSAC PnP (outlier 제거)
     * @param points3d 3D 점들
     * @param points2d 2D 관측점들
     * @param K 카메라 내부 파라미터
     * @param rvec 회전 벡터 (출력)
     * @param tvec 이동 벡터 (출력)
     * @param inliers Inlier 인덱스 (출력)
     * @param reprojection_error RANSAC 임계값 (픽셀)
     * @return Inlier 개수
     */
    static int solvePnPRansac(const std::vector<cv::Point3f>& points3d,
                             const std::vector<cv::Point2f>& points2d,
                             const cv::Mat& K,
                             cv::Mat& rvec,
                             cv::Mat& tvec,
                             std::vector<int>& inliers,
                             double reprojection_error = 8.0);
    
    /**
     * @brief 회전 벡터를 회전 행렬로 변환
     * @param rvec 회전 벡터 (Rodrigues)
     * @param R 회전 행렬 (출력)
     */
    static void rodrigues(const cv::Mat& rvec, cv::Mat& R);
    
    /**
     * @brief 포즈 품질 평가
     * @param points3d 3D 점들
     * @param points2d 2D 관측점들
     * @param K 카메라 내부 파라미터
     * @param rvec 회전 벡터
     * @param tvec 이동 벡터
     * @return 평균 재투영 오차 (픽셀)
     */
    static double evaluatePose(const std::vector<cv::Point3f>& points3d,
                               const std::vector<cv::Point2f>& points2d,
                               const cv::Mat& K,
                               const cv::Mat& rvec,
                               const cv::Mat& tvec);
    
    /**
     * @brief 2D-3D 대응 시각화
     * @param img 이미지
     * @param points3d 3D 점들
     * @param points2d 2D 관측점들
     * @param K 카메라 내부 파라미터
     * @param rvec 회전 벡터
     * @param tvec 이동 벡터
     * @param output 출력 이미지
     */
    static void visualizePnP(const cv::Mat& img,
                            const std::vector<cv::Point3f>& points3d,
                            const std::vector<cv::Point2f>& points2d,
                            const cv::Mat& K,
                            const cv::Mat& rvec,
                            const cv::Mat& tvec,
                            cv::Mat& output);
    
    /**
     * @brief Visual Odometry 시뮬레이션
     * @param K 카메라 내부 파라미터
     */
    static void demoVisualOdometry(const cv::Mat& K);
    
    /**
     * @brief PnP vs 다른 방법 비교
     * - 2D-2D (Essential Matrix)
     * - 3D-2D (PnP)
     * - 3D-3D (ICP)
     */
    static void compareMethods();
    
    /**
     * @brief 전체 파이프라인 데모
     * @param K 카메라 내부 파라미터
     */
    static void demoPipeline(const cv::Mat& K);
};

#endif // PNP_BASIC_H
