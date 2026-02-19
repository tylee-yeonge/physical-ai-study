#ifndef SCHUR_BASIC_H_
#define SCHUR_BASIC_H_

#include <Eigen/Dense>
#include <vector>

/**
 * @brief BA Hessian 블록 구조
 *
 * H = [Hcc  Hcp]   b = [bc]
 *     [Hpc  Hpp]       [bp]
 *
 * c: 카메라 변수, p: 점 변수
 */
struct HessianBlocks
{
    Eigen::MatrixXd Hcc;  // 카메라-카메라 블록
    Eigen::MatrixXd Hcp;  // 카메라-점 블록
    Eigen::MatrixXd Hpp;  // 점-점 블록 (블록 대각)
    Eigen::VectorXd bc;   // 카메라 잔차
    Eigen::VectorXd bp;   // 점 잔차
};

/**
 * @brief Schur Complement 기반 BA 최적화 기법 시연
 *
 * BA의 Hessian 희소 구조를 이해하고,
 * Schur Complement로 효율적으로 푸는 방법을 학습한다.
 */
class SchurBasic
{
   public:
    /**
     * @brief 간단한 BA Hessian 구성
     * @param num_cameras 카메라 수
     * @param num_points 3D 점 수
     * @param observations 관측 (camera_idx, point_idx) 쌍
     * @return Hessian 블록 구조
     */
    static HessianBlocks build_hessian(int num_cameras, int num_points,
                                       const std::vector<std::pair<int, int>>& observations);

    /**
     * @brief Hessian에서 블록 추출
     * @param H 전체 Hessian 행렬
     * @param cam_dim 카메라 변수 차원
     * @param pt_dim 점 변수 차원
     * @return 블록 구조
     */
    static HessianBlocks extract_blocks(const Eigen::MatrixXd& H, const Eigen::VectorXd& b,
                                        int cam_dim, int pt_dim);

    /**
     * @brief Schur Complement 계산
     *
     * H_reduced = Hcc - Hcp * Hpp^{-1} * Hpc
     * b_reduced = bc  - Hcp * Hpp^{-1} * bp
     *
     * @param blocks Hessian 블록
     * @param H_reduced 출력: 축소된 Hessian
     * @param b_reduced 출력: 축소된 잔차
     */
    static void schur_complement(const HessianBlocks& blocks,
                                 Eigen::MatrixXd& H_reduced, Eigen::VectorXd& b_reduced);

    /**
     * @brief 축소된 시스템으로 카메라 변수 풀기
     * @param H_reduced Schur 후 Hessian
     * @param b_reduced Schur 후 잔차
     * @return 카메라 변수 업데이트 delta_c
     */
    static Eigen::VectorXd solve_reduced(const Eigen::MatrixXd& H_reduced,
                                         const Eigen::VectorXd& b_reduced);

    /**
     * @brief 역대입으로 점 변수 풀기
     *
     * delta_p = Hpp^{-1} * (bp - Hpc * delta_c)
     *
     * @param blocks Hessian 블록
     * @param delta_c 카메라 변수 업데이트
     * @return 점 변수 업데이트 delta_p
     */
    static Eigen::VectorXd back_substitute(const HessianBlocks& blocks,
                                           const Eigen::VectorXd& delta_c);
};

#endif  // SCHUR_BASIC_H_
