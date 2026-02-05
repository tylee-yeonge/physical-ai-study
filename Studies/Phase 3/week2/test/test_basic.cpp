/**
 * Phase 3 Week 2 - Google Test
 */

#include <gtest/gtest.h>
#include "../motion_2d2d.h"

TEST(Motion2D2D, Normalization) {
    cv::Mat K = (cv::Mat_<double>(3,3) << 600,0,400, 0,600,300, 0,0,1);
    std::vector<cv::Point2f> pts = {cv::Point2f(400, 300)};
    std::vector<cv::Point2f> norm;
    
    Motion2D2D::normalizePoints(K, pts, norm);
    
    EXPECT_NEAR(norm[0].x, 0.0, 1e-6);
    EXPECT_NEAR(norm[0].y, 0.0, 1e-6);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
