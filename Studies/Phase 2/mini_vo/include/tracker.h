#ifndef MINI_VO_TRACKER_H_
#define MINI_VO_TRACKER_H_

#include <opencv2/core.hpp>
#include <vector>

/**
 * @brief Lucas-Kanade 광류 추적기 (P2-W8 구현 예정)
 *
 * 구현 내용:
 *  - LK 직접 구현: AᵀA (Structure Tensor) → 최소제곱 → 반복
 *  - 이미지 피라미드 (큰 움직임 처리)
 *  - W4 FeatureMatcher를 이것으로 대체
 */
class Tracker
{
public:
    /**
     * @brief LK 광류로 이전 프레임의 특징점을 현재 프레임에서 추적
     * @param prev_img  이전 그레이스케일 이미지
     * @param curr_img  현재 그레이스케일 이미지
     * @param prev_pts  이전 프레임의 특징점 좌표
     * @param curr_pts  출력: 추적된 현재 프레임 좌표
     * @param status    출력: 추적 성공 여부 (1=성공, 0=실패)
     */
    // TODO (W8): LK 직접 구현 (AᵀA + 피라미드)
    void track(
        const cv::Mat& prev_img,
        const cv::Mat& curr_img,
        const std::vector<cv::Point2f>& prev_pts,
        std::vector<cv::Point2f>& curr_pts,
        std::vector<uchar>& status) const;
};

#endif  // MINI_VO_TRACKER_H_
