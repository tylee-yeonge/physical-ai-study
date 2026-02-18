/**
 * Phase 2 Week 2 - 카메라 캘리브레이션 중급 퀴즈
 *
 * 이 퀴즈는 캘리브레이션의 실전 응용을 다룹니다.
 *
 * 난이도: ⭐⭐
 * 예상 소요 시간: 30-60분
 */

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

/**
 * 문제 1: 캘리브레이션 정확도 향상
 *
 * 다음 두 캘리브레이션 시나리오를 비교하고,
 * 어떤 것이 더 정확한 결과를 낼지 예측한 후 실제로 테스트하세요.
 *
 * 시나리오 A: 10장의 이미지, 모두 정면에서 촬영
 * 시나리오 B: 10장의 이미지, 다양한 각도에서 촬영
 *
 * TODO: 두 시나리오를 시뮬레이션하고 RMS를 비교하세요.
 */
void problem1_calibration_accuracy()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 캘리브레이션 정확도 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 체커보드 설정
    cv::Size board_size(9, 6);
    float square_size = 30.0f;

    // 3D 객체 점 생성
    std::vector<cv::Point3f> obj_points;
    for (int i = 0; i < board_size.height; i++)
    {
        for (int j = 0; j < board_size.width; j++)
        {
            obj_points.push_back(cv::Point3f(j * square_size, i * square_size, 0.0f));
        }
    }

    // 실제 카메라 파라미터 (ground truth)
    cv::Mat K_true =
        (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);
    cv::Mat dist_true = (cv::Mat_<double>(1, 5) << -0.25, 0.05, 0.0, 0.0, 0.0);
    cv::Size image_size(800, 600);

    // TODO: 시나리오 A - 정면 촬영만 (z=500, 회전 거의 없음)
    std::vector<std::vector<cv::Point2f>> image_points_A;
    // 힌트: for문으로 10장 생성, 각도 변화는 매우 작게

    // TODO: 시나리오 B - 다양한 각도 (z=400~600, 회전 -30°~30°)
    std::vector<std::vector<cv::Point2f>> image_points_B;
    // 힌트: for문으로 10장 생성, 각도 변화는 크게

    std::cout << "💡 힌트:" << std::endl;
    std::cout << "   - cv::projectPoints()로 3D → 2D 투영" << std::endl;
    std::cout << "   - cv::calibrateCamera()로 캘리브레이션" << std::endl;
    std::cout << "   - 노이즈 추가로 실제 상황 시뮬레이션\n" << std::endl;

    std::cout << "📊 예상 결과:" << std::endl;
    std::cout << "   - 시나리오 A: RMS는 낮을 수 있으나 일반화 능력 떨어짐" << std::endl;
    std::cout << "   - 시나리오 B: 다양한 각도로 더 robust한 캘리브레이션" << std::endl;
}

/**
 * 문제 2: 왜곡 보정 구현
 *
 * cv::undistort() 함수 없이 직접 왜곡 보정을 구현하세요.
 *
 * 왜곡 모델:
 *   x_corrected = x(1 + k1·r² + k2·r⁴)
 *   y_corrected = y(1 + k1·r² + k2·r⁴)
 *
 * TODO: 픽셀별로 왜곡 보정을 적용하는 함수를 작성하세요.
 */
void problem2_manual_undistortion()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 왜곡 보정 직접 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);
    cv::Mat dist = (cv::Mat_<double>(1, 5) << -0.3, 0.1, 0.0, 0.0, 0.0);

    double fx = K.at<double>(0, 0);
    double fy = K.at<double>(1, 1);
    double cx = K.at<double>(0, 2);
    double cy = K.at<double>(1, 2);

    double k1 = dist.at<double>(0);
    double k2 = dist.at<double>(1);

    std::cout << "카메라 파라미터:" << std::endl;
    std::cout << "   fx=" << fx << ", fy=" << fy << std::endl;
    std::cout << "   cx=" << cx << ", cy=" << cy << std::endl;
    std::cout << "   k1=" << k1 << ", k2=" << k2 << "\n" << std::endl;

    // TODO: 테스트 점 (이미지 모서리)
    cv::Point2f distorted_point(700, 500);

    std::cout << "왜곡된 점: (" << distorted_point.x << ", " << distorted_point.y << ")"
              << std::endl;

    // TODO: 왜곡 보정 구현
    // 단계:
    // 1. 정규화 좌표로 변환: x = (u - cx) / fx
    // 2. 방사 거리 계산: r² = x² + y²
    // 3. 왜곡 계수 적용: radial_dist = 1 + k1·r² + k2·r⁴
    // 4. 보정된 정규화 좌표: x' = x / radial_dist
    // 5. 픽셀 좌표로 복원: u' = fx·x' + cx

    cv::Point2f undistorted_point(0, 0);  // TODO: 계산

    std::cout << "보정된 점: (" << undistorted_point.x << ", " << undistorted_point.y << ")"
              << std::endl;

    // OpenCV 결과와 비교
    std::vector<cv::Point2f> dist_pts = {distorted_point};
    std::vector<cv::Point2f> undist_pts;
    cv::undistortPoints(dist_pts, undist_pts, K, dist, cv::noArray(), K);

    std::cout << "\nOpenCV 결과: (" << undist_pts[0].x << ", " << undist_pts[0].y << ")"
              << std::endl;

    double error = cv::norm(undistorted_point - undist_pts[0]);
    std::cout << "오차: " << error << " 픽셀" << std::endl;
}

