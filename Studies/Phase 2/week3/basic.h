#ifndef FEATURE_DETECTION_BASIC_H
#define FEATURE_DETECTION_BASIC_H

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <vector>
#include <string>

/**
 * @brief 특징점 검출 기본 클래스
 * 
 * FAST, ORB 등의 특징점 검출기를 다루는 간소화된 예제입니다.
 * Week 3: 특징점 검출 (Feature Detection)
 */
class FeatureDetectionBasic {
public:
    /**
     * @brief FAST 코너 검출
     * @param image 입력 이미지
     * @param keypoints 검출된 특징점 (출력)
     * @param threshold FAST 임계값 (기본: 20)
     * @param nonmaxSuppression NMS 적용 여부
     * @return 검출 시간 (ms)
     */
    static double detectFAST(const cv::Mat& image,
                            std::vector<cv::KeyPoint>& keypoints,
                            int threshold = 20,
                            bool nonmaxSuppression = true);
    
    /**
     * @brief ORB 특징점 검출 및 디스크립터 계산
     * @param image 입력 이미지
     * @param keypoints 검출된 특징점 (출력)
     * @param descriptors 계산된 디스크립터 (출력)
     * @param nfeatures 최대 특징점 개수
     * @return 검출 시간 (ms)
     */
    static double detectORB(const cv::Mat& image,
                           std::vector<cv::KeyPoint>& keypoints,
                           cv::Mat& descriptors,
                           int nfeatures = 500);
    
    /**
     * @brief 특징점 시각화
     * @param image 원본 이미지
     * @param keypoints 특징점들
     * @param output 출력 이미지
     * @param title 윈도우 제목
     */
    static void visualizeKeypoints(const cv::Mat& image,
                                   const std::vector<cv::KeyPoint>& keypoints,
                                   cv::Mat& output,
                                   const std::string& title = "Keypoints");
    
    /**
     * @brief 특징점 분포 분석
     * @param keypoints 특징점들
     * @param imageSize 이미지 크기
     * @param gridSize 그리드 크기 (기본: 8x8)
     * @return 그리드별 특징점 개수
     */
    static cv::Mat analyzeDistribution(const std::vector<cv::KeyPoint>& keypoints,
                                       cv::Size imageSize,
                                       int gridSize = 8);
    
    /**
     * @brief 검출기 성능 비교
     * @param image 테스트 이미지
     */
    static void compareDetectors(const cv::Mat& image);
    
    /**
     * @brief Non-Maximum Suppression 데모
     * @param image 입력 이미지
     */
    static void demoNMS(const cv::Mat& image);
};

#endif // FEATURE_DETECTION_BASIC_H
