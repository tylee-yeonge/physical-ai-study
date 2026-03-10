#ifndef MINI_VO_POSE_RECOVERY_H_
#define MINI_VO_POSE_RECOVERY_H_

#include <opencv2/core.hpp>
#include <vector>

/**
 * @brief E Matrix → R, t 분해 (P2-W6 구현 예정)
 *
 * 구현 내용:
 *  - SVD 분해 → W 행렬 → 4가지 (R, t) 후보
 *  - Cheirality Check로 올바른 해 선택
 */
class PoseRecovery
{
public:
    /**
     * @brief E Matrix에서 R, t 복원
     * @param E     Essential Matrix (3×3)
     * @param pts1  이미지 1의 정규화 좌표
     * @param pts2  이미지 2의 정규화 좌표
     * @param R     출력: 회전 행렬 (3×3)
     * @param t     출력: 평행이동 벡터 (단위 벡터)
     * @return Cheirality Check를 통과한 inlier 수
     */
    // TODO (W6): E → R, t 직접 구현 (SVD + Cheirality)
    int recover(
        const cv::Mat& E,
        const std::vector<cv::Point2f>& pts1,
        const std::vector<cv::Point2f>& pts2,
        cv::Mat& R,
        cv::Mat& t) const;
};

#endif  // MINI_VO_POSE_RECOVERY_H_
