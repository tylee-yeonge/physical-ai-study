#include "scale_basic.h"
#include <iostream>
#include <cmath>
#include <random>

// ====================
// ScaleBasic 구현
// ====================

// 원형 GT(Ground Truth) 궤적 생성 — 스케일 드리프트 시뮬레이션의 기준 경로
//
// 원형 궤적을 사용하는 이유:
//   ① 출발점과 도착점이 같으므로 "Loop Closure" 상황을 시뮬레이션 가능
//   ② 스케일 드리프트가 누적되면 시작점으로 돌아오지 못함
//      → 드리프트를 시각적으로 확인하기 좋음
//   ③ 일정한 곡률로 이동하므로 매 프레임의 이동 크기가 비슷
//
// 좌표 계산:
//   θ = 2π · i / n_frames    (0 ~ 2π를 n_frames 등분)
//   x = radius · cos(θ)
//   y = radius · sin(θ)
//   → 반지름 radius(미터)의 원을 n_frames개 점으로 균등 분할
//
// 예시: n_frames=100, radius=5.0m
//   → 둘레 = 2π·5 ≈ 31.4m, 프레임 간 이동 ≈ 0.31m
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

// 스케일 드리프트 시뮬레이션 — 단안 VO의 핵심 약점을 재현
//
// 단안 VO에서 스케일 드리프트가 발생하는 이유:
//   ① Essential Matrix에서 t를 ||t||=1로 정규화하므로 절대 스케일 손실
//   ② 매 프레임마다 독립적으로 t를 추정하므로 상대 스케일이 조금씩 다름
//   ③ 이전 프레임의 스케일 오차가 다음 프레임에 전파 → 누적!
//
// 시뮬레이션 방법:
//   GT의 프레임 간 이동 벡터 Δ에 랜덤 스케일 팩터를 곱함:
//     estimated[i] = estimated[i-1] + scale · (gt[i] - gt[i-1])
//     scale ~ N(1.0, scale_noise_std)
//   → scale이 정확히 1.0이면 GT와 동일
//   → 1.02면 이 프레임에서 2% 길게, 0.98이면 2% 짧게 추정
//
// ★ scale_noise_std = 0.01 (1%)이라도 100프레임 누적 시:
//   랜덤 워크 특성: 오차 ∝ √N → 100프레임에서 ~10% 누적 오차
//   실제로는 편향(bias)도 있어 더 심해질 수 있음
std::vector<Eigen::Vector2d> ScaleBasic::apply_scale_drift(
    const std::vector<Eigen::Vector2d>& gt, double scale_noise_std, int seed)
{
    std::default_random_engine gen(seed);
    std::normal_distribution<double> noise(1.0, scale_noise_std);

    std::vector<Eigen::Vector2d> estimated;
    estimated.push_back(gt[0]);

    for (size_t i = 1; i < gt.size(); i++)
    {
        // GT의 프레임 간 이동 벡터
        Eigen::Vector2d delta = gt[i] - gt[i - 1];
        // 스케일 노이즈 적용: 평균 1.0, 표준편차 scale_noise_std
        double scale = noise(gen);
        // 이전 추정 위치에서 노이즈가 적용된 이동만큼 전진
        //   ★ estimated.back()을 사용하므로 이전 오차가 그대로 전파됨!
        estimated.push_back(estimated.back() + scale * delta);
    }
    return estimated;
}

// ATE(Absolute Trajectory Error) 계산 — SLAM 평가의 표준 지표
//
// ATE = 절대 궤적 오차
//   각 프레임에서 GT와 추정 궤적의 위치 차이를 RMSE로 측정
//
// 공식:
//   ATE = sqrt( (1/N) · Σᵢ ||gt[i] - est[i]||² )
//
// 의미:
//   ATE = 0: 추정이 GT와 완전 일치
//   ATE = 1.0m: 평균적으로 1미터 오차
//
// ★ 논문에서 SLAM 성능 비교 시 가장 많이 사용하는 지표
//   단, 단안 SLAM은 스케일이 다르므로 Sim(3) 정렬 후 ATE를 계산
//   → "Sim(3) aligned ATE"라고 표기
double ScaleBasic::compute_ate(const std::vector<Eigen::Vector2d>& gt,
                               const std::vector<Eigen::Vector2d>& est)
{
    double sum_sq = 0.0;
    for (size_t i = 0; i < gt.size(); i++)
    {
        // 각 프레임의 위치 오차 제곱
        sum_sq += (gt[i] - est[i]).squaredNorm();
    }
    // RMSE = Root Mean Square Error
    return std::sqrt(sum_sq / gt.size());
}