/**
 * 문제 3: 성능 최적화
 *
 * cv::undistort()와 cv::remap()의 성능을 비교하세요.
 * remap은 맵을 미리 계산하여 재사용할 수 있어 반복 사용 시 훨씬 빠릅니다.
 *
 * TODO: 1000번 왜곡 보정을 수행하고 실행 시간을 비교하세요.
 */
void problem3_performance_optimization()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 왜곡 보정 성능 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 생성
    cv::Mat test_image = cv::Mat::zeros(600, 800, CV_8UC3);
    cv::circle(test_image, cv::Point(400, 300), 100, cv::Scalar(255, 255, 255), -1);

    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);
    cv::Mat dist = (cv::Mat_<double>(1, 5) << -0.2, 0.05, 0.0, 0.0, 0.0);

    int iterations = 1000;

    // TODO: Method 1 - cv::undistort() 직접 호출
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++)
    {
        cv::Mat undistorted;
        // TODO: cv::undistort() 호출
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);

    // TODO: Method 2 - cv::remap() 사용 (맵 미리 계산)
    cv::Mat map1, map2;
    // TODO: cv::initUndistortRectifyMap() 호출

    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++)
    {
        cv::Mat undistorted;
        // TODO: cv::remap() 호출
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);

    std::cout << "📊 성능 결과 (" << iterations << "회 반복):" << std::endl;
    std::cout << "   Method 1 (undistort):  " << duration1.count() << " ms" << std::endl;
    std::cout << "   Method 2 (remap):      " << duration2.count() << " ms" << std::endl;

    if (duration2.count() > 0)
    {
        double speedup = (double)duration1.count() / duration2.count();
        std::cout << "   → 가속비: " << speedup << "x\n" << std::endl;

        std::cout << "💡 결론:" << std::endl;
        std::cout << "   remap()은 맵을 한 번만 계산하므로 반복 사용 시 훨씬 효율적입니다."
                  << std::endl;
        std::cout << "   실시간 SLAM에서는 remap() 사용이 필수적입니다!" << std::endl;
    }
}

/**
 * @brief 문제 4: 반복적 역왜곡 (Iterative Undistortion)
 *
 * Newton-like 반복법으로 왜곡된 좌표를 보정합니다.
 * 왜곡 함수의 역함수는 해석적으로 구하기 어려우므로,
 * 반복적으로 추정을 개선합니다.
 *
 * 알고리즘:
 *   1. 초기 추정: 왜곡 좌표 = 보정 좌표
 *   2. 현재 추정에 왜곡 적용 → 왜곡 좌표와 비교
 *   3. 오차만큼 추정 업데이트
 *   4. 수렴할 때까지 반복
 *
 * TODO: 반복 횟수에 따른 수렴 과정을 출력하세요.
 */
