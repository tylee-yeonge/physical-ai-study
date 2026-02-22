/**
 * Phase 3 Week 4 - 3D-3D 모션 추정 (ICP: Iterative Closest Point)
 *
 * 두 3D Point Cloud 간의 변환(R, t)을 추정하는 알고리즘.
 * RGB-D 카메라나 LiDAR에서 직접 3D 점을 얻을 수 있을 때 사용.
 *
 * Week 2~3 vs Week 4:
 *   Week 2 (2D-2D): 2D↔2D 대응 → Essential Matrix → R, t (스케일 모호)
 *   Week 3 (3D-2D): 3D↔2D 대응 → PnP → R, t (절대 스케일)
 *   Week 4 (3D-3D): 3D↔3D 대응 → ICP(SVD) → R, t (절대 스케일)
 *
 * ICP의 핵심 아이디어:
 *   "두 시점의 3D 점들을 가장 잘 겹치게(align) 하는 R, t를 찾는다"
 *   → 목적함수: min Σ ||qᵢ - (R·pᵢ + t)||²
 *   → 대응점이 주어지면 SVD로 closed-form 풀이 가능!
 *
 * 사용 환경:
 *   RGB-D SLAM: RealSense, Azure Kinect 등 깊이 카메라
 *   LiDAR SLAM: Velodyne, Ouster 등 3D LiDAR
 *   3D 스캔 정합: 산업용 3D 스캐너
 */

#include "icp_basic.h"
#include <iostream>
#include <cmath>
#include <random>

// ====================
// ICPBasic 구현
// ====================

// 점 집합의 중심점(centroid) 계산
//
// 중심점 = 모든 점의 평균 위치
//   centroid = (1/N) · Σ pᵢ
//
// ICP에서 중심점이 필요한 이유:
//   SVD 기반 풀이는 "중심화된 좌표"로 작업해야 정확.
//   중심화: pᵢ' = pᵢ - centroid
//   → 중심화를 하면 이동(t) 성분이 제거되고
//     순수한 회전(R) 성분만 남아 SVD로 깔끔하게 풀 수 있음.
//   → 이동은 나중에 t = q_bar - R · p_bar 로 복원.
Eigen::Vector3d ICPBasic::compute_centroid(const std::vector<Eigen::Vector3d>& points)
{
    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    for (const auto& p : points)
    {
        centroid += p;
    }
    return centroid / points.size();
}

// Cross-Covariance 행렬 H 계산
//
// H = Σ pᵢ' · qᵢ'^T  (3×3 행렬)
//   pᵢ': 중심화된 소스 점 (pᵢ - p_bar)
//   qᵢ': 중심화된 타겟 점 (qᵢ - q_bar)
//
// H의 의미:
//   소스와 타겟 간의 "교차 공분산(cross-covariance)"
//   → 두 점 집합이 어떤 방향으로 얼마나 연관되어 있는지를 나타냄
//   → H를 SVD 분해하면 최적의 회전 R을 구할 수 있음
//
// ★ 왜 H로 R을 구할 수 있는가?
//   목적함수: min Σ ||qᵢ' - R·pᵢ'||²
//   전개하면: max Σ qᵢ'^T · R · pᵢ' = max trace(R · Σ pᵢ'·qᵢ'^T) = max trace(R · H)
//   → trace(R · H)를 최대화하는 R을 찾는 문제
//   → SVD(H) = UΣVᵀ이면, R = VUᵀ가 최적해 (증명: 특이값 분해의 성질)
Eigen::Matrix3d ICPBasic::compute_h(const std::vector<Eigen::Vector3d>& src,
                                    const std::vector<Eigen::Vector3d>& tgt)
{
    Eigen::Matrix3d H = Eigen::Matrix3d::Zero();
    for (size_t i = 0; i < src.size(); i++)
    {
        // 외적(outer product): pᵢ' · qᵢ'^T → 3×3 행렬
        // 이것들의 합 = cross-covariance 행렬 H
        H += src[i] * tgt[i].transpose();
    }
    return H;
}

