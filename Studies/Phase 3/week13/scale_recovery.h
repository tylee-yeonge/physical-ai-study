#ifndef SCALE_RECOVERY_H_
#define SCALE_RECOVERY_H_

#include <Eigen/Dense>
#include <vector>

/**
 * @brief 스케일 복구 방법 시연
 *
 * Stereo depth, IMU 적분 등 스케일 복구 방법을 비교한다.
 */
class ScaleRecovery
{
   public:
    /**
     * @brief Stereo 깊이 계산
     * @param f 초점 거리 (pixel)
     * @param baseline 베이스라인 (미터)
     * @param disparity 좌우 시차 (pixel)
     * @return 깊이 (미터), disparity <= 0이면 -1
     */
    static double compute_stereo_depth(double f, double baseline, double disparity);

    /**
     * @brief IMU 가속도 적분 (1축 단순화)
     * @param accel 가속도 시계열 (m/s²)
     * @param dt 시간 간격 (초)
     * @param positions 출력: 각 스텝의 위치
     * @param velocities 출력: 각 스텝의 속도
     */
    static void simulate_imu(const std::vector<double>& accel, double dt,
                             std::vector<double>& positions, std::vector<double>& velocities);

    /**
     * @brief Mono vs Stereo vs VIO 비교
     * @param true_distance 실제 이동 거리 (미터)
     * @param mono_scale_noise Mono 스케일 노이즈
     * @param stereo_noise Stereo 노이즈
     * @param imu_bias IMU 바이어스
     * @param n_frames 프레임 수
     */
    static void compare_scale_methods(double true_distance, double mono_scale_noise,
                                      double stereo_noise, double imu_bias, int n_frames);

    /**
     * @brief Vision-IMU 융합 시뮬레이션 (가중 평균)
     * @param vision_estimate Vision 추정 거리
     * @param imu_estimate IMU 추정 거리
     * @param vision_weight Vision 가중치 (0~1)
     * @return 융합 추정 거리
     */
    static double fuse_vision_imu(double vision_estimate, double imu_estimate,
                                  double vision_weight);

    /**
     * @brief 스케일 복구 결과 분석
     * @param true_val 참값
     * @param estimates 추정값 목록
     * @param labels 각 추정의 라벨
     */
    static void analyze_results(double true_val, const std::vector<double>& estimates,
                                const std::vector<std::string>& labels);
};

#endif  // SCALE_RECOVERY_H_
