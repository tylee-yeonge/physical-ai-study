/**
 * Phase 3 Week 4 - ICP 직접 구현
 *
 * icp_basic.h의 함수들을 직접 구현해보세요.
 *
 * ┌──────┬──────────────────────┬────────┬──────────────┐
 * │ Step │ 함수                 │ 난이도  │ 검증 방법     │
 * ├──────┼──────────────────────┼────────┼──────────────┤
 * │  1   │ compute_centroid     │ 쉬움   │ ./my_basic   │
 * │  2   │ compute_h            │ 보통   │ ./my_basic   │
 * │  3   │ svd_decompose        │ 핵심   │ ./my_basic   │
 * │  4   │ iterative_icp        │ 핵심   │ ./my_basic   │
 * │  5   │ evaluate_alignment   │ 쉬움   │ ./my_basic   │
 * └──────┴──────────────────────┴────────┴──────────────┘
 */
#include "icp_basic.h"
#include <iostream>
#include <cmath>

// [Step 1] compute_centroid — 점 집합의 평균 계산
// 힌트: 모든 점을 더한 뒤 개수로 나눔
// 기대값: {(0,0,0), (2,0,0), (0,2,0)} → (2/3, 2/3, 0)
Eigen::Vector3d ICPBasic::compute_centroid(const std::vector<Eigen::Vector3d>& points)
{
    // TODO: 벡터 합산 → 개수로 나눔
    return Eigen::Vector3d::Zero();
}

// [Step 2] compute_h — Covariance 행렬 H = Σ src_i * tgt_i^T
// 힌트: 이미 중심화된 점을 받음. 외적합(outer product) 누적
// 참고: basic.cpp demoSVDAlignment() 수식 3번
// 기대값: 3x3 행렬
Eigen::Matrix3d ICPBasic::compute_h(const std::vector<Eigen::Vector3d>& src,
                                    const std::vector<Eigen::Vector3d>& tgt)
{
    // TODO: H = Zero → 순회 → H += src[i] * tgt[i].transpose()
    return Eigen::Matrix3d::Zero();
}

// [Step 3] svd_decompose — SVD로 R, t 계산 (핵심!)
// 힌트:
//   1. compute_centroid로 중심점 계산
//   2. 중심화: p'_i = p_i - p_bar
//   3. compute_h로 H 계산
//   4. JacobiSVD(H, ComputeFullU | ComputeFullV)
//   5. R = V * U^T,  t = q_bar - R * p_bar
//   6. det(R) < 0이면 V.col(2) *= -1 후 재계산
// 참고: basic.cpp demoSVDAlignment()
// 기대값: 노이즈 없으면 RMSE ≈ 0
double ICPBasic::svd_decompose(const std::vector<Eigen::Vector3d>& src,
                               const std::vector<Eigen::Vector3d>& tgt,
                               Eigen::Matrix3d& R, Eigen::Vector3d& t)
{
    // TODO: 위 6단계 구현
    R = Eigen::Matrix3d::Identity();
    t = Eigen::Vector3d::Zero();
    return -1.0;
}

// [Step 4] iterative_icp — 반복 ICP
// 힌트:
//   1. 작업용 복사본 생성
//   2. 반복: svd_decompose → 점 변환 → 누적 → 수렴 체크
//   3. |prev_error - error| < tol이면 수렴
// 참고: basic.cpp demoIterativeICP()
// 기대값: 2-3회 반복 후 수렴
ICPResult ICPBasic::iterative_icp(const std::vector<Eigen::Vector3d>& src,
                                  const std::vector<Eigen::Vector3d>& tgt,
                                  int max_iter, double tol)
{
    ICPResult result;
    result.R = Eigen::Matrix3d::Identity();
    result.t = Eigen::Vector3d::Zero();
    result.converged = false;
    result.error = -1.0;
    result.iterations = 0;

    // TODO:
    // 1. current = src (복사)
    // 2. for iter < max_iter:
    //    a. svd_decompose(current, tgt, R_step, t_step)
    //    b. current 점들 변환: p = R_step * p + t_step
    //    c. 누적: result.R = R_step * result.R, result.t = R_step * result.t + t_step
    //    d. 수렴 체크

    return result;
}

