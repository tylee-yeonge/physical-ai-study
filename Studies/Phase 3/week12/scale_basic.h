#ifndef SCALE_BASIC_H_
#define SCALE_BASIC_H_

#include <Eigen/Dense>
#include <vector>

/**
 * @brief 스케일 모호성 / 드리프트 시뮬레이션
 *
 * 단안 VO의 스케일 모호성 원인과 드리프트 누적을
 * 수치적으로 시연한다.
 */
class ScaleBasic
{
   public:
    /**
     * @brief 원형 GT 궤적 생성
     * @param n_frames 프레임 수
     * @param radius 반지름 (미터)
     * @return 각 프레임의 (x, y) 위치
     */
    static std::vector<Eigen::Vector2d> generate_circular_trajectory(int n_frames, double radius);

    /**
     * @brief 스케일 드리프트 시뮬레이션
     * @param gt Ground truth 궤적
     * @param scale_noise_std 스케일 노이즈 표준편차
     * @param seed 랜덤 시드
     * @return 드리프트가 포함된 추정 궤적
     */
    static std::vector<Eigen::Vector2d> apply_scale_drift(
        const std::vector<Eigen::Vector2d>& gt, double scale_noise_std, int seed = 42);

    /**
     * @brief 절대 궤적 오차 (ATE) 계산
     * @param gt Ground truth 궤적
     * @param est 추정 궤적
     * @return RMSE (미터)
     */
    static double compute_ate(const std::vector<Eigen::Vector2d>& gt,
                              const std::vector<Eigen::Vector2d>& est);

    /**
     * @brief Sim(3) 정렬 (스케일 + 회전 + 이동)
     * @param gt Ground truth 궤적
     * @param est 추정 궤적
     * @param scale 출력: 추정된 스케일
     * @return 정렬된 궤적
     */
    static std::vector<Eigen::Vector2d> align_sim3(const std::vector<Eigen::Vector2d>& gt,
                                                   const std::vector<Eigen::Vector2d>& est,
                                                   double& scale);

    /**
     * @brief 궤적 분석 통계 출력
     * @param gt Ground truth
     * @param est 추정 궤적
     * @param label 출력 라벨
     */
    static void analyze_trajectory(const std::vector<Eigen::Vector2d>& gt,
                                   const std::vector<Eigen::Vector2d>& est,
                                   const std::string& label);
};

#endif  // SCALE_BASIC_H_
