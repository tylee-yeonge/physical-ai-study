#include "basic.h"
#include <iostream>
#include <chrono>
#include <iomanip>

double FeatureDetectionBasic::detectFAST(const cv::Mat& image,
                                         std::vector<cv::KeyPoint>& keypoints,
                                         int threshold,
                                         bool nonmaxSuppression) {
    // 그레이스케일 변환
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // 성능 측정
    auto start = std::chrono::high_resolution_clock::now();
    
    // FAST 검출 (OpenCV는 FAST-9, FAST-12 등 제공)
    cv::FAST(gray, keypoints, threshold, nonmaxSuppression);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    return duration.count() / 1000.0;  // ms로 변환
}

double FeatureDetectionBasic::detectORB(const cv::Mat& image,
                                        std::vector<cv::KeyPoint>& keypoints,
                                        cv::Mat& descriptors,
                                        int nfeatures) {
    // ORB 검출기 생성
    cv::Ptr<cv::ORB> orb = cv::ORB::create(nfeatures);
    
    // 성능 측정
    auto start = std::chrono::high_resolution_clock::now();
    
    // 특징점 검출 및 디스크립터 계산
    orb->detectAndCompute(image, cv::noArray(), keypoints, descriptors);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    return duration.count() / 1000.0;  // ms
}

void FeatureDetectionBasic::visualizeKeypoints(const cv::Mat& image,
                                               const std::vector<cv::KeyPoint>& keypoints,
                                               cv::Mat& output,
                                               const std::string& title) {
    // 특징점 그리기
    cv::drawKeypoints(image, keypoints, output,
                     cv::Scalar(0, 255, 0),  // 녹색
                     cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);  // 방향과 크기 표시
    
    // 정보 텍스트 추가
    std::string info = "Keypoints: " + std::to_string(keypoints.size());
    cv::putText(output, info, cv::Point(10, 30),
               cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
}

cv::Mat FeatureDetectionBasic::analyzeDistribution(const std::vector<cv::KeyPoint>& keypoints,
                                                   cv::Size imageSize,
                                                   int gridSize) {
    // 그리드별 특징점 개수 카운트
    cv::Mat distribution = cv::Mat::zeros(gridSize, gridSize, CV_32S);
    
    int cellWidth = imageSize.width / gridSize;
    int cellHeight = imageSize.height / gridSize;
    
    for (const auto& kp : keypoints) {
        int gridX = static_cast<int>(kp.pt.x / cellWidth);
        int gridY = static_cast<int>(kp.pt.y / cellHeight);
        
        // 범위 체크
        if (gridX >= 0 && gridX < gridSize && gridY >= 0 && gridY < gridSize) {
            distribution.at<int>(gridY, gridX)++;
        }
    }
    
    return distribution;
}

void FeatureDetectionBasic::compareDetectors(const cv::Mat& image) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "특징점 검출기 성능 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    
    // 1. FAST
    double time_fast = detectFAST(image, keypoints, 20, true);
    int count_fast = keypoints.size();
    
    std::cout << "📍 FAST:" << std::endl;
    std::cout << "   - 검출 시간: " << std::fixed << std::setprecision(2) 
              << time_fast << " ms" << std::endl;
    std::cout << "   - 특징점 개수: " << count_fast << std::endl;
    std::cout << "   - 특징: 매우 빠름, 코너 검출, 디스크립터 없음\n" << std::endl;
    
    // 2. ORB
    keypoints.clear();
    double time_orb = detectORB(image, keypoints, descriptors, 500);
    int count_orb = keypoints.size();
    
    std::cout << "📍 ORB:" << std::endl;
    std::cout << "   - 검출 시간: " << time_orb << " ms" << std::endl;
    std::cout << "   - 특징점 개수: " << count_orb << std::endl;
    std::cout << "   - 디스크립터 크기: " << descriptors.size() << std::endl;
    std::cout << "   - 특징: FAST + BRIEF, 회전 불변성\n" << std::endl;
    
    // 3. SIFT (참고 - 특허 만료로 OpenCV 4.x에서 사용 가능)
    try {
        cv::Ptr<cv::SIFT> sift = cv::SIFT::create(500);
        keypoints.clear();
        
        auto start = std::chrono::high_resolution_clock::now();
        sift->detectAndCompute(image, cv::noArray(), keypoints, descriptors);
        auto end = std::chrono::high_resolution_clock::now();
        double time_sift = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
        
        std::cout << "📍 SIFT:" << std::endl;
        std::cout << "   - 검출 시간: " << time_sift << " ms" << std::endl;
        std::cout << "   - 특징점 개수: " << keypoints.size() << std::endl;
        std::cout << "   - 특징: 스케일/회전 불변, 느림, 고품질\n" << std::endl;
    } catch (...) {
        std::cout << "⚠️  SIFT: 사용 불가 (OpenCV contrib 필요)\n" << std::endl;
    }
    
    std::cout << "💡 SLAM에서의 선택:" << std::endl;
    std::cout << "   - 실시간 중요: FAST + KLT (VINS-Fusion)" << std::endl;
    std::cout << "   - Loop Closure: ORB (ORB-SLAM)" << std::endl;
    std::cout << "   - 고정밀: SIFT (오프라인 처리)" << std::endl;
}