// SVD 기반 R, t 추정 — ICP의 핵심 풀이
//
// 알고리즘 (5단계):
//   ① 중심점 계산: p_bar, q_bar
//   ② 중심화: pᵢ' = pᵢ - p_bar,  qᵢ' = qᵢ - q_bar
//   ③ Cross-covariance 행렬: H = Σ pᵢ' · qᵢ'^T
//   ④ SVD 분해: H = U · Σ · Vᵀ
//   ⑤ 최적 R, t 계산: R = V · Uᵀ,  t = q_bar - R · p_bar
//
// 수학적 배경:
//   목적: min Σ ||qᵢ - (R·pᵢ + t)||²
//   중심화 후: min Σ ||qᵢ' - R·pᵢ'||²  (t 성분 분리)
//   전개: Σ ||qᵢ'||² - 2·trace(R·H) + Σ ||pᵢ'||²
//   → 1항, 3항은 상수 → trace(R·H)를 최대화하는 R을 찾으면 됨
//   SVD(H) = UΣVᵀ이면, trace(R·H) ≤ Σσᵢ (특이값 합)
//   등호 조건: R = VUᵀ
//
// ★ Closed-form 해: 반복 없이 한 번에 정확한 R, t를 구할 수 있음!
//   단, 대응점이 "이미 주어져 있어야" 함.
//   대응점을 모르면 → 반복 ICP로 대응 탐색과 R,t 추정을 교대 수행.
double ICPBasic::svd_decompose(const std::vector<Eigen::Vector3d>& src,
                               const std::vector<Eigen::Vector3d>& tgt,
                               Eigen::Matrix3d& R, Eigen::Vector3d& t)
{
    // ① 중심점 계산
    Eigen::Vector3d src_mean = compute_centroid(src);
    Eigen::Vector3d tgt_mean = compute_centroid(tgt);

    // ② 중심화 — 각 점에서 중심점을 빼서 원점을 중심으로 이동
    //   이렇게 하면 이동(t) 성분이 분리되어 회전(R)만 따로 추정 가능
    std::vector<Eigen::Vector3d> src_c(src.size()), tgt_c(tgt.size());
    for (size_t i = 0; i < src.size(); i++)
    {
        src_c[i] = src[i] - src_mean;
        tgt_c[i] = tgt[i] - tgt_mean;
    }

    // ③ Cross-covariance 행렬 H = Σ pᵢ' · qᵢ'^T
    Eigen::Matrix3d H = compute_h(src_c, tgt_c);

    // ④ SVD 분해: H = U · Σ · Vᵀ
    //   JacobiSVD: 3×3 작은 행렬에 적합한 SVD 알고리즘
    //   ComputeFullU, ComputeFullV: U, V 행렬을 완전히 계산
    //
    //   SVD의 기하학적 의미:
    //     U: H의 "출력 방향" (타겟 공간의 주축)
    //     V: H의 "입력 방향" (소스 공간의 주축)
    //     Σ: 각 방향의 "크기" (대응 강도)
    //   → R = VUᵀ는 "소스 주축을 타겟 주축에 맞추는 회전"
    Eigen::JacobiSVD<Eigen::Matrix3d> svd(H, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Eigen::Matrix3d V = svd.matrixV();
    Eigen::Matrix3d U = svd.matrixU();

    // ⑤ 최적 R, t 계산
    R = V * U.transpose();

    // Reflection 체크 — det(R)이 -1이면 반사(reflection)가 포함됨
    //   정상적인 회전 행렬: det(R) = +1
    //   반사 포함: det(R) = -1 → 물리적으로 불가능한 변환
    //   해결: V의 마지막 열의 부호를 반전시킨 후 R 재계산
    //   이 현상이 발생하는 경우: 점들이 평면 위에 있을 때 (퇴화 상황)
    if (R.determinant() < 0)
    {
        V.col(2) *= -1;
        R = V * U.transpose();
    }

    // 이동 벡터: t = q_bar - R · p_bar
    //   직관: "소스를 R로 회전한 뒤, 타겟 중심으로 이동"
    t = tgt_mean - R * src_mean;

    // 정합 오차 반환 — RMSE로 품질 평가
    return evaluate_alignment(src, tgt, R, t);
}

// 반복 ICP (Iterative Closest Point) — 대응을 모를 때의 풀이
//
// 표준 ICP는 "대응점을 모르는" 상황에서 동작:
//   가장 가까운 점(nearest neighbor)을 대응으로 가정하고,
//   그 대응으로 R, t를 추정하고,
//   변환된 점으로 다시 대응을 찾고,
//   수렴할 때까지 반복.
//
// 반복 단계:
//   1. 대응 찾기: 각 소스 점에서 가장 가까운 타겟 점을 매칭
//      → 이 구현에서는 교육용으로 대응이 이미 주어진 단순 버전
//   2. 변환 계산: SVD로 현재 대응에 대한 R_step, t_step 계산
//   3. 점 변환: p ← R_step · p + t_step (소스 점을 타겟 쪽으로 이동)
//   4. 수렴 체크: 오차 변화가 tol 미만이면 중단
//
// 수렴 특성:
//   Point-to-Point ICP: 보통 10~20회 반복에 수렴
//   Point-to-Plane ICP: 보통 3~5회 반복에 수렴 (훨씬 빠름)
//   초기값이 나쁘면: 지역 최솟값(local minimum)에 빠질 수 있음!
//     → 초기 정합이 대략적으로라도 맞아야 함 (IMU, wheel odometry 등 활용)
//
// ★ 누적 변환:
//   매 반복마다 R_step, t_step을 구하는데,
//   최종 R = R_n · ... · R_2 · R_1 (순차 곱)
//   최종 t = 각 단계의 누적 결과
//   result.R, result.t에 이것을 누적 저장.
ICPResult ICPBasic::iterative_icp(const std::vector<Eigen::Vector3d>& src,
                                  const std::vector<Eigen::Vector3d>& tgt,
                                  int max_iter, double tol)
{
    ICPResult result;
    result.R = Eigen::Matrix3d::Identity();
    result.t = Eigen::Vector3d::Zero();
    result.converged = false;

    // 소스 점의 작업용 복사본 — 반복마다 변환 적용
    std::vector<Eigen::Vector3d> current = src;
    double prev_error = std::numeric_limits<double>::max();

    for (int iter = 0; iter < max_iter; iter++)
    {
        // 현재 대응으로 최적 R_step, t_step 계산 (SVD closed-form)
        Eigen::Matrix3d R_step;
        Eigen::Vector3d t_step;
        double error = svd_decompose(current, tgt, R_step, t_step);

        // 소스 점을 R_step, t_step으로 변환 → 타겟에 더 가까워짐
        for (auto& p : current)
        {
            p = R_step * p + t_step;
        }

        // 누적 변환 업데이트
        //   R_total = R_step · R_prev
        //   t_total = R_step · t_prev + t_step
        //   → 이전 변환에 새 변환을 왼쪽에서 곱하는 형태
        result.R = R_step * result.R;
        result.t = R_step * result.t + t_step;
        result.error = error;
        result.iterations = iter + 1;

        std::cout << "  Iteration " << iter << ": error = " << error << std::endl;

        // 수렴 판정: 이전 오차와의 차이가 tol 미만이면 수렴
        //   오차가 더 이상 줄지 않으면 → 최적해에 도달했다고 판단
        if (std::abs(prev_error - error) < tol)
        {
            result.converged = true;
            break;
        }
        prev_error = error;
    }

    return result;
}

// 정합 품질 평가 — RMSE (Root Mean Square Error)
//
// RMSE = sqrt( (1/N) · Σ ||qᵢ - (R·pᵢ + t)||² )
//
// 의미: "변환 후 소스와 타겟 사이의 평균 거리"
//   0에 가까울수록 정합이 정확
//   노이즈가 없으면 대응이 정확한 경우 RMSE ≈ 0
//   노이즈 σ가 있으면 RMSE ≈ σ 수준
//
// 일반적 기준 (단위: meter):
//   < 0.01m: 매우 정확 (1cm 이내)
//   < 0.05m: 양호
//   > 0.10m: 정합에 문제
double ICPBasic::evaluate_alignment(const std::vector<Eigen::Vector3d>& src,
                                    const std::vector<Eigen::Vector3d>& tgt,
                                    const Eigen::Matrix3d& R, const Eigen::Vector3d& t)
{
    double sum_sq = 0.0;
    for (size_t i = 0; i < src.size(); i++)
    {
        // 변환 적용: pᵢ → R · pᵢ + t
        Eigen::Vector3d transformed = R * src[i] + t;
        // 타겟과의 거리 제곱: ||qᵢ - (R·pᵢ + t)||²
        sum_sq += (tgt[i] - transformed).squaredNorm();
    }
    return std::sqrt(sum_sq / src.size());  // RMSE
}

// ====================
// 교육 블록
// ====================

// SVD 기반 정합 데모 — 알려진 변환을 SVD로 복원하는 과정
//
// 시뮬레이션 전략:
//   ① 소스 점 생성 (단위 큐브의 8개 꼭짓점)
//   ② 알려진 R_true(30도 Z축 회전), t_true 적용 → 타겟 생성
//   ③ SVD로 R, t 복원
//   ④ 복원된 R, t와 정답 비교 → 오차가 0에 가까우면 정확
//
// ★ 노이즈가 없으면 SVD는 정확한 R, t를 복원함 (오차 ≈ 0)
//   이것이 "closed-form" 해법의 강점: 반복 없이 한번에 정답 도달
void demoSVDAlignment()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 SVD 기반 R, t 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // SVD 풀이의 5단계를 명시적으로 보여줌
    //   이 5단계는 svd_decompose() 내부에서 자동으로 수행되며,
    //   여기서는 교육 목적으로 각 단계를 설명.
    std::cout << "원리: 대응점이 주어지면 Closed-form으로 풀 수 있다!\n" << std::endl;
    std::cout << "  1. 중심점 계산: p_bar, q_bar" << std::endl;
    std::cout << "  2. 중심화: p'_i = p_i - p_bar" << std::endl;
    std::cout << "  3. H = Σ p'_i * q'_i^T" << std::endl;
    std::cout << "  4. SVD(H) = U * Σ * V^T" << std::endl;
    std::cout << "  5. R = V * U^T,  t = q_bar - R * p_bar\n" << std::endl;

    // 정답 변환: Z축 기준 30도 회전 + 이동
    //   30도 ≈ π/6 ≈ 0.524 rad
    //   회전 행렬 (Z축):
    //     [cos(θ)  -sin(θ)  0]
    //     [sin(θ)   cos(θ)  0]
    //     [  0        0     1]
    Eigen::Matrix3d R_true;
    double angle = M_PI / 6;  // 30도
    R_true << std::cos(angle), -std::sin(angle), 0,
              std::sin(angle),  std::cos(angle), 0,
              0,                0,               1;
    Eigen::Vector3d t_true(1.0, 0.5, -0.3);

    // 소스 점: 단위 큐브의 8개 꼭짓점 (0,0,0) ~ (1,1,1)
    //   간단한 기하 구조로 SVD 검증에 적합
    //   실제 ICP에서는 수천~수만 개의 점을 사용
    std::vector<Eigen::Vector3d> src = {
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
        {1, 1, 0}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}
    };

    // 타겟 = R_true · src + t_true
    //   알려진 변환을 적용하여 "정답" 타겟 점 생성
    std::vector<Eigen::Vector3d> tgt(src.size());
    for (size_t i = 0; i < src.size(); i++)
    {
        tgt[i] = R_true * src[i] + t_true;
    }

    // SVD로 R, t 복원 — 정답과 비교
    Eigen::Matrix3d R_est;
    Eigen::Vector3d t_est;
    double error = ICPBasic::svd_decompose(src, tgt, R_est, t_est);

    std::cout << "정답 R (30도 회전):" << std::endl;
    std::cout << R_true << "\n" << std::endl;
    std::cout << "추정 R:" << std::endl;
    std::cout << R_est << "\n" << std::endl;

    // 오차 비교
    //   R 오차: ||R_true - R_est|| (Frobenius norm)
    //     0에 가까우면 회전 추정 정확
    //   t 오차: ||t_true - t_est|| (L2 norm)
    //     0에 가까우면 이동 추정 정확
    //   RMSE: 변환 후 점들의 평균 정합 오차
    //
    //   노이즈가 없으면 세 값 모두 ≈ 0 (수치 오차 수준, ~1e-15)
    double R_error = (R_true - R_est).norm();
    double t_error = (t_true - t_est).norm();
    std::cout << "R 오차: " << R_error << std::endl;
    std::cout << "t 오차: " << t_error << std::endl;
    std::cout << "RMSE:   " << error << std::endl;
}