void problem4_iterative_undistortion()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 반복적 역왜곡" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double fx = 600.0, fy = 600.0;
    double cx = 400.0, cy = 300.0;
    double k1 = -0.3, k2 = 0.1;

    // 원본 정규화 좌표 (왜곡 전)
    double x_orig = 0.4;
    double y_orig = 0.3;

    // 왜곡 적용 (forward)
    double r2 = x_orig * x_orig + y_orig * y_orig;
    double radial = 1.0 + k1 * r2 + k2 * r2 * r2;
    double x_distorted = x_orig * radial;
    double y_distorted = y_orig * radial;

    std::cout << "원본 정규화 좌표: (" << x_orig << ", " << y_orig << ")" << std::endl;
    std::cout << "왜곡된 정규화 좌표: (" << x_distorted << ", " << y_distorted << ")\n"
              << std::endl;

    // TODO: 반복적으로 역왜곡 수행
    // 초기 추정: 왜곡 좌표를 그대로 사용
    double x_est = x_distorted;
    double y_est = y_distorted;

    int max_iterations = 10;

    std::cout << "반복 수렴 과정:" << std::endl;
    std::cout << "  반복 |  x_est   |  y_est   |  오차" << std::endl;
    std::cout << std::string(50, '-') << std::endl;

    for (int iter = 0; iter < max_iterations; iter++)
    {
        // TODO: 현재 추정에 왜곡 적용
        // r2_est = x_est^2 + y_est^2
        // radial_est = 1 + k1 * r2_est + k2 * r2_est^2
        // x_redist = x_est * radial_est
        // y_redist = y_est * radial_est

        // TODO: 오차 계산
        // err_x = x_distorted - x_redist
        // err_y = y_distorted - y_redist

        // TODO: 추정 업데이트
        // x_est = x_est + err_x
        // y_est = y_est + err_y

        double error = 0.0;  // TODO: sqrt(err_x^2 + err_y^2)

        std::cout << "  " << iter << "     | " << x_est << " | " << y_est << " | " << error
                  << std::endl;
    }

    double final_error =
        std::sqrt((x_est - x_orig) * (x_est - x_orig) + (y_est - y_orig) * (y_est - y_orig));
    std::cout << "\n최종 복원 오차: " << final_error << std::endl;
    std::cout << "복원된 좌표: (" << x_est << ", " << y_est << ")" << std::endl;
    std::cout << "원본 좌표:   (" << x_orig << ", " << y_orig << ")" << std::endl;

    std::cout << "\n💡 관찰:" << std::endl;
    std::cout << "   보통 5-10회 반복이면 충분히 수렴합니다." << std::endl;
    std::cout << "   OpenCV의 undistortPoints도 내부적으로 이 방법을 사용합니다." << std::endl;
}

/**
 * @brief 문제 5: 캘리브레이션 시뮬레이션
 *
 * 가상 체커보드 데이터를 생성하고, cv::calibrateCamera()로
 * 카메라 파라미터를 추정한 뒤, 보정 정확도를 측정합니다.
 *
 * 과정:
 *   1. 3D 체커보드 점 생성
 *   2. 여러 포즈에서 cv::projectPoints로 2D 투영
 *   3. cv::calibrateCamera로 K, dist 추정
 *   4. 추정값과 실제값 비교
 *
 * TODO: 캘리브레이션을 수행하고 mean/max 오차를 계산하세요.
 */
void problem5_calibration_simulation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: 캘리브레이션 시뮬레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 실제 카메라 파라미터 (ground truth)
    cv::Mat K_true =
        (cv::Mat_<double>(3, 3) << 525.0, 0.0, 319.5, 0.0, 525.0, 239.5, 0.0, 0.0, 1.0);
    cv::Mat dist_true = (cv::Mat_<double>(1, 5) << -0.28, 0.09, 0.0005, -0.0002, 0.0);
    cv::Size image_size(640, 480);

    // 체커보드 설정
    cv::Size board_size(9, 6);
    float square_size = 30.0f;  // mm

    // TODO: 3D 객체 점 생성
    std::vector<cv::Point3f> objp;
    // 힌트: 이중 for문으로 (j * square_size, i * square_size, 0) 생성

    // TODO: 여러 포즈에서 2D 이미지 점 생성
    // 힌트: cv::projectPoints(objp, rvec, tvec, K_true, dist_true, img_points)
    std::vector<std::vector<cv::Point3f>> all_obj_points;
    std::vector<std::vector<cv::Point2f>> all_img_points;

    // 다양한 포즈 생성 (10장)
    // 힌트: rvec = 작은 회전, tvec = (tx, ty, tz) with tz = 400~600

    // TODO: 캘리브레이션 수행
    // cv::Mat K_estimated, dist_estimated;
    // std::vector<cv::Mat> rvecs, tvecs;
    // double rms = cv::calibrateCamera(all_obj_points, all_img_points,
    //     image_size, K_estimated, dist_estimated, rvecs, tvecs);

    // TODO: 결과 비교
    // mean/max 왜곡 보정 오차 계산

    std::cout << "💡 힌트:" << std::endl;
    std::cout << "   1. cv::projectPoints()로 가상 관측 데이터 생성" << std::endl;
    std::cout << "   2. 노이즈 추가 (0.3 픽셀 정도)" << std::endl;
    std::cout << "   3. cv::calibrateCamera()로 파라미터 추정" << std::endl;
    std::cout << "   4. 추정된 K, dist와 실제값 비교\n" << std::endl;

    std::cout << "📊 기대 결과:" << std::endl;
    std::cout << "   - RMS 재투영 오차: < 0.5 픽셀" << std::endl;
    std::cout << "   - fx, fy 오차: < 5 픽셀" << std::endl;
    std::cout << "   - cx, cy 오차: < 2 픽셀" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 2 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_calibration_accuracy();
    problem2_manual_undistortion();
    problem3_performance_optimization();
    problem4_iterative_undistortion();
    problem5_calibration_simulation();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
