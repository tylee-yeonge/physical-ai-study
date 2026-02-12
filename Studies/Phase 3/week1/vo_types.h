#ifndef VO_TYPES_H
#define VO_TYPES_H

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <vector>
#include <memory>

namespace vo
{

/**
 * @brief 카메라 내부 파라미터
 */
struct Camera
{
    double fx;  // 초점거리 X
    double fy;  // 초점거리 Y
    double cx;  // 주점 X
    double cy;  // 주점 Y

    cv::Mat K;  // 카메라 행렬

    Camera(double fx_, double fy_, double cx_, double cy_) : fx(fx_), fy(fy_), cx(cx_), cy(cy_)
    {
        K = (cv::Mat_<double>(3, 3) << fx, 0, cx, 0, fy, cy, 0, 0, 1);
    }
};

/**
 * @brief 카메라 포즈 (SE(3))
 */
struct Pose
{
    Eigen::Matrix3d R;  // 회전
    Eigen::Vector3d t;  // 이동

    Pose() : R(Eigen::Matrix3d::Identity()), t(Eigen::Vector3d::Zero())
    {
    }

    Pose(const Eigen::Matrix3d& R_, const Eigen::Vector3d& t_) : R(R_), t(t_)
    {
    }

    // 포즈 역변환
    Pose inverse() const
    {
        return Pose(R.transpose(), -R.transpose() * t);
    }

    // 포즈 합성
    Pose operator*(const Pose& other) const
    {
        return Pose(R * other.R, R * other.t + t);
    }
};

/**
 * @brief 특징점
 */
struct Feature
{
    cv::Point2f pt;     // 2D 위치
    int id;             // 고유 ID
    bool has_3d;        // 3D 점 연결 여부
    cv::Point3f pt_3d;  // 3D 위치 (있는 경우)

    Feature() : id(-1), has_3d(false)
    {
    }
    Feature(const cv::Point2f& pt_, int id_) : pt(pt_), id(id_), has_3d(false)
    {
    }
};

/**
 * @brief 프레임
 */
struct Frame
{
    int id;
    cv::Mat image;
    std::vector<Feature> features;
    Pose pose;
    double timestamp;

    Frame() : id(-1), timestamp(0.0)
    {
    }
    Frame(int id_, const cv::Mat& img, double ts) : id(id_), image(img), timestamp(ts)
    {
    }
};

/**
 * @brief 맵 포인트
 */
struct MapPoint
{
    int id;
    Eigen::Vector3d position;
    std::vector<int> observed_by;  // 이 점을 관측한 프레임 ID
    int observations;

    MapPoint() : id(-1), observations(0)
    {
    }
    MapPoint(int id_, const Eigen::Vector3d& pos) : id(id_), position(pos), observations(0)
    {
    }
};

/**
 * @brief VO 통계
 */
struct VOStatistics
{
    int total_frames;
    int tracked_features;
    int lost_tracks;
    int new_landmarks;
    double avg_reprojection_error;

    VOStatistics()
        : total_frames(0),
          tracked_features(0),
          lost_tracks(0),
          new_landmarks(0),
          avg_reprojection_error(0.0)
    {
    }

    void print() const
    {
        std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout << "VO 통계" << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout << "총 프레임: " << total_frames << std::endl;
        std::cout << "추적 특징점: " << tracked_features << std::endl;
        std::cout << "손실 특징점: " << lost_tracks << std::endl;
        std::cout << "새 랜드마크: " << new_landmarks << std::endl;
        std::cout << "평균 재투영 오차: " << avg_reprojection_error << " px" << std::endl;
    }
};

}  // namespace vo

#endif  // VO_TYPES_H