// [Step 5] evaluate_alignment — RMSE 계산
// 힌트: sqrt(Σ ||tgt_i - (R*src_i + t)||² / n)
// 기대값: 완벽한 정합이면 0, 노이즈 있으면 노이즈 수준
double ICPBasic::evaluate_alignment(const std::vector<Eigen::Vector3d>& src,
                                    const std::vector<Eigen::Vector3d>& tgt,
                                    const Eigen::Matrix3d& R, const Eigen::Vector3d& t)
{
    // TODO: 변환 후 거리의 제곱합 → 평균 → sqrt
    return -1.0;
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 3D-3D ICP 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 데이터: 30도 회전 + 이동
    double angle = M_PI / 6;
    Eigen::Matrix3d R_true;
    R_true << std::cos(angle), -std::sin(angle), 0,
              std::sin(angle),  std::cos(angle), 0,
              0,                0,               1;
    Eigen::Vector3d t_true(1.0, 0.5, -0.3);

    std::vector<Eigen::Vector3d> src = {
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
        {1, 1, 0}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}
    };

    std::vector<Eigen::Vector3d> tgt(src.size());
    for (size_t i = 0; i < src.size(); i++)
    {
        tgt[i] = R_true * src[i] + t_true;
    }

    // ── Step 1 테스트: compute_centroid ──
    std::cout << "[Step 1] compute_centroid" << std::endl;
    Eigen::Vector3d centroid = ICPBasic::compute_centroid(src);
    Eigen::Vector3d expected_centroid(0.5, 0.5, 0.5);
    bool step1 = (centroid - expected_centroid).norm() < 1e-10;
    std::cout << "  중심점: " << centroid.transpose()
              << (step1 ? " ✅" : " ❌ (기대: 0.5 0.5 0.5)") << "\n" << std::endl;

    // ── Step 2 테스트: compute_h ──
    std::cout << "[Step 2] compute_h" << std::endl;
    Eigen::Vector3d src_mean = ICPBasic::compute_centroid(src);
    Eigen::Vector3d tgt_mean = ICPBasic::compute_centroid(tgt);
    std::vector<Eigen::Vector3d> src_c(src.size()), tgt_c(tgt.size());
    for (size_t i = 0; i < src.size(); i++)
    {
        src_c[i] = src[i] - src_mean;
        tgt_c[i] = tgt[i] - tgt_mean;
    }
    Eigen::Matrix3d H = ICPBasic::compute_h(src_c, tgt_c);
    bool step2 = (H.norm() > 0.1);  // H가 0이 아님
    std::cout << "  H norm: " << H.norm()
              << (step2 ? " ✅" : " ❌ (H가 0이면 안 됨)") << "\n" << std::endl;

    // ── Step 3 테스트: svd_decompose ──
    std::cout << "[Step 3] svd_decompose" << std::endl;
    Eigen::Matrix3d R_est;
    Eigen::Vector3d t_est;
    double error = ICPBasic::svd_decompose(src, tgt, R_est, t_est);
    double R_err = (R_true - R_est).norm();
    double t_err = (t_true - t_est).norm();
    bool step3 = (R_err < 1e-6 && t_err < 1e-6);
    std::cout << "  R 오차: " << R_err << (R_err < 1e-6 ? " ✅" : " ❌") << std::endl;
    std::cout << "  t 오차: " << t_err << (t_err < 1e-6 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 4 테스트: iterative_icp ──
    std::cout << "[Step 4] iterative_icp" << std::endl;
    ICPResult result = ICPBasic::iterative_icp(src, tgt, 10, 1e-8);
    bool step4 = (result.converged && result.error < 1e-6);
    std::cout << "  수렴: " << (result.converged ? "예 ✅" : "아니오 ❌") << std::endl;
    std::cout << "  반복: " << result.iterations << "회" << std::endl;
    std::cout << "  RMSE: " << result.error << "\n" << std::endl;

    // ── Step 5 테스트: evaluate_alignment ──
    std::cout << "[Step 5] evaluate_alignment" << std::endl;
    double rmse_perfect = ICPBasic::evaluate_alignment(src, tgt, R_true, t_true);
    double rmse_identity = ICPBasic::evaluate_alignment(src, tgt,
                           Eigen::Matrix3d::Identity(), Eigen::Vector3d::Zero());
    bool step5 = (rmse_perfect < 1e-10 && rmse_identity > 0.5);
    std::cout << "  완벽 정합: " << rmse_perfect << (rmse_perfect < 1e-10 ? " ✅" : " ❌") << std::endl;
    std::cout << "  미정합:    " << rmse_identity << (rmse_identity > 0.5 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── 종합 결과 ──
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  종합: Step1 " << (step1 ? "✅" : "❌")
              << " Step2 " << (step2 ? "✅" : "❌")
              << " Step3 " << (step3 ? "✅" : "❌")
              << " Step4 " << (step4 ? "✅" : "❌")
              << " Step5 " << (step5 ? "✅" : "❌") << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
