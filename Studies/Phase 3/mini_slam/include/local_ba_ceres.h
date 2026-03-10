#ifndef MINI_SLAM_LOCAL_BA_CERES_H_
#define MINI_SLAM_LOCAL_BA_CERES_H_

#include "keyframe.h"
#include "map.h"
#include "types.h"

#include <opencv2/core.hpp>
#include <vector>

namespace slam {

/**
 * @brief Ceres Solver 기반 Local Bundle Adjustment (P3-W8)
 *
 * 재투영 오차를 비용 함수로 정의하고, Ceres Solver로 최적화한다.
 *
 * 최적화 변수:
 *  - 키프레임 포즈 (R, t) → Rodrigues 벡터 (3) + 이동 (3) = 6DoF
 *  - 맵 포인트 3D 위치 (3)
 *
 * 비용 함수:
 *  - 재투영 오차 = || π(K [R|t] X) - x_obs ||²
 *
 * TODO (W8): Ceres 비용 함수 정의 + Solve 호출
 */
class LocalBACeres
{
public:
    /**
     * @param K             카메라 내부 파라미터 (3×3, CV_64F)
     * @param max_iterations 최대 반복 횟수
     */
    explicit LocalBACeres(const cv::Mat& K, int max_iterations = 20);

    /**
     * @brief Local BA 실행
     *
     * 주어진 키프레임들의 포즈와 맵 포인트를 동시에 최적화한다.
     * 첫 번째 키프레임은 고정(anchor)으로 설정한다.
     *
     * @param map 맵 (키프레임 + 맵 포인트 수정됨)
     * @return 최종 평균 재투영 오차 (픽셀)
     */
    // TODO (W8): Ceres BA 구현
    double optimize(Map& map) const;

private:
    cv::Mat K_;
    int max_iterations_;
};

}  // namespace slam

#endif  // MINI_SLAM_LOCAL_BA_CERES_H_
