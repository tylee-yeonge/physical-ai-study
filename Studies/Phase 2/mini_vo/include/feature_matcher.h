#ifndef MINI_VO_FEATURE_MATCHER_H_
#define MINI_VO_FEATURE_MATCHER_H_

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <vector>

/**
 * @brief 특징점 매칭기 (P2-W4 구현 예정)
 *
 * 구현 내용:
 *  - Brute-Force 매칭 + Lowe's Ratio Test
 *  - RANSAC으로 아웃라이어 제거 (직접 구현)
 */
class FeatureMatcher
{
public:
    // TODO (W4): BF 매칭 + Ratio Test + RANSAC 직접 구현
    std::vector<cv::DMatch> match(
        const cv::Mat& desc1,
        const cv::Mat& desc2) const;
};

#endif  // MINI_VO_FEATURE_MATCHER_H_
