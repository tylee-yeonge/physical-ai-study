/**
 * Phase 3 Week 3 - Google Test
 */

#include <gtest/gtest.h>
#include "../motion_3d2d.h"

TEST(Motion3D2D, PnP)
{
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);

    std::vector<cv::Point3f> pts3d = {cv::Point3f(0, 0, 5), cv::Point3f(1, 0, 5),
                                      cv::Point3f(0, 1, 5), cv::Point3f(1, 1, 5)};

    cv::Mat rvec = (cv::Mat_<double>(3, 1) << 0, 0, 0);
    cv::Mat tvec = (cv::Mat_<double>(3, 1) << 0, 0, 0);

    std::vector<cv::Point2f> pts2d;
    Motion3D2D::projectPoints(pts3d, K, rvec, tvec, pts2d);

    cv::Mat rvec_est, tvec_est;
    bool success = Motion3D2D::solvePnP(pts3d, pts2d, K, rvec_est, tvec_est);

    EXPECT_TRUE(success);
    EXPECT_NEAR(cv::norm(rvec - rvec_est), 0.0, 0.1);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
