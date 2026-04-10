#include "tracker.h"

// TODO (W8): LK 직접 구현 (AᵀA + 피라미드)
void Tracker::track(
    const cv::Mat& /*prev_img*/,
    const cv::Mat& /*curr_img*/,
    const std::vector<cv::Point2f>& /*prev_pts*/,
    std::vector<cv::Point2f>& curr_pts,
    std::vector<uchar>& status) const
{
    curr_pts.clear();
    status.clear();
}
