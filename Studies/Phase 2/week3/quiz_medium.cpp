/**
 * Phase 2 Week 3 - 특징점 검출 중급 퀴즈
 * 
 * 실전 응용 문제: 특징점 분포 개선, 스케일 불변성 등
 */

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <chrono>

/**
 * 문제 1: 균등한 특징점 분포 구현
 * 
 * 이미지를 그리드로 나누고 각 셀에서 일정 개수의 특징점을 검출하여
 * 균등한 분포를 만드세요. (VINS-Fusion 방식)
 */
void problem1_uniform_distribution() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 균등한 특징점 분포 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC1);
    // 테스트 패턴: 왼쪽에 많은 특징
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 8; j++) {  // 왼쪽 절반만
            if ((i+j) % 2 == 0) {
                cv::rectangle(image, cv::Point(j*50, i*50),
                            cv::Point((j+1)*50, (i+1)*50),
                            cv::Scalar(255), -1);
            }
        }
    }
    
    // TODO: 일반 FAST 검출
    std::vector<cv::KeyPoint> kp_normal;
    // cv::FAST(image, kp_normal, 20, true);
    
    // TODO: 그리드 기반 균등 분포
    std::vector<cv::KeyPoint> kp_uniform;
    int grid_rows = 6, grid_cols = 8;
    int max_per_cell = 10;
    
    // 힌트: 이미지를 grid로 나누고 각 셀에서 독립적으로 검출
    /*
    for (int i = 0; i < grid_rows; i++) {
        for (int j = 0; j < grid_cols; j++) {
            cv::Rect roi(...);  // 현재 셀의 ROI
            cv::Mat cell = image(roi);
            
            std::vector<cv::KeyPoint> cell_kp;
            cv::FAST(cell, cell_kp, 20, true);
            
            // 상위 N개만 선택
            // keypoint의 response로 정렬 후 상위 max_per_cell개
            
            // 전역 좌표로 변환하여 추가
        }
    }
    */
    
    std::cout << "결과:" << std::endl;
    std::cout << "   일반 검출: " << kp_normal.size() << "개" << std::endl;
    std::cout << "   균등 분포: " << kp_uniform.size() << "개\n" << std::endl;
    
    std::cout << "💡 왜필요한가?" << std::endl;
    std::cout << "   - 특징이 한쪽에 몰리면 포즈 추정 불안정" << std::endl;
    std::cout << "   - 균등 분포 → 더 robust한 tracking" << std::endl;
    std::cout << "   - VINS-Fusion은 6x4 그리드 사용" << std::endl;
}

/**
 * 문제 2: Adaptive Threshold
 * 
 * 이미지 밝기에 따라 FAST 임계값을 자동으로 조정하여
 * 목표 특징점 개수를 달성하세요.
 */
void problem2_adaptive_threshold() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 적응형 임계값" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC1);
    for (int i = 0; i < 50; i++) {
        cv::Point center(rand() % 800, rand() % 600);
        int radius = 10 + rand() % 20;
        cv::circle(image, center, radius, cv::Scalar(200), -1);
    }
    
    int target_keypoints = 200;  // 목표: 200개
    
    // TODO: 이진 탐색으로 적절한 임계값 찾기
    int threshold = 20;  // 초기값
    int min_thresh = 1, max_thresh = 100;
    
    std::vector<cv::KeyPoint> keypoints;
    
    /*
    while (min_thresh <= max_thresh) {
        threshold = (min_thresh + max_thresh) / 2;
        cv::FAST(image, keypoints, threshold, true);
        
        if (keypoints.size() > target_keypoints) {
            min_thresh = threshold + 1;  // 임계값 높이기
        } else {
            max_thresh = threshold - 1;  // 임계값 낮추기
        }
    }
    */
    
    std::cout << "결과:" << std::endl;
    std::cout << "   목표: " << target_keypoints << "개" << std::endl;
    std::cout << "   실제: " << keypoints.size() << "개" << std::endl;
    std::cout << "   임계값: " << threshold << "\n" << std::endl;
    
    std::cout << "💡 응용:" << std::endl;
    std::cout << "   - 밝은 환경: 임계값 ↑" << std::endl;
    std::cout << "   - 어두운 환경: 임계값 ↓" << std::endl;
    std::cout << "   - 실시간으로 조정하여 일정한 개수 유지" << std::endl;
}

/**
 * 문제 3: 멀티스케일 검출
 * 
 * 이미지 피라미드를 만들고 여러 스케일에서 특징점을 검출하세요.
 * ORB는 이미 내부적으로 구현되어 있습니다.
 */
void problem3_multiscale_detection() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 멀티스케일 검출" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC1);
    // 다양한 크기의 원
    cv::circle(image, cv::Point(200, 200), 80, cv::Scalar(255), -1);
    cv::circle(image, cv::Point(500, 300), 40, cv::Scalar(255), -1);
    cv::circle(image, cv::Point(600, 450), 20, cv::Scalar(255), -1);
    
    // TODO: 단일 스케일
    std::vector<cv::KeyPoint> kp_single;
    cv::FAST(image, kp_single, 20, true);
    
    // TODO: 멀티스케일 (이미지 피라미드)
    std::vector<cv::KeyPoint> kp_multi;
    int num_levels = 4;
    float scale_factor = 1.2f;
    
    /*
    for (int level = 0; level < num_levels; level++) {
        float scale = pow(scale_factor, level);
        cv::Mat scaled;
        cv::resize(image, scaled, cv::Size(), 1.0/scale, 1.0/scale);
        
        std::vector<cv::KeyPoint> level_kp;
        cv::FAST(scaled, level_kp, 20, true);
        
        // 원본 스케일로 변환
        for (auto& kp : level_kp) {
            kp.pt *= scale;
            kp.size *= scale;
            kp_multi.push_back(kp);
        }
    }
    */
    
    std::cout << "결과:" << std::endl;
    std::cout << "   단일 스케일: " << kp_single.size() << "개" << std::endl;
    std::cout << "   멀티스케일: " << kp_multi.size() << "개\n" << std::endl;
    
    std::cout << "💡 장점:" << std::endl;
    std::cout << "   - 다양한 크기의 물체 검출" << std::endl;
    std::cout << "   - 스케일 불변성 (물체가 가까워져도 추적 가능)" << std::endl;
    std::cout << "   - ORB, SIFT는 자동으로 수행" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 3 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    problem1_uniform_distribution()
    problem2_adaptive_threshold()
    problem3_multiscale_detection()
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
