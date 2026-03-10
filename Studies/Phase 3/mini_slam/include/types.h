#ifndef MINI_SLAM_TYPES_H_
#define MINI_SLAM_TYPES_H_

#include <opencv2/core.hpp>
#include <vector>

namespace slam {

/**
 * @brief SE(3) 포즈 (회전 + 이동)
 *
 * 카메라 또는 키프레임의 월드 좌표계 기준 포즈를 나타낸다.
 * R: 월드 → 카메라 회전, t: 월드 → 카메라 이동
 */
struct Pose
{
    cv::Mat R;  ///< 회전 행렬 (3×3, CV_64F)
    cv::Mat t;  ///< 이동 벡터 (3×1, CV_64F)

    Pose()
        : R(cv::Mat::eye(3, 3, CV_64F)),
          t(cv::Mat::zeros(3, 1, CV_64F))
    {
    }

    Pose(const cv::Mat& R_, const cv::Mat& t_)
        : R(R_.clone()), t(t_.clone())
    {
    }

    /** @brief 4×4 변환 행렬로 변환 */
    cv::Mat toMatrix44() const
    {
        cv::Mat T = cv::Mat::eye(4, 4, CV_64F);
        R.copyTo(T(cv::Rect(0, 0, 3, 3)));
        t.copyTo(T(cv::Rect(3, 0, 1, 3)));
        return T;
    }

    /** @brief 역변환 (T^{-1}) */
    Pose inverse() const
    {
        cv::Mat R_inv = R.t();
        cv::Mat t_inv = -R_inv * t;
        return Pose(R_inv, t_inv);
    }

    /** @brief 포즈 합성: this * other */
    Pose operator*(const Pose& other) const
    {
        return Pose(R * other.R, R * other.t + t);
    }
};

/**
 * @brief 3D 맵 포인트
 *
 * 월드 좌표계의 3D 점과 관측 정보를 보관한다.
 */
struct MapPoint
{
    int id;                      ///< 고유 ID
    cv::Point3f position;        ///< 3D 위치 (월드 좌표)
    int num_observations;        ///< 관측된 키프레임 수
    bool is_outlier;             ///< 아웃라이어 여부
    std::vector<int> observer_keyframe_ids;  ///< 관측한 키프레임 ID 목록

    MapPoint()
        : id(-1), position(0, 0, 0), num_observations(0), is_outlier(false)
    {
    }

    MapPoint(int id_, const cv::Point3f& pos)
        : id(id_), position(pos), num_observations(1), is_outlier(false)
    {
    }
};

}  // namespace slam

#endif  // MINI_SLAM_TYPES_H_
