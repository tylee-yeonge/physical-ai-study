#ifndef MINI_VO_TRIANGULATOR_H_
#define MINI_VO_TRIANGULATOR_H_

#include <opencv2/core.hpp>
#include <vector>

/**
 * @brief DLT 삼각측량 (P2-W7 구현 예정)
 *
 * 구현 내용:
 *  - DLT 직접 구현: 크로스곱 → 4×4 A 행렬 → SVD
 *  - 재투영 오차 계산
 */
class Triangulator
{
public:
    /**
     * @brief DLT 삼각측량으로 3D 점 복원
     * @param P1   카메라 1 투영 행렬 (3×4)
     * @param P2   카메라 2 투영 행렬 (3×4)
     * @param pts1 이미지 1의 픽셀 좌표
     * @param pts2 이미지 2의 픽셀 좌표
     * @return 복원된 3D 점 목록
     */
    // TODO (W7): DLT 직접 구현
    std::vector<cv::Point3f> triangulate(
        const cv::Mat& P1,
        const cv::Mat& P2,
        const std::vector<cv::Point2f>& pts1,
        const std::vector<cv::Point2f>& pts2) const;
};

#endif  // MINI_VO_TRIANGULATOR_H_
