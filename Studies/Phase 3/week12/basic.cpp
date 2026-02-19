#include "scale_basic.h"
#include <iostream>
#include <cmath>
#include <random>

// ====================
// ScaleBasic 구현
// ====================

std::vector<Eigen::Vector2d> ScaleBasic::generate_circular_trajectory(int n_frames, double radius)
{
    std::vector<Eigen::Vector2d> trajectory;
    for (int i = 0; i < n_frames; i++)
    {
        double theta = 2.0 * M_PI * i / n_frames;
        trajectory.push_back({radius * std::cos(theta), radius * std::sin(theta)});
    }
    return trajectory;
}

std::vector<Eigen::Vector2d> ScaleBasic::apply_scale_drift(
    const std::vector<Eigen::Vector2d>& gt, double scale_noise_std, int seed)
{
    std::default_random_engine gen(seed);
    std::normal_distribution<double> noise(1.0, scale_noise_std);

    std::vector<Eigen::Vector2d> estimated;
    estimated.push_back(gt[0]);

    for (size_t i = 1; i < gt.size(); i++)
    {
        Eigen::Vector2d delta = gt[i] - gt[i - 1];
        double scale = noise(gen);
        estimated.push_back(estimated.back() + scale * delta);
    }
    return estimated;
}

double ScaleBasic::compute_ate(const std::vector<Eigen::Vector2d>& gt,
                               const std::vector<Eigen::Vector2d>& est)
{
    double sum_sq = 0.0;
    for (size_t i = 0; i < gt.size(); i++)
    {
        sum_sq += (gt[i] - est[i]).squaredNorm();
    }
    return std::sqrt(sum_sq / gt.size());
}

std::vector<Eigen::Vector2d> ScaleBasic::align_sim3(const std::vector<Eigen::Vector2d>& gt,
                                                    const std::vector<Eigen::Vector2d>& est,
                                                    double& scale)
{
    // 간단한 스케일 추정: gt와 est의 이동 크기 비율
    double gt_dist = 0, est_dist = 0;
    for (size_t i = 1; i < gt.size(); i++)
    {
        gt_dist += (gt[i] - gt[i - 1]).norm();
        est_dist += (est[i] - est[i - 1]).norm();
    }
    scale = gt_dist / est_dist;

    // 스케일 적용 (중심 기준)
    Eigen::Vector2d est_center = Eigen::Vector2d::Zero();
    for (const auto& p : est) est_center += p;
    est_center /= est.size();

    std::vector<Eigen::Vector2d> aligned;
    for (const auto& p : est)
    {
        aligned.push_back(est_center + scale * (p - est_center));
    }
    return aligned;
}

void ScaleBasic::analyze_trajectory(const std::vector<Eigen::Vector2d>& gt,
                                    const std::vector<Eigen::Vector2d>& est,
                                    const std::string& label)
{
    double final_err = (gt.back() - est.back()).norm();
    double ate = compute_ate(gt, est);
    double loop_err = (est.back() - est.front()).norm();

    std::cout << "  [" << label << "]" << std::endl;
    std::cout << "  최종 오차: " << final_err << " m" << std::endl;
    std::cout << "  ATE(RMSE): " << ate << " m" << std::endl;
    std::cout << "  Loop 오차: " << loop_err << " m\n" << std::endl;
}

// ====================
// 교육 블록
// ====================

