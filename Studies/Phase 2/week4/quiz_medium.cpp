/**
 * Phase 2 Week 4 - 특징점 매칭 중급 퀴즈
 * 
 * 실전 응용: 매칭 성능 개선, Essential Matrix 추정
 */

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <chrono>

/**
 * 문제 1: 최적의 Ratio 임계값 찾기
 * 
 * 다양한 ratio threshold를 시도하여
 * precision-recall 트레이드오프를 관찰하세요.
 */
void problem1_optimal_ratio() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 최적의 Ratio Threshold" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // TODO: 여러 ratio 값 테스트
    std::vector<float> ratios = {0.5, 0.6, 0.7, 0.8, 0.9};
    
    std::cout << "Ratio  |  매칭 개수  |  Inlier 비율" << std::endl;
    std::cout << "-------+-------------+-------------" << std::endl;
    
    for (float ratio : ratios) {
        // TODO: ratio test 수행
        // int matches_count = ...
        // double inlier_ratio = ...
        
        std::cout << " " << ratio << "  |     ???     |    ???" << std::endl;
    }
    
    std::cout << "\n💡 관찰:" << std::endl;
    std::cout << "   - Ratio ↓: 매칭 개수 ↓, 품질 ↑ (precision)" << std::endl;
    std::cout << "   - Ratio ↑: 매칭 개수 ↑, 품질 ↓ (recall)" << std::endl;
    std::cout << "   - 0.7은 좋은 균형점 (Lowe 논문)" << std::endl;
}

/**
 * 문제 2: Essential Matrix 추정
 * 
 * 매칭된 점들로부터 Essential Matrix를 구하고
 * 카메라 포즈를 복원하세요. (SLAM의 핵심!)
 */
void problem2_essential_matrix() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Essential Matrix 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 카메라 행렬 (캘리브레이션 결과)
    cv::Mat K = (cv::Mat_<double>(3, 3) <<
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
    
    // TODO: 매칭된 점들 (실제로는 특징점 매칭 결과)
    std::vector<cv::Point2f> points1, points2;
    // 시뮬레이션 데이터 생성...
    
    // TODO: Essential Matrix 추정
    /*
    cv::Mat E = cv::findEssentialMat(
        points1, points2, K,
        cv::RANSAC, 0.999, 1.0
    );
    */
    
    std::cout << "Essential Matrix E:" << std::endl;
    // std::cout << E << "\n" << std::endl;
    
    // TODO: R, t 복원
    /*
    cv::Mat R, t;
    cv::recoverPose(E, points1, points2, K, R, t);
    */
    
   std::cout << "💡 SLAM에서의 의미:" << std::endl;
    std::cout << "   - E를 분해 → R (회전), t (이동)" << std::endl;
    std::cout << "   - 두 프레임 간 상대 포즈!" << std::endl;
    std::cout << "   - Visual Odometry의 핵심" << std::endl;
}

/**
 * 문제 3: 매칭 성능 벤치마크
 * 
 * BF vs FLANN의 속도와 정확도를 비교하세요.
 */
void problem3_matching_benchmark() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 매칭 성능 벤치마크" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // TODO: 대량의 특징점으로 테스트
    int num_features = 2000;
    
    std::cout << "특징점 개수: " << num_features << "개\n" << std::endl;
    
    // TODO: BF 매칭 시간 측정
    // auto start_bf = ...
    // auto end_bf = ...
    
    // TODO: FLANN 매칭 시간 측정
    // auto start_flann = ...
    // auto end_flann = ...
    
    std::cout << "매칭 알고리즘  |  시간 (ms)  |  속도비" << std::endl;
    std::cout << "---------------+-------------+---------" << std::endl;
    std::cout << "Brute-Force    |     ???     |   1.0x" << std::endl;
    std::cout << "FLANN          |     ???     |   ???x" << std::endl;
    
    std::cout << "\n💡 결론:" << std::endl;
    std::cout << "   - BF: 정확하지만 느림 O(N²)" << std::endl;
    std::cout << "   - FLANN: 빠른 근사 O(N log N)" << std::endl;
    std::cout << "   - 실시간 SLAM: FLANN 또는 NN 기반" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 4 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    problem1_optimal_ratio()
    problem2_essential_matrix()
    problem3_matching_benchmark()
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
