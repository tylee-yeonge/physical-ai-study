/**
 * Phase 2 Week 2 - 중급 퀴즈 정답 (요약 버전)
 * 
 * 중급 퀴즈는 코드량이 많으므로, 핵심 알고리즘만 제공합니다.
 * 전체 구현은 직접 완성해보세요!
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <chrono>

/**
 * 문제 2 정답: 왜곡 보정 직접 구현
 */
cv::Point2f manualUndistort(const cv::Point2f& distorted,
                            const cv::Mat& K,
                            const cv::Mat& dist) {
    double fx = K.at<double>(0, 0);
    double fy = K.at<double>(1, 1);
    double cx = K.at<double>(0, 2);
    double cy = K.at<double>(1, 2);
    
    double k1 = dist.at<double>(0);
    double k2 = dist.at<double>(1);
    
    // 1. 정규화 좌표로 변환
    double x = (distorted.x - cx) / fx;
    double y = (distorted.y - cy) / fy;
    
    // 2. 반복적으로 왜곡 보정 (Newton-Raphson 방법)
    // 간단한 근사: 역방향 왜곡 적용
    double r2 = x*x + y*y;
    double radial_dist = 1.0 + k1*r2 + k2*r2*r2;
    
    // 3. 보정된 정규화 좌표
    double x_corrected = x / radial_dist;
    double y_corrected = y / radial_dist;
    
    // 4. 픽셀 좌표로 복원
    cv::Point2f undistorted;
    undistorted.x = fx * x_corrected + cx;
    undistorted.y = fy * y_corrected + cy;
    
    return undistorted;
}

/**
 * 문제 3 정답: 성능 최적화
 */
void performanceComparison() {
    cv::Mat test_image = cv::Mat::zeros(600, 800, CV_8UC3);
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);
    cv::Mat dist = (cv::Mat_<double>(1, 5) << -0.2, 0.05, 0, 0, 0);
    
    int iterations = 1000;
    
    // Method 1: undistort
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        cv::Mat undistorted;
        cv::undistort(test_image, undistorted, K, dist);
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    
    // Method 2: remap (맵 미리 계산)
    cv::Mat map1, map2;
    cv::initUndistortRectifyMap(K, dist, cv::Mat(), K, 
                                test_image.size(), CV_32FC1, map1, map2);
    
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        cv::Mat undistorted;
        cv::remap(test_image, undistorted, map1, map2, cv::INTER_LINEAR);
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    
    std::cout << "Method 1 (undistort): " << duration1.count() << " ms" << std::endl;
    std::cout << "Method 2 (remap):     " << duration2.count() << " ms" << std::endl;
    std::cout << "가속비: " << (double)duration1.count() / duration2.count() << "x" << std::endl;
}

int main() {
    std::cout << "\n중급 퀴즈 정답 (핵심 알고리즘)\n" << std::endl;
    
    // 문제 2 테스트
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);
    cv::Mat dist = (cv::Mat_<double>(1, 5) << -0.3, 0.1, 0, 0, 0);
    
    cv::Point2f test_point(700, 500);
    cv::Point2f result = manualUndistort(test_point, K, dist);
    
    std::cout << "왜곡 보정 결과: (" << result.x << ", " << result.y << ")\n" << std::endl;
    
    // 문제 3 테스트
    performanceComparison();
    
    return 0;
}
