/**
 * Phase 2 Week 6 - Google Test
 */

#include <gtest/gtest.h>
#include "../basic.h"

TEST(Triangulation, DepthCalculation)
{
    double depth = TriangulationBasic::disparityToDepth(60.0, 0.12, 600.0);
    EXPECT_NEAR(depth, 1.2, 0.01);  // (0.12 * 600) / 60 = 1.2
}

TEST(Triangulation, ReprojectionError)
{
    cv::Point3f pt3d(1.0, 0.5, 3.0);
    cv::Point2f pt2d(600.0, 400.0);

    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);
    cv::Mat P = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P(cv::Rect(0, 0, 3, 3)));

    double error = TriangulationBasic::reprojectionError(pt3d, pt2d, P);
    EXPECT_LT(error, 1000.0);  // 합리적인 오차
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
