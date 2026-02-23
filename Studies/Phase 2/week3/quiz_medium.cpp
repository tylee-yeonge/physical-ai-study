/**
 * Phase 2 Week 3 - 특징점 검출 중급 퀴즈
 *
 * 실전 응용 문제: 특징점 분포 개선, 스케일 불변성 등
 */

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <chrono>

/**
 * 문제 1: 균등한 특징점 분포 구현
 *
 * 이미지를 그리드로 나누고 각 셀에서 일정 개수의 특징점을 검출하여
 * 균등한 분포를 만드세요. (VINS-Fusion 방식)
 */
void problem1_uniform_distribution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 균등한 특징점 분포 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC1);
    // 테스트 패턴: 왼쪽에 많은 특징
    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 8; j++)
        {  // 왼쪽 절반만
            if ((i + j) % 2 == 0)
            {
                cv::rectangle(image, cv::Point(j * 50, i * 50),
                              cv::Point((j + 1) * 50, (i + 1) * 50), cv::Scalar(255), -1);
            }
        }
    }

    // TODO: 일반 FAST 검출 (threshold=20, NMS=true)
    std::vector<cv::KeyPoint> kp_normal;

    // TODO: 그리드 기반 균등 분포
    std::vector<cv::KeyPoint> kp_uniform;
    int grid_rows = 6, grid_cols = 8;
    int max_per_cell = 10;

    // 힌트:
    //   1. grid_rows x grid_cols 이중 루프로 각 셀의 ROI(cv::Rect) 계산
    //   2. image(roi)로 셀 영역을 잘라내고 FAST 검출
    //   3. response 기준 정렬 후 상위 max_per_cell개만 선택
    //   4. 셀 내 좌표를 전역 좌표로 변환(offset 추가)하여 kp_uniform에 추가

    std::cout << "결과:" << std::endl;
    std::cout << "   일반 검출: " << kp_normal.size() << "개" << std::endl;
    std::cout << "   균등 분포: " << kp_uniform.size() << "개\n" << std::endl;

    std::cout << "💡 왜필요한가?" << std::endl;
    std::cout << "   - 특징이 한쪽에 몰리면 포즈 추정 불안정" << std::endl;
    std::cout << "   - 균등 분포 → 더 robust한 tracking" << std::endl;
    std::cout << "   - VINS-Fusion은 6x4 그리드 사용" << std::endl;
}

/**
 * 문제 2: Adaptive Threshold
 *
 * 이미지 밝기에 따라 FAST 임계값을 자동으로 조정하여
 * 목표 특징점 개수를 달성하세요.
 */
void problem2_adaptive_threshold()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 적응형 임계값" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC1);
    for (int i = 0; i < 50; i++)
    {
        cv::Point center(rand() % 800, rand() % 600);
        int radius = 10 + rand() % 20;
        cv::circle(image, center, radius, cv::Scalar(200), -1);
    }

    int target_keypoints = 200;  // 목표: 200개

    // TODO: 이진 탐색으로 적절한 임계값 찾기
    int threshold = 20;  // 초기값
    int min_thresh = 1, max_thresh = 100;

    std::vector<cv::KeyPoint> keypoints;

    // 힌트:
    //   1. 이진 탐색 (while min_thresh <= max_thresh)
    //   2. 중간값 threshold로 FAST 검출
    //   3. 검출 개수 > 목표 → 임계값 높이기 (min_thresh = mid + 1)
    //   4. 검출 개수 ≤ 목표 → 임계값 낮추기 (max_thresh = mid - 1)

    std::cout << "결과:" << std::endl;
    std::cout << "   목표: " << target_keypoints << "개" << std::endl;
    std::cout << "   실제: " << keypoints.size() << "개" << std::endl;
    std::cout << "   임계값: " << threshold << "\n" << std::endl;

    std::cout << "💡 응용:" << std::endl;
    std::cout << "   - 밝은 환경: 임계값 ↑" << std::endl;
    std::cout << "   - 어두운 환경: 임계값 ↓" << std::endl;
    std::cout << "   - 실시간으로 조정하여 일정한 개수 유지" << std::endl;
}

