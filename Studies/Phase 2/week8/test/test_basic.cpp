/**
 * Phase 2 Week 8 - Google Test
 */

#include <gtest/gtest.h>
#include "../basic.h"

TEST(OpticalFlow, LucasKanade)
{
    cv::Mat frame1 = cv::Mat::zeros(300, 400, CV_8UC1);
    cv::Mat frame2 = cv::Mat::zeros(300, 400, CV_8UC1);

    cv::circle(frame1, cv::Point(150, 150), 30, cv::Scalar(255), -1);
    cv::circle(frame2, cv::Point(180, 160), 30, cv::Scalar(255), -1);

    std::vector<cv::Point2f> pts1 = {cv::Point2f(150, 150)};
    std::vector<cv::Point2f> pts2;
    std::vector<uchar> status;

    double success_rate = OpticalFlowBasic::lucasKanade(frame1, frame2, pts1, pts2, status);

    EXPECT_GT(success_rate, 0.5);
    EXPECT_EQ(status.size(), pts1.size());
}

TEST(OpticalFlow, FlowEvaluation)
{
    std::vector<cv::Point2f> pts1 = {cv::Point2f(100, 100)};
    std::vector<cv::Point2f> pts2 = {cv::Point2f(130, 110)};
    std::vector<uchar> status = {1};

    double avg_motion = OpticalFlowBasic::evaluateFlow(pts1, pts2, status);

    EXPECT_NEAR(avg_motion, 31.62, 0.1);  // sqrt(30^2 + 10^2)
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
