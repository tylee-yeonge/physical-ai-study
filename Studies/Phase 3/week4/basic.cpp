#include "icp_basic.h"
#include <iostream>
#include <cmath>
#include <random>

// ====================
// ICPBasic 구현
// ====================

Eigen::Vector3d ICPBasic::compute_centroid(const std::vector<Eigen::Vector3d>& points)
{
    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    for (const auto& p : points)
    {
        centroid += p;
    }
    return centroid / points.size();
}

Eigen::Matrix3d ICPBasic::compute_h(const std::vector<Eigen::Vector3d>& src,
                                    const std::vector<Eigen::Vector3d>& tgt)
{
    Eigen::Matrix3d H = Eigen::Matrix3d::Zero();
    for (size_t i = 0; i < src.size(); i++)
    {
        H += src[i] * tgt[i].transpose();
    }
    return H;
}

double ICPBasic::svd_decompose(const std::vector<Eigen::Vector3d>& src,
                               const std::vector<Eigen::Vector3d>& tgt,
                               Eigen::Matrix3d& R, Eigen::Vector3d& t)
{
    // 1. 중심점 계산
    Eigen::Vector3d src_mean = compute_centroid(src);
    Eigen::Vector3d tgt_mean = compute_centroid(tgt);

    // 2. 중심화
    std::vector<Eigen::Vector3d> src_c(src.size()), tgt_c(tgt.size());
    for (size_t i = 0; i < src.size(); i++)
    {
        src_c[i] = src[i] - src_mean;
        tgt_c[i] = tgt[i] - tgt_mean;
    }

    // 3. Covariance 행렬
    Eigen::Matrix3d H = compute_h(src_c, tgt_c);

    // 4. SVD 분해
    Eigen::JacobiSVD<Eigen::Matrix3d> svd(H, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Eigen::Matrix3d V = svd.matrixV();
    Eigen::Matrix3d U = svd.matrixU();

    // 5. R, t 계산
    R = V * U.transpose();

    // Reflection 체크 (det(R) = -1이면 보정)
    if (R.determinant() < 0)
    {
        V.col(2) *= -1;
        R = V * U.transpose();
    }

    t = tgt_mean - R * src_mean;

    // 오차 계산
    return evaluate_alignment(src, tgt, R, t);
}

ICPResult ICPBasic::iterative_icp(const std::vector<Eigen::Vector3d>& src,
                                  const std::vector<Eigen::Vector3d>& tgt,
                                  int max_iter, double tol)
{
    ICPResult result;
    result.R = Eigen::Matrix3d::Identity();
    result.t = Eigen::Vector3d::Zero();
    result.converged = false;

    // 작업용 복사본
    std::vector<Eigen::Vector3d> current = src;
    double prev_error = std::numeric_limits<double>::max();

    for (int iter = 0; iter < max_iter; iter++)
    {
        // SVD로 R, t 계산 (대응이 이미 주어진 단순 버전)
        Eigen::Matrix3d R_step;
        Eigen::Vector3d t_step;
        double error = svd_decompose(current, tgt, R_step, t_step);

        // 점 변환
        for (auto& p : current)
        {
            p = R_step * p + t_step;
        }

        // 누적 변환
        result.R = R_step * result.R;
        result.t = R_step * result.t + t_step;
        result.error = error;
        result.iterations = iter + 1;

        std::cout << "  Iteration " << iter << ": error = " << error << std::endl;

        // 수렴 체크
        if (std::abs(prev_error - error) < tol)
        {
            result.converged = true;
            break;
        }
        prev_error = error;
    }

    return result;
}

double ICPBasic::evaluate_alignment(const std::vector<Eigen::Vector3d>& src,
                                    const std::vector<Eigen::Vector3d>& tgt,
                                    const Eigen::Matrix3d& R, const Eigen::Vector3d& t)
{
    double sum_sq = 0.0;
    for (size_t i = 0; i < src.size(); i++)
    {
        Eigen::Vector3d transformed = R * src[i] + t;
        sum_sq += (tgt[i] - transformed).squaredNorm();
    }
    return std::sqrt(sum_sq / src.size());  // RMSE
}

// ====================
// 교육 블록
// ====================

void demoSVDAlignment()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 SVD 기반 R, t 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "원리: 대응점이 주어지면 Closed-form으로 풀 수 있다!\n" << std::endl;
    std::cout << "  1. 중심점 계산: p_bar, q_bar" << std::endl;
    std::cout << "  2. 중심화: p'_i = p_i - p_bar" << std::endl;
    std::cout << "  3. H = Σ p'_i * q'_i^T" << std::endl;
    std::cout << "  4. SVD(H) = U * Σ * V^T" << std::endl;
    std::cout << "  5. R = V * U^T,  t = q_bar - R * p_bar\n" << std::endl;

    // 테스트 데이터: 알려진 변환 적용 후 복원
    Eigen::Matrix3d R_true;
    double angle = M_PI / 6;  // 30도
    R_true << std::cos(angle), -std::sin(angle), 0,
              std::sin(angle),  std::cos(angle), 0,
              0,                0,               1;
    Eigen::Vector3d t_true(1.0, 0.5, -0.3);

    // 소스 점 생성
    std::vector<Eigen::Vector3d> src = {
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
        {1, 1, 0}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}
    };

    // 타겟 = R * src + t
    std::vector<Eigen::Vector3d> tgt(src.size());
    for (size_t i = 0; i < src.size(); i++)
    {
        tgt[i] = R_true * src[i] + t_true;
    }

    // SVD로 복원
    Eigen::Matrix3d R_est;
    Eigen::Vector3d t_est;
    double error = ICPBasic::svd_decompose(src, tgt, R_est, t_est);

    std::cout << "정답 R (30도 회전):" << std::endl;
    std::cout << R_true << "\n" << std::endl;
    std::cout << "추정 R:" << std::endl;
    std::cout << R_est << "\n" << std::endl;

    double R_error = (R_true - R_est).norm();
    double t_error = (t_true - t_est).norm();
    std::cout << "R 오차: " << R_error << std::endl;
    std::cout << "t 오차: " << t_error << std::endl;
    std::cout << "RMSE:   " << error << std::endl;
}

