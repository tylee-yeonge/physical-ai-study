/**
 * Phase 2 Week 2 - 카메라 캘리브레이션 중급 퀴즈 (정답)
 *
 * 다루는 개념:
 *   - 캘리브레이션 이미지 다양성과 정확도의 관계
 *   - 왜곡 보정 직접 구현 (정규화→왜곡 역산→복원)
 *   - undistort vs remap 성능 비교 (실시간 최적화)
 *   - 반복적 역왜곡 (Iterative Undistortion) 알고리즘
 *   - 전체 캘리브레이션 시뮬레이션 (가상 데이터 → calibrateCamera)
 *
 * 선수 지식: week2 easy (K 행렬, 왜곡 모델, RMS)
 *
 * 역왜곡의 핵심 문제:
 *   왜곡 함수 f(x) = x·(1 + k1·r² + k2·r⁴)는 알지만,
 *   역함수 f⁻¹은 해석적으로 구할 수 없다.
 *   → 반복법(Newton-like)으로 수치적 역산 필요
 */

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

// 캘리브레이션 정확도 — 이미지 다양성이 결과에 미치는 영향
//
// 캘리브레이션 품질을 결정하는 핵심 요소:
//   1. 이미지 수: 많을수록 좋음 (최소 10-20장 권장)
//   2. 각도 다양성: 다양한 기울기에서 촬영해야 모든 파라미터 관측 가능
//   3. 이미지 내 위치: 체커보드가 이미지 전 영역을 커버해야 함
//   4. 노이즈: 흔들림, 블러 최소화
//
// 왜 각도 다양성이 중요한가?
//   - 정면 촬영만: cx, cy 결정 어려움 (체커보드가 항상 비슷한 위치)
//   - 기울어진 촬영: 원근 변형이 fx, fy 분리에 도움
//   - 다양한 거리: 왜곡 계수 k1, k2 추정에 필수 (다양한 r 값 필요)
//
// ★ 좋은 캘리브레이션을 위한 실전 가이드:
//   - 체커보드를 이미지 모서리까지 포함하도록 촬영
//   - ±30° 이상 기울인 이미지 포함
//   - 가까운 거리 + 먼 거리 모두 포함
//
// TODO: 정면 촬영만(시나리오A)과 다양한 각도(시나리오B)의 RMS를 비교하세요
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
    // cv::Mat_<T>는 cv::Mat의 타입 지정 템플릿 버전
    // 일반 cv::Mat은 << 초기화 문법을 지원하지 않으므로,
    // 소규모 행렬을 리터럴로 만들 때 cv::Mat_<T>를 사용한다
    cv::Mat K_true =
        (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);
    cv::Mat dist_true = (cv::Mat_<double>(1, 5) << -0.25, 0.05, 0.0, 0.0, 0.0);
    cv::Size image_size(800, 600);

    // ✅ 정답: 시나리오 A - 정면 촬영만 (z=500, 회전 거의 없음)
    cv::RNG rng(42);
    std::vector<std::vector<cv::Point3f>> obj_points_all_A;
    std::vector<std::vector<cv::Point2f>> image_points_A;
    for (int v = 0; v < 10; v++)
    {
        // rvec: Rodrigues 회전 벡터 (축-각 방식)
        // ±0.05 rad (~3°) → 거의 정면에서 촬영한 것을 시뮬레이션
        cv::Mat rvec = (cv::Mat_<double>(3, 1) << rng.uniform(-0.05, 0.05),
                        rng.uniform(-0.05, 0.05), rng.uniform(-0.02, 0.02));
        // tvec: 카메라에서 체커보드까지의 상대 위치
        // X,Y는 소폭 변동, Z는 500으로 고정 → 일정 거리에서 촬영
        cv::Mat tvec = (cv::Mat_<double>(3, 1) << rng.uniform(-10.0, 10.0),
                        rng.uniform(-10.0, 10.0), 500.0);
        std::vector<cv::Point2f> img_pts;
        // projectPoints(3D점, 회전벡터, 이동벡터, 내부파라미터, 왜곡계수, 출력 2D점)
        // 3D 월드 좌표 → [R|t]로 카메라 좌표 변환 → 왜곡 적용 → K로 픽셀 좌표 변환
        cv::projectPoints(obj_points, rvec, tvec, K_true, dist_true, img_pts);
        // 가우시안 노이즈 추가 (표준편차 0.3 픽셀)
        // 실제 카메라의 센서 노이즈, 코너 검출 오차를 시뮬레이션
        for (auto& p : img_pts)
        {
            p.x += rng.gaussian(0.3);
            p.y += rng.gaussian(0.3);
        }
        obj_points_all_A.push_back(obj_points);
        image_points_A.push_back(img_pts);
    }

    // ✅ 정답: 시나리오 B - 다양한 각도 (z=400~600, 회전 -30°~30°)
    std::vector<std::vector<cv::Point3f>> obj_points_all_B;
    std::vector<std::vector<cv::Point2f>> image_points_B;
    for (int v = 0; v < 10; v++)
    {
        // ±0.5 rad (~30°) → 다양한 각도에서 촬영
        cv::Mat rvec = (cv::Mat_<double>(3, 1) << rng.uniform(-0.5, 0.5),
                        rng.uniform(-0.5, 0.5), rng.uniform(-0.3, 0.3));
        // Z도 400~600으로 랜덤 → 거리도 다양하게
        cv::Mat tvec = (cv::Mat_<double>(3, 1) << rng.uniform(-30.0, 30.0),
                        rng.uniform(-30.0, 30.0), rng.uniform(400.0, 600.0));
        std::vector<cv::Point2f> img_pts;
        cv::projectPoints(obj_points, rvec, tvec, K_true, dist_true, img_pts);
        for (auto& p : img_pts)
        {
            p.x += rng.gaussian(0.3);
            p.y += rng.gaussian(0.3);
        }
        obj_points_all_B.push_back(obj_points);
        image_points_B.push_back(img_pts);
    }

    // ✅ 정답: 캘리브레이션 비교
    cv::Mat K_A, dist_A;
    std::vector<cv::Mat> rvecs_A, tvecs_A;
    double rms_A = cv::calibrateCamera(obj_points_all_A, image_points_A, image_size, K_A, dist_A, rvecs_A, tvecs_A);

    cv::Mat K_B, dist_B;
    std::vector<cv::Mat> rvecs_B, tvecs_B;
    double rms_B = cv::calibrateCamera(obj_points_all_B, image_points_B, image_size, K_B, dist_B, rvecs_B, tvecs_B);

    std::cout << "📊 시나리오 A (정면만):" << std::endl;
    std::cout << "   RMS: " << rms_A << " 픽셀" << std::endl;
    std::cout << "   fx 오차: " << std::abs(K_A.at<double>(0, 0) - 600.0) << std::endl;

    std::cout << "\n📊 시나리오 B (다양한 각도):" << std::endl;
    std::cout << "   RMS: " << rms_B << " 픽셀" << std::endl;
    std::cout << "   fx 오차: " << std::abs(K_B.at<double>(0, 0) - 600.0) << std::endl;
    std::cout << "\n→ 다양한 각도가 더 정확한 캘리브레이션 결과를 줌" << std::endl;
}

