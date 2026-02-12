/**
 * Phase 3 Week 11 - Ceres 중급 퀴즈 정답 및 해설
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <iomanip>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 11 Medium Quiz - 정답 및 해설" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 문제 1 해설
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Cost Function 잔차 차원 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    int num_cams = 10, cam_dim = 9;
    int num_pts = 100, pt_dim = 3;
    int num_obs = 500, res_dim = 2;

    int total_res = num_obs * res_dim;
    int total_params = num_cams * cam_dim + num_pts * pt_dim;
    int nnz_per_obs = res_dim * (cam_dim + pt_dim);  // 비영 원소/관측
    int total_nnz = num_obs * nnz_per_obs;
    int total_elements = total_res * total_params;
    double sparsity = 100.0 * (1.0 - (double)total_nnz / total_elements);

    std::cout << "정답:" << std::endl;
    std::cout << "  전체 잔차 벡터 차원: " << total_res << " (500 관측 x 2)\n" << std::endl;

    std::cout << "해설 - 자코비안 행렬 분석:" << std::endl;
    std::cout << "  J 크기: " << total_res << " x " << total_params << std::endl;
    std::cout << "  = (관측수 x 잔차차원) x (카메라파라미터 + 점파라미터)" << std::endl;
    std::cout << "  = (500 x 2) x (10 x 9 + 100 x 3)" << std::endl;
    std::cout << "  = 1000 x 390\n" << std::endl;

    std::cout << "  희소성 분석:" << std::endl;
    std::cout << "  전체 원소: " << total_elements << std::endl;
    std::cout << "  비영 원소: " << total_nnz << std::endl;
    std::cout << "  희소율: " << std::fixed << std::setprecision(1) << sparsity << "%" << std::endl;
    std::cout << "\n  왜 희소한가?" << std::endl;
    std::cout << "  → 각 관측은 카메라 1개(9파라미터) + 점 1개(3파라미터)에만 의존" << std::endl;
    std::cout << "  → 나머지 카메라/점에 대한 Jacobian은 모두 0" << std::endl;
    std::cout << "  → 이 희소 구조가 Schur Complement의 핵심!" << std::endl;

    // 문제 2 해설
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 파라미터 블록 크기와 Schur Complement" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    int N = 50, M = 10000;
    int cd = 9, pd = 3;
    int total_dim = N * cd + M * pd;
    int cam_total = N * cd;
    int pt_total = M * pd;

    // Eigen으로 크기 시각화
    Eigen::Vector2i H_full(total_dim, total_dim);
    Eigen::Vector2i S_schur(cam_total, cam_total);

    double H_mem = (double)total_dim * total_dim * 8.0 / (1024 * 1024);
    double S_mem = (double)cam_total * cam_total * 8.0 / (1024 * 1024);
    double ratio = (double)(total_dim)*total_dim / ((double)cam_total * cam_total);

    std::cout << "정답:" << std::endl;
    std::cout << "  Schur 전: " << total_dim << " x " << total_dim << " = "
              << (long long)total_dim * total_dim << " 원소" << std::endl;
    std::cout << "  Schur 후: " << cam_total << " x " << cam_total << " = "
              << (long long)cam_total * cam_total << " 원소\n"
              << std::endl;

    std::cout << "해설:" << std::endl;
    std::cout << "  전체 파라미터:" << std::endl;
    std::cout << "  - 카메라: " << N << " x " << cd << " = " << cam_total << "차원" << std::endl;
    std::cout << "  - 점:     " << M << " x " << pd << " = " << pt_total << "차원" << std::endl;
    std::cout << "  - 합계:   " << total_dim << "차원\n" << std::endl;

    std::cout << "  Schur Complement 전 (전체 Hessian H):" << std::endl;
    std::cout << "  H 크기: " << total_dim << " x " << total_dim << std::endl;
    std::cout << "  메모리 (dense 가정): ~" << std::fixed << std::setprecision(0) << H_mem
              << " MB\n"
              << std::endl;

    std::cout << "  Schur Complement 후 (S 행렬):" << std::endl;
    std::cout << "  S 크기: " << cam_total << " x " << cam_total << std::endl;
    std::cout << "  메모리 (dense 가정): ~" << std::fixed << std::setprecision(2) << S_mem
              << " MB\n"
              << std::endl;

    std::cout << "  크기 감소: " << std::fixed << std::setprecision(0) << ratio << "배!"
              << std::endl;
    std::cout << "  " << total_dim << "^2 → " << cam_total << "^2\n" << std::endl;

    std::cout << "  핵심 포인트:" << std::endl;
    std::cout << "  - 점(30,000차원)이 카메라(450차원)보다 훨씬 많음" << std::endl;
    std::cout << "  - Schur로 점을 소거하면 450x450만 풀면 됨" << std::endl;
    std::cout << "  - Hpp는 블록 대각이라 역행렬이 빠름 (3x3 블록 10,000개)" << std::endl;
    std::cout << "  - 이것이 DENSE_SCHUR / SPARSE_SCHUR의 원리" << std::endl;

    // 문제 3 해설
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 수렴 행동 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double costs[] = {1.234e+07, 5.678e+05, 1.234e+04, 8.901e+03,
                      8.567e+03, 8.456e+03, 8.450e+03, 8.449e+03};
    int n = 8;

    std::cout << "정답:" << std::endl;
    std::cout << "  전형적인 Levenberg-Marquardt 수렴 패턴\n" << std::endl;

    std::cout << "해설:" << std::endl;
    std::cout << "  1. 수렴 단계 분석:\n" << std::endl;

    std::cout << "     Phase 1 (Iter 0-1): 급격한 감소" << std::endl;
    double p1_reduction = (1.0 - costs[1] / costs[0]) * 100.0;
    std::cout << "       " << std::scientific << costs[0] << " → " << costs[1] << std::endl;
    std::cout << "       감소율: " << std::fixed << std::setprecision(1) << p1_reduction << "%"
              << std::endl;
    std::cout << "       → 초기 추정이 나빴지만, 첫 step에서 크게 개선\n" << std::endl;

    std::cout << "     Phase 2 (Iter 1-3): 빠른 수렴" << std::endl;
    std::cout << "       cost가 10^5 → 10^3으로 감소" << std::endl;
    std::cout << "       → 대략적인 해에 접근\n" << std::endl;

    std::cout << "     Phase 3 (Iter 3-7): 미세 조정" << std::endl;
    std::cout << "       cost 변화가 매우 작음 (8901 → 8449)" << std::endl;
    std::cout << "       → 정밀한 해에 수렴 중\n" << std::endl;

    double total_reduction = (1.0 - costs[n - 1] / costs[0]) * 100.0;
    std::cout << "  2. 전체 통계:" << std::endl;
    std::cout << "     초기 cost: " << std::scientific << costs[0] << std::endl;
    std::cout << "     최종 cost: " << costs[n - 1] << std::endl;
    std::cout << "     총 감소율: " << std::fixed << std::setprecision(4) << total_reduction << "%"
              << std::endl;
    std::cout << "     Iteration 수: " << n << "\n" << std::endl;

    std::cout << "  3. Ceres 수렴 조건 (기본값):" << std::endl;
    std::cout << "     function_tolerance = 1e-6" << std::endl;
    std::cout << "       → |cost_change| / cost < 1e-6" << std::endl;
    std::cout << "     gradient_tolerance = 1e-10" << std::endl;
    std::cout << "       → |gradient|_inf < 1e-10" << std::endl;
    std::cout << "     parameter_tolerance = 1e-8" << std::endl;
    std::cout << "       → |step| / |x| < 1e-8\n" << std::endl;

    std::cout << "  4. 실무 팁:" << std::endl;
    std::cout << "     - cost가 줄지 않으면: 초기값이 나쁘거나 문제 설정 오류" << std::endl;
    std::cout << "     - 진동하면: damping (lambda) 조절 필요" << std::endl;
    std::cout << "     - 너무 빨리 수렴: local minimum에 빠졌을 수 있음" << std::endl;
    std::cout << "     - max_iterations 늘리기 전에 초기값 개선 권장" << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Medium Quiz 해설 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
