/**
 * Phase 2 Week 1 - Google Test 단위 테스트
 *
 * PinholeProjection 클래스의 기본 기능을 테스트합니다.
 */

#include "../basic.h"
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <cmath>

// 테스트 픽스처
class PinholeProjectionTest : public ::testing::Test {
protected:
    PinholeProjection* camera;
    cv::Mat K, R, t;

    void SetUp() override {
        K = (cv::Mat_<double>(3, 3) <<
            600.0, 0.0, 400.0,
            0.0, 600.0, 300.0,
            0.0, 0.0, 1.0);
        R = cv::Mat::eye(3, 3, CV_64F);
        t = cv::Mat::zeros(3, 1, CV_64F);

        camera = new PinholeProjection(K, R, t);
    }

    void TearDown() override {
        delete camera;
    }
};

// 테스트 1: 이미지 중심 투영
TEST_F(PinholeProjectionTest, ProjectCenter) {
    // 정면 Z축 위의 점 → 이미지 중심으로 투영
    cv::Point2d pixel = camera->project(cv::Point3d(0, 0, 5));

    EXPECT_NEAR(pixel.x, 400.0, 1e-6);  // cx
    EXPECT_NEAR(pixel.y, 300.0, 1e-6);  // cy
}

// 테스트 2: 일반적인 투영
TEST_F(PinholeProjectionTest, ProjectGeneral) {
    // P = (2, 1, 5) → u = 600*2/5 + 400 = 640, v = 600*1/5 + 300 = 420
    cv::Point2d pixel = camera->project(cv::Point3d(2, 1, 5));

    EXPECT_NEAR(pixel.x, 640.0, 1e-6);
    EXPECT_NEAR(pixel.y, 420.0, 1e-6);
}

// 테스트 3: 카메라 뒤 점 (Zc <= 0)
TEST_F(PinholeProjectionTest, ProjectBehindCamera) {
    cv::Point2d pixel = camera->project(cv::Point3d(0, 0, -5));

    EXPECT_EQ(pixel.x, -1);
    EXPECT_EQ(pixel.y, -1);
}

// 테스트 4: 역투영 - 이미지 중심
TEST_F(PinholeProjectionTest, BackProjectCenter) {
    cv::Vec3d ray = camera->backProject(cv::Point2d(400, 300));

    // 이미지 중심 → Z축 방향 [0, 0, 1]
    EXPECT_NEAR(ray[0], 0.0, 1e-6);
    EXPECT_NEAR(ray[1], 0.0, 1e-6);
    EXPECT_NEAR(ray[2], 1.0, 1e-6);
}

// 테스트 5: 역투영 - 단위 벡터 확인
TEST_F(PinholeProjectionTest, BackProjectNormalized) {
    cv::Vec3d ray = camera->backProject(cv::Point2d(0, 0));

    double norm = cv::norm(ray);
    EXPECT_NEAR(norm, 1.0, 1e-6);
}

// 테스트 6: FOV 계산
TEST_F(PinholeProjectionTest, ComputeFOV) {
    cv::Size2d fov = camera->computeFOV(cv::Size(800, 600));

    // FOV_h = 2 * atan(800 / (2*600)) ≈ 67.38°
    double expected_h = 2.0 * std::atan2(800, 2.0 * 600) * 180.0 / CV_PI;
    double expected_v = 2.0 * std::atan2(600, 2.0 * 600) * 180.0 / CV_PI;

    EXPECT_NEAR(fov.width, expected_h, 0.01);
    EXPECT_NEAR(fov.height, expected_v, 0.01);
}

// 테스트 7: 재투영 오차 - 정확한 경우
TEST_F(PinholeProjectionTest, ReprojectionErrorZero) {
    cv::Point3d P(2, 1, 5);
    cv::Point2d projected = camera->project(P);

    double error = camera->reprojectionError(P, projected);
    EXPECT_NEAR(error, 0.0, 1e-6);
}

// 테스트 8: 재투영 오차 - 노이즈 포함
TEST_F(PinholeProjectionTest, ReprojectionErrorWithNoise) {
    cv::Point3d P(2, 1, 5);
    cv::Point2d projected = camera->project(P);

    // 3-4-5 삼각형: (3, 4) 오프셋 → 오차 = 5
    cv::Point2d noisy(projected.x + 3.0, projected.y + 4.0);
    double error = camera->reprojectionError(P, noisy);
    EXPECT_NEAR(error, 5.0, 1e-6);
}

// 테스트 9: 이미지 내부 판별
TEST_F(PinholeProjectionTest, IsInImage) {
    cv::Size imageSize(800, 600);

    EXPECT_TRUE(PinholeProjection::isInImage(cv::Point2d(400, 300), imageSize));
    EXPECT_TRUE(PinholeProjection::isInImage(cv::Point2d(0, 0), imageSize));
    EXPECT_FALSE(PinholeProjection::isInImage(cv::Point2d(-1, 300), imageSize));
    EXPECT_FALSE(PinholeProjection::isInImage(cv::Point2d(800, 300), imageSize));
}

// 테스트 10: Getter 함수
TEST_F(PinholeProjectionTest, GetterFunctions) {
    EXPECT_NEAR(camera->getFx(), 600.0, 1e-6);
    EXPECT_NEAR(camera->getFy(), 600.0, 1e-6);
    EXPECT_NEAR(camera->getCx(), 400.0, 1e-6);
    EXPECT_NEAR(camera->getCy(), 300.0, 1e-6);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
