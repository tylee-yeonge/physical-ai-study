#include <gtest/gtest.h>
#include "../mini_vo.h"

TEST(MiniVOTest, Initialization) {
    cv::Mat K = (cv::Mat_<double>(3,3) <<
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
    
    MiniVO vo(K);
    
    VOState state = vo.getState();
    EXPECT_EQ(state.status, INITIALIZING);
}

TEST(MiniVOTest, StateUpdate) {
    // TODO: 상태 업데이트 테스트
    EXPECT_TRUE(true);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
