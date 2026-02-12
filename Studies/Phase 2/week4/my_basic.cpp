/**
 * Phase 2 Week 4 - 특징점 매칭 직접 구현
 *
 * basic.h의 FeatureMatchingBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make test_my_basic
 * 테스트: ./test_my_basic
 */
#include "basic.h"
#include <iostream>

double FeatureMatchingBasic::matchBruteForce(const cv::Mat& descriptors1,
                                             const cv::Mat& descriptors2,
                                             std::vector<cv::DMatch>& matches, int normType)
{
    // TODO: cv::BFMatcher 사용
    return 0.0;
}

double FeatureMatchingBasic::matchFLANN(const cv::Mat& descriptors1, const cv::Mat& descriptors2,
                                        std::vector<cv::DMatch>& matches)
{
    // TODO: cv::FlannBasedMatcher 사용
    return 0.0;
}

int FeatureMatchingBasic::ratioTest(const cv::Mat& descriptors1, const cv::Mat& descriptors2,
                                    std::vector<cv::DMatch>& good_matches, float ratio_thresh)
{
    // TODO: knnMatch + Lowe's Ratio Test
    return 0;
}

double FeatureMatchingBasic::filterRANSAC(const std::vector<cv::KeyPoint>& keypoints1,
                                          const std::vector<cv::KeyPoint>& keypoints2,
                                          const std::vector<cv::DMatch>& matches,
                                          std::vector<cv::DMatch>& inlier_matches,
                                          cv::Mat& homography, double ransac_thresh)
{
    // TODO: cv::findHomography + RANSAC
    return 0.0;
}

void FeatureMatchingBasic::visualizeMatches(const cv::Mat& img1,
                                            const std::vector<cv::KeyPoint>& kp1,
                                            const cv::Mat& img2,
                                            const std::vector<cv::KeyPoint>& kp2,
                                            const std::vector<cv::DMatch>& matches, cv::Mat& output)
{
    // TODO: cv::drawMatches 사용
}

double FeatureMatchingBasic::evaluateMatchQuality(const std::vector<cv::DMatch>& matches)
{
    // TODO: 매칭들의 평균 거리 계산
    return 0.0;
}

void FeatureMatchingBasic::demoPipeline(const cv::Mat& img1, const cv::Mat& img2)
{
    // TODO: 전체 매칭 파이프라인
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 특징점 매칭 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트용 디스크립터 생성
    cv::Mat desc1 = cv::Mat::zeros(10, 32, CV_8UC1);
    cv::Mat desc2 = cv::Mat::zeros(10, 32, CV_8UC1);
    cv::randu(desc1, 0, 255);
    desc2 = desc1.clone();

    // Brute Force 매칭
    std::vector<cv::DMatch> matches;
    double t_bf = FeatureMatchingBasic::matchBruteForce(desc1, desc2, matches);
    std::cout << "BF 매칭: " << matches.size() << "개 (" << t_bf << " ms)" << std::endl;

    // 매칭 품질
    double quality = FeatureMatchingBasic::evaluateMatchQuality(matches);
    std::cout << "평균 매칭 거리: " << quality << std::endl;

    std::cout << "\n구현 완료 후 테스트: ./test_my_basic" << std::endl;
    return 0;
}
#endif
