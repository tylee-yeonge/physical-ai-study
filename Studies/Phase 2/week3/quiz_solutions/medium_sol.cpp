// Phase 2 Week 3 - 중급 퀴즈 정답 (핵심 알고리즘)
#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>

// 문제 1: 균등 분포
std::vector<cv::KeyPoint> uniformDistribution(const cv::Mat& image, int grid_rows, int grid_cols, int max_per_cell) {
    std::vector<cv::KeyPoint> result;
    
    int cell_h = image.rows / grid_rows;
    int cell_w = image.cols / grid_cols;
    
    for (int i = 0; i < grid_rows; i++) {
        for (int j = 0; j < grid_cols; j++) {
            cv::Rect roi(j * cell_w, i * cell_h, cell_w, cell_h);
            cv::Mat cell = image(roi);
            
            std::vector<cv::KeyPoint> cell_kp;
            cv::FAST(cell, cell_kp, 20, true);
            
            // response로 정렬
            std::sort(cell_kp.begin(), cell_kp.end(),
                     [](const cv::KeyPoint& a, const cv::KeyPoint& b) {
                         return a.response > b.response;
                     });
            
            // 상위 N개만
            int count = std::min((int)cell_kp.size(), max_per_cell);
            for (int k = 0; k < count; k++) {
                cell_kp[k].pt.x += roi.x;
                cell_kp[k].pt.y += roi.y;
                result.push_back(cell_kp[k]);
            }
        }
    }
    
    return result;
}

// 문제 2: Adaptive threshold
int adaptiveThreshold(const cv::Mat& image, int target_count) {
    int min_t = 1, max_t = 100;
    int best_threshold = 20;
    
    while (min_t <= max_t) {
        int threshold = (min_t + max_t) / 2;
        std::vector<cv::KeyPoint> kp;
        cv::FAST(image, kp, threshold, true);
        
        if (kp.size() > (size_t)target_count) {
            min_t = threshold + 1;
        } else {
            max_t = threshold - 1;
            best_threshold = threshold;
        }
    }
    
    return best_threshold;
}

int main() {
    std::cout << "Week 3 중급 퀴즈 핵심 정답\n" << std::endl;
    
    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC1);
    
    // 테스트
    auto kp = uniformDistribution(image, 6, 8, 10);
    std::cout << "균등 분포: " << kp.size() << "개\n" << std::endl;
    
    int threshold = adaptiveThreshold(image, 200);
    std::cout << "적응형 임계값: " << threshold << std::endl;
    
    return 0;
}
