#ifndef MINI_VO_H
#define MINI_VO_H

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <vector>
#include <deque>

/**
 * @brief Mini VO 시스템 상태
 */
enum VOStatus {
    INITIALIZING,  // 초기화 중
    TRACKING,      // 추적 중
    LOST           // 추적 실패
};

/**
 * @brief 3D 맵 포인트
 */
struct MapPoint {
    cv::Point3f position;     // 3D 위치
    int age;                  // 나이 (프레임 수)
    int observations;         // 관측 횟수
    cv::Point2f last_2d;      // 마지막 2D 위치
    
    MapPoint() : age(0), observations(0) {}
};

/**
 * @brief VO 시스템 상태
 */
struct VOState {
    VOStatus status;
    
    // 현재 포즈
    cv::Mat R_curr;
    cv::Mat t_curr;
    
    // 3D 맵
    std::vector<MapPoint> map_points;
    
    // 현재 프레임 정보
    int frame_id;
    std::vector<cv::Point2f> curr_keypoints;
    std::vector<int> curr_to_map;  // 현재 특징점 → 맵 인덱스
    
    // 통계
    int num_inliers;
    double reprojection_error;
    double inlier_ratio;
    
    // Keyframe 관리
    int last_keyframe_id;
    cv::Mat last_keyframe;
    
    VOState() : status(INITIALIZING), frame_id(0), 
                num_inliers(0), reprojection_error(0.0),
                inlier_ratio(0.0), last_keyframe_id(-1) {
        R_curr = cv::Mat::eye(3, 3, CV_64F);
        t_curr = cv::Mat::zeros(3, 1, CV_64F);
    }
};

/**
 * @brief Mini VO 클래스
 */
class MiniVO {
public:
    MiniVO(const cv::Mat& K);
    
    /**
     * @brief 프레임 처리
     * @param frame 입력 프레임
     * @return 성공 여부
     */
    bool processFrame(const cv::Mat& frame);
    
    /**
     * @brief 현재 상태 가져오기
     */
    VOState getState() const { return state_; }
    
    /**
     * @brief 궤적 가져오기
     */
    std::vector<cv::Point3f> getTrajectory() const { return trajectory_; }
    
private:
    // 카메라 파라미터
    cv::Mat K_;
    
    // 상태
    VOState state_;
    
    // 이전 프레임
    cv::Mat prev_frame_;
    
    // 궤적
    std::vector<cv::Point3f> trajectory_;
    
    // 특징점 검출기
    cv::Ptr<cv::ORB> detector_;
    
    /**
     * @brief 초기화 시도
     */
    bool tryInitialize(const cv::Mat& frame0, const cv::Mat& frame1);
    
    /**
     * @brief 추적
     */
    bool track(const cv::Mat& prev_frame, const cv::Mat& curr_frame);
    
    /**
     * @brief Keyframe 체크
     */
    bool isKeyframe() const;
    
    /**
     * @brief 맵 업데이트
     */
    void updateMap(const cv::Mat& curr_frame);
    
    /**
     * @brief 나쁜 점 제거
     */
    void cullBadPoints();
    
    /**
     * @brief 베이스라인 체크 (초기화용)
     */
    bool checkBaseline(const std::vector<cv::Point2f>& pts0,
                      const std::vector<cv::Point2f>& pts1) const;
};

#endif // MINI_VO_H