void FeatureDetectionBasic::demoNMS(const cv::Mat& image) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Non-Maximum Suppression 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::vector<cv::KeyPoint> kp_without_nms, kp_with_nms;
    
    // NMS 없이
    double time1 = detectFAST(image, kp_without_nms, 20, false);
    
    // NMS 적용
    double time2 = detectFAST(image, kp_with_nms, 20, true);
    
    std::cout << "📊 결과 비교:" << std::endl;
    std::cout << "   NMS 없음: " << kp_without_nms.size() << "개 특징점, "
              << time1 << " ms" << std::endl;
    std::cout << "   NMS 적용: " << kp_with_nms.size() << "개 특징점, "
              << time2 << " ms" << std::endl;
    
    double reduction = 100.0 * (1.0 - (double)kp_with_nms.size() / kp_without_nms.size());
    std::cout << "   → " << std::fixed << std::setprecision(1) 
              << reduction << "% 감소\n" << std::endl;
    
    std::cout << "💡 NMS의 역할:" << std::endl;
    std::cout << "   - 인접한 여러 응답 중 최대값만 유지" << std::endl;
    std::cout << "   - 중복 특징점 제거 → 더 분산된 분포" << std::endl;
    std::cout << "   - SLAM에서 필수! (맵 점의 품질 향상)" << std::endl;
}

// 메인 함수 - 데모
#ifndef BASIC_LIB_ONLY
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  특징점 검출 기본 데모 (Week 3)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 테스트 이미지 생성 (체커보드 패턴)
    cv::Mat test_image = cv::Mat::zeros(600, 800, CV_8UC3);
    
    // 체커보드 패턴 그리기
    int square_size = 50;
    for (int i = 0; i < test_image.rows; i += square_size) {
        for (int j = 0; j < test_image.cols; j += square_size) {
            if ((i/square_size + j/square_size) % 2 == 0) {
                cv::rectangle(test_image, 
                            cv::Point(j, i),
                            cv::Point(j + square_size, i + square_size),
                            cv::Scalar(255, 255, 255), -1);
            }
        }
    }
    
    // 원 추가 (다양한 특징)
    cv::circle(test_image, cv::Point(400, 300), 80, cv::Scalar(200, 100, 50), -1);
    cv::circle(test_image, cv::Point(200, 200), 50, cv::Scalar(100, 200, 150), -1);
    
    std::cout << "📸 테스트 이미지 생성 완료 (800x600)" << std::endl;
    std::cout << "   - 체커보드 패턴 + 원형 객체\n" << std::endl;
    
    // 검출기 비교
    FeatureDetectionBasic::compareDetectors(test_image);
    
    // NMS 데모
    FeatureDetectionBasic::demoNMS(test_image);
    
    // FAST 검출 및 시각화
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "FAST 특징점 검출 및 분포 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::vector<cv::KeyPoint> keypoints;
    double time = FeatureDetectionBasic::detectFAST(test_image, keypoints, 20, true);
    
    std::cout << "✅ 검출 완료:" << std::endl;
    std::cout << "   - 특징점 개수: " << keypoints.size() << std::endl;
    std::cout << "   - 검출 시간: " << time << " ms" << std::endl;
    
    // 분포 분석
    cv::Mat distribution = FeatureDetectionBasic::analyzeDistribution(
        keypoints, test_image.size(), 8);
    
    std::cout << "\n📊 8x8 그리드 분포:" << std::endl;
    for (int i = 0; i < distribution.rows; i++) {
        std::cout << "   ";
        for (int j = 0; j < distribution.cols; j++) {
            std::cout << std::setw(4) << distribution.at<int>(i, j);
        }
        std::cout << std::endl;
    }
    
    // 시각화 (저장)
    cv::Mat output;
    FeatureDetectionBasic::visualizeKeypoints(test_image, keypoints, output);
    
    std::cout << "\n💾 시각화 이미지 저장: keypoints_demo.png" << std::endl;
    cv::imwrite("keypoints_demo.png", output);
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ Week 3 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "💡 다음 단계:" << std::endl;
    std::cout << "   1. quiz_easy.cpp로 기초 개념 확인" << std::endl;
    std::cout << "   2. quiz_medium.cpp로 실전 문제 풀이" << std::endl;
    std::cout << "   3. PRACTICE.md로 실제 카메라 영상 처리\n" << std::endl;
    
    return 0;
}
#endif // BASIC_LIB_ONLY
