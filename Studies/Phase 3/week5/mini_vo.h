#ifndef PHASE3_WEEK5_MINI_VO_H_
#define PHASE3_WEEK5_MINI_VO_H_

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

/**
 * @brief Mini VO 파이프라인 기초 클래스
 *
 * Week 2 (Essential Matrix 초기화) + Week 3 (PnP 추적) 통합.
 * 합성 데이터로 VO 파이프라인 전과정을 시연한다.
 *
 * 파이프라인:
 *   detectFeatures → trackFeatures → initialize2D2D
 *   → trackPnP → triangulateNewPoints → computeTrajectoryError
 */
class MiniVOBasic
{
public:
    /**
     * @brief 특징점 검출 (GFTT)
     * @param frame 그레이스케일 이미지
     * @param max_corners 최대 코너 수
     * @return 검출된 2D 특징점
     */
    static std::vector<cv::Point2f> detect_features(
        const cv::Mat& frame, int max_corners = 500);

    /**
     * @brief Optical Flow 기반 특징점 추적
     * @param prev 이전 프레임
     * @param curr 현재 프레임
     * @param prev_pts 이전 프레임 특징점
     * @param curr_pts 현재 프레임에서 추적된 점 (출력)
     * @param status 추적 성공 여부 (출력)
     */
    static void track_features(
        const cv::Mat& prev, const cv::Mat& curr,
        const std::vector<cv::Point2f>& prev_pts,
        std::vector<cv::Point2f>& curr_pts,
        std::vector<uchar>& status);

    /**
     * @brief 2D-2D 초기화 (Essential Matrix, Week 2)
     * @param pts1 첫 번째 프레임 점
     * @param pts2 두 번째 프레임 점
     * @param K 카메라 내부 파라미터
     * @param R 회전 행렬 (출력)
     * @param t 이동 벡터 (출력)
     * @param map_points 삼각측량된 3D 점 (출력)
     * @param map_keypoints map_points에 대응하는 프레임2의 2D 점 (출력)
     * @return 성공 여부
     */
    static bool initialize_2d2d(
        const std::vector<cv::Point2f>& pts1,
        const std::vector<cv::Point2f>& pts2,
        const cv::Mat& K,
        cv::Mat& R, cv::Mat& t,
        std::vector<cv::Point3f>& map_points,
        std::vector<cv::Point2f>& map_keypoints);

    /**
     * @brief 3D-2D 추적 (PnP + RANSAC, Week 3)
     * @param pts3d 3D 맵 점
     * @param pts2d 현재 프레임 관측
     * @param K 카메라 내부 파라미터
     * @param R 회전 행렬 (출력)
     * @param t 이동 벡터 (출력)
     * @param inliers inlier 인덱스 (출력)
     * @return inlier 수 (0이면 실패)
     */
    static int track_pnp(
        const std::vector<cv::Point3f>& pts3d,
        const std::vector<cv::Point2f>& pts2d,
        const cv::Mat& K,
        cv::Mat& R, cv::Mat& t,
        std::vector<int>& inliers);

    /**
     * @brief 새 3D 점 삼각측량
     * @param pts1 프레임1 관측
     * @param pts2 프레임2 관측
     * @param K 카메라 내부 파라미터
     * @param R1, t1 프레임1 포즈
     * @param R2, t2 프레임2 포즈
     * @return 삼각측량된 3D 점
     */
    static std::vector<cv::Point3f> triangulate_new_points(
        const std::vector<cv::Point2f>& pts1,
        const std::vector<cv::Point2f>& pts2,
        const cv::Mat& K,
        const cv::Mat& R1, const cv::Mat& t1,
        const cv::Mat& R2, const cv::Mat& t2);

    /**
     * @brief 궤적 오차 계산 (ATE RMSE)
     * @param est_t 추정된 이동 벡터 시퀀스
     * @param gt_t GT 이동 벡터 시퀀스
     * @return ATE RMSE (미터)
     */
    static double compute_trajectory_error(
        const std::vector<cv::Mat>& est_t,
        const std::vector<cv::Mat>& gt_t);

    // ── 합성 데이터 ──

    /**
     * @brief 합성 이미지 시퀀스 생성 (전진 궤적 + 3D 점 투영)
     * @param K 카메라 내부 파라미터
     * @param num_frames 프레임 수
     * @param images 생성된 이미지 (출력)
     * @param gt_R GT 회전 시퀀스 (출력)
     * @param gt_t GT 이동 시퀀스 (출력)
     */
    static void generate_synthetic_sequence(
        const cv::Mat& K, int num_frames,
        std::vector<cv::Mat>& images,
        std::vector<cv::Mat>& gt_R,
        std::vector<cv::Mat>& gt_t);

    /**
     * @brief 전체 VO 파이프라인 데모
     */
    static void demo();
};

#endif  // PHASE3_WEEK5_MINI_VO_H_
