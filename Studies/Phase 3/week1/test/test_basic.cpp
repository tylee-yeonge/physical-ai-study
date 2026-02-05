/**
 * Phase 3 Week 1 - Google Test
 */

#include <gtest/gtest.h>
#include "../vo_types.h"

using namespace vo;

TEST(VO, PoseInverse) {
    Pose p;
    p.t = Eigen::Vector3d(1, 0, 0);
    Pose p_inv = p.inverse();
    
    EXPECT_NEAR(p_inv.t.x(), -1.0, 1e-6);
}

TEST(VO, PoseComposition) {
    Pose p1, p2;
    p1.t = Eigen::Vector3d(1, 0, 0);
    p2.t = Eigen::Vector3d(0, 1, 0);
    
    Pose p_comp = p1 * p2;
    EXPECT_NEAR(p_comp.t.x(), 1.0, 1e-6);
    EXPECT_NEAR(p_comp.t.y(), 1.0, 1e-6);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
