#include "epipolar.h"

// TODO (W5): 8-Point Algorithm 직접 구현
cv::Mat Epipolar::computeF(
    const std::vector<cv::Point2f>& /*pts1*/,
    const std::vector<cv::Point2f>& /*pts2*/) const
{
    return cv::Mat();
}

std::vector<cv::Vec3f> Epipolar::computeLines(
    const cv::Mat& /*F*/,
    const std::vector<cv::Point2f>& /*pts*/) const
{
    return {};
}
