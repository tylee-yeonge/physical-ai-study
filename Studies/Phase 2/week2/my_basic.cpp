/**
 * Phase 2 Week 2 - 카메라 캘리브레이션 직접 구현
 *
 * basic.h의 CameraCalibrationBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make my_basic
 * 실행: ./my_basic (각 Step 구현 후 실행하여 확인)
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │                    구현 순서 가이드                            │
 * ├──────┬──────────────────────┬────────┬───────────────────────┤
 * │ Step │ 함수                  │ 난이도 │ 검증 방법              │
 * ├──────┼──────────────────────┼────────┼───────────────────────┤
 * │  1   │ 생성자                │ 쉬움   │ ./my_basic            │
 * │  2   │ evaluateQuality      │ 쉬움   │ ./my_basic            │
 * │  3   │ generateObjectPoints │ 쉬움   │ ./my_basic            │
 * │  4   │ undistortImage       │ 쉬움   │ ./my_basic            │
 * │  5   │ detectChessboard     │ 보통   │ ./my_basic            │
 * │  6   │ saveCalibration      │ 보통   │ ./my_basic            │
 * │  7   │ calibrate            │ 어려움 │ ./my_basic            │
 * └──────┴──────────────────────┴────────┴───────────────────────┘
 *
 * 💡 각 Step 구현 후 ./my_basic을 실행하면 해당 단계의 ✅/❌를 확인할 수 있습니다.
 */
#include "basic.h"
#include <iostream>
#include <cmath>
#include <random>
#include <fstream>

CameraCalibrationBasic::CameraCalibrationBasic(cv::Size boardSize, float squareSize)
{
    // [Step 1] boardSize_, squareSize_ 멤버 변수 초기화
    // 힌트: 이니셜라이저 리스트 또는 대입문 사용
    // 참고: basic.cpp 생성자 (멤버 이니셜라이저 리스트 사용)
    // 기대값: boardSize_ = boardSize, squareSize_ = squareSize
}

bool CameraCalibrationBasic::detectChessboard(const cv::Mat& image,
                                              std::vector<cv::Point2f>& corners)
{
    // [Step 5] 체커보드 코너 검출
    // 1) 그레이스케일 변환 (image.channels() == 3이면)
    // 2) cv::findChessboardCorners(gray, boardSize_, corners, flags)
    //    flags: CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE | CALIB_CB_FAST_CHECK
    // 3) 검출 성공 시 cv::cornerSubPix()로 서브픽셀 정확도 개선
    // 참고: basic.cpp의 detectChessboard()
    // 기대값: 체커보드 이미지→true, 빈 이미지→false
    return false;
}

double CameraCalibrationBasic::calibrate(const std::vector<std::vector<cv::Point2f>>& imagePoints,
                                         cv::Size imageSize, cv::Mat& cameraMatrix,
                                         cv::Mat& distCoeffs)
{
    // [Step 7] 캘리브레이션 수행 (가장 어려운 단계!)
    // 1) generateObjectPoints()로 3D 점 생성
    // 2) 모든 이미지에 동일한 objectPoints 배열 구성
    // 3) cv::calibrateCamera(objectPoints, imagePoints, imageSize,
    //                        cameraMatrix, distCoeffs, rvecs, tvecs)
    // 참고: basic.cpp의 calibrate()
    // 기대값: 시뮬레이션 데이터에서 RMS < 1.0
    return -1.0;
}

void CameraCalibrationBasic::saveCalibration(const std::string& filename, const cv::Mat& K,
                                             const cv::Mat& dist, cv::Size imageSize)
{
    // [Step 6] 캘리브레이션 결과를 YAML 파일로 저장
    // 1) cv::FileStorage fs(filename, cv::FileStorage::WRITE)
    // 2) fs << "camera_matrix" << K
    // 3) fs << "distortion_coefficients" << dist
    // 4) imageSize.width, imageSize.height도 저장
    // 참고: basic.cpp의 saveCalibration()
    // 기대값: YAML 파일 생성됨
}

