#ifndef KEYFRAME_MANAGER_H
#define KEYFRAME_MANAGER_H

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <vector>
#include <map>

/**
 * @brief 맵 포인트 (간략화)
 */
struct MapPoint {
    cv::Point3f position;
    int num_observations;
    std::vector<int> observed_by_keyframes;  // Keyframe IDs
    
    MapPoint() : num_observations(0) {}
};

/**
 * @brief Keyframe 클래스
 */
class Keyframe {
public:
    int id;
    cv::Mat image;
    
    // 포즈
    Eigen::Matrix3d R;
    Eigen::Vector3d t;
    
    // 특징점
    std::vector<cv::Point2f> keypoints;
    cv::Mat descriptors;
    
    // 맵 포인트 (인덱스)
    std::vector<int> map_point_indices;
    
    // Covisibility
    std::map<Keyframe*, int> covisible_keyframes;
    
    // 통계
    int num_tracked;
    
    Keyframe(int id, const cv::Mat& img);
    
    /**
     * @brief Covisibility 추가
     */
    void addCovisibility(Keyframe* other, int num_shared);
    
    /**
     * @brief Covisible Keyframes 가져오기
     */
    std::vector<Keyframe*> getCovisibleKeyframes(int min_shared = 15);
};

/**
 * @brief Keyframe 선택기
 */
class KeyframeSelector {
public:
    KeyframeSelector(int min_frames = 20,
                    int max_frames = 30,
                    double min_tracked_ratio = 0.5,
                    double min_parallax = 10.0);
    
    /**
     * @brief 새 Keyframe이 필요한지 판단
     * @param num_frames_since_last 마지막 KF 이후 프레임 수
     * @param num_tracked 현재 추적된 점 개수
     * @param total_map_points 전체 맵 포인트 개수
     * @param avg_parallax 평균 시차 (픽셀)
     * @return true if new keyframe needed
     */
    bool needNewKeyframe(int num_frames_since_last,
                       int num_tracked,
                        int total_map_points,
                        double avg_parallax);
    
private:
    int min_frames_;
    int max_frames_;
    double min_tracked_ratio_;
    double min_parallax_;
};

/**
 * @brief Keyframe Culler
 */
class KeyframeCuller {
public:
    KeyframeCuller(double redundancy_threshold = 0.9);
    
    /**
     * @brief 중복 Keyframe 제거
     * @param keyframes Keyframe 목록
     * @param map_points 맵 포인트 목록
     */
    void cullRedundantKeyframes(std::vector<Keyframe*>& keyframes,
                               const std::vector<MapPoint>& map_points);
    
private:
    double redundancy_threshold_;
    
    /**
     * @brief Keyframe이 중복인지 판단
     */
    bool isRedundant(const Keyframe* kf,
                    const std::vector<MapPoint>& map_points);
};

/**
 * @brief Covisibility Graph 관리
 */
class CovisibilityGraph {
public:
    /**
     * @brief Keyframe 간 covisibility 업데이트
     */
    void updateCovisibility(Keyframe* kf1,
                           Keyframe* kf2,
                           const std::vector<MapPoint>& map_points);
    
    /**
     * @brief Local Keyframes 가져오기 (BA용)
     */
    std::vector<Keyframe*> getLocalKeyframes(Keyframe* curr_kf,
                                            int max_keyframes = 10);
};

#endif // KEYFRAME_MANAGER_H
