/**
 * Phase 2 Week 5 - Google Test
 */

#include <gtest/gtest.h>
#include "../basic.h"

TEST(EpipolarGeometry, EpipolarConstraint) {
    cv::Mat E = cv::Mat::eye(3, 3, CV_64F);
    cv::Point2f p1(1.0, 1.0);
    cv::Point2f p2(1.0, 1.0);
    
    double error = EpipolarGeometryBasic::verifyEpipolarConstraint(p1, p2, E);
    
    EXPECT_LT(error, 1.0);  // 작은 오차 기대
}

TEST(EpipolarGeometry, EpipolarLine) {
    cv::Mat F = cv::Mat::eye(3, 3, CV_64F);
    cv::Point2f p(100, 150);
    
    cv::Vec3f line = EpipolarGeometryBasic::computeEpipolarLine(p, F, true);
    
    EXPECT_EQ(line.val[0] * line.val[0] + line.val[1] * line.val[1], 1.0);  // 정규화
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
