/**
 * Phase 3 Week 4 - Google Test
 */

#include <gtest/gtest.h>
#include "../motion_3d3d.h"

TEST(Motion3D3D, SVDAlignment) {
    std::vector<Eigen::Vector3d> src = {
        Eigen::Vector3d(0, 0, 0),
        Eigen::Vector3d(1, 0, 0),
        Eigen::Vector3d(0, 1, 0)
    };
    
    Eigen::Matrix3d R_gt = Eigen::Matrix3d::Identity();
    Eigen::Vector3d t_gt(0.5, 0.3, 0.1);
    
    std::vector<Eigen::Vector3d> tgt;
    Motion3D3D::transformPoints(src, R_gt, t_gt, tgt);
    
    Eigen::Matrix3d R_est;
    Eigen::Vector3d t_est;
    Motion3D3D::alignSVD(src, tgt, R_est, t_est);
    
    EXPECT_NEAR((R_est - R_gt).norm(), 0.0, 1e-6);
    EXPECT_NEAR((t_est - t_gt).norm(), 0.0, 1e-6);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
