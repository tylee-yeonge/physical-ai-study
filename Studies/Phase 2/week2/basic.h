#ifndef CAMERA_CALIBRATION_BASIC_H
#define CAMERA_CALIBRATION_BASIC_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

/**
 * @brief 간단한 카메라 캘리브레이션 데모 클래스
 *
 * 이 클래스는 캘리브레이션의 핵심 개념을 이해하기 위한 간소화된 버전입니다.
 * 실제 카메라 캘리브레이션은 PRACTICE.md를 참고하세요.
 */
class CameraCalibrationBasic
{
   public:
    /**
     * @brief 생성자
     * @param boardSize 체커보드 내부 코너 개수 (가로, 세로)
     * @param squareSize 체커보드 한 칸의 크기 (단위: mm)
     */
    CameraCalibrationBasic(cv::Size boardSize, float squareSize);

    /**
     * @brief 체커보드 코너 검출
     * @param image 입력 이미지
     * @param corners 검출된 코너 좌표 (출력)
     * @return 성공 여부
     */
    bool detectChessboard(const cv::Mat& image, std::vector<cv::Point2f>& corners);

    /**
     * @brief 캘리브레이션 수행
     * @param imagePoints 여러 이미지에서 검출된 2D 코너 좌표들
     * @param imageSize 이미지 크기
     * @param cameraMatrix 출력: 카메라 행렬 K
     * @param distCoeffs 출력: 왜곡 계수 [k1, k2, p1, p2, k3]
     * @return RMS 재투영 오차
     */
    double calibrate(const std::vector<std::vector<cv::Point2f>>& imagePoints, cv::Size imageSize,
                     cv::Mat& cameraMatrix, cv::Mat& distCoeffs);

    /**
     * @brief 캘리브레이션 결과 저장
     * @param filename 저장할 파일 이름 (.yaml)
     * @param K 카메라 행렬
     * @param dist 왜곡 계수
     */
    void saveCalibration(const std::string& filename, const cv::Mat& K, const cv::Mat& dist);

    /**
     * @brief 왜곡 보정 적용
     * @param distorted 왜곡된 이미지
     * @param undistorted 보정된 이미지 (출력)
     * @param K 카메라 행렬
     * @param dist 왜곡 계수
     */
    static void undistortImage(const cv::Mat& distorted, cv::Mat& undistorted, const cv::Mat& K,
                               const cv::Mat& dist);

    /**
     * @brief 캘리브레이션 품질 평가
     * @param rms RMS 재투영 오차
     * @return 품질 등급 문자열
     */
    static std::string evaluateQuality(double rms);

   private:
    cv::Size boundSize_;  ///< 체커보드 크기 (내부 코너 개수)
    float squareSize_;    ///< 한 칸 크기 (mm)

    /**
     * @brief 3D 객체 점 생성
     * @return 체커보드의 3D 좌표 (Z=0 평면)
     */
    std::vector<cv::Point3f> generateObjectPoints();
};

#endif  // CAMERA_CALIBRATION_BASIC_H
