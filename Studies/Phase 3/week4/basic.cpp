#include "motion_3d3d.h"
#include <iostream>
#include <iomanip>
#include <limits>

void Motion3D3D::alignSVD(const std::vector<Eigen::Vector3d>& src,
                          const std::vector<Eigen::Vector3d>& tgt,
                          Eigen::Matrix3d& R,
                          Eigen::Vector3d& t) {
    int n = src.size();
    
    // 1. 중심점 계산
    Eigen::Vector3d centroid_src = Eigen::Vector3d::Zero();
    Eigen::Vector3d centroid_tgt = Eigen::Vector3d::Zero();
    for (int i = 0; i < n; i++) {
        centroid_src += src[i];
        centroid_tgt += tgt[i];
    }
    centroid_src /= n;
    centroid_tgt /= n;
    
    // 2. 중심 정렬
    std::vector<Eigen::Vector3d> src_centered(n);
    std::vector<Eigen::Vector3d> tgt_centered(n);
    for (int i = 0; i < n; i++) {
        src_centered[i] = src[i] - centroid_src;
        tgt_centered[i] = tgt[i] - centroid_tgt;
    }
    
    // 3. Covariance 행렬
    Eigen::Matrix3d H = Eigen::Matrix3d::Zero();
    for (int i = 0; i < n; i++) {
        H += src_centered[i] * tgt_centered[i].transpose();
    }
    
    // 4. SVD
    Eigen::JacobiSVD<Eigen::Matrix3d> svd(H, 
        Eigen::ComputeFullU | Eigen::ComputeFullV);
    Eigen::Matrix3d U = svd.matrixU();
    Eigen::Matrix3d V = svd.matrixV();
    
    // 5. R, t 계산
    R = V * U.transpose();
    
    // Reflection 체크
    if (R.determinant() < 0) {
        V.col(2) *= -1;
        R = V * U.transpose();
    }
    
    t = centroid_tgt - R * centroid_src;
}

int Motion3D3D::findNearestNeighbor(const Eigen::Vector3d& point,
                                    const std::vector<Eigen::Vector3d>& cloud) {
    double min_dist = std::numeric_limits<double>::max();
    int min_idx = 0;
    
    for (size_t i = 0; i < cloud.size(); i++) {
        double dist = (point - cloud[i]).squaredNorm();
        if (dist < min_dist) {
            min_dist = dist;
            min_idx = i;
        }
    }
    
    return min_idx;
}

double Motion3D3D::computeError(const std::vector<Eigen::Vector3d>& src,
                                const std::vector<Eigen::Vector3d>& tgt,
                                const Eigen::Matrix3d& R,
                                const Eigen::Vector3d& t) {
    double sum_error = 0.0;
    
    for (size_t i = 0; i < src.size(); i++) {
        Eigen::Vector3d transformed = R * src[i] + t;
        sum_error += (transformed - tgt[i]).squaredNorm();
    }
    
    return std::sqrt(sum_error / src.size());
}

void Motion3D3D::transformPoints(const std::vector<Eigen::Vector3d>& points,
                                 const Eigen::Matrix3d& R,
                                 const Eigen::Vector3d& t,
                                 std::vector<Eigen::Vector3d>& transformed) {
    transformed.resize(points.size());
    for (size_t i = 0; i < points.size(); i++) {
        transformed[i] = R * points[i] + t;
    }
}

