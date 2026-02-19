#ifndef PHASE3_WEEK7_BA_BASIC_H_
#define PHASE3_WEEK7_BA_BASIC_H_

#include <g2o/core/sparse_optimizer.h>
#include <g2o/core/block_solver.h>
#include <g2o/core/optimization_algorithm_levenberg.h>
#include <g2o/solvers/eigen/linear_solver_eigen.h>
#include <g2o/types/sba/types_six_dof_expmap.h>
#include <g2o/core/robust_kernel_impl.h>

#include <Eigen/Dense>
#include <vector>

/**
 * @brief g2o 기반 Bundle Adjustment 기초 클래스
 *
 * 간단한 BA 문제 (카메라 포즈 + 3D 점)를 g2o로 최적화.
 * Vertex = 최적화 변수 (포즈, 3D점)
 * Edge = 제약 조건 (재투영 오차)
 *
 * 파이프라인:
 *   createVertexPose → createVertexPoint → createEdge
 *   → setupOptimizer → optimize → evaluateResult
 */
class BABasic
{
public:
    /// BA 결과 구조체
    struct BAResult
    {
        double initial_error;   ///< 최적화 전 총 오차
        double final_error;     ///< 최적화 후 총 오차
        int iterations;         ///< 반복 횟수
        int num_poses;          ///< 포즈(카메라) 수
        int num_points;         ///< 3D 점 수
        int num_edges;          ///< Edge(관측) 수
    };

    /**
     * @brief 카메라 포즈 Vertex 생성 (SE3)
     * @param optimizer g2o 옵티마이저
     * @param id Vertex ID
     * @param R 회전 행렬 3x3
     * @param t 이동 벡터 3x1
     * @param fixed 고정 여부 (첫 포즈는 true)
     */
    static void create_vertex_pose(
        g2o::SparseOptimizer& optimizer,
        int id,
        const Eigen::Matrix3d& R,
        const Eigen::Vector3d& t,
        bool fixed = false);

    /**
     * @brief 3D 점 Vertex 생성
     * @param optimizer g2o 옵티마이저
     * @param id Vertex ID
     * @param point 3D 점 좌표
     * @param marginalized Schur Complement 적용 여부
     */
    static void create_vertex_point(
        g2o::SparseOptimizer& optimizer,
        int id,
        const Eigen::Vector3d& point,
        bool marginalized = true);

    /**
     * @brief 재투영 오차 Edge 생성
     * @param optimizer g2o 옵티마이저
     * @param pose_id 포즈 Vertex ID
     * @param point_id 3D 점 Vertex ID
     * @param observation 2D 관측 (픽셀)
     * @param K 카메라 내부 파라미터 (fx, fy, cx, cy)
     * @param use_robust Huber Robust Kernel 사용 여부
     */
    static void create_edge(
        g2o::SparseOptimizer& optimizer,
        int pose_id, int point_id,
        const Eigen::Vector2d& observation,
        const Eigen::Vector4d& K,
        bool use_robust = false);

    /**
     * @brief 옵티마이저 설정 (BlockSolver_6_3 + LM)
     * @param optimizer g2o 옵티마이저
     */
    static void setup_optimizer(g2o::SparseOptimizer& optimizer);

    /**
     * @brief 최적화 실행
     * @param optimizer g2o 옵티마이저
     * @param iterations 최대 반복 횟수
     * @return BA 결과
     */
    static BAResult optimize(g2o::SparseOptimizer& optimizer, int iterations = 10);

    /**
     * @brief 결과 평가 및 출력
     * @param optimizer g2o 옵티마이저
     * @param result BA 결과
     * @param label 출력 라벨
     */
    static void evaluate_result(
        const g2o::SparseOptimizer& optimizer,
        const BAResult& result,
        const std::string& label = "BA");

    /**
     * @brief 전체 데모
     */
    static void demo();
};

#endif  // PHASE3_WEEK7_BA_BASIC_H_
