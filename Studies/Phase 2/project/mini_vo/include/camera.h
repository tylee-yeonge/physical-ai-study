#ifndef MINI_VO_CAMERA_H_
#define MINI_VO_CAMERA_H_

#include <opencv2/core.hpp>

/**
 * @brief 핀홀 카메라 모델 (P2-W1 ~ W2)
 *
 * 내부 파라미터(K)와 왜곡 계수를 보관하고,
 * 3D 점 투영 및 왜곡 보정 기능을 제공한다.
 */
class Camera
{
public:
    /**
     * @brief 카메라 생성자
     * @param fx  x축 초점 거리 (픽셀)
     * @param fy  y축 초점 거리 (픽셀)
     * @param cx  주점 x (픽셀)
     * @param cy  주점 y (픽셀)
     */
    Camera(double fx, double fy, double cx, double cy);

    /**
     * @brief 3D 점을 2D 픽셀 좌표로 투영 (왜곡 없음)
     * @param point_3d 카메라 좌표계의 3D 점 [X, Y, Z]
     * @return 픽셀 좌표 (u, v)
     */
    cv::Point2f project(const cv::Point3f& point_3d) const;

    /**
     * @brief 픽셀 좌표를 정규화 좌표로 변환 (K 역적용)
     * @param pixel 픽셀 좌표 (u, v)
     * @return 정규화 좌표 (x_n, y_n)
     */
    cv::Point2f normalize(const cv::Point2f& pixel) const;

    /** @brief 내부 파라미터 행렬 K 반환 (3×3) */
    cv::Mat K() const;

    double fx_, fy_, cx_, cy_;
};

#endif  // MINI_VO_CAMERA_H_
