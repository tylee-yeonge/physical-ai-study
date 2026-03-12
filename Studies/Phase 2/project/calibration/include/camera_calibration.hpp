#ifndef CAMERA_CALIBRATION_HPP
#define CAMERA_CALIBRATION_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class CameraCalibration
{
public:
    CameraCalibration(cv::Size boardSize, float squareSize);

    bool findChessboardCorners(const cv::Mat& image, std::vector<cv::Point2f>& corners);
    double calibrate(const std::vector<std::vector<cv::Point2f>>& imagePoints,
                     cv::Size imgaeSize,
                     cv::Mat& cameraMatrix,
                     cv::Mat& distCoeffs);

    void saveCalibration(const std::string& filename,
                         const cv::Mat& K,
                         const cv::Mat& dist,
                         cv::Size imageSize);

private:
    cv::Size boardSize_;
    float squareSize_;
    std::vector<std::vector<cv::Point3f>> objectPoints_;

    void generateObjectPoints();
};

#endif