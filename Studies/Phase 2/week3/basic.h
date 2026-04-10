#ifndef PHASE2_WEEK3_BASIC_H_
#define PHASE2_WEEK3_BASIC_H_

/**
 * @file basic.h
 * @brief Week 3: Multi-view 기하 + Stereo Rectification 헬퍼
 *
 * Perception 맥락:
 *   - Stereo Depth 모델(HITNet, CRE-Stereo)의 입력은 rectified stereo pair
 *   - 이 파일의 유틸리티는 rectification 파이프라인의 각 단계를 구현
 */

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

/**
 * @brief 합성 스테레오 쌍과 카메라 파라미터를 생성
 * @param image_size 출력 이미지 크기
 * @param K1 왼쪽 카메라 내부 행렬 (출력)
 * @param K2 오른쪽 카메라 내부 행렬 (출력)
 * @param dist1 왼쪽 왜곡 계수 (출력)
 * @param dist2 오른쪽 왜곡 계수 (출력)
 * @param R 카메라 간 회전 (출력)
 * @param T 카메라 간 이동 (출력)
 * @param left 왼쪽 이미지 (출력)
 * @param right 오른쪽 이미지 (출력)
 */
void generate_synthetic_stereo(
    const cv::Size& image_size,
    cv::Mat& K1, cv::Mat& K2,
    cv::Mat& dist1, cv::Mat& dist2,
    cv::Mat& R, cv::Mat& T,
    cv::Mat& left, cv::Mat& right);

/**
 * @brief Fundamental Matrix 계산 후 에피폴라 선을 이미지에 그리기
 * @param left 왼쪽 이미지
 * @param right 오른쪽 이미지
 * @param pts1 왼쪽 대응점
 * @param pts2 오른쪽 대응점
 * @param output_path 결과 저장 경로 (빈 문자열이면 저장 안 함)
 * @return Fundamental Matrix F
 */
cv::Mat draw_epipolar_lines(
    const cv::Mat& left, const cv::Mat& right,
    const std::vector<cv::Point2f>& pts1,
    const std::vector<cv::Point2f>& pts2,
    const std::string& output_path = "");

/**
 * @brief Stereo Rectification 수행 후 결과 이미지 반환
 * @param left 왼쪽 이미지
 * @param right 오른쪽 이미지
 * @param K1 왼쪽 카메라 내부 행렬
 * @param K2 오른쪽 카메라 내부 행렬
 * @param dist1 왼쪽 왜곡 계수
 * @param dist2 오른쪽 왜곡 계수
 * @param R 카메라 간 회전
 * @param T 카메라 간 이동
 * @param image_size 이미지 크기
 * @param rect_left Rectified 왼쪽 이미지 (출력)
 * @param rect_right Rectified 오른쪽 이미지 (출력)
 * @param Q Disparity → 3D 변환 행렬 (출력)
 */
void stereo_rectify(
    const cv::Mat& left, const cv::Mat& right,
    const cv::Mat& K1, const cv::Mat& K2,
    const cv::Mat& dist1, const cv::Mat& dist2,
    const cv::Mat& R, const cv::Mat& T,
    const cv::Size& image_size,
    cv::Mat& rect_left, cv::Mat& rect_right,
    cv::Mat& Q);

/**
 * @brief Disparity map 계산 (StereoBM 사용)
 * @param rect_left Rectified 왼쪽 이미지
 * @param rect_right Rectified 오른쪽 이미지
 * @return Disparity map (CV_32F, 단위: 픽셀)
 */
cv::Mat compute_disparity(const cv::Mat& rect_left, const cv::Mat& rect_right);

/**
 * @brief Disparity → Depth 변환: Z = fB / d
 * @param disparity Disparity map (CV_32F)
 * @param focal_length 초점 거리 (픽셀 단위)
 * @param baseline 스테레오 baseline (미터 단위)
 * @return Depth map (CV_32F, 단위: 미터)
 */
cv::Mat disparity_to_depth(const cv::Mat& disparity, double focal_length, double baseline);

/**
 * @brief 결과 이미지를 output/ 디렉토리에 저장
 * @param name 파일 이름 (확장자 제외)
 * @param image 저장할 이미지
 */
void save_output(const std::string& name, const cv::Mat& image);

#endif  // PHASE2_WEEK3_BASIC_H_