// 왜곡 보정 직접 구현 — undistort의 내부 동작 이해
//
// 왜곡 보정 = 왜곡된 좌표에서 보정된(정상) 좌표를 복원하는 과정
//
// 단계:
//   1. 왜곡된 픽셀 → 정규화 좌표:
//      x = (u - cx) / fx,  y = (v - cy) / fy
//
//   2. 방사 거리 계산:
//      r² = x² + y²
//
//   3. 왜곡 계수 (이 값으로 좌표가 얼마나 왜곡되었는지 계산):
//      radial = 1 + k1·r² + k2·r⁴
//
//   4. 보정된 정규화 좌표 (역산):
//      x' = x / radial,  y' = y / radial
//      (★ 이것은 근사해: 정확한 역산은 반복법 필요 → problem4)
//
//   5. 다시 픽셀 좌표로:
//      u' = fx·x' + cx,  v' = fy·y' + cy
//
// OpenCV cv::undistortPoints와 비교하여 정확도 확인
//
// TODO: 이미지 모서리 점(700, 500)의 왜곡 보정을 직접 구현하세요
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

    // ✅ 정답: 왜곡 보정 구현
    // 1. 정규화 좌표로 변환
    double x = (distorted_point.x - cx) / fx;
    double y = (distorted_point.y - cy) / fy;
    // 2. 방사 거리 계산
    double r2 = x * x + y * y;
    // 3. 왜곡 계수 적용
    double radial_dist = 1.0 + k1 * r2 + k2 * r2 * r2;
    // 4. 보정된 정규화 좌표 (근사 역산)
    double x_corrected = x / radial_dist;
    double y_corrected = y / radial_dist;
    // 5. 픽셀 좌표로 복원
    cv::Point2f undistorted_point(fx * x_corrected + cx, fy * y_corrected + cy);

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

