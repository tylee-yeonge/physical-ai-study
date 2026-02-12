#include "basic.h"
#include <iostream>
#include <chrono>

CameraCalibrationBasic::CameraCalibrationBasic(cv::Size boardSize, float squareSize)
    : boardSize_(boardSize), squareSize_(squareSize)
{
}

bool CameraCalibrationBasic::detectChessboard(const cv::Mat& image,
                                              std::vector<cv::Point2f>& corners)
{
    // 그레이스케일로 변환
    cv::Mat gray;
    if (image.channels() == 3)
    {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        gray = image.clone();
    }

    // 체커보드 코너 검출
    // 플래그:
    // - CALIB_CB_ADAPTIVE_THRESH: 적응형 임계값 사용
    // - CALIB_CB_NORMALIZE_IMAGE: 이미지 정규화
    // - CALIB_CB_FAST_CHECK: 빠른 사전 체크
    bool found = cv::findChessboardCorners(
        gray, boardSize_, corners,
        cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK);

    if (found)
    {
        // 서브픽셀 정확도로 코너 위치 개선
        // 11x11 윈도우 내에서 코너를 더 정확히 찾음
        cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER,
                                  30,  // 최대 반복 횟수
                                  0.1  // 정확도 epsilon
        );

        cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), criteria);
    }

    return found;
}

std::vector<cv::Point3f> CameraCalibrationBasic::generateObjectPoints()
{
    std::vector<cv::Point3f> objectPoints;

    // 체커보드를 3D 공간의 Z=0 평면에 배치
    // (0,0,0), (30,0,0), (60,0,0), ... (단위: mm)
    for (int i = 0; i < boardSize_.height; i++)
    {
        for (int j = 0; j < boardSize_.width; j++)
        {
            objectPoints.push_back(cv::Point3f(j * squareSize_,  // X 좌표
                                               i * squareSize_,  // Y 좌표
                                               0.0f              // Z 좌표 (평면)
                                               ));
        }
    }

    return objectPoints;
}

double CameraCalibrationBasic::calibrate(const std::vector<std::vector<cv::Point2f>>& imagePoints,
                                         cv::Size imageSize, cv::Mat& cameraMatrix,
                                         cv::Mat& distCoeffs)
{
    // 성능 측정 시작
    auto start = std::chrono::high_resolution_clock::now();

    // 3D 객체 점 생성 (모든 이미지에서 동일)
    std::vector<cv::Point3f> objp = generateObjectPoints();

    // 모든 이미지에 대해 동일한 3D 점 사용
    std::vector<std::vector<cv::Point3f>> objectPoints;
    for (size_t i = 0; i < imagePoints.size(); i++)
    {
        objectPoints.push_back(objp);
    }

    // 각 이미지의 회전/이동 벡터 (캘리브레이션 후 출력됨)
    std::vector<cv::Mat> rvecs, tvecs;

    std::cout << "\n⚙️  캘리브레이션 수행 중..." << std::endl;
    std::cout << "   - 이미지 개수: " << imagePoints.size() << std::endl;
    std::cout << "   - 이미지 크기: " << imageSize << std::endl;

    // OpenCV 캘리브레이션 실행
    // 반환값: RMS 재투영 오차
    double rms = cv::calibrateCamera(objectPoints,  // 3D 점들
                                     imagePoints,   // 2D 점들
                                     imageSize,     // 이미지 크기
                                     cameraMatrix,  // 출력: 카메라 행렬 K
                                     distCoeffs,    // 출력: 왜곡 계수
                                     rvecs,         // 출력: 각 이미지의 회전 벡터
                                     tvecs          // 출력: 각 이미지의 이동 벡터
    );

    // 성능 측정 종료
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "   ✅ 완료! (소요시간: " << duration.count() << " ms)" << std::endl;
    std::cout << "\n📊 RMS 재투영 오차: " << rms << " 픽셀" << std::endl;
    std::cout << "   → " << evaluateQuality(rms) << std::endl;

    return rms;
}

void CameraCalibrationBasic::saveCalibration(const std::string& filename, const cv::Mat& K,
                                             const cv::Mat& dist)
{
    // OpenCV FileStorage (YAML 형식)
    cv::FileStorage fs(filename, cv::FileStorage::WRITE);

    if (!fs.isOpened())
    {
        std::cerr << "❌ 파일 저장 실패: " << filename << std::endl;
        return;
    }

    // 현재 시간 저장
    time_t rawtime;
    time(&rawtime);

    fs << "calibration_time" << asctime(localtime(&rawtime));
    fs << "camera_matrix" << K;
    fs << "distortion_coefficients" << dist;
    fs << "image_width" << 800;  // 예시 값
    fs << "image_height" << 600;

    fs.release();

    std::cout << "💾 캘리브레이션 결과 저장: " << filename << std::endl;
}

