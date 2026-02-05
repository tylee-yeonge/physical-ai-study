/**
 * Phase 2 Week 7 - Google Test
 */

#include <gtest/gtest.h>
#include "../basic.h"

TEST(PnP, BasicSolve) {
    std::vector<cv::Point3f> pts3d = {
        cv::Point3f(0,0,5), cv::Point3f(1,0,5),
        cv::Point3f(0,1,5), cv::Point3f(1,1,5)
    };
    
    cv::Mat K = (cv::Mat_<double>(3,3) << 600,0,400, 0,600,300, 0,0,1);
    cv::Mat rvec_gt = (cv::Mat_<double>(3,1) << 0,0,0);
    cv::Mat tvec_gt = (cv::Mat_<double>(3,1) << 0.5,0,0);
    
    std::vector<cv::Point2f> pts2d;
    cv::projectPoints(pts3d, rvec_gt, tvec_gt, K, cv::Mat(), pts2d);
    
    cv::Mat rvec, tvec;
    bool success = PnPBasic::solvePnP(pts3d, pts2d, K, rvec, tvec);
    
    EXPECT_TRUE(success);
    EXPECT_LT(cv::norm(tvec - tvec_gt), 0.01);
}

TEST(PnP, ReprojectionError) {
    std::vector<cv::Point3f> pts3d = {cv::Point3f(0,0,5)};
    std::vector<cv::Point2f> pts2d = {cv::Point2f(400,300)};
    cv::Mat K = (cv::Mat_<double>(3,3) << 600,0,400, 0,600,300, 0,0,1);
    cv::Mat rvec = cv::Mat::zeros(3,1,CV_64F);
    cv::Mat tvec = cv::Mat::zeros(3,1,CV_64F);
    
    double error = PnPBasic::evaluatePose(pts3d, pts2d, K, rvec, tvec);
    EXPECT_LT(error, 1.0);  // 작은 오차
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
