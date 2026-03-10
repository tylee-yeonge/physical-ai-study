#ifndef MINI_SLAM_LOCAL_BA_G2O_H_
#define MINI_SLAM_LOCAL_BA_G2O_H_

#include "keyframe.h"
#include "map.h"
#include "types.h"

#include <opencv2/core.hpp>
#include <vector>

namespace slam {

/**
 * @brief g2o 기반 Local Bundle Adjustment (P3-W9~W10)
 *
 * g2o 그래프 최적화 프레임워크를 사용한 BA.
 * Schur Complement를 활용하여 효율적으로 최적화한다.
 *
 * 그래프 구조:
 *  - 정점(Vertex): 카메라 포즈 (SE3), 맵 포인트 (3D)
 *  - 간선(Edge):   재투영 오차 (이진 간선: 포즈 ↔ 포인트)
 *
 * Schur Complement:
 *  - 맵 포인트를 marginalize하여 포즈만의 축소된 시스템을 풂
 *  - H = [Hpp  Hpl] → Hpp - Hpl Hll^{-1} Hlp
 *        [Hlp  Hll]
 *
 * TODO (W9):  g2o 정점·간선 세팅 + Schur 옵션 설정
 * TODO (W10): mini_slam에 통합 + 결과 비교
 */
class LocalBAG2O
{
public:
    /**
     * @param K              카메라 내부 파라미터 (3×3, CV_64F)
     * @param max_iterations 최대 반복 횟수
     * @param use_schur      Schur Complement 사용 여부
     */
    explicit LocalBAG2O(const cv::Mat& K,
                        int max_iterations = 10,
                        bool use_schur = true);

    /**
     * @brief Local BA 실행 (g2o)
     *
     * g2o 그래프를 구성하고 Levenberg-Marquardt로 최적화한다.
     * 첫 번째 키프레임은 고정(fixed)으로 설정한다.
     *
     * @param map 맵 (키프레임 + 맵 포인트 수정됨)
     * @return 최종 평균 재투영 오차 (픽셀)
     */
    // TODO (W9-W10): g2o BA 구현
    double optimize(Map& map) const;

private:
    cv::Mat K_;
    int max_iterations_;
    bool use_schur_;
};

}  // namespace slam

#endif  // MINI_SLAM_LOCAL_BA_G2O_H_
