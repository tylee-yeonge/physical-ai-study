/**
 * Phase 2 Week 3 - 특징점 검출 직접 구현
 *
 * basic.h의 FeatureDetectionBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make test_my_basic
 * 테스트: ./test_my_basic
 */
#include "basic.h"
#include <iostream>

double FeatureDetectionBasic::detectFAST(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints,
                                         int threshold, bool nonmaxSuppression)
{
    // TODO: cv::FAST() 사용, 시간 측정 후 반환 (ms)
    return 0.0;
}

double FeatureDetectionBasic::detectORB(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints,
                                        cv::Mat& descriptors, int nfeatures)
{
    // TODO: cv::ORB::create() 사용, detectAndCompute
    return 0.0;
}

void FeatureDetectionBasic::visualizeKeypoints(const cv::Mat& image,
                                               const std::vector<cv::KeyPoint>& keypoints,
                                               cv::Mat& output, const std::string& title)
{
    // TODO: cv::drawKeypoints 사용
}

cv::Mat FeatureDetectionBasic::analyzeDistribution(const std::vector<cv::KeyPoint>& keypoints,
                                                   cv::Size imageSize, int gridSize)
{
    // TODO: 이미지를 gridSize x gridSize로 나누고 각 셀의 특징점 개수 계산
    return cv::Mat::zeros(gridSize, gridSize, CV_32S);
}

void FeatureDetectionBasic::compareDetectors(const cv::Mat& image)
{
    // TODO: FAST vs ORB 비교
}

void FeatureDetectionBasic::demoNMS(const cv::Mat& image)
{
    // TODO: Non-Maximum Suppression 데모
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 특징점 검출 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 생성
    cv::Mat test_img = cv::Mat::zeros(480, 640, CV_8UC1);
    cv::rectangle(test_img, cv::Point(100, 100), cv::Point(300, 300), cv::Scalar(255), 2);
    cv::circle(test_img, cv::Point(400, 200), 50, cv::Scalar(200), 2);

    // FAST 검출
    std::vector<cv::KeyPoint> kp_fast;
    double t_fast = FeatureDetectionBasic::detectFAST(test_img, kp_fast);
    std::cout << "FAST: " << kp_fast.size() << "개 (" << t_fast << " ms)" << std::endl;

    // ORB 검출
    std::vector<cv::KeyPoint> kp_orb;
    cv::Mat desc;
    double t_orb = FeatureDetectionBasic::detectORB(test_img, kp_orb, desc);
    std::cout << "ORB: " << kp_orb.size() << "개 (" << t_orb << " ms)" << std::endl;

    std::cout << "\n구현 완료 후 테스트: ./test_my_basic" << std::endl;
    return 0;
}
#endif