// 💡 quiz_easy Q1: 스케일 모호성 원인
// 💡 quiz_easy Q2: ||t|| = 1 정규화
void demoScaleAmbiguity()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 스케일 모호성의 원인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "핀홀 투영: λ[u,v,1]^T = K[R|t][X,1]^T" << std::endl;
    std::cout << "  → X와 λX는 같은 픽셀에 투영!\n" << std::endl;

    // 수치 증명
    Eigen::Matrix3d K;
    K << 500, 0, 320, 0, 500, 240, 0, 0, 1;
    Eigen::Vector3d X(1.0, 2.0, 5.0);

    Eigen::Vector3d proj = K * X;
    Eigen::Vector2d uv(proj(0) / proj(2), proj(1) / proj(2));

    std::cout << "  X = (1, 2, 5)  → 픽셀 (" << uv(0) << ", " << uv(1) << ")" << std::endl;

    for (double lambda : {2.0, 5.0, 10.0})
    {
        Eigen::Vector3d proj_s = K * (lambda * X);
        Eigen::Vector2d uv_s(proj_s(0) / proj_s(2), proj_s(1) / proj_s(2));
        std::cout << "  " << lambda << "X = (" << lambda * X.transpose()
                  << ") → 픽셀 (" << uv_s(0) << ", " << uv_s(1) << ")" << std::endl;
    }

    std::cout << "\n💡 Essential Matrix에서 ||t|| = 1로 정규화하는 이유:" << std::endl;
    std::cout << "   E = [αt]× R 에서 α가 소거됨 → 크기 결정 불가" << std::endl;
}

// 💡 quiz_easy Q3: 스케일 드리프트
// 💡 quiz_medium Q2: 드리프트 누적 시뮬레이션
void demoScaleDrift()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 스케일 드리프트 시뮬레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    auto gt = ScaleBasic::generate_circular_trajectory(100, 5.0);

    std::cout << "원형 궤적 (100 프레임, 반지름 5m):\n" << std::endl;

    for (double noise : {0.01, 0.02, 0.05})
    {
        auto est = ScaleBasic::apply_scale_drift(gt, noise);
        char label[32];
        snprintf(label, sizeof(label), "노이즈 %.0f%%", noise * 100);
        ScaleBasic::analyze_trajectory(gt, est, label);
    }

    std::cout << "💡 1%의 작은 노이즈도 100프레임 누적 → 큰 드리프트!" << std::endl;
}

// 💡 quiz_easy Q4: 스케일 결정 가능 경우
// 💡 quiz_medium Q3: Sim(3) vs SE(3)
void demoSim3Alignment()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Sim(3) 정렬 효과" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "SE(3): [R t; 0 1]  → 6 DoF (스케일 모름)" << std::endl;
    std::cout << "Sim(3): [sR t; 0 1] → 7 DoF (스케일 포함)\n" << std::endl;

    auto gt = ScaleBasic::generate_circular_trajectory(100, 5.0);
    auto est = ScaleBasic::apply_scale_drift(gt, 0.02);

    // 정렬 전 ATE
    double ate_before = ScaleBasic::compute_ate(gt, est);

    // Sim(3) 정렬
    double scale;
    auto aligned = ScaleBasic::align_sim3(gt, est, scale);
    double ate_after = ScaleBasic::compute_ate(gt, aligned);

    std::cout << "정렬 전 ATE: " << ate_before << " m" << std::endl;
    std::cout << "추정 스케일: " << scale << std::endl;
    std::cout << "정렬 후 ATE: " << ate_after << " m\n" << std::endl;

    std::cout << "💡 Sim(3) 정렬이 스케일 드리프트를 부분 보정" << std::endl;
    std::cout << "   (Loop Closure에서 Sim(3) 최적화 사용)" << std::endl;
}

// ====================
// Main
// ====================

#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 12: 스케일 모호성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    demoScaleAmbiguity();
    demoScaleDrift();
    demoSim3Alignment();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📌 다음 단계:" << std::endl;
    std::cout << "  1. quiz_easy    → 스케일 모호성 원인, 정규화, 드리프트" << std::endl;
    std::cout << "  2. quiz_medium  → λ 증명, 드리프트 시뮬, Sim(3)" << std::endl;
    std::cout << "  3. my_basic.cpp → 직접 구현 (5 Step)" << std::endl;
    std::cout << "  4. Week 13      → 스케일 복구 방법" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