double Motion3D3D::ICP(const std::vector<Eigen::Vector3d>& src,
                       const std::vector<Eigen::Vector3d>& tgt,
                       Eigen::Matrix3d& R,
                       Eigen::Vector3d& t,
                       int max_iter,
                       double tolerance) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "ICP 반복" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 초기값
    R = Eigen::Matrix3d::Identity();
    t = Eigen::Vector3d::Zero();
    
    std::vector<Eigen::Vector3d> src_current = src;
    double prev_error = std::numeric_limits<double>::max();
    
    for (int iter = 0; iter < max_iter; iter++) {
        // 1. 대응점 찾기
        std::vector<Eigen::Vector3d> tgt_matched(src.size());
        for (size_t i = 0; i < src_current.size(); i++) {
            int idx = findNearestNeighbor(src_current[i], tgt);
            tgt_matched[i] = tgt[idx];
        }
        
        // 2. 현재 오차
        double error = computeError(src_current, tgt_matched, 
                                    Eigen::Matrix3d::Identity(), 
                                    Eigen::Vector3d::Zero());
        
        std::cout << "Iteration " << iter << ":" << std::endl;
        std::cout << "   Error: " << std::fixed << std::setprecision(2) 
                  << error;
        
        if (iter > 0) {
            double improvement = (prev_error - error) / prev_error * 100;
            std::cout << " (" << std::setprecision(0) 
                      << improvement << "% improvement)";
        }
        std::cout << std::endl;
        
        // 3. 수렴 체크
        if (std::abs(prev_error - error) < tolerance) {
            std::cout << "\n✅ Converged at iteration " << iter << std::endl;
            break;
        }
        
        // 4. 변환 추정 (SVD)
        Eigen::Matrix3d R_iter;
        Eigen::Vector3d t_iter;
        alignSVD(src_current, tgt_matched, R_iter, t_iter);
        
        // 5. 누적 변환
        t = t + R * t_iter;
        R = R * R_iter;
        
        // 6. 점 변환
        transformPoints(src, R, t, src_current);
        
        prev_error = error;
    }
    
    return computeError(src, tgt, R, t);
}

void Motion3D3D::demo() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "3D-3D 모션 추정 (ICP) 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // Ground truth 변환
    Eigen::Matrix3d R_gt;
    double theta = 0.1;  // 약 5.7도
    R_gt << std::cos(theta), -std::sin(theta), 0,
            std::sin(theta),  std::cos(theta), 0,
            0, 0, 1;
    Eigen::Vector3d t_gt(0.5, 0.2, 0.1);
    
    std::cout << "Ground Truth 변환:" << std::endl;
    std::cout << "   Rotation angle: " << std::fixed << std::setprecision(1) 
              << theta * 180 / M_PI << " degrees" << std::endl;
    std::cout << "   Translation: " << t_gt.transpose() << "\n" << std::endl;
    
    // 소스 포인트 생성
    std::vector<Eigen::Vector3d> src;
    for (int i = 0; i < 50; i++) {
        src.push_back(Eigen::Vector3d(
            -2.0 + (rand() % 40) / 10.0,
            -2.0 + (rand() % 40) / 10.0,
            -2.0 + (rand() % 40) / 10.0
        ));
    }
    
    // 타겟 포인트 (변환 + 노이즈)
    std::vector<Eigen::Vector3d> tgt;
    transformPoints(src, R_gt, t_gt, tgt);
    
    // 노이즈 추가
    for (auto& p : tgt) {
        p += Eigen::Vector3d(
            (rand() % 20 - 10) / 100.0,
            (rand() % 20 - 10) / 100.0,
            (rand() % 20 - 10) / 100.0
        );
    }
    
    std::cout << "포인트 클라우드:" << std::endl;
    std::cout << "   Source: " << src.size() << " points" << std::endl;
    std::cout << "   Target: " << tgt.size() << " points" << std::endl;
    
    // ICP 실행
    Eigen::Matrix3d R_est;
    Eigen::Vector3d t_est;
    double final_error = ICP(src, tgt, R_est, t_est, 20, 1e-6);
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "ICP 결과" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "추정 Translation:" << std::endl;
    std::cout << "   " << t_est.transpose() << std::endl;
    
    std::cout << "\nGround Truth Translation:" << std::endl;
    std::cout << "   " << t_gt.transpose() << std::endl;
    
    double t_error = (t_est - t_gt).norm();
    std::cout << "\nTranslation 오차: " << std::setprecision(4) 
              << t_error << std::endl;
    
    std::cout << "최종 RMS 오차: " << std::setprecision(4) 
              << final_error << std::endl;
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 4: 3D-3D 모션 추정 (ICP)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    Motion3D3D::demo();
    
    std::cout << "\n💡 핵심 내용:" << std::endl;
    std::cout << "   - ICP로 Point Cloud 정합" << std::endl;
    std::cout << "   - SVD로 Closed-form 해" << std::endl;
    std::cout << "   - Iterative refinement" << std::endl;
    std::cout << "   - RGB-D, LiDAR SLAM 기초\n" << std::endl;
    
    std::cout << "다음: Week 5 - Mini VO 프로젝트\n" << std::endl;
    
    return 0;
}
