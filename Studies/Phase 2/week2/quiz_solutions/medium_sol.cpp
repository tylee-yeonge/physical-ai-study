/**
 * Phase 2 Week 2 - 중급 퀴즈 정답 (요약 버전)
 *
 * 중급 퀴즈는 코드량이 많으므로, 핵심 알고리즘만 제공합니다.
 * 전체 구현은 직접 완성해보세요!
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <chrono>

/**
 * 문제 2 정답: 왜곡 보정 직접 구현
 */
cv::Point2f manualUndistort(const cv::Point2f& distorted, const cv::Mat& K, const cv::Mat& dist)
{
    double fx = K.at<double>(0, 0);
    double fy = K.at<double>(1, 1);
    double cx = K.at<double>(0, 2);
    double cy = K.at<double>(1, 2);

    double k1 = dist.at<double>(0);
    double k2 = dist.at<double>(1);

    // 1. 정규화 좌표로 변환
    double x = (distorted.x - cx) / fx;
    double y = (distorted.y - cy) / fy;

    // 2. 반복적으로 왜곡 보정 (Newton-Raphson 방법)
    // 간단한 근사: 역방향 왜곡 적용
    double r2 = x * x + y * y;
    double radial_dist = 1.0 + k1 * r2 + k2 * r2 * r2;

    // 3. 보정된 정규화 좌표
    double x_corrected = x / radial_dist;
    double y_corrected = y / radial_dist;

    // 4. 픽셀 좌표로 복원
    cv::Point2f undistorted;
    undistorted.x = fx * x_corrected + cx;
    undistorted.y = fy * y_corrected + cy;

    return undistorted;
}

/**
 * 문제 3 정답: 성능 최적화
 */
void performanceComparison()
{
    cv::Mat test_image = cv::Mat::zeros(600, 800, CV_8UC3);
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);
    cv::Mat dist = (cv::Mat_<double>(1, 5) << -0.2, 0.05, 0, 0, 0);

    int iterations = 1000;

    // Method 1: undistort
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++)
    {
        cv::Mat undistorted;
        cv::undistort(test_image, undistorted, K, dist);
    }
    auto end1 = std::chrono::high_resolution_clock::now();

    // Method 2: remap (맵 미리 계산)
    cv::Mat map1, map2;
    cv::initUndistortRectifyMap(K, dist, cv::Mat(), K, test_image.size(), CV_32FC1, map1, map2);

    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++)
    {
        cv::Mat undistorted;
        cv::remap(test_image, undistorted, map1, map2, cv::INTER_LINEAR);
    }
    auto end2 = std::chrono::high_resolution_clock::now();

    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);

    std::cout << "Method 1 (undistort): " << duration1.count() << " ms" << std::endl;
    std::cout << "Method 2 (remap):     " << duration2.count() << " ms" << std::endl;
    std::cout << "가속비: " << (double)duration1.count() / duration2.count() << "x" << std::endl;
}

/**
 * @brief 문제 4 정답: 반복적 역왜곡
 *
 * Newton-like 반복법으로 왜곡된 정규화 좌표를 복원합니다.
 */
void iterativeUndistortion()
{
    std::cout << "\n━━━ 문제 4 정답: 반복적 역왜곡 ━━━\n" << std::endl;

    double k1 = -0.3, k2 = 0.1;

    // 원본 정규화 좌표
    double x_orig = 0.4, y_orig = 0.3;

    // 왜곡 적용 (forward)
    double r2 = x_orig * x_orig + y_orig * y_orig;
    double radial = 1.0 + k1 * r2 + k2 * r2 * r2;
    double x_distorted = x_orig * radial;
    double y_distorted = y_orig * radial;

    std::cout << "원본: (" << x_orig << ", " << y_orig << ")" << std::endl;
    std::cout << "왜곡: (" << x_distorted << ", " << y_distorted << ")\n" << std::endl;

    // 반복적 역왜곡
    double x_est = x_distorted;
    double y_est = y_distorted;

    std::cout << "반복 수렴:" << std::endl;
    for (int iter = 0; iter < 10; iter++)
    {
        double r2_est = x_est * x_est + y_est * y_est;
        double radial_est = 1.0 + k1 * r2_est + k2 * r2_est * r2_est;
        double x_redist = x_est * radial_est;
        double y_redist = y_est * radial_est;

        double err_x = x_distorted - x_redist;
        double err_y = y_distorted - y_redist;

        x_est += err_x;
        y_est += err_y;

        double error = std::sqrt(err_x * err_x + err_y * err_y);
        std::cout << "  iter " << iter << ": (" << x_est << ", " << y_est << ") err=" << error
                  << std::endl;
    }

    double final_error =
        std::sqrt((x_est - x_orig) * (x_est - x_orig) + (y_est - y_orig) * (y_est - y_orig));
    std::cout << "\n최종 오차: " << final_error << std::endl;
}

