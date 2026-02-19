#ifndef ICP_BASIC_H_
#define ICP_BASIC_H_

#include <Eigen/Dense>
#include <vector>

/**
 * @brief ICP 정합 결과
 */
struct ICPResult
{
    Eigen::Matrix3d R;   // 회전
    Eigen::Vector3d t;   // 이동
    double error;        // 평균 오차
    int iterations;      // 반복 횟수
    bool converged;      // 수렴 여부
};

/**
 * @brief SVD 기반 ICP 구현
 *
 * 두 3D Point Cloud 간의 변환(R, t)을 추정한다.
 * Point-to-Point ICP를 SVD closed-form으로 풀이.
 */
class ICPBasic
{
   public:
    /**
     * @brief 점 집합의 중심점 계산
     * @param points 3D 점 집합
     * @return 중심점 (평균)
     */
    static Eigen::Vector3d compute_centroid(const std::vector<Eigen::Vector3d>& points);

    /**
     * @brief Covariance 행렬 H 계산
     * @param src 소스 점 (중심화된)
     * @param tgt 타겟 점 (중심화된)
     * @return 3x3 행렬 H = Σ src_i * tgt_i^T
     */
    static Eigen::Matrix3d compute_h(const std::vector<Eigen::Vector3d>& src,
                                     const std::vector<Eigen::Vector3d>& tgt);

    /**
     * @brief SVD로 R, t 계산 (대응점 주어짐)
     * @param src 소스 점 집합
     * @param tgt 타겟 점 집합 (대응 순서 일치)
     * @param R 출력 회전 행렬
     * @param t 출력 이동 벡터
     * @return 평균 정합 오차
     */
    static double svd_decompose(const std::vector<Eigen::Vector3d>& src,
                                const std::vector<Eigen::Vector3d>& tgt,
                                Eigen::Matrix3d& R, Eigen::Vector3d& t);

    /**
     * @brief 반복 ICP (대응점이 이미 주어진 경우 — 교육용 단순 버전)
     * @param src 소스 점 집합
     * @param tgt 타겟 점 집합
     * @param max_iter 최대 반복 횟수
     * @param tol 수렴 임계값
     * @return ICP 결과 (R, t, 오차, 반복 횟수)
     */
    static ICPResult iterative_icp(const std::vector<Eigen::Vector3d>& src,
                                   const std::vector<Eigen::Vector3d>& tgt,
                                   int max_iter = 20, double tol = 1e-6);

    /**
     * @brief 정합 품질 평가
     * @param src 소스 점 집합
     * @param tgt 타겟 점 집합
     * @param R 추정된 회전
     * @param t 추정된 이동
     * @return 평균 정합 오차 (RMSE)
     */
    static double evaluate_alignment(const std::vector<Eigen::Vector3d>& src,
                                     const std::vector<Eigen::Vector3d>& tgt,
                                     const Eigen::Matrix3d& R, const Eigen::Vector3d& t);
};

#endif  // ICP_BASIC_H_
