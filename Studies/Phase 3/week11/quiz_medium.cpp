/**
 * Phase 3 Week 11 - Ceres 중급 퀴즈
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. 잔차 차원 분석 — BA에서 자코비안 크기와 희소성
 *   2. Schur Complement 크기 비교 — 적용 전후 행렬 크기
 *   3. 수렴 행동 분석 — LM 알고리즘의 전형적인 수렴 패턴
 *
 * BA의 자코비안 희소성:
 *
 *   자코비안 J: (2·N_obs) × (9·N_cam + 3·N_pts)
 *   각 관측은 카메라 1개 + 점 1개에만 의존
 *   → J의 대부분이 0
 *   → 비영 비율: (9+3)·2·N_obs / (total_rows × total_cols) ≈ 3%
 *
 * LM(Levenberg-Marquardt) 수렴 패턴:
 *
 *   Cost
 *   ↑  |
 *   |  |\
 *   |  | \
 *   |  |  \___________  ← 수렴
 *   |  |
 *   └──┼──────────── → Iteration
 *     0  1  2  3  ...
 *
 *   초기: 급격한 감소 (Gradient Descent에 가까움)
 *   후반: 미세 조정 (Gauss-Newton에 가까움)
 *
 * 난이도: ★★☆ (수치 분석, 코드 포함)
 * 선수 지식: quiz_easy, Week 8 (Ceres BA), Week 9 (Schur Complement)
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>

// 문제 1: Cost Function 잔차 차원 분석
//
// BA 설정: 카메라 10대, 3D 점 100개, 관측 500개
//
// 잔차 차원:
//   각 관측 = 2D 재투영 오차 (Δu, Δv) → 2차원
//   전체 잔차 = 500 × 2 = 1000
//
// 파라미터 차원:
//   카메라: 10 × 9(Snavely) = 90
//   3D 점: 100 × 3 = 300
//   전체: 90 + 300 = 390
//
// 자코비안 J: 1000 × 390 행렬
//   전체 원소: 390,000
//   비영 원소: 500 × (9+3) × 2 = 12,000 (약 3%)
//
// ★ 이 희소성이 BA를 효율적으로 풀 수 있게 하는 핵심
//   밀집(dense)이었다면 O(390³) ≈ 불가능
void problem1_residual_dimension()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Cost Function 잔차 차원 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 다음 BA 설정에서 전체 잔차 벡터의 차원은?\n" << std::endl;
    std::cout << "  - 카메라 10대" << std::endl;
    std::cout << "  - 3D 점 100개" << std::endl;
    std::cout << "  - 관측 500개" << std::endl;
    std::cout << "  - 각 관측의 잔차: 2차원 (재투영 오차 x, y)\n" << std::endl;

    // 계산
    int num_observations = 500;
    int residual_per_obs = 2;
    int total_residuals = num_observations * residual_per_obs;

    int num_cameras = 10;
    int camera_params = 9;  // Snavely 모델
    int num_points = 100;
    int point_params = 3;
    int total_params = num_cameras * camera_params + num_points * point_params;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   전체 잔차 차원: " << num_observations << " x " << residual_per_obs << " = "
              << total_residuals << std::endl;
    std::cout << "   전체 파라미터 차원: " << num_cameras << "x" << camera_params << " + "
              << num_points << "x" << point_params << " = " << total_params << std::endl;
    std::cout << "\n   자코비안 J의 크기: " << total_residuals << " x " << total_params
              << std::endl;
    std::cout << "   = 1000 x 390 행렬!" << std::endl;
    std::cout << "\n   하지만 대부분 0 (희소 행렬)" << std::endl;
    std::cout << "   → 각 관측은 카메라 1개(9) + 점 1개(3)에만 의존" << std::endl;
    std::cout << "   → 비영 원소: 500 x (9+3) x 2 = 12,000개만" << std::endl;
    std::cout << "   → 전체 원소: 390,000개 중 3%만 비영!" << std::endl;
}

// 문제 2: Schur Complement 적용 전후 행렬 크기 비교
//
// 조건: 카메라 50대 (9 param), 3D 점 10,000개 (3 param)
//
// Schur 적용 전 (전체 Hessian H):
//   크기: (50×9 + 10000×3) = 30,450 × 30,450
//   메모리(dense): ~7 GB → 메모리에 올릴 수도 없음!
//
// Schur 적용 후 (카메라만의 Schur complement S):
//   크기: 50×9 = 450 × 450
//   메모리(dense): ~1.5 MB → 순식간에 풀림
//
// 크기 비율: 30450² / 450² ≈ 4577배 감소!
//
// ★ 점이 많을수록 Schur Complement의 효과가 극적
//   실제 BA: 점 수만~수십만 → Schur 없이는 풀 수 없음
void problem2_parameter_blocks()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 파라미터 블록 크기와 Schur Complement" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: Schur Complement 적용 전후의 행렬 크기를 비교하세요.\n" << std::endl;
    std::cout << "  조건: 카메라 50대 (9파라미터), 3D 점 10,000개 (3파라미터)\n" << std::endl;

    int num_cameras = 50;
    int cam_dim = 9;
    int num_points = 10000;
    int pt_dim = 3;

    int total_dim = num_cameras * cam_dim + num_points * pt_dim;
    int camera_dim = num_cameras * cam_dim;
    int point_dim = num_points * pt_dim;

    // Hessian 크기 계산
    Eigen::Vector2i H_size(total_dim, total_dim);
    Eigen::Vector2i S_size(camera_dim, camera_dim);

    double H_memory_MB = (double)(total_dim)*total_dim * 8 / (1024 * 1024);
    double S_memory_MB = (double)(camera_dim)*camera_dim * 8 / (1024 * 1024);

    std::cout << "💡 답:" << std::endl;
    std::cout << "   Schur 적용 전 (전체 Hessian H):" << std::endl;
    std::cout << "   크기: " << total_dim << " x " << total_dim << std::endl;
    std::cout << "   = " << H_size(0) << " x " << H_size(1) << std::endl;
    std::cout << "   메모리 (dense): ~" << (int)H_memory_MB << " MB\n" << std::endl;

    std::cout << "   Schur 적용 후 (카메라만의 Schur complement S):" << std::endl;
    std::cout << "   크기: " << camera_dim << " x " << camera_dim << std::endl;
    std::cout << "   = " << S_size(0) << " x " << S_size(1) << std::endl;
    std::cout << "   메모리 (dense): ~" << std::fixed << std::setprecision(2) << S_memory_MB
              << " MB\n"
              << std::endl;

    double ratio = (double)(total_dim * total_dim) / (camera_dim * camera_dim);
    std::cout << "   크기 비율: " << (int)ratio << "배 감소!" << std::endl;
    std::cout << "   " << total_dim << "^2 / " << camera_dim << "^2 = " << (int)ratio << std::endl;
    std::cout << "\n   핵심: 점이 많을수록 Schur Complement 효과가 큼!" << std::endl;
}

// 문제 3: LM 수렴 행동 분석
//
// Levenberg-Marquardt (LM) 알고리즘:
//   λ가 크면 → Gradient Descent (큰 step, 탐색적)
//   λ가 작으면 → Gauss-Newton (작은 step, 정밀)
//
// 전형적인 수렴 패턴:
//   Iter 0-1: 급격한 cost 감소 (~95%)
//     → 초기 추정이 많이 틀릴 때, 큰 보정
//   Iter 2-4: 중간 감소 (~30%)
//     → 점점 좋은 방향으로 수렴
//   Iter 5+: 미세 변화 (<1%)
//     → 거의 수렴, 미세 조정
//
// Ceres 수렴 조건:
//   function_tolerance = 1e-6: (cost_k - cost_{k+1}) / cost_k < 1e-6
//   gradient_tolerance = 1e-10: ||gradient||_∞ < 1e-10
//   parameter_tolerance = 1e-8: ||Δx|| / ||x|| < 1e-8
//
// ★ 과도한 iteration은 시간 낭비
//   Ceres는 수렴 조건 충족 시 자동 종료
//   max_num_iterations(기본 50)로 상한 설정
void problem3_convergence_analysis()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 수렴 행동 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 다음 Solver 출력에서 수렴 상태를 분석하세요.\n" << std::endl;

    // 시뮬레이션된 수렴 과정
    double costs[] = {1.234e+07, 5.678e+05, 1.234e+04, 8.901e+03,
                      8.567e+03, 8.456e+03, 8.450e+03, 8.449e+03};
    int num_iters = 8;

    std::cout << "  Iter |     Cost     | Cost Change | Reduction(%)" << std::endl;
    std::cout << "  -----|--------------|-------------|-------------" << std::endl;

    for (int i = 0; i < num_iters; i++)
    {
        double change = (i == 0) ? 0.0 : costs[i - 1] - costs[i];
        double reduction = (i == 0) ? 0.0 : (change / costs[i - 1]) * 100.0;
        printf("  %4d | %12.3e | %11.3e | %8.2f%%\n", i, costs[i], change, reduction);
    }

    // 수렴 분석
    double total_reduction = (1.0 - costs[num_iters - 1] / costs[0]) * 100.0;
    double iter1_reduction = (1.0 - costs[1] / costs[0]) * 100.0;

    std::cout << "\n💡 분석:" << std::endl;
    std::cout << "   1. 전체 cost 감소: " << std::fixed << std::setprecision(2) << total_reduction
              << "%" << std::endl;
    std::cout << "   2. 첫 iteration에서 " << std::fixed << std::setprecision(1) << iter1_reduction
              << "% 감소 (가장 큰 감소!)" << std::endl;
    std::cout << "   3. Iter 3 이후 cost 변화가 매우 작음 → 수렴 상태" << std::endl;
    std::cout << "   4. 이것은 Levenberg-Marquardt의 전형적인 수렴 패턴:" << std::endl;
    std::cout << "      → 초기: 급격한 감소 (큰 step)" << std::endl;
    std::cout << "      → 후반: 미세 조정 (작은 step)" << std::endl;
    std::cout << "\n   수렴 판정 기준 (Ceres 기본값):" << std::endl;
    std::cout << "      function_tolerance = 1e-6" << std::endl;
    std::cout << "      → cost_change / cost < 1e-6 이면 수렴" << std::endl;

    double last_ratio = (costs[num_iters - 2] - costs[num_iters - 1]) / costs[num_iters - 2];
    std::cout << "      → 마지막 iteration: " << std::scientific << last_ratio << std::endl;
    if (last_ratio < 1e-6)
    {
        std::cout << "      → 수렴 조건 충족!" << std::endl;
    }
    else
    {
        std::cout << "      → 아직 수렴 조건 미충족 (더 많은 iteration 필요)" << std::endl;
    }
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 11 Quiz - Medium (Ceres 실습)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_residual_dimension();
    problem2_parameter_blocks();
    problem3_convergence_analysis();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
