#ifndef OPTICAL_FLOW_BASIC_H
#define OPTICAL_FLOW_BASIC_H

#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>
#include <vector>
#include <string>

/**
 * @brief Optical Flow 기본 클래스
 * 
 * Week 8: Optical Flow - 픽셀 움직임 추적
 * - Lucas-Kanade (Sparse)
 * - Farneback (Dense)
 * - Pyramidal Optical Flow
 * - Feature Tracking
 */
class OpticalFlowBasic {
public:
    /**
     * @brief Lucas-Kanade Optical Flow (Sparse)
     * @param prev_img 이전 프레임
     * @param curr_img 현재 프레임
     * @param prev_pts 추적할 점들
     * @param curr_pts 추적된 점들 (출력)
     * @param status 추적 성공 여부 (출력)
     * @return 추적 성공률
     */
    static double lucasKanade(const cv::Mat& prev_img,
                             const cv::Mat& curr_img,
                             const std::vector<cv::Point2f>& prev_pts,
                             std::vector<cv::Point2f>& curr_pts,
                             std::vector<uchar>& status);
    
    /**
     * @brief Farneback Optical Flow (Dense)
     * @param prev_img 이전 프레임
     * @param curr_img 현재 프레임
     * @param flow 광학 흐름 (출력)
     */
    static void farneback(const cv::Mat& prev_img,
                         const cv::Mat& curr_img,
                         cv::Mat& flow);
    
    /**
     * @brief Flow 시각화
     * @param flow 광학 흐름
     * @param output 출력 이미지
     * @param step 화살표 간격
     */
    static void visualizeFlow(const cv::Mat& flow,
                             cv::Mat& output,
                             int step = 16);
    
    /**
     * @brief Sparse Flow 시각화
     * @param img 이미지
     * @param prev_pts 이전 점들
     * @param curr_pts 현재 점들
     * @param status 추적 성공 여부
     * @param output 출력 이미지
     */
    static void visualizeSparseFlow(const cv::Mat& img,
                                   const std::vector<cv::Point2f>& prev_pts,
                                   const std::vector<cv::Point2f>& curr_pts,
                                   const std::vector<uchar>& status,
                                   cv::Mat& output);
    
    /**
     * @brief Feature Tracking 데모
     * @param video_path 비디오 경로 (빈 문자열이면 합성 데이터)
     */
    static void demoFeatureTracking(const std::string& video_path = "");
    
    /**
     * @brief Optical Flow 품질 평가
     * @param prev_pts 이전 점들
     * @param curr_pts 현재 점들
     * @param status 추적 성공 여부
     * @return 평균 이동 거리
     */
    static double evaluateFlow(const std::vector<cv::Point2f>& prev_pts,
                               const std::vector<cv::Point2f>& curr_pts,
                               const std::vector<uchar>& status);
    
    /**
     * @brief Sparse vs Dense 비교
     */
    static void compareSparseVsDense();
    
    /**
     * @brief 전체 파이프라인 데모
     */
    static void demoPipeline();
};

#endif // OPTICAL_FLOW_BASIC_H