/**
 * 문제 3: 멀티스케일 검출
 *
 * 이미지 피라미드를 만들고 여러 스케일에서 특징점을 검출하세요.
 * ORB는 이미 내부적으로 구현되어 있습니다.
 */
void problem3_multiscale_detection()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 멀티스케일 검출" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC1);
    // 다양한 크기의 원
    cv::circle(image, cv::Point(200, 200), 80, cv::Scalar(255), -1);
    cv::circle(image, cv::Point(500, 300), 40, cv::Scalar(255), -1);
    cv::circle(image, cv::Point(600, 450), 20, cv::Scalar(255), -1);

    // TODO: 단일 스케일
    std::vector<cv::KeyPoint> kp_single;
    cv::FAST(image, kp_single, 20, true);

    // TODO: 멀티스케일 (이미지 피라미드)
    std::vector<cv::KeyPoint> kp_multi;
    int num_levels = 4;
    float scale_factor = 1.2f;

    // 힌트:
    //   1. 각 레벨에서 scale = pow(scale_factor, level) 계산
    //   2. cv::resize로 이미지를 1/scale 크기로 축소
    //   3. 축소된 이미지에서 FAST 검출
    //   4. 검출된 keypoint의 pt와 size에 scale을 곱하여 원본 좌표로 복원
    //   5. kp_multi에 추가

    std::cout << "결과:" << std::endl;
    std::cout << "   단일 스케일: " << kp_single.size() << "개" << std::endl;
    std::cout << "   멀티스케일: " << kp_multi.size() << "개\n" << std::endl;

    std::cout << "💡 장점:" << std::endl;
    std::cout << "   - 다양한 크기의 물체 검출" << std::endl;
    std::cout << "   - 스케일 불변성 (물체가 가까워져도 추적 가능)" << std::endl;
    std::cout << "   - ORB, SIFT는 자동으로 수행" << std::endl;
}

/**
 * @brief Harris 코너 검출 직접 구현
 *
 * Sobel로 Ix, Iy 그래디언트를 구하고 Structure Tensor 요소(Ix^2, Iy^2, IxIy)에
 * 가우시안 블러를 적용한 후 Harris 응답 R을 계산하세요.
 * OpenCV cornerHarris() 결과와 비교합니다.
 */
void problem4_harris_implementation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Harris 코너 검출 직접 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 생성 (체커보드)
    cv::Mat image = cv::Mat::zeros(400, 400, CV_8UC1);
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if ((i + j) % 2 == 0)
            {
                cv::rectangle(image, cv::Point(j * 50, i * 50),
                              cv::Point((j + 1) * 50, (i + 1) * 50), cv::Scalar(255), -1);
            }
        }
    }

    cv::Mat image_f;
    image.convertTo(image_f, CV_32F);

    // TODO: 1. Sobel 필터로 Ix, Iy 계산
    cv::Mat Ix, Iy;
    // 힌트: Sobel 함수로 x방향, y방향 그래디언트를 각각 구하세요 (커널 크기 3)

    // TODO: 2. Structure Tensor 요소 계산
    cv::Mat Ixx, Iyy, Ixy;
    // 힌트: 각 그래디언트의 요소별 곱으로 Ix², Iy², Ix·Iy 행렬을 만드세요

    // TODO: 3. 가우시안 블러 적용
    int ksize = 5;
    // 힌트: 각 Structure Tensor 요소에 가우시안 블러를 적용하여 윈도우 내 가중 합산하세요

    // TODO: 4. Harris 응답 R = det(M) - k * trace(M)^2
    double k = 0.04;
    cv::Mat harris_response;
    // 힌트: det(M)과 trace(M)을 구한 뒤 Harris 응답 공식을 적용하세요

    // OpenCV cornerHarris로 비교
    cv::Mat harris_cv;
    cv::cornerHarris(image_f, harris_cv, 2, 3, k);

    std::cout << "📊 직접 구현 결과:" << std::endl;
    if (!harris_response.empty())
    {
        double min_val, max_val;
        cv::minMaxLoc(harris_response, &min_val, &max_val);
        std::cout << "   응답 범위: [" << min_val << ", " << max_val << "]" << std::endl;
    }
    else
    {
        std::cout << "   (TODO를 구현하세요)" << std::endl;
    }

    std::cout << "\n📊 OpenCV cornerHarris 결과:" << std::endl;
    double cv_min, cv_max;
    cv::minMaxLoc(harris_cv, &cv_min, &cv_max);
    std::cout << "   응답 범위: [" << cv_min << ", " << cv_max << "]\n" << std::endl;

    std::cout << "💡 구현 파이프라인:" << std::endl;
    std::cout << "   Sobel(Ix, Iy) → Ix^2, Iy^2, IxIy → GaussianBlur → R 계산" << std::endl;
    std::cout << "   직접 구현 결과와 OpenCV 결과가 유사해야 합니다." << std::endl;
}

