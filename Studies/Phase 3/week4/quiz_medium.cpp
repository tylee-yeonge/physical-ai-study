/**
 * Phase 3 Week 4 - ICP 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>

void problem1_svd_alignment() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: SVD 정합 과정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "SVD 기반 정합 단계:\n" << std::endl;
    std::cout << "   1. 중심점 계산" << std::endl;
    std::cout << "      p̄ = Σp_i/n, q̄ = Σq_i/n\n" << std::endl;
    std::cout << "   2. 중심 정렬" << std::endl;
    std::cout << "      p'_i = p_i - p̄\n" << std::endl;
    std::cout << "   3. Covariance 행렬" << std::endl;
    std::cout << "      H = Σ p'_i * q'_i^T\n" << std::endl;
    std::cout << "   4. SVD 분해" << std::endl;
    std::cout << "      H = U * Σ * V^T\n" << std::endl;
    std::cout << "   5. R, t 계산" << std::endl;
    std::cout << "      R = V * U^T" << std::endl;
    std::cout << "      t = q̄ - R * p̄" << std::endl;
}

void problem2_kdtree_complexity() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: K-D Tree 복잡도" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "💡 답:" << std::endl;
    std::cout << "   구축: O(n log n)" << std::endl;
    std::cout << "   검색: O(log n)\n" << std::endl;
    
    std::cout << "Naive 비교:" << std::endl;
    std::cout << "   ICP 1회: O(n²)" << std::endl;
    std::cout << "   K-D Tree: O(n log n)\n" << std::endl;
    
    std::cout << "→ 대규모 Point Cloud에 필수!" << std::endl;
}

void problem3_vio_vs_lidar() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: VIO에서 ICP를 안 쓰는 이유" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "💡 답:" << std::endl;
    std::cout << "VIO (Visual-Inertial Odometry):" << std::endl;
    std::cout << "   - Monocular 카메라" << std::endl;
    std::cout << "   - 3D 정보 부족" << std::endl;
    std::cout << "   - 2D-2D, 3D-2D 적합\n" << std::endl;
    
    std::cout << "ICP가 필요한 경우:" << std::endl;
    std::cout << "   - RGB-D SLAM" << std::endl;
    std::cout << "   - LiDAR-Inertial Odometry" << std::endl;
    std::cout << "   - AMR의 LiDAR SLAM" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 4 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    problem1_svd_alignment()
    problem2_kdtree_complexity()
    problem3_vio_vs_lidar()
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
