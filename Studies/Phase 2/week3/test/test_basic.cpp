/**
 * Phase 2 Week 3 - Google Test 단위 테스트
 */

#include <gtest/gtest.h>
#include "../basic.h"

TEST(FeatureDetection, FASTDetection)
{
    cv::Mat image = cv::Mat::zeros(400, 600, CV_8UC1);
    cv::rectangle(image, cv::Point(100, 100), cv::Point(500, 300), cv::Scalar(255), -1);

    std::vector<cv::KeyPoint> keypoints;
    double time = FeatureDetectionBasic::detectFAST(image, keypoints, 20, true);

    EXPECT_GT(keypoints.size(), 0);
    EXPECT_GT(time, 0.0);
}

TEST(FeatureDetection, ORBDetection)
{
    cv::Mat image = cv::Mat::zeros(400, 600, CV_8UC1);
    cv::circle(image, cv::Point(300, 200), 100, cv::Scalar(255), -1);

    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    double time = FeatureDetectionBasic::detectORB(image, keypoints, descriptors, 50);

    EXPECT_GT(keypoints.size(), 0);
    EXPECT_EQ(descriptors.cols, 32);  // ORB = 256 bits = 32 bytes
}

TEST(FeatureDetection, Distribution)
{
    cv::Mat image = cv::Mat::zeros(400, 600, CV_8UC1);
    std::vector<cv::KeyPoint> keypoints;
    keypoints.push_back(cv::KeyPoint(100, 100, 1));
    keypoints.push_back(cv::KeyPoint(500, 300, 1));

    cv::Mat dist = FeatureDetectionBasic::analyzeDistribution(keypoints, cv::Size(600, 400), 8);

    EXPECT_EQ(dist.rows, 8);
    EXPECT_EQ(dist.cols, 8);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