void demoIterativeICP()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 반복 ICP 수렴 과정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "ICP 4단계 반복:" << std::endl;
    std::cout << "  1. 대응 찾기 (Correspondence)" << std::endl;
    std::cout << "  2. 변환 계산 (SVD)" << std::endl;
    std::cout << "  3. 점 변환 (R*p + t)" << std::endl;
    std::cout << "  4. 수렴 체크\n" << std::endl;

    // 노이즈 추가한 데이터
    Eigen::Matrix3d R_true;
    double angle = M_PI / 12;  // 15도
    R_true << std::cos(angle), -std::sin(angle), 0,
              std::sin(angle),  std::cos(angle), 0,
              0,                0,               1;
    Eigen::Vector3d t_true(0.3, 0.2, 0.1);

    std::mt19937 gen(42);
    std::normal_distribution<> noise(0, 0.01);

    std::vector<Eigen::Vector3d> src;
    for (int i = 0; i < 20; i++)
    {
        src.push_back({(double)(i % 5), (double)(i / 5), noise(gen)});
    }

    std::vector<Eigen::Vector3d> tgt(src.size());
    for (size_t i = 0; i < src.size(); i++)
    {
        tgt[i] = R_true * src[i] + t_true;
        tgt[i] += Eigen::Vector3d(noise(gen), noise(gen), noise(gen));
    }

    std::cout << "수렴 과정 (노이즈 σ=0.01):" << std::endl;
    ICPResult result = ICPBasic::iterative_icp(src, tgt, 10, 1e-8);

    std::cout << "\n결과:" << std::endl;
    std::cout << "  수렴: " << (result.converged ? "예" : "아니오") << std::endl;
    std::cout << "  반복: " << result.iterations << "회" << std::endl;
    std::cout << "  RMSE: " << result.error << std::endl;
    std::cout << "  t 오차: " << (t_true - result.t).norm() << std::endl;
}

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