// 반복 ICP 수렴 과정 데모
//
// 노이즈가 있는 데이터에서 반복 ICP가 어떻게 수렴하는지 관찰.
// 각 반복마다 오차가 감소하는 과정을 출력.
//
// 이 구현은 "대응이 주어진" 단순 버전이므로 1~2회에 수렴.
// 실제 ICP는 "최근접 점 탐색" 단계가 추가되어 더 많은 반복 필요.
void demoIterativeICP()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 반복 ICP 수렴 과정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ICP의 4단계 반복 루프
    //   1. 대응 찾기: 소스 점 pᵢ에 대해 가장 가까운 타겟 점 qⱼ 찾기
    //      → 보통 KD-tree로 O(N log N) (이 구현에서는 인덱스 순서 사용)
    //   2. 변환 계산: 현재 대응으로 SVD → R_step, t_step
    //   3. 점 변환: 소스 점에 R_step, t_step 적용 → 타겟에 가까워짐
    //   4. 수렴 체크: 오차 변화 < tol이면 종료
    std::cout << "ICP 4단계 반복:" << std::endl;
    std::cout << "  1. 대응 찾기 (Correspondence)" << std::endl;
    std::cout << "  2. 변환 계산 (SVD)" << std::endl;
    std::cout << "  3. 점 변환 (R*p + t)" << std::endl;
    std::cout << "  4. 수렴 체크\n" << std::endl;

    // 정답 변환: Z축 15도 회전 + 작은 이동
    Eigen::Matrix3d R_true;
    double angle = M_PI / 12;  // 15도
    R_true << std::cos(angle), -std::sin(angle), 0,
              std::sin(angle),  std::cos(angle), 0,
              0,                0,               1;
    Eigen::Vector3d t_true(0.3, 0.2, 0.1);

    // 노이즈 생성 — σ=0.01 (1cm 수준의 센서 노이즈)
    //   실제 RGB-D 카메라: σ ≈ 1~5mm (가까운 거리)
    //   실제 LiDAR: σ ≈ 1~3cm
    std::mt19937 gen(42);
    std::normal_distribution<> noise(0, 0.01);

    // 소스 점: 5×4 = 20개 점 (격자 형태)
    std::vector<Eigen::Vector3d> src;
    for (int i = 0; i < 20; i++)
    {
        src.push_back({(double)(i % 5), (double)(i / 5), noise(gen)});
    }

    // 타겟 = R_true · src + t_true + noise
    //   노이즈를 추가하여 실제 센서 측정 오차를 모사
    std::vector<Eigen::Vector3d> tgt(src.size());
    for (size_t i = 0; i < src.size(); i++)
    {
        tgt[i] = R_true * src[i] + t_true;
        tgt[i] += Eigen::Vector3d(noise(gen), noise(gen), noise(gen));
    }

    // 반복 ICP 실행 — 수렴 과정 관찰
    //   max_iter=10: 최대 10회 반복
    //   tol=1e-8: 오차 변화가 이 값 미만이면 수렴
    std::cout << "수렴 과정 (노이즈 σ=0.01):" << std::endl;
    ICPResult result = ICPBasic::iterative_icp(src, tgt, 10, 1e-8);

    std::cout << "\n결과:" << std::endl;
    std::cout << "  수렴: " << (result.converged ? "예" : "아니오") << std::endl;
    std::cout << "  반복: " << result.iterations << "회" << std::endl;
    std::cout << "  RMSE: " << result.error << std::endl;
    // t 오차 — 노이즈 때문에 0은 아니지만, σ 수준으로 작아야 함
    std::cout << "  t 오차: " << (t_true - result.t).norm() << std::endl;
}

