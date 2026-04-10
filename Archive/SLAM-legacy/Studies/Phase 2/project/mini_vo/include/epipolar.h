#ifndef MINI_VO_EPIPOLAR_H_
#define MINI_VO_EPIPOLAR_H_

#include <opencv2/core.hpp>
#include <vector>

/**
 * @brief 에피폴라 기하학 (P2-W5 구현 예정)
 *
 * 구현 내용:
 *  - 8-Point Algorithm 직접 구현 (Hartley 정규화 포함)
 *  - F Matrix → E Matrix 변환
 *  - 에피폴라 선 시각화
 */
class Epipolar
{
public:
    // TODO (W5): 8-Point Algorithm 직접 구현
    cv::Mat computeF(
        const std::vector<cv::Point2f>& pts1,
        const std::vector<cv::Point2f>& pts2) const;

    // TODO (W5): 에피폴라 선 계산
    std::vector<cv::Vec3f> computeLines(
        const cv::Mat& F,
        const std::vector<cv::Point2f>& pts) const;
};

#endif  // MINI_VO_EPIPOLAR_H_
