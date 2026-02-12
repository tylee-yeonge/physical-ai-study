/**
 * Phase 2 Week 4 - Google Test 단위 테스트
 */

#include <gtest/gtest.h>
#include "../basic.h"

TEST(FeatureMatching, BruteForceMatching)
{
    cv::Mat desc1 = cv::Mat::ones(10, 32, CV_8U) * 100;
    cv::Mat desc2 = cv::Mat::ones(10, 32, CV_8U) * 110;

    std::vector<cv::DMatch> matches;
    double time = FeatureMatchingBasic::matchBruteForce(desc1, desc2, matches, cv::NORM_HAMMING);

    EXPECT_EQ(matches.size(), 10);
    EXPECT_GT(time, 0.0);
}

TEST(FeatureMatching, RatioTest)
{
    cv::Mat desc1 = cv::Mat::ones(10, 32, CV_8U) * 100;
    cv::Mat desc2 = cv::Mat::ones(10, 32, CV_8U) * 110;

    std::vector<cv::DMatch> good_matches;
    int count = FeatureMatchingBasic::ratioTest(desc1, desc2, good_matches, 0.7f);

    EXPECT_GE(count, 0);
}

TEST(FeatureMatching, MatchQuality)
{
    std::vector<cv::DMatch> matches;
    matches.push_back(cv::DMatch(0, 0, 10.0f));
    matches.push_back(cv::DMatch(1, 1, 20.0f));

    double avg_dist = FeatureMatchingBasic::evaluateMatchQuality(matches);

    EXPECT_DOUBLE_EQ(avg_dist, 15.0);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
