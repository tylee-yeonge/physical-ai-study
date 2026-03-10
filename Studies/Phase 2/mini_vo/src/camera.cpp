#include "camera.h"

Camera::Camera(double fx, double fy, double cx, double cy)
    : fx_(fx), fy_(fy), cx_(cx), cy_(cy)
{
}

cv::Point2f Camera::project(const cv::Point3f& point_3d) const
{
    // 투영: (X, Y, Z) → (fx*X/Z + cx, fy*Y/Z + cy)
    float u = static_cast<float>(fx_ * point_3d.x / point_3d.z + cx_);
    float v = static_cast<float>(fy_ * point_3d.y / point_3d.z + cy_);
    return {u, v};
}

cv::Point2f Camera::normalize(const cv::Point2f& pixel) const
{
    // K 역적용: 픽셀 → 정규화 좌표
    float x = static_cast<float>((pixel.x - cx_) / fx_);
    float y = static_cast<float>((pixel.y - cy_) / fy_);
    return {x, y};
}

cv::Mat Camera::K() const
{
    cv::Mat k = cv::Mat::eye(3, 3, CV_64F);
    k.at<double>(0, 0) = fx_;
    k.at<double>(1, 1) = fy_;
    k.at<double>(0, 2) = cx_;
    k.at<double>(1, 2) = cy_;
    return k;
}