// Sim(3) 정렬 — 스케일이 다른 두 궤적을 맞추는 변환
//
// SE(3) vs Sim(3):
//   SE(3) = [R t; 0 1]   → 6 DoF (회전 3 + 이동 3)
//     → 강체 변환, 크기 보존
//   Sim(3) = [sR t; 0 1]  → 7 DoF (회전 3 + 이동 3 + 스케일 1)
//     → 유사 변환, 크기 변환 가능
//
// 왜 Sim(3)이 필요한가?
//   단안 SLAM의 궤적은 스케일이 임의적
//   GT(미터 단위)와 비교하려면 먼저 스케일을 맞춰야 함
//   → s*, R*, t* = argmin Σ ||gt[i] - (s·R·est[i] + t)||²
//
// 여기서는 간략한 스케일 추정:
//   GT의 총 이동 거리 / 추정의 총 이동 거리 = 스케일 비율
//   → 회전이 없다고 가정한 단순화 버전
//
// ORB-SLAM에서의 활용:
//   Loop Closure 시 현재 맵과 이전 맵의 스케일이 다를 수 있음
//   → Sim(3) 최적화로 스케일 + 포즈를 동시 정렬
//   💡 quiz_medium Q3: Sim(3) vs SE(3) 차이
std::vector<Eigen::Vector2d> ScaleBasic::align_sim3(const std::vector<Eigen::Vector2d>& gt,
                                                    const std::vector<Eigen::Vector2d>& est,
                                                    double& scale)
{
    // 간단한 스케일 추정: GT와 추정의 총 이동 거리 비율
    //   gt_dist = Σ ||gt[i] - gt[i-1]|| = GT 총 경로 길이
    //   est_dist = Σ ||est[i] - est[i-1]|| = 추정 총 경로 길이
    //   scale = gt_dist / est_dist
    //   → scale > 1이면 추정이 GT보다 작게(짧게) 추정한 것
    //   → scale < 1이면 추정이 GT보다 크게(길게) 추정한 것
    double gt_dist = 0, est_dist = 0;
    for (size_t i = 1; i < gt.size(); i++)
    {
        gt_dist += (gt[i] - gt[i - 1]).norm();
        est_dist += (est[i] - est[i - 1]).norm();
    }
    scale = gt_dist / est_dist;

    // 스케일 적용 (중심 기준)
    //   추정 궤적의 중심을 구하고, 중심으로부터의 거리에 scale을 곱함
    //   → 궤적의 형태는 유지하면서 크기만 조정
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

// 궤적 분석 통계 출력 — 드리프트의 영향을 수치로 확인
//
// 출력 지표:
//   최종 오차: 마지막 프레임에서의 GT↔추정 위치 차이
//     → 클수록 전체 궤적이 크게 벗어남
//   ATE(RMSE): 전 구간 평균 위치 오차
//     → SLAM 성능의 대표 지표
//   Loop 오차: 추정 궤적의 시작점↔끝점 거리
//     → 원형 궤적에서 0이어야 하지만, 드리프트로 인해 벌어짐
//     → Loop Closure 없이는 이 오차를 보정할 수 없음
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

// ── 스케일 모호성의 원인 시연 ──
//
// 핀홀 투영의 근본적 한계:
//   λ · [u, v, 1]ᵀ = K · [R|t] · [X, 1]ᵀ
//   → 3D 점 X와 λX (스케일이 다른 점)가 동일한 2D 픽셀에 투영!
//   → 하나의 이미지만으로는 깊이(Z)와 스케일(λ)을 분리할 수 없음
//
// Essential Matrix와 스케일:
//   E = [t]× R 에서 t를 αt로 바꿔도 에피폴라 제약이 성립:
//   x₂ᵀ [αt]× R x₁ = α · (x₂ᵀ [t]× R x₁) = 0
//   → t의 크기(norm)는 결정 불가 → ||t|| = 1로 정규화
//   → "방향만 알 수 있고 거리는 모른다"
//
// 💡 quiz_easy Q1: 스케일 모호성 원인
// 💡 quiz_easy Q2: ||t|| = 1 정규화
void demoScaleAmbiguity()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 스케일 모호성의 원인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "핀홀 투영: λ[u,v,1]^T = K[R|t][X,1]^T" << std::endl;
    std::cout << "  → X와 λX는 같은 픽셀에 투영!\n" << std::endl;

    // 수치 증명 — 같은 방향의 점이 깊이와 무관하게 같은 픽셀에 투영됨을 보임
    //
    // K = [500 0 320; 0 500 240; 0 0 1] (Week 2의 캘리브레이션 결과와 유사)
    // X = (1, 2, 5) 인 점과 λX (λ=2, 5, 10)인 점들을 투영
    //
    // 투영: proj = K · X, 그 후 (proj[0]/proj[2], proj[1]/proj[2])
    //   X = (1, 2, 5) → u = 500·1/5 + 320 = 420, v = 500·2/5 + 240 = 440
    //   2X = (2, 4, 10) → u = 500·2/10 + 320 = 420 (동일!)
    //   → 깊이가 달라도 투영된 픽셀은 같음 = 스케일 모호성
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

// ── 스케일 드리프트 시뮬레이션 ──
//
// 원형 궤적(100프레임, 반지름 5m)에서 다양한 노이즈로 드리프트 관찰.
//
// 노이즈 수준별 예상:
//   σ=0.01 (1%): 소량의 드리프트, Loop 오차 작음
//   σ=0.02 (2%): 눈에 띄는 드리프트, Loop 닫히지 않음
//   σ=0.05 (5%): 심각한 드리프트, 궤적 형태까지 왜곡
//
// ★ 핵심 관찰: 1%의 "작은" 노이즈도 100프레임 누적 시 큰 오차!
//   이것이 단안 VO의 가장 큰 약점이며, IMU 융합이 필요한 이유.
//
// 💡 quiz_easy Q3: 스케일 드리프트의 정의
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

// ── Sim(3) 정렬 효과 시연 ──
//
// 스케일 드리프트가 있는 궤적을 Sim(3) 정렬하여 ATE 개선 관찰.
//
// SE(3) vs Sim(3) 비교:
//   SE(3): T = [R t; 0 1]  → 6 DoF, 스케일 고정
//     → 스케일이 다른 궤적을 정렬할 수 없음
//   Sim(3): S = [sR t; 0 1] → 7 DoF, 스케일 포함
//     → 스케일 차이까지 보정하여 정렬 가능
//
// ★ ORB-SLAM의 Loop Closure에서 Sim(3) 사용:
//   단안 모드에서 Loop 감지 후, 현재 맵과 이전 맵의 스케일 차이를
//   Sim(3) 최적화로 보정 → BlockSolver_7_3 (7 = Sim(3) DoF)
//
// 💡 quiz_easy Q4: 스케일이 결정 가능한 경우
// 💡 quiz_medium Q3: Sim(3) vs SE(3) 차이
void demoSim3Alignment()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Sim(3) 정렬 효과" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "SE(3): [R t; 0 1]  → 6 DoF (스케일 모름)" << std::endl;
    std::cout << "Sim(3): [sR t; 0 1] → 7 DoF (스케일 포함)\n" << std::endl;

    auto gt = ScaleBasic::generate_circular_trajectory(100, 5.0);
    auto est = ScaleBasic::apply_scale_drift(gt, 0.02);

    // 정렬 전 ATE — 스케일 드리프트로 인해 GT와 차이가 큼
    double ate_before = ScaleBasic::compute_ate(gt, est);

    // Sim(3) 정렬 — 스케일 팩터를 추정하고 적용
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

// 전체 데모 — 단안 카메라의 스케일 모호성을 3가지 관점에서 시연
//
// 학습 흐름:
//   ① 스케일 모호성의 수학적 원인: λX → 같은 픽셀 투영
//   ② 스케일 드리프트의 누적 효과: 노이즈별 궤적 비교
//   ③ Sim(3) 정렬을 통한 부분적 보정: ATE 개선 확인
//
// Week 13에서 이어지는 내용:
//   → "스케일을 모르면 어떻게 복구하지?"
//   → Stereo depth, IMU 적분, VIO로 해결하는 방법 학습
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