// Point-to-Point vs Point-to-Plane ICP 비교
//
// ICP의 두 가지 변형:
//
//   Point-to-Point (이 구현):
//     목적함수: min Σ ||qᵢ - (R·pᵢ + t)||²
//     = "대응점 간의 유클리드 거리 제곱 합 최소화"
//     장점: SVD로 closed-form 풀이 → 구현 간단
//     단점: 수렴 느림 (특히 평면이 많은 환경)
//       이유: 평면 위의 점은 법선 방향으로만 정합하면 되는데,
//             접선 방향의 작은 차이도 오차에 포함시킴
//
//   Point-to-Plane:
//     목적함수: min Σ (nᵢ · (R·pᵢ + t - qᵢ))²
//     = "법선 방향의 거리만 최소화" (접선 방향은 무시)
//     nᵢ: 타겟 점 qᵢ에서의 표면 법선(surface normal)
//     장점: 수렴 매우 빠름 (3~5회, P2P의 1/3~1/5)
//       이유: 평면 위에서 접선 방향의 슬라이딩을 허용
//     단점: 법선 계산 필요, 비선형 → closed-form 없음
//       → Gauss-Newton이나 Levenberg-Marquardt로 반복 풀이
//
//   ★ 실제 LiDAR SLAM에서는 Point-to-Plane이 표준!
//     실내/도시 환경은 벽, 바닥, 천장 등 평면이 풍부하여
//     Point-to-Plane의 빠른 수렴이 큰 이점.
//     LOAM, LeGO-LOAM, LIO-SAM 등이 이 방식 사용.
void demoP2PvsP2Plane()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Point-to-Point vs Point-to-Plane" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Point-to-Point:" << std::endl;
    std::cout << "  min Σ ||q_i - (R*p_i + t)||²" << std::endl;
    std::cout << "  ✅ SVD closed-form" << std::endl;
    std::cout << "  ❌ 수렴 느림 (10-20회)\n" << std::endl;

    std::cout << "Point-to-Plane:" << std::endl;
    std::cout << "  min Σ (n_i · (R*p_i + t - q_i))²" << std::endl;
    std::cout << "  ✅ 수렴 빠름 (3-5회)" << std::endl;
    std::cout << "  ❌ 법선 필요, 비선형 최적화\n" << std::endl;

    std::cout << "💡 LiDAR SLAM에서는 Point-to-Plane이 표준" << std::endl;
    std::cout << "   (평면이 풍부한 실내/도시 환경)" << std::endl;
}

// ====================
// Main
// ====================

// 메인 함수 — Week 4 데모 실행
//
// 학습 흐름:
//   1. demoSVDAlignment(): SVD closed-form 풀이 검증 (노이즈 없음)
//   2. demoIterativeICP(): 반복 ICP의 수렴 과정 관찰 (노이즈 있음)
//   3. demoP2PvsP2Plane(): 두 ICP 변형의 이론적 비교
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 4: 3D-3D ICP" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 교육 블록
    demoSVDAlignment();
    demoIterativeICP();
    demoP2PvsP2Plane();

    // 다음 단계 안내
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📌 다음 단계:" << std::endl;
    std::cout << "  1. my_basic.cpp → 직접 구현 (5 Step)" << std::endl;
    std::cout << "  2. Week 5       → Mini VO 파이프라인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