// 왜곡 보정 성능 최적화 — undistort vs remap
//
// cv::undistort():
//   - 호출할 때마다 모든 픽셀의 왜곡 맵핑을 재계산
//   - 간단하지만 반복 호출 시 비효율적
//
// cv::remap() + initUndistortRectifyMap():
//   - 맵(map1, map2)을 한 번만 미리 계산하여 저장
//   - 이후에는 map만 참조하여 빠르게 보정
//   - 실시간 처리에 필수적
//
// initUndistortRectifyMap(K, dist, R, newK, size, CV_32FC1, map1, map2)
//   - 각 출력 픽셀이 입력 이미지의 어디에서 값을 가져올지 미리 계산
//   - map1: x 좌표 맵, map2: y 좌표 맵
//
// remap(src, dst, map1, map2, INTER_LINEAR)
//   - 미리 계산된 맵으로 픽셀 재배치 (보간 포함)
//
// ★ 실시간 Perception (Jetson 등) 에서는 remap() 필수 — 30fps 처리 시 undistort는 병목
//
// TODO: 1000번 반복 실행으로 두 방법의 시간을 비교하세요
void problem3_performance_optimization()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 왜곡 보정 성능 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 생성
    const int kImageHeight = 600;
    const int kImageWidth = 800;
    cv::Mat test_image = cv::Mat::zeros(kImageHeight, kImageWidth, CV_8UC3);
    cv::circle(test_image, cv::Point(400, 300), 100, cv::Scalar(255, 255, 255), -1);

    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);
    cv::Mat dist = (cv::Mat_<double>(1, 5) << -0.2, 0.05, 0.0, 0.0, 0.0);

    const int kBenchmarkIterations = 1000;  // 벤치마크 반복 횟수

    // TODO: Method 1 - cv::undistort() 직접 호출
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kBenchmarkIterations; i++)
    {
        cv::Mat undistorted;
        // ✅ 정답: undistort 호출
        cv::undistort(test_image, undistorted, K, dist);
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);

    // ✅ 정답: Method 2 - remap (맵 미리 계산)
    cv::Mat map1, map2;
    cv::initUndistortRectifyMap(K, dist, cv::Mat(), K, test_image.size(), CV_32FC1, map1, map2);

    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kBenchmarkIterations; i++)
    {
        cv::Mat undistorted;
        // ✅ 정답: remap 호출
        cv::remap(test_image, undistorted, map1, map2, cv::INTER_LINEAR);
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

        std::cout << "💡 정답 해설:" << std::endl;
        std::cout << "   [두 방법의 차이]" << std::endl;
        std::cout << "   undistort(): 매 호출마다 '각 픽셀의 왜곡 좌표'를 처음부터 다시 계산" << std::endl;
        std::cout << "   remap(): initUndistortRectifyMap()에서 맵을 한 번만 계산 후 재사용" << std::endl;
        std::cout << "   → remap은 '어디서 픽셀을 가져올지'를 룩업 테이블로 미리 만들어두므로" << std::endl;
        std::cout << "     실제 보정 시에는 테이블 참조 + 보간만 하면 됨 → 훨씬 빠름" << std::endl;
        std::cout << std::endl;
        std::cout << "   [실시간 Perception 에서의 중요성]" << std::endl;
        std::cout << "   30FPS에서 매 프레임 왜곡 보정을 해야 하므로," << std::endl;
        std::cout << "   초기화 시 맵을 한 번 계산하고 remap()으로 재사용하는 것이 필수입니다." << std::endl;
    }
}

