#ifndef FEATURE_MATCHING_BASIC_H
#define FEATURE_MATCHING_BASIC_H

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <vector>
#include <string>

/**
 * @brief 특징점 매칭 기본 클래스
 *
 * Week 4: 특징점 매칭 (Feature Matching)
 * - Brute-Force vs FLANN 매칭
 * - Lowe's Ratio Test
 * - RANSAC outlier 제거
 */
class FeatureMatchingBasic
{
   public:
    /**
     * @brief Brute-Force 매칭
     * @param descriptors1 첫 번째 이미지의 디스크립터
     * @param descriptors2 두 번째 이미지의 디스크립터
     * @param matches 매칭 결과 (출력)
     * @param normType 거리 측정 방식 (NORM_HAMMING for ORB, NORM_L2 for SIFT)
     * @return 매칭 시간 (ms)
     */
    static double matchBruteForce(const cv::Mat& descriptors1, const cv::Mat& descriptors2,
                                  std::vector<cv::DMatch>& matches,
                                  int normType = cv::NORM_HAMMING);

    /**
     * @brief FLANN 매칭 (빠른 근사)
     * @param descriptors1 첫 번째 이미지의 디스크립터
     * @param descriptors2 두 번째 이미지의 디스크립터
     * @param matches 매칭 결과 (출력)
     * @return 매칭 시간 (ms)
     */
    static double matchFLANN(const cv::Mat& descriptors1, const cv::Mat& descriptors2,
                             std::vector<cv::DMatch>& matches);

    /**
     * @brief Lowe's Ratio Test로 매칭 필터링
     * @param descriptors1 첫 번째 이미지의 디스크립터
     * @param descriptors2 두 번째 이미지의 디스크립터
     * @param good_matches 좋은 매칭들 (출력)
     * @param ratio_thresh 비율 임계값 (기본: 0.7)
     * @return 필터링 후 매칭 개수
     */
    static int ratioTest(const cv::Mat& descriptors1, const cv::Mat& descriptors2,
                         std::vector<cv::DMatch>& good_matches, float ratio_thresh = 0.7f);

    /**
     * @brief RANSAC으로 outlier 제거
     * @param keypoints1 첫 번째 이미지의 특징점
     * @param keypoints2 두 번째 이미지의 특징점
     * @param matches 매칭들
     * @param inlier_matches inlier 매칭들 (출력)
     * @param homography 계산된 호모그래피 행렬 (출력)
     * @param ransac_thresh RANSAC 임계값 (기본: 3.0)
     * @return inlier 비율 (0~1)
     */
    static double filterRANSAC(const std::vector<cv::KeyPoint>& keypoints1,
                               const std::vector<cv::KeyPoint>& keypoints2,
                               const std::vector<cv::DMatch>& matches,
                               std::vector<cv::DMatch>& inlier_matches, cv::Mat& homography,
                               double ransac_thresh = 3.0);

    /**
     * @brief 매칭 시각화
     * @param img1 첫 번째 이미지
     * @param kp1 첫 번째 이미지의 특징점
     * @param img2 두 번째 이미지
     * @param kp2 두 번째 이미지의 특징점
     * @param matches 매칭들
     * @param output 출력 이미지
     */
    static void visualizeMatches(const cv::Mat& img1, const std::vector<cv::KeyPoint>& kp1,
                                 const cv::Mat& img2, const std::vector<cv::KeyPoint>& kp2,
                                 const std::vector<cv::DMatch>& matches, cv::Mat& output);

    /**
     * @brief 매칭 품질 평가
     * @param matches 매칭들
     * @return 평균 매칭 거리
     */
    static double evaluateMatchQuality(const std::vector<cv::DMatch>& matches);

    /**
     * @brief 전체 매칭 파이프라인 데모
     * @param img1 첫 번째 이미지
     * @param img2 두 번째 이미지
     */
    static void demoPipeline(const cv::Mat& img1, const cv::Mat& img2);
};

#endif  // FEATURE_MATCHING_BASIC_H
