/**
 * Phase 3 Week 12 - 스케일 모호성 직접 구현
 *
 * scale_basic.h의 함수들을 직접 구현해보세요.
 *
 * ┌──────┬──────────────────────────────┬────────┬──────────────┐
 * │ Step │ 함수                         │ 난이도  │ 검증 방법     │
 * ├──────┼──────────────────────────────┼────────┼──────────────┤
 * │  1   │ generate_circular_trajectory │ 쉬움   │ ./my_basic   │
 * │  2   │ apply_scale_drift            │ 핵심   │ ./my_basic   │
 * │  3   │ compute_ate                  │ 쉬움   │ ./my_basic   │
 * │  4   │ align_sim3                   │ 어려움  │ ./my_basic   │
 * │  5   │ analyze_trajectory           │ 쉬움   │ ./my_basic   │
 * └──────┴──────────────────────────────┴────────┴──────────────┘
 */
#include "scale_basic.h"
#include <iostream>
#include <cmath>
#include <random>

// [Step 1] generate_circular_trajectory — 원형 궤적 생성
// 힌트: theta = 2π * i / n,  x = r*cos(θ),  y = r*sin(θ)
// 기대값: n=4, r=1 → (1,0), (0,1), (-1,0), (0,-1)
std::vector<Eigen::Vector2d> ScaleBasic::generate_circular_trajectory(int n_frames, double radius)
{
    // TODO: n_frames개 점을 원 위에 생성
    return {};
}

// [Step 2] apply_scale_drift — 스케일 노이즈 적용 (핵심!)
// 힌트:
//   1. 시작점은 gt[0]과 동일
//   2. 매 프레임: delta = gt[i] - gt[i-1]
//   3. scale = normal_distribution(1.0, noise_std) 샘플
//   4. est[i] = est[i-1] + scale * delta
// 참고: basic.cpp demoScaleDrift()
// 기대값: noise_std가 클수록 GT와 차이가 커짐
std::vector<Eigen::Vector2d> ScaleBasic::apply_scale_drift(
    const std::vector<Eigen::Vector2d>& gt, double scale_noise_std, int seed)
{
    // TODO: 위 4단계 구현
    return gt;  // 임시: GT 그대로 반환
}

// [Step 3] compute_ate — ATE (Absolute Trajectory Error) RMSE 계산
// 힌트: sqrt(Σ ||gt[i] - est[i]||² / n)
// 기대값: gt == est → 0,  드리프트 있으면 > 0
double ScaleBasic::compute_ate(const std::vector<Eigen::Vector2d>& gt,
                               const std::vector<Eigen::Vector2d>& est)
{
    // TODO: 거리의 제곱합 → 평균 → sqrt
    return -1.0;
}

// [Step 4] align_sim3 — Sim(3) 정렬 (스케일 추정 + 적용)
// 힌트:
//   1. GT와 EST의 총 이동 거리 비율로 스케일 추정
//   2. est 중심점 기준으로 스케일 적용
// 참고: basic.cpp demoSim3Alignment()
// 기대값: 정렬 후 ATE가 정렬 전보다 작아짐
std::vector<Eigen::Vector2d> ScaleBasic::align_sim3(const std::vector<Eigen::Vector2d>& gt,
                                                    const std::vector<Eigen::Vector2d>& est,
                                                    double& scale)
{
    // TODO:
    // 1. gt_dist = Σ ||gt[i] - gt[i-1]||
    // 2. est_dist = Σ ||est[i] - est[i-1]||
    // 3. scale = gt_dist / est_dist
    // 4. est_center 계산 → aligned[i] = center + scale * (est[i] - center)
    scale = 1.0;
    return est;  // 임시: 그대로 반환
}

// [Step 5] analyze_trajectory — 통계 출력
// 힌트: 최종 오차, ATE, Loop closure 오차 출력
void ScaleBasic::analyze_trajectory(const std::vector<Eigen::Vector2d>& gt,
                                    const std::vector<Eigen::Vector2d>& est,
                                    const std::string& label)
{
    // TODO:
    // 1. final_err = (gt.back() - est.back()).norm()
    // 2. ate = compute_ate(gt, est)
    // 3. loop_err = (est.back() - est.front()).norm()
    // 4. 출력
    std::cout << "  [" << label << "] TODO: 통계 출력 구현" << std::endl;
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 스케일 모호성 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ── Step 1 테스트: generate_circular_trajectory ──
    std::cout << "[Step 1] generate_circular_trajectory" << std::endl;
    auto gt = ScaleBasic::generate_circular_trajectory(100, 5.0);
    bool step1 = (gt.size() == 100 && std::abs(gt[0](0) - 5.0) < 1e-10);
    std::cout << "  궤적 크기: " << gt.size()
              << (step1 ? " ✅" : " ❌ (기대: 100, 시작 (5,0))") << "\n" << std::endl;

    // ── Step 2 테스트: apply_scale_drift ──
    std::cout << "[Step 2] apply_scale_drift" << std::endl;
    auto est = ScaleBasic::apply_scale_drift(gt, 0.02);
    double final_diff = (gt.back() - est.back()).norm();
    bool step2 = (final_diff > 0.01);  // 드리프트가 있어야 함
    std::cout << "  최종 위치 차이: " << final_diff
              << (step2 ? " ✅ (드리프트 확인)" : " ❌ (드리프트 없음)") << "\n" << std::endl;

    // ── Step 3 테스트: compute_ate ──
    std::cout << "[Step 3] compute_ate" << std::endl;
    double ate_zero = ScaleBasic::compute_ate(gt, gt);  // 동일 → 0
    double ate_drift = ScaleBasic::compute_ate(gt, est);  // 차이 > 0
    bool step3 = (ate_zero < 1e-10 && ate_drift > 0);
    std::cout << "  동일 궤적 ATE: " << ate_zero << (ate_zero < 1e-10 ? " ✅" : " ❌") << std::endl;
    std::cout << "  드리프트 ATE:  " << ate_drift << (ate_drift > 0 ? " ✅" : " ❌") << "\n" << std::endl;

    // ── Step 4 테스트: align_sim3 ──
    std::cout << "[Step 4] align_sim3" << std::endl;
    double scale;
    auto aligned = ScaleBasic::align_sim3(gt, est, scale);
    double ate_aligned = ScaleBasic::compute_ate(gt, aligned);
    bool step4 = (ate_aligned < ate_drift);
    std::cout << "  추정 스케일: " << scale << std::endl;
    std::cout << "  정렬 전 ATE: " << ate_drift << std::endl;
    std::cout << "  정렬 후 ATE: " << ate_aligned
              << (step4 ? " ✅ (개선됨)" : " ❌") << "\n" << std::endl;

    // ── Step 5 테스트: analyze_trajectory ──
    std::cout << "[Step 5] analyze_trajectory" << std::endl;
    ScaleBasic::analyze_trajectory(gt, est, "노이즈 2%");

    // ── 종합 결과 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  종합: Step1 " << (step1 ? "✅" : "❌")
              << " Step2 " << (step2 ? "✅" : "❌")
              << " Step3 " << (step3 ? "✅" : "❌")
              << " Step4 " << (step4 ? "✅" : "❌") << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