// 반복적 역왜곡 (Iterative Undistortion) — 정확한 역산 방법
//
// 문제: 왜곡 함수 f(x) = x · (1 + k1·r² + k2·r⁴)의 역함수를
//       해석적으로 구할 수 없다 (다항식의 역함수는 일반해 없음)
//
// 해결: Fixed-Point Iteration (고정점 반복법)
//
//   알고리즘:
//   x_est₀ = x_distorted              ← 초기 추정 (왜곡 좌표 그대로)
//   반복:
//     r² = x_est² + y_est²
//     radial = 1 + k1·r² + k2·r⁴
//     x_redist = x_est · radial        ← 현재 추정에 왜곡 적용
//     err = x_distorted - x_redist     ← 실제 왜곡 좌표와의 차이
//     x_est = x_est + err              ← 오차만큼 추정 보정
//   수렴 시 종료
//
// 수렴 속도:
//   - 일반적으로 5-10회 반복이면 10⁻¹⁰ 이하 오차 달성
//   - 왜곡이 크면 (k1이 크면) 더 많은 반복 필요
//
// ★ OpenCV undistortPoints 내부에서 동일한 반복법 사용
//
// TODO: 10회 반복 과정에서 오차가 줄어드는 과정을 관찰하세요
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
        // ✅ 정답: 현재 추정에 왜곡 적용
        double r2_est = x_est * x_est + y_est * y_est;
        double radial_est = 1.0 + k1 * r2_est + k2 * r2_est * r2_est;
        double x_redist = x_est * radial_est;
        double y_redist = y_est * radial_est;

        // ✅ 정답: 오차 계산 + 추정 업데이트
        double err_x = x_distorted - x_redist;
        double err_y = y_distorted - y_redist;
        x_est += err_x;
        y_est += err_y;

        double error = std::sqrt(err_x * err_x + err_y * err_y);

        std::cout << "  " << iter << "     | " << x_est << " | " << y_est << " | " << error
                  << std::endl;
    }

    double final_error =
        std::sqrt((x_est - x_orig) * (x_est - x_orig) + (y_est - y_orig) * (y_est - y_orig));
    std::cout << "\n최종 복원 오차: " << final_error << std::endl;
    std::cout << "복원된 좌표: (" << x_est << ", " << y_est << ")" << std::endl;
    std::cout << "원본 좌표:   (" << x_orig << ", " << y_orig << ")" << std::endl;

    std::cout << "\n💡 정답 해설:" << std::endl;
    std::cout << "   [알고리즘 핵심 — Fixed-Point Iteration]" << std::endl;
    std::cout << "   왜곡 함수의 역함수를 직접 구할 수 없으므로, 반복적으로 근사합니다:" << std::endl;
    std::cout << "   ① 초기 추정 = 왜곡된 좌표 그대로 (왜곡이 없었다고 가정)" << std::endl;
    std::cout << "   ② 현재 추정에 왜곡 함수를 적용 → 실제 왜곡 좌표와 비교" << std::endl;
    std::cout << "   ③ 차이(오차)만큼 추정을 보정 → 다시 ②로" << std::endl;
    std::cout << "   ④ 5-10회면 오차가 10⁻¹⁰ 이하로 수렴" << std::endl;
    std::cout << std::endl;
    std::cout << "   [왜 역함수를 못 구하나?]" << std::endl;
    std::cout << "   x_dist = x · (1 + k1·r² + k2·r⁴)에서 r²도 x에 의존하므로" << std::endl;
    std::cout << "   이 다항식의 역함수는 해석적(공식)으로 존재하지 않습니다." << std::endl;
    std::cout << "   → OpenCV의 undistortPoints()도 내부적으로 동일한 반복법을 사용합니다." << std::endl;
}

