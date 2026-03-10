#ifndef MINI_SLAM_KEYFRAME_H_
#define MINI_SLAM_KEYFRAME_H_

#include "types.h"

#include <map>
#include <opencv2/core.hpp>
#include <vector>

namespace slam {

/**
 * @brief 키프레임 클래스 (P3-W6)
 *
 * 일반 프레임 중 중요한 프레임만 선택하여 저장한다.
 * 포즈, 특징점, 맵 포인트 연결 정보, 공가시성(covisibility)을 보관한다.
 */
class Keyframe
{
public:
    /**
     * @brief 키프레임 생성
     * @param id    고유 ID
     * @param image 그레이스케일 이미지
     * @param pose  카메라 포즈
     */
    Keyframe(int id, const cv::Mat& image, const Pose& pose);

    /** @brief 공가시성 키프레임 추가 */
    void addCovisibility(int other_kf_id, int num_shared_points);

    /** @brief 공유 맵 포인트 수 기준 상위 N개 공가시성 키프레임 반환 */
    std::vector<int> getTopCovisibleKeyframes(int n) const;

    int id_;                              ///< 고유 ID
    cv::Mat image_;                       ///< 그레이스케일 이미지
    Pose pose_;                           ///< 카메라 포즈 (월드 → 카메라)

    std::vector<cv::KeyPoint> keypoints_; ///< 검출된 특징점
    std::vector<int> map_point_ids_;      ///< 각 특징점에 대응하는 맵 포인트 ID (-1: 없음)

    std::map<int, int> covisibility_;     ///< {키프레임 ID → 공유 맵 포인트 수}
};

/**
 * @brief 키프레임 선택기 (P3-W6)
 *
 * 새 키프레임이 필요한지 판단한다.
 * ORB-SLAM 스타일: 시차, 추적 품질, 최소/최대 간격 기준.
 */
class KeyframeSelector
{
public:
    /**
     * @param min_frames         최소 프레임 간격
     * @param max_frames         최대 프레임 간격
     * @param min_tracked_ratio  추적 특징점 비율 하한 (이하이면 새 키프레임)
     * @param min_parallax_deg   최소 시차 (도 단위)
     */
    KeyframeSelector(int min_frames = 10,
                     int max_frames = 30,
                     double min_tracked_ratio = 0.8,
                     double min_parallax_deg = 5.0);

    /**
     * @brief 새 키프레임 필요 여부 판단
     * @param frames_since_last  마지막 키프레임 이후 프레임 수
     * @param num_tracked        현재 추적 중인 특징점 수
     * @param total_map_points   전체 맵 포인트 수
     * @param avg_parallax_deg   평균 시차 (도)
     * @return 새 키프레임 필요 여부
     */
    bool needNewKeyframe(int frames_since_last,
                         int num_tracked,
                         int total_map_points,
                         double avg_parallax_deg) const;

private:
    int min_frames_;
    int max_frames_;
    double min_tracked_ratio_;
    double min_parallax_deg_;
};

/**
 * @brief 키프레임 제거기 (P3-W6)
 *
 * 90% rule: 맵 포인트의 90% 이상이 다른 3개 이상 키프레임에서
 * 관측되면 해당 키프레임은 중복으로 판단하여 제거한다.
 */
class KeyframeCuller
{
public:
    explicit KeyframeCuller(double redundancy_threshold = 0.9);

    /**
     * @brief 중복 키프레임 제거
     * @param keyframes     키프레임 목록
     * @param map_points    맵 포인트 목록 (관측 횟수 확인용)
     * @return 제거된 키프레임 수
     */
    int cullRedundant(std::vector<Keyframe*>& keyframes,
                      const std::vector<MapPoint>& map_points) const;

private:
    double redundancy_threshold_;
};

}  // namespace slam

#endif  // MINI_SLAM_KEYFRAME_H_