/**
 * @brief Non-Maximum Suppression 직접 구현
 *
 * Harris 응답 맵에서 지역 최대값만 남기는 NMS를 구현하세요.
 * 적용 전/후 키포인트 수를 비교하고 제거 비율을 계산합니다.
 */
void problem5_nms_implementation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: NMS 직접 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 + Harris 응답 계산
    cv::Mat image = cv::Mat::zeros(400, 400, CV_8UC1);
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if ((i + j) % 2 == 0)
            {
                cv::rectangle(image, cv::Point(j * 50, i * 50),
                              cv::Point((j + 1) * 50, (i + 1) * 50), cv::Scalar(255), -1);
            }
        }
    }

    cv::Mat image_f;
    image.convertTo(image_f, CV_32F);

    cv::Mat response;
    cv::cornerHarris(image_f, response, 2, 3, 0.04);

    // NMS 전: 임계값만 적용
    double threshold_ratio = 0.01;
    double r_min, r_max;
    cv::minMaxLoc(response, &r_min, &r_max);
    double thresh_value = threshold_ratio * r_max;

    int count_before = 0;
    for (int y = 0; y < response.rows; y++)
    {
        for (int x = 0; x < response.cols; x++)
        {
            if (response.at<float>(y, x) > thresh_value)
            {
                count_before++;
            }
        }
    }

    // TODO: NMS 구현
    // 윈도우 크기
    int window_size = 7;
    int half_win = window_size / 2;
    int count_after = 0;

    // 힌트:
    //   1. 이미지의 각 픽셀 (y, x)에 대해 순회
    //   2. response.at<float>(y, x) > thresh_value 인지 확인
    //   3. (y, x) 주변 window_size x window_size 윈도우에서 최대값을 찾기
    //   4. 현재 값이 윈도우 내 최대값과 같으면 → 지역 최대 → 유지 (count_after++)
    //   5. 경계 처리에 주의 (half_win만큼 안쪽에서 순회)

    std::cout << "NMS 결과 (window=" << window_size << "):" << std::endl;
    std::cout << "   NMS 전: " << count_before << "개" << std::endl;
    std::cout << "   NMS 후: " << count_after << "개" << std::endl;

    if (count_before > 0)
    {
        double reduction = 100.0 * (1.0 - double(count_after) / count_before);
        std::cout << "   제거 비율: " << reduction << "%\n" << std::endl;
    }

    std::cout << "💡 NMS 핵심:" << std::endl;
    std::cout << "   - 지역 윈도우 내 최대값만 남김" << std::endl;
    std::cout << "   - 같은 코너에 대한 중복 검출 제거" << std::endl;
    std::cout << "   - 윈도우 크기 ↑ → 더 희소한 결과" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 3 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_uniform_distribution();
    problem2_adaptive_threshold();
    problem3_multiscale_detection();
    problem4_harris_implementation();
    problem5_nms_implementation();

    std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