// 전체 캘리브레이션 시뮬레이션 — 가상 데이터로 end-to-end 테스트
//
// 시뮬레이션 과정:
//   1. Ground Truth K, dist 설정
//   2. 체커보드 3D 점 생성 (9×6, 30mm 간격)
//   3. 여러 포즈(rvec, tvec)에서 projectPoints → 가상 2D 관측
//   4. 노이즈 추가 (실제 환경 시뮬레이션, σ≈0.3px)
//   5. calibrateCamera(objPoints, imgPoints, ...) 호출
//   6. 추정된 K, dist와 Ground Truth 비교
//
// cv::calibrateCamera 반환값:
//   - 반환값: RMS 재투영 오차
//   - K: 추정된 내부 파라미터
//   - dist: 추정된 왜곡 계수
//   - rvecs, tvecs: 각 이미지의 외부 파라미터
//
// 기대 정확도 (10장, σ=0.3px):
//   - RMS < 0.5 px
//   - fx, fy 오차 < 5 px
//   - cx, cy 오차 < 2 px
//   - k1 오차 < 0.05
//
// ★ 실제 캘리브레이션도 동일한 과정이지만,
//   2D 점은 findChessboardCorners로 자동 검출한다
//
// TODO: 전체 캘리브레이션 파이프라인을 구현하세요
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

    // ✅ 정답: 3D 객체 점 생성
    std::vector<cv::Point3f> objp;
    for (int i = 0; i < board_size.height; i++)
        for (int j = 0; j < board_size.width; j++)
            objp.push_back(cv::Point3f(j * square_size, i * square_size, 0.0f));

    // ✅ 정답: 여러 포즈에서 2D 이미지 점 생성
    std::vector<std::vector<cv::Point3f>> all_obj_points;
    std::vector<std::vector<cv::Point2f>> all_img_points;

    cv::RNG rng_cal(42);
    for (int view = 0; view < 15; view++)
    {
        cv::Mat rvec = (cv::Mat_<double>(3, 1) << rng_cal.uniform(-0.5, 0.5),
                        rng_cal.uniform(-0.5, 0.5), rng_cal.uniform(-0.3, 0.3));
        cv::Mat tvec = (cv::Mat_<double>(3, 1) << rng_cal.uniform(-30.0, 30.0),
                        rng_cal.uniform(-30.0, 30.0), rng_cal.uniform(400.0, 600.0));
        std::vector<cv::Point2f> img_points;
        cv::projectPoints(objp, rvec, tvec, K_true, dist_true, img_points);
        for (auto& p : img_points)
        {
            p.x += rng_cal.gaussian(0.3);
            p.y += rng_cal.gaussian(0.3);
        }
        all_obj_points.push_back(objp);
        all_img_points.push_back(img_points);
    }

    // ✅ 정답: 캘리브레이션 수행
    cv::Mat K_est, dist_est;
    std::vector<cv::Mat> rvecs, tvecs;
    double rms = cv::calibrateCamera(all_obj_points, all_img_points, image_size,
                                     K_est, dist_est, rvecs, tvecs);

    std::cout << "📊 RMS 재투영 오차: " << rms << " 픽셀" << std::endl;
    std::cout << "\n추정 K:\n" << K_est << std::endl;
    std::cout << "실제 K:\n" << K_true << std::endl;
    std::cout << "\nfx 오차: " << std::abs(K_est.at<double>(0, 0) - K_true.at<double>(0, 0)) << std::endl;
    std::cout << "fy 오차: " << std::abs(K_est.at<double>(1, 1) - K_true.at<double>(1, 1)) << std::endl;
    std::cout << "cx 오차: " << std::abs(K_est.at<double>(0, 2) - K_true.at<double>(0, 2)) << std::endl;
    std::cout << "cy 오차: " << std::abs(K_est.at<double>(1, 2) - K_true.at<double>(1, 2)) << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 2 Quiz - Medium (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_calibration_accuracy();
    problem2_manual_undistortion();
    problem3_performance_optimization();
    problem4_iterative_undistortion();
    problem5_calibration_simulation();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