void CameraCalibrationBasic::undistortImage(const cv::Mat& distorted, cv::Mat& undistorted,
                                            const cv::Mat& K, const cv::Mat& dist)
{
    // [Step 4] 왜곡 보정 (한 줄!)
    // 힌트: cv::undistort(distorted, undistorted, K, dist)
    // 참고: basic.cpp의 undistortImage()
    // 기대값: 출력 이미지 크기 = 입력 이미지 크기
}

std::string CameraCalibrationBasic::evaluateQuality(double rms)
{
    // [Step 2] RMS 값에 따라 품질 등급 반환
    // 기준: < 0.3 "매우 우수", < 0.5 "우수", < 1.0 "양호", 그 외 "불량"
    // 참고: basic.cpp의 evaluateQuality(), kRmsExcellent/kRmsGood/kRmsFair 상수
    // 기대값: 0.25 → "매우 우수" 포함, 1.5 → "불량" 포함
    return "Unknown";
}

std::vector<cv::Point3f> CameraCalibrationBasic::generateObjectPoints()
{
    // [Step 3] 체커보드 3D 좌표 생성 (Z=0 평면)
    // 이중 for 루프: i=[0, boardSize_.height), j=[0, boardSize_.width)
    // 각 점: (j * squareSize_, i * squareSize_, 0.0f)
    // 참고: basic.cpp의 generateObjectPoints()
    // 기대값: 9×6=54개 점, 첫 점=(0,0,0), 간격=squareSize_
    return {};
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 카메라 캘리브레이션 - 단계별 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ── Step 1: 생성자 ──────────────────────────
    std::cout << "Step 1: 생성자" << std::endl;
    CameraCalibrationBasic calib(cv::Size(9, 6), 30.0f);
    std::cout << "   객체 생성 완료 (9×6, 30mm)" << std::endl;
    std::cout << "   → Step 3에서 generateObjectPoints()로 간접 검증\n" << std::endl;

    // ── Step 2: evaluateQuality ─────────────────
    std::cout << "Step 2: evaluateQuality" << std::endl;
    std::string q1 = CameraCalibrationBasic::evaluateQuality(0.25);
    std::string q2 = CameraCalibrationBasic::evaluateQuality(0.4);
    std::string q3 = CameraCalibrationBasic::evaluateQuality(0.8);
    std::string q4 = CameraCalibrationBasic::evaluateQuality(1.5);
    std::cout << "   RMS 0.25 → " << q1
              << (q1.find("우수") != std::string::npos ? " ✅" : " ❌ 기대: 매우 우수")
              << std::endl;
    std::cout << "   RMS 0.40 → " << q2
              << (q2 != "Unknown" && q2.find("우수") != std::string::npos ? " ✅"
                                                                          : " ❌ 기대: 우수")
              << std::endl;
    std::cout << "   RMS 0.80 → " << q3
              << (q3.find("양호") != std::string::npos ? " ✅" : " ❌ 기대: 양호")
              << std::endl;
    std::cout << "   RMS 1.50 → " << q4
              << (q4.find("불량") != std::string::npos ? " ✅" : " ❌ 기대: 불량")
              << std::endl;

    // ── Step 3: generateObjectPoints ────────────
    std::cout << "\nStep 3: generateObjectPoints" << std::endl;
    auto obj_pts = calib.generateObjectPoints();
    std::cout << "   포인트 수: " << obj_pts.size()
              << (obj_pts.size() == 54 ? " ✅ (9×6=54)" : " ❌ 기대: 54") << std::endl;
    if (!obj_pts.empty())
    {
        auto& first = obj_pts[0];
        bool first_ok = (first.x == 0 && first.y == 0 && first.z == 0);
        std::cout << "   첫 점: (" << first.x << ", " << first.y << ", " << first.z << ")"
                  << (first_ok ? " ✅" : " ❌ 기대: (0,0,0)") << std::endl;
        if (obj_pts.size() > 1)
        {
            auto& second = obj_pts[1];
            bool gap_ok = (std::abs(second.x - 30.0f) < 0.01f);
            std::cout << "   둘째 점: (" << second.x << ", " << second.y << ", " << second.z
                      << ")" << (gap_ok ? " ✅ 간격=30mm" : " ❌ 기대: (30,0,0)") << std::endl;
        }
    }

    // ── Step 4: undistortImage ──────────────────
    std::cout << "\nStep 4: undistortImage" << std::endl;
    cv::Mat test_img = cv::Mat::zeros(600, 800, CV_8UC3);
    cv::Mat undist;
    cv::Mat K_test = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);
    cv::Mat dist_test = (cv::Mat_<double>(1, 5) << -0.2, 0.05, 0, 0, 0);
    CameraCalibrationBasic::undistortImage(test_img, undist, K_test, dist_test);
    bool undist_ok = (!undist.empty() && undist.size() == test_img.size());
    std::cout << "   입력: 800×600, 출력: "
              << (undist.empty() ? "비어있음"
                                 : std::to_string(undist.cols) + "×" + std::to_string(undist.rows))
              << (undist_ok ? " ✅" : " ❌ 기대: 800×600") << std::endl;

    // ── Step 5: detectChessboard ────────────────
    std::cout << "\nStep 5: detectChessboard" << std::endl;
    cv::Mat empty_img = cv::Mat::zeros(480, 640, CV_8UC1);
    std::vector<cv::Point2f> corners;
    bool detected = calib.detectChessboard(empty_img, corners);
    std::cout << "   빈 이미지 검출: " << (detected ? "true" : "false")
              << (!detected ? " ✅ (빈 이미지 → false)" : " ❌ 기대: false") << std::endl;

    // ── Step 6: saveCalibration ─────────────────
    std::cout << "\nStep 6: saveCalibration" << std::endl;
    std::string test_file = "test_calib_result.yaml";
    calib.saveCalibration(test_file, K_test, dist_test, cv::Size(800, 600));
    std::ifstream check_file(test_file);
    bool file_ok = check_file.good();
    check_file.close();
    std::cout << "   파일 생성: " << test_file << (file_ok ? " ✅" : " ❌ 파일 없음")
              << std::endl;
    if (file_ok)
    {
        std::remove(test_file.c_str());
    }

    // ── Step 7: calibrate ───────────────────────
    std::cout << "\nStep 7: calibrate (시뮬레이션)" << std::endl;
    {
        auto objp = calib.generateObjectPoints();
        if (objp.empty())
        {
            std::cout << "   ⚠️  Step 3 (generateObjectPoints) 먼저 구현하세요!" << std::endl;
        }
        else
        {
            // 시뮬레이션 데이터 생성 (basic.cpp와 동일한 방법)
            cv::Mat K_true = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);
            cv::Mat dist_true = (cv::Mat_<double>(1, 5) << -0.2, 0.05, 0, 0, 0);
            std::mt19937 rng(42);
            std::normal_distribution<double> noise(0.0, 0.5);
            std::vector<std::vector<cv::Point2f>> imagePoints;
            for (int i = 0; i < 15; i++)
            {
                std::vector<cv::Point2f> projected;
                cv::Mat rvec =
                    (cv::Mat_<double>(3, 1) << 0.1 * (i - 7), 0.05 * (i - 7), 0.02 * i);
                cv::Mat tvec =
                    (cv::Mat_<double>(3, 1) << 10 * (i - 7), 10 * (i - 7), 500 + 50 * i);
                cv::projectPoints(objp, rvec, tvec, K_true, dist_true, projected);
                for (auto& pt : projected)
                {
                    pt.x += static_cast<float>(noise(rng));
                    pt.y += static_cast<float>(noise(rng));
                }
                imagePoints.push_back(projected);
            }

            cv::Mat K_est, dist_est;
            double rms = calib.calibrate(imagePoints, cv::Size(800, 600), K_est, dist_est);
            std::cout << "   RMS: " << rms
                      << (rms > 0 && rms < 1.0 ? " ✅ (< 1.0)" : " ❌ 기대: 0 < RMS < 1.0")
                      << std::endl;
            if (rms > 0 && !K_est.empty())
            {
                double fx_err = std::abs(K_est.at<double>(0, 0) - 600.0);
                std::cout << "   fx 오차: " << fx_err << " 픽셀"
                          << (fx_err < 5.0 ? " ✅" : " ❌ 기대: < 5.0") << std::endl;
            }
        }
    }

    // ── 요약 ────────────────────────────────────
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  모든 Step ✅ 확인 후 → quiz_easy.cpp, quiz_medium.cpp" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