void CameraCalibrationBasic::undistortImage(const cv::Mat& distorted, cv::Mat& undistorted,
                                            const cv::Mat& K, const cv::Mat& dist)
{
    // OpenCV의 왜곡 보정 함수
    // 내부적으로 다음을 수행:
    // 1. 각 픽셀에 대해 왜곡 모델 적용
    // 2. 원본 이미지에서 보간(interpolation)
    cv::undistort(distorted, undistorted, K, dist);
}

std::string CameraCalibrationBasic::evaluateQuality(double rms)
{
    if (rms < 0.3)
    {
        return "✅ 매우 우수 (Excellent)";
    }
    else if (rms < 0.5)
    {
        return "✅ 우수 (Good)";
    }
    else if (rms < 1.0)
    {
        return "⚠️  양호 (Fair) - 사용 가능";
    }
    else
    {
        return "❌ 불량 (Poor) - 재캘리브레이션 권장";
    }
}

// 메인 함수 (데모)
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  카메라 캘리브레이션 기본 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 체커보드 설정 (9x6 내부 코너, 30mm 정사각형)
    CameraCalibrationBasic calib(cv::Size(9, 6), 30.0f);

    // 시뮬레이션: 합성 데이터 생성
    std::cout << "📸 시뮬레이션 데이터 생성 중..." << std::endl;

    // 가상의 카메라 파라미터
    cv::Mat K_true =
        (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    cv::Mat dist_true = (cv::Mat_<double>(1, 5) << -0.2, 0.05, 0.0, 0.0, 0.0);

    // 여러 각도에서의 체커보드 이미지 시뮬레이션
    std::vector<std::vector<cv::Point2f>> imagePoints;
    cv::Size imageSize(800, 600);

    // 15개 이미지 시뮬레이션
    std::vector<cv::Point3f> objectPoints = calib.generateObjectPoints();

    for (int i = 0; i < 15; i++)
    {
        std::vector<cv::Point2f> projected;

        // 랜덤 회전/이동 생성
        cv::Mat rvec = (cv::Mat_<double>(3, 1) << 0.1 * (i - 7), 0.05 * (i - 7), 0.02 * i);
        cv::Mat tvec = (cv::Mat_<double>(3, 1) << 10 * (i - 7), 10 * (i - 7), 500 + 50 * i);

        // 3D 점을 2D로 투영
        cv::projectPoints(objectPoints, rvec, tvec, K_true, dist_true, projected);

        // 노이즈 추가 (실제 측정 오차 시뮬레이션)
        for (auto& point : projected)
        {
            point.x += (rand() % 100 - 50) / 100.0;
            point.y += (rand() % 100 - 50) / 100.0;
        }

        imagePoints.push_back(projected);
    }

    std::cout << "   ✅ " << imagePoints.size() << "개 이미지 생성 완료\n" << std::endl;

    // 캘리브레이션 수행
    cv::Mat K_estimated, dist_estimated;
    double rms = calib.calibrate(imagePoints, imageSize, K_estimated, dist_estimated);

    // 결과 출력
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  캘리브레이션 결과" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "📐 카메라 행렬 K:" << std::endl;
    std::cout << K_estimated << "\n" << std::endl;

    std::cout << "📉 왜곡 계수 [k1, k2, p1, p2, k3]:" << std::endl;
    std::cout << dist_estimated << "\n" << std::endl;

    // 참값과 비교
    std::cout << "🎯 실제 값 (참조):" << std::endl;
    std::cout << "   K_true:" << std::endl;
    std::cout << K_true << "\n" << std::endl;
    std::cout << "   dist_true:" << std::endl;
    std::cout << dist_true << "\n" << std::endl;

    // 오차 계산
    double fx_error = std::abs(K_estimated.at<double>(0, 0) - K_true.at<double>(0, 0));
    double k1_error = std::abs(dist_estimated.at<double>(0) - dist_true.at<double>(0));

    std::cout << "📊 추정 오차:" << std::endl;
    std::cout << "   fx 오차: " << fx_error << " 픽셀" << std::endl;
    std::cout << "   k1 오차: " << k1_error << std::endl;

    // 결과 저장
    calib.saveCalibration("calibration_result.yaml", K_estimated, dist_estimated);

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  ✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "💡 다음 단계:" << std::endl;
    std::cout << "   1. PRACTICE.md에서 실제 카메라 캘리브레이션 실습" << std::endl;
    std::cout << "   2. quiz_easy.cpp로 기초 개념 확인" << std::endl;
    std::cout << "   3. quiz_medium.cpp로 실전 문제 풀이\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
