#include "camera_calibration.hpp"
#include <opencv2/calib3d.hpp>
#include <iostream>

CameraCalibration::CameraCalibration(cv::Size boardSize, float squareSize)
    : boardSize_(boardSize), squareSize_(squareSize)
{
}

bool CameraCalibration::findChessboardCorners(const cv::Mat& image,
                                              std::vector<cv::Point2f>& corners)
{
    cv::Mat gray;
    if (image.channels() == 3)
    {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        gray = image.clone();
    }

    bool found = cv::findChessboardCorners(
        gray,
        boardSize_,
        corners,
        cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK);

    if (found)
    {
        cv::cornerSubPix(
            gray,
            corners,
            cv::Size(11, 11),
            cv::Size(-1, -1),
            cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
    }

    return found;
}

void CameraCalibration::generateObjectPoints()
{
    std::vector<cv::Point3f> obj;
    for (int i = 0; i < boardSize_.height; i++)
    {
        for (int j = 0; j < boardSize_.width; j++)
        {
            obj.push_back(cv::Point3f(j * squareSize_, i * squareSize_, 0));
        }
    }
    objectPoints_.clear();
    objectPoints_.push_back(obj);
}

double CameraCalibration::calibrate(const std::vector<std::vector<cv::Point2f>>& imagePoints,
                                    cv::Size imageSize,
                                    cv::Mat& cameraMatrix,
                                    cv::Mat& disCoeffs)
{
    generateObjectPoints();

    std::vector<std::vector<cv::Point3f>> allObjectPoints;
    for (size_t i = 0; i < imagePoints.size(); i++)
    {
        allObjectPoints.push_back(objectPoints_[0]);
    }

    std::vector<cv::Mat> rvecs, tvecs;
    double rms = cv::calibrateCamera(
        allObjectPoints, imagePoints, imageSize, cameraMatrix, disCoeffs, rvecs, tvecs);

    std::cout << "RMS re-projection error: " << rms << " pixels" << std::endl;

    return rms;
}

void CameraCalibration::saveCalibration(const std::string& filename,
                                        const cv::Mat& K,
                                        const cv::Mat& dist,
                                        cv::Size imageSize)
{
    cv::FileStorage fs(filename, cv::FileStorage::WRITE);
    fs << "camera_matrix" << K;
    fs << "distortion_coefficients" << dist;
    fs << "image_width" << imageSize.width;
    fs << "image_height" << imageSize.height;
    fs.release();

    std::cout << "Saved calibration to " << filename << std::endl;
}