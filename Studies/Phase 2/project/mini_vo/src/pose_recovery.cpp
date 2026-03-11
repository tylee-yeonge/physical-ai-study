#include "pose_recovery.h"

// TODO (W6): E → R, t 직접 구현 (SVD + Cheirality)
int PoseRecovery::recover(
    const cv::Mat& /*E*/,
    const std::vector<cv::Point2f>& /*pts1*/,
    const std::vector<cv::Point2f>& /*pts2*/,
    cv::Mat& /*R*/,
    cv::Mat& /*t*/) const
{
    return 0;
}
