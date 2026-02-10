/**
 * Phase 2 Week 2 - 카메라 캘리브레이션 직접 구현
 *
 * basic.h의 CameraCalibrationBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make test_my_basic
 * 테스트: ./test_my_basic
 */
#include "basic.h"
#include <iostream>

CameraCalibrationBasic::CameraCalibrationBasic(cv::Size boardSize, float squareSize) {
    // TODO: boardSize_, squareSize_ 초기화
}

bool CameraCalibrationBasic::detectChessboard(const cv::Mat& image,
                                              std::vector<cv::Point2f>& corners) {
    // TODO: cv::findChessboardCorners 사용
    return false;
}

double CameraCalibrationBasic::calibrate(
    const std::vector<std::vector<cv::Point2f>>& imagePoints,
    cv::Size imageSize,
    cv::Mat& cameraMatrix,
    cv::Mat& distCoeffs) {
    // TODO: cv::calibrateCamera 사용
    return -1.0;
}

void CameraCalibrationBasic::saveCalibration(const std::string& filename,
                                             const cv::Mat& K,
                                             const cv::Mat& dist) {
    // TODO: cv::FileStorage로 저장
}

void CameraCalibrationBasic::undistortImage(const cv::Mat& distorted,
                                            cv::Mat& undistorted,
                                            const cv::Mat& K,
                                            const cv::Mat& dist) {
    // TODO: cv::undistort 사용
}

std::string CameraCalibrationBasic::evaluateQuality(double rms) {
    // TODO: RMS 값에 따라 품질 등급 반환
    // 예: < 0.5 → "Excellent", < 1.0 → "Good", etc.
    return "Unknown";
}

std::vector<cv::Point3f> CameraCalibrationBasic::generateObjectPoints() {
    // TODO: 체커보드 3D 좌표 생성 (Z=0 평면)
    return {};
}

#ifndef MY_BASIC_LIB_ONLY
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 카메라 캘리브레이션 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    CameraCalibrationBasic calib(cv::Size(9, 6), 25.0f);

    // 3D 오브젝트 포인트 생성
    auto obj_pts = calib.generateObjectPoints();
    std::cout << "체커보드 3D 포인트 수: " << obj_pts.size() << std::endl;

    // 품질 평가
    std::cout << "RMS 0.3 평가: " << calib.evaluateQuality(0.3) << std::endl;
    std::cout << "RMS 0.8 평가: " << calib.evaluateQuality(0.8) << std::endl;
    std::cout << "RMS 1.5 평가: " << calib.evaluateQuality(1.5) << std::endl;

    std::cout << "\n구현 완료 후 테스트: ./test_my_basic" << std::endl;
    return 0;
}
#endif
