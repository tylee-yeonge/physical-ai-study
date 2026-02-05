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
void problem1_calibration_accuracy() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 캘리브레이션 정확도 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 체커보드 설정
    cv::Size board_size(9, 6);
    float square_size = 30.0f;
    
    // 3D 객체 점 생성
    std::vector<cv::Point3f> obj_points;
    for (int i = 0; i < board_size.height; i++) {
        for (int j = 0; j < board_size.width; j++) {
            obj_points.push_back(cv::Point3f(j * square_size, i * square_size, 0.0f));
        }
    }
    
    // 실제 카메라 파라미터 (ground truth)
    cv::Mat K_true = (cv::Mat_<double>(3, 3) << 
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
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
void problem2_manual_undistortion() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 왜곡 보정 직접 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
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
    
    std::cout << "왜곡된 점: (" << distorted_point.x << ", " << distorted_point.y << ")" << std::endl;
    
    // TODO: 왜곡 보정 구현
    // 단계:
    // 1. 정규화 좌표로 변환: x = (u - cx) / fx
    // 2. 방사 거리 계산: r² = x² + y²
    // 3. 왜곡 계수 적용: radial_dist = 1 + k1·r² + k2·r⁴
    // 4. 보정된 정규화 좌표: x' = x / radial_dist
    // 5. 픽셀 좌표로 복원: u' = fx·x' + cx
    
    cv::Point2f undistorted_point(0, 0);  // TODO: 계산
    
    std::cout << "보정된 점: (" << undistorted_point.x << ", " << undistorted_point.y << ")" << std::endl;
    
    // OpenCV 결과와 비교
    std::vector<cv::Point2f> dist_pts = {distorted_point};
    std::vector<cv::Point2f> undist_pts;
    cv::undistortPoints(dist_pts, undist_pts, K, dist, cv::noArray(), K);
    
    std::cout << "\nOpenCV 결과: (" << undist_pts[0].x << ", " << undist_pts[0].y << ")" << std::endl;
    
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
void problem3_performance_optimization() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 왜곡 보정 성능 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 테스트 이미지 생성
    cv::Mat test_image = cv::Mat::zeros(600, 800, CV_8UC3);
    cv::circle(test_image, cv::Point(400, 300), 100, cv::Scalar(255, 255, 255), -1);
    
    cv::Mat K = (cv::Mat_<double>(3, 3) << 
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
    cv::Mat dist = (cv::Mat_<double>(1, 5) << -0.2, 0.05, 0.0, 0.0, 0.0);
    
    int iterations = 1000;
    
    // TODO: Method 1 - cv::undistort() 직접 호출
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        cv::Mat undistorted;
        // TODO: cv::undistort() 호출
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    
    // TODO: Method 2 - cv::remap() 사용 (맵 미리 계산)
    cv::Mat map1, map2;
    // TODO: cv::initUndistortRectifyMap() 호출
    
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        cv::Mat undistorted;
        // TODO: cv::remap() 호출
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    
    std::cout << "📊 성능 결과 (" << iterations << "회 반복):" << std::endl;
    std::cout << "   Method 1 (undistort):  " << duration1.count() << " ms" << std::endl;
    std::cout << "   Method 2 (remap):      " << duration2.count() << " ms" << std::endl;
    
    if (duration2.count() > 0) {
        double speedup = (double)duration1.count() / duration2.count();
        std::cout << "   → 가속비: " << speedup << "x\n" << std::endl;
        
        std::cout << "💡 결론:" << std::endl;
        std::cout << "   remap()은 맵을 한 번만 계산하므로 반복 사용 시 훨씬 효율적입니다." << std::endl;
        std::cout << "   실시간 SLAM에서는 remap() 사용이 필수적입니다!" << std::endl;
    }
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 2 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
" << std::endl;
    
    problem1_calibration_accuracy();
    problem2_manual_undistortion();
    problem3_performance_optimization();
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