/**
 * @brief 문제 5 정답: 캘리브레이션 시뮬레이션
 *
 * 가상 체커보드 데이터로 cv::calibrateCamera를 호출하고
 * mean/max 재투영 오차를 계산합니다.
 */
void calibrationSimulation()
{
    std::cout << "\n━━━ 문제 5 정답: 캘리브레이션 시뮬레이션 ━━━\n" << std::endl;

    // 실제 파라미터
    cv::Mat K_true =
        (cv::Mat_<double>(3, 3) << 525.0, 0.0, 319.5, 0.0, 525.0, 239.5, 0.0, 0.0, 1.0);
    cv::Mat dist_true = (cv::Mat_<double>(1, 5) << -0.28, 0.09, 0.0005, -0.0002, 0.0);
    cv::Size image_size(640, 480);

    // 체커보드 점 생성
    cv::Size board_size(9, 6);
    float square_size = 30.0f;

    std::vector<cv::Point3f> objp;
    for (int i = 0; i < board_size.height; i++)
    {
        for (int j = 0; j < board_size.width; j++)
        {
            objp.push_back(cv::Point3f(j * square_size, i * square_size, 0.0f));
        }
    }

    // 여러 포즈에서 투영
    std::vector<std::vector<cv::Point3f>> all_obj_points;
    std::vector<std::vector<cv::Point2f>> all_img_points;

    cv::RNG rng(42);
    for (int view = 0; view < 15; view++)
    {
        cv::Mat rvec = (cv::Mat_<double>(3, 1) << rng.uniform(-0.5, 0.5),
                        rng.uniform(-0.5, 0.5), rng.uniform(-0.3, 0.3));
        cv::Mat tvec = (cv::Mat_<double>(3, 1) << rng.uniform(-30.0, 30.0),
                        rng.uniform(-30.0, 30.0), rng.uniform(400.0, 600.0));

        std::vector<cv::Point2f> img_points;
        cv::projectPoints(objp, rvec, tvec, K_true, dist_true, img_points);

        // 노이즈 추가
        for (auto& p : img_points)
        {
            p.x += rng.gaussian(0.3);
            p.y += rng.gaussian(0.3);
        }

        all_obj_points.push_back(objp);
        all_img_points.push_back(img_points);
    }

    // 캘리브레이션
    cv::Mat K_est, dist_est;
    std::vector<cv::Mat> rvecs, tvecs;
    double rms = cv::calibrateCamera(all_obj_points, all_img_points, image_size, K_est, dist_est,
                                     rvecs, tvecs);

    std::cout << "RMS 재투영 오차: " << rms << " 픽셀" << std::endl;
    std::cout << "\n추정 K:\n" << K_est << std::endl;
    std::cout << "실제 K:\n" << K_true << std::endl;

    std::cout << "\nfx 오차: " << std::abs(K_est.at<double>(0, 0) - K_true.at<double>(0, 0))
              << std::endl;
    std::cout << "fy 오차: " << std::abs(K_est.at<double>(1, 1) - K_true.at<double>(1, 1))
              << std::endl;
    std::cout << "cx 오차: " << std::abs(K_est.at<double>(0, 2) - K_true.at<double>(0, 2))
              << std::endl;
    std::cout << "cy 오차: " << std::abs(K_est.at<double>(1, 2) - K_true.at<double>(1, 2))
              << std::endl;

    // 왜곡 보정 오차 측정
    std::vector<double> errors;
    for (size_t v = 0; v < all_obj_points.size(); v++)
    {
        std::vector<cv::Point2f> reprojected;
        cv::projectPoints(all_obj_points[v], rvecs[v], tvecs[v], K_est, dist_est, reprojected);

        for (size_t p = 0; p < reprojected.size(); p++)
        {
            double err = cv::norm(reprojected[p] - all_img_points[v][p]);
            errors.push_back(err);
        }
    }

    double mean_err = 0.0;
    double max_err = 0.0;
    for (double e : errors)
    {
        mean_err += e;
        if (e > max_err)
        {
            max_err = e;
        }
    }
    mean_err /= errors.size();

    std::cout << "\n보정 후 재투영 오차:" << std::endl;
    std::cout << "  mean: " << mean_err << " 픽셀" << std::endl;
    std::cout << "  max:  " << max_err << " 픽셀" << std::endl;
}

int main()
{
    std::cout << "\n중급 퀴즈 정답 (핵심 알고리즘)\n" << std::endl;

    // 문제 2 테스트
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);
    cv::Mat dist = (cv::Mat_<double>(1, 5) << -0.3, 0.1, 0, 0, 0);

    cv::Point2f test_point(700, 500);
    cv::Point2f result = manualUndistort(test_point, K, dist);

    std::cout << "왜곡 보정 결과: (" << result.x << ", " << result.y << ")\n" << std::endl;

    // 문제 3 테스트
    performanceComparison();

    // 문제 4 테스트
    iterativeUndistortion();

    // 문제 5 테스트
    calibrationSimulation();

    return 0;
}
