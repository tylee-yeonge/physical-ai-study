#ifndef MOTION_3D3D_H
#define MOTION_3D3D_H

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <Eigen/SVD>
#include <vector>

/**
 * @brief 3D-3D 모션 추정 클래스 (ICP)
 */
class Motion3D3D {
public:
    /**
     * @brief SVD 기반 정합 (대응점 명확할 때)
     * @param src 소스 포인트
     * @param tgt 타겟 포인트
     * @param R 회전 (출력)
     * @param t 이동 (출력)
     */
    static void alignSVD(const std::vector<Eigen::Vector3d>& src,
                        const std::vector<Eigen::Vector3d>& tgt,
                        Eigen::Matrix3d& R,
                        Eigen::Vector3d& t);
    
    /**
     * @brief Point-to-Point ICP
     * @param src 소스 포인트
     * @param tgt 타겟 포인트
     * @param R 회전 (출력)
     * @param t 이동 (출력)
     * @param max_iter 최대 반복 횟수
     * @param tolerance 수렴 임계값
     * @return 최종 오차
     */
    static double ICP(const std::vector<Eigen::Vector3d>& src,
                     const std::vector<Eigen::Vector3d>& tgt,
                     Eigen::Matrix3d& R,
                     Eigen::Vector3d& t,
                     int max_iter = 20,
                     double tolerance = 1e-6);
    
    /**
     * @brief 최근접 이웃 찾기 (naive)
     * @param point 쿼리 포인트
     * @param cloud 포인트 클라우드
     * @return 가장 가까운 점의 인덱스
     */
    static int findNearestNeighbor(const Eigen::Vector3d& point,
                                   const std::vector<Eigen::Vector3d>& cloud);
    
    /**
     * @brief 평균 오차 계산
     * @param src 소스 포인트
     * @param tgt 타겟 포인트
     * @param R 회전
     * @param t 이동
     * @return RMS 오차
     */
    static double computeError(const std::vector<Eigen::Vector3d>& src,
                              const std::vector<Eigen::Vector3d>& tgt,
                              const Eigen::Matrix3d& R,
                              const Eigen::Vector3d& t);
    
    /**
     * @brief 포인트 변환
     * @param points 입력 포인트
     * @param R 회전
     * @param t 이동
     * @param transformed 변환된 포인트 (출력)
     */
    static void transformPoints(const std::vector<Eigen::Vector3d>& points,
                               const Eigen::Matrix3d& R,
                               const Eigen::Vector3d& t,
                               std::vector<Eigen::Vector3d>& transformed);
    
    /**
     * @brief 전체 데모
     */
    static void demo();
};

#endif // MOTION_3D3D_H
