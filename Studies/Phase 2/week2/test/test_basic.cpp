/**
 * Phase 2 Week 2 - Google Test 단위 테스트
 * 
 * 캘리브레이션 클래스의 기본 기능을 테스트합니다.
 */

#include "../basic.h"
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

// 테스트 픽스처
class CameraCalibrationTest : public ::testing::Test {
protected:
    CameraCalibrationBasic* calib;
    
    void SetUp() override {
        calib = new CameraCalibrationBasic(cv::Size(9, 6), 30.0f);
    }
    
    void TearDown() override {
        delete calib;
    }
};

// 테스트 1: 체커보드 검출
TEST_F(CameraCalibrationTest, ChessboardDetection) {
    // 간단한 체커보드 이미지 생성 (실제로는 실제 이미지 사용)
    cv::Mat test_image = cv::Mat::zeros(600, 800, CV_8UC1);
    std::vector<cv::Point2f> corners;
    
    // 실제 체커보드가 없으면 검출 실패해야 함
    bool found = calib->detectChessboard(test_image, corners);
    
    // 빈 이미지에서는 체커보드를 찾지 못해야 함
    EXPECT_FALSE(found);
}

// 테스트 2: 왜곡 보정
TEST_F(CameraCalibrationTest, Undistortion) {
    cv::Mat K = (cv::Mat_<double>(3, 3) << 
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
    cv::Mat dist = (cv::Mat_<double>(1, 5) << 0.0, 0.0, 0.0, 0.0, 0.0);
    
    cv::Mat distorted = cv::Mat::zeros(600, 800, CV_8UC3);
    cv::Mat undistorted;
    
    // 왜곡이 없으면 입력과 출력이 같아야 함
    CameraCalibrationBasic::undistortImage(distorted, undistorted, K, dist);
    
    EXPECT_EQ(distorted.size(), undistorted.size());
}

// 테스트 3: RMS 품질 평가
TEST(CalibrationQuality, RMSEvaluation) {
    EXPECT_EQ(CameraCalibrationBasic::evaluateQuality(0.3), "✅ 매우 우수 (Excellent)");
    EXPECT_EQ(CameraCalibrationBasic::evaluateQuality(0.45), "✅ 우수 (Good)");
    EXPECT_EQ(CameraCalibrationBasic::evaluateQuality(0.75), "⚠️  양호 (Fair) - 사용 가능");
    EXPECT_EQ(CameraCalibrationBasic::evaluateQuality(1.5), "❌ 불량 (Poor) - 재캘리브레이션 권장");
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
