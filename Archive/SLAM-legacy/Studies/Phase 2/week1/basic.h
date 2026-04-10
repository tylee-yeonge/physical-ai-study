#ifndef PINHOLE_PROJECTION_BASIC_H
#define PINHOLE_PROJECTION_BASIC_H

#include <opencv2/opencv.hpp>
#include <vector>

/**
 * @brief 핀홀 카메라 투영 데모 클래스
 *
 * 이 클래스는 핀홀 카메라 모델의 핵심 개념을 이해하기 위한 구현입니다.
 * 3D→2D 투영, 역투영, FOV 계산, 재투영 오차 등을 다룹니다.
 */
class PinholeProjection
{
   public:
    /**
     * @brief 생성자
     * @param K 3x3 카메라 내부 파라미터 행렬
     * @param R 3x3 회전 행렬 (월드→카메라)
     * @param t 3x1 이동 벡터 (월드→카메라)
     */
    PinholeProjection(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t);

    /**
     * @brief 3D 월드 점을 2D 픽셀로 투영
     * @param P_world 3D 월드 좌표 (X, Y, Z)
     * @return 2D 픽셀 좌표 (u, v)
     */
    cv::Point2d project(const cv::Point3d& P_world) const;

    /**
     * @brief 여러 3D 점을 한번에 투영
     * @param points_3d 3D 점 벡터
     * @return 2D 픽셀 좌표 벡터
     */
    std::vector<cv::Point2d> projectMultiple(const std::vector<cv::Point3d>& points_3d) const;

    /**
     * @brief 2D 픽셀에서 3D 광선 방향 역투영
     * @param pixel 2D 픽셀 좌표 (u, v)
     * @return 정규화된 3D 광선 방향 벡터
     */
    cv::Vec3d backProject(const cv::Point2d& pixel) const;

    /**
     * @brief 수평/수직 시야각(FOV) 계산
     * @param imageSize 이미지 크기 (width, height)
     * @return FOV (horizontal_deg, vertical_deg)
     */
    cv::Size2d computeFOV(const cv::Size& imageSize) const;

    /**
     * @brief 재투영 오차 계산
     * @param P_world 3D 월드 좌표
     * @param observed_pixel 실제 관측된 2D 좌표
     * @return 재투영 오차 (픽셀)
     */
    double reprojectionError(const cv::Point3d& P_world, const cv::Point2d& observed_pixel) const;

    /**
     * @brief 점이 이미지 안에 있는지 확인
     * @param pixel 2D 좌표
     * @param imageSize 이미지 크기
     * @return 이미지 안에 있으면 true
     */
    static bool isInImage(const cv::Point2d& pixel, const cv::Size& imageSize);

    // Getter
    cv::Mat getK() const
    {
        return K_.clone();
    }
    cv::Mat getR() const
    {
        return R_.clone();
    }
    cv::Mat getT() const
    {
        return t_.clone();
    }
    double getFx() const
    {
        return K_.at<double>(0, 0);
    }
    double getFy() const
    {
        return K_.at<double>(1, 1);
    }
    double getCx() const
    {
        return K_.at<double>(0, 2);
    }
    double getCy() const
    {
        return K_.at<double>(1, 2);
    }

   private:
    cv::Mat K_;  ///< 3x3 내부 파라미터 행렬
    cv::Mat R_;  ///< 3x3 회전 행렬
    cv::Mat t_;  ///< 3x1 이동 벡터
};

#endif  // PINHOLE_PROJECTION_BASIC_H
