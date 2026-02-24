/**
 * Phase 2 Week 3 - 특징점 검출 중급 퀴즈 (정답)
 *
 * 다루는 개념:
 *   - 균등한 특징점 분포 (Grid-based detection, VINS-Fusion 방식)
 *   - 적응형 임계값 (목표 개수 달성을 위한 이진 탐색)
 *   - 멀티스케일 검출 (이미지 피라미드, 스케일 불변성)
 *   - Harris 코너 검출 직접 구현 (Sobel → Structure Tensor → R)
 *   - NMS 직접 구현 (지역 최대값 필터링)
 *
 * 선수 지식: week3 easy (FAST, ORB, NMS, Harris 이론)
 *
 * 이 퀴즈는 특징점 검출의 실전 최적화 기법을 다룬다.
 * 단순히 "검출"만이 아닌, "얼마나 좋은 특징점을 선택하느냐"가
 * SLAM 성능을 결정하는 핵심 요소이다.
 */

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <chrono>

// 균등한 특징점 분포 구현 — Grid-based Detection (VINS-Fusion 방식)
//
// 문제: 텍스처가 풍부한 영역에 특징점이 몰리고,
//       텍스처 없는 영역에는 특징점이 거의 없음
//       → 한쪽으로 편중되면 포즈 추정이 특정 방향에서만 제약됨
//
// 해결: Grid-based Uniform Distribution
//   1. 이미지를 grid_rows × grid_cols 격자로 분할
//   2. 각 셀에서 독립적으로 FAST 검출
//   3. response 기준 정렬 후 상위 max_per_cell개만 선택
//   4. 전역 좌표로 변환하여 합침
//
// 장점:
//   - 이미지 전체에 골고루 특징점 분포
//   - 포즈 추정의 기하학적 제약이 균등해짐
//   - 추적 안정성 향상
//
// ★ ORB-SLAM은 Quadtree 기반, VINS-Fusion은 Grid 기반 분포 사용
//
// TODO: 6×8 그리드에서 셀당 10개씩 검출하세요
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

    // ✅ 정답: 일반 FAST 검출
    std::vector<cv::KeyPoint> kp_normal;
    cv::FAST(image, kp_normal, 20, true);

    // ✅ 정답: 그리드 기반 균등 분포
    std::vector<cv::KeyPoint> kp_uniform;
    int grid_rows = 6, grid_cols = 8;
    int max_per_cell = 10;

    int cell_h = image.rows / grid_rows;
    int cell_w = image.cols / grid_cols;

    for (int i = 0; i < grid_rows; i++)
    {
        for (int j = 0; j < grid_cols; j++)
        {
            cv::Rect roi(j * cell_w, i * cell_h, cell_w, cell_h);
            cv::Mat cell = image(roi);
            std::vector<cv::KeyPoint> cell_kp;
            cv::FAST(cell, cell_kp, 20, true);

            std::sort(cell_kp.begin(), cell_kp.end(),
                      [](const cv::KeyPoint& a, const cv::KeyPoint& b)
                      { return a.response > b.response; });

            int count = std::min((int)cell_kp.size(), max_per_cell);
            for (int k = 0; k < count; k++)
            {
                cell_kp[k].pt.x += roi.x;
                cell_kp[k].pt.y += roi.y;
                kp_uniform.push_back(cell_kp[k]);
            }
        }
    }

    std::cout << "결과:" << std::endl;
    std::cout << "   일반 검출: " << kp_normal.size() << "개" << std::endl;
    std::cout << "   균등 분포: " << kp_uniform.size() << "개\n" << std::endl;

    std::cout << "💡 왜필요한가?" << std::endl;
    std::cout << "   - 특징이 한쪽에 몰리면 포즈 추정 불안정" << std::endl;
    std::cout << "   - 균등 분포 → 더 robust한 tracking" << std::endl;
    std::cout << "   - VINS-Fusion은 6x4 그리드 사용" << std::endl;
}

// 적응형 임계값 — 이진 탐색으로 목표 특징점 수 달성
//
// 문제: FAST 임계값이 고정되면 환경에 따라 검출 수가 크게 변동
//   - 밝은 고대비 장면: 너무 많은 특징점 → 처리 시간 초과
//   - 어두운 저대비 장면: 너무 적은 특징점 → 추적 실패
//
// 해결: 이진 탐색(Binary Search)으로 임계값 자동 조정
//   목표: 200개 특징점 달성
//   1. min_thresh=1, max_thresh=100 범위 설정
//   2. mid = (min + max) / 2
//   3. threshold=mid로 FAST 검출
//   4. 검출 수 > 목표 → 임계값 높이기 (min = mid + 1)
//      검출 수 < 목표 → 임계값 낮추기 (max = mid - 1)
//   5. 수렴할 때까지 반복 (log2(100) ≈ 7회)
//
// ★ ORB-SLAM에서는 octave별로 목표 특징점 수를 배분하여
//   전체 목표(예: 2000개)를 달성하는 전략 사용
//
// TODO: 이진 탐색으로 200개에 가까운 임계값을 찾으세요
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

    // ✅ 정답: 이진 탐색으로 적절한 임계값 찾기
    int threshold = 20;
    int min_thresh = 1, max_thresh = 100;

    std::vector<cv::KeyPoint> keypoints;

    while (min_thresh <= max_thresh)
    {
        threshold = (min_thresh + max_thresh) / 2;
        keypoints.clear();
        cv::FAST(image, keypoints, threshold, true);

        if ((int)keypoints.size() > target_keypoints)
            min_thresh = threshold + 1;
        else
            max_thresh = threshold - 1;
    }

    std::cout << "결과:" << std::endl;
    std::cout << "   목표: " << target_keypoints << "개" << std::endl;
    std::cout << "   실제: " << keypoints.size() << "개" << std::endl;
    std::cout << "   임계값: " << threshold << "\n" << std::endl;

    std::cout << "💡 응용:" << std::endl;
    std::cout << "   - 밝은 환경: 임계값 ↑" << std::endl;
    std::cout << "   - 어두운 환경: 임계값 ↓" << std::endl;
    std::cout << "   - 실시간으로 조정하여 일정한 개수 유지" << std::endl;
}

// 멀티스케일 검출 — 이미지 피라미드로 스케일 불변성 확보
//
// 문제: FAST는 단일 스케일에서만 검출
//   - 가까운 물체의 큰 코너: 검출 ✓
//   - 먼 물체의 작은 코너: 검출 ✗ (너무 작아서)
//   - 물체가 가까워지면: 이전 코너와 매칭 실패
//
// 해결: Image Pyramid (다중 해상도 검출)
//   Level 0: 원본 (1×)
//   Level 1: 1/1.2× 축소
//   Level 2: 1/1.44× 축소
//   Level 3: 1/1.728× 축소
//
// 각 레벨에서:
//   1. 이미지를 scale = 1.2^level 비율로 축소
//   2. 축소된 이미지에서 FAST 검출
//   3. 검출된 keypoint의 좌표에 scale을 곱하여 원본 크기로 복원
//   4. keypoint.octave에 레벨 정보 저장
//
// ★ ORB::create(nFeatures, scaleFactor, nLevels)에서
//   scaleFactor=1.2, nLevels=8이 기본값으로 자동 수행
//
// TODO: 4레벨 피라미드에서 FAST 검출을 직접 구현하세요
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

    // ✅ 정답: 멀티스케일 (이미지 피라미드)
    std::vector<cv::KeyPoint> kp_multi;
    int num_levels = 4;
    float scale_factor = 1.2f;

    for (int level = 0; level < num_levels; level++)
    {
        float scale = std::pow(scale_factor, level);
        int new_w = (int)(image.cols / scale);
        int new_h = (int)(image.rows / scale);

        cv::Mat scaled;
        cv::resize(image, scaled, cv::Size(new_w, new_h));

        std::vector<cv::KeyPoint> kp;
        cv::FAST(scaled, kp, 20, true);

        for (auto& k : kp)
        {
            k.pt.x *= scale;
            k.pt.y *= scale;
            k.size *= scale;
            k.octave = level;
            kp_multi.push_back(k);
        }
    }

    std::cout << "결과:" << std::endl;
    std::cout << "   단일 스케일: " << kp_single.size() << "개" << std::endl;
    std::cout << "   멀티스케일: " << kp_multi.size() << "개\n" << std::endl;

    std::cout << "💡 장점:" << std::endl;
    std::cout << "   - 다양한 크기의 물체 검출" << std::endl;
    std::cout << "   - 스케일 불변성 (물체가 가까워져도 추적 가능)" << std::endl;
    std::cout << "   - ORB, SIFT는 자동으로 수행" << std::endl;
}

// Harris 코너 검출 직접 구현 — Sobel부터 응답 R까지
//
// Harris 코너 검출 파이프라인:
//
//   원본 이미지 I
//        ↓
//   1. Sobel 필터 → Ix, Iy (x, y 방향 그래디언트)
//        ↓
//   2. Structure Tensor 요소:
//      Ixx = Ix · Ix,  Iyy = Iy · Iy,  Ixy = Ix · Iy
//        ↓
//   3. 가우시안 블러 (윈도우 내 가중 합산):
//      Sxx = G * Ixx,  Syy = G * Iyy,  Sxy = G * Ixy
//      (블러가 "윈도우" 역할 → 주변 그래디언트를 부드럽게 합산)
//        ↓
//   4. Harris 응답:
//      R = det(M) - k · trace(M)²
//      = Sxx·Syy - Sxy² - k·(Sxx + Syy)²
//        ↓
//   5. R > threshold인 점 = 코너
//
// ★ cornerHarris 한 줄이 이 전체 과정을 수행한다
//
// TODO: 위 파이프라인을 직접 구현하고 cornerHarris 결과와 비교하세요
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

    // ✅ 정답: 1. Sobel 필터로 Ix, Iy 계산
    cv::Mat Ix, Iy;
    cv::Sobel(image_f, Ix, CV_32F, 1, 0, 3);
    cv::Sobel(image_f, Iy, CV_32F, 0, 1, 3);

    // ✅ 정답: 2. Structure Tensor 요소 계산
    cv::Mat Ixx = Ix.mul(Ix);
    cv::Mat Iyy = Iy.mul(Iy);
    cv::Mat Ixy = Ix.mul(Iy);

    // ✅ 정답: 3. 가우시안 블러 적용
    int ksize = 5;
    cv::GaussianBlur(Ixx, Ixx, cv::Size(ksize, ksize), 0);
    cv::GaussianBlur(Iyy, Iyy, cv::Size(ksize, ksize), 0);
    cv::GaussianBlur(Ixy, Ixy, cv::Size(ksize, ksize), 0);

    // ✅ 정답: 4. Harris 응답 계산
    double k = 0.04;
    cv::Mat det = Ixx.mul(Iyy) - Ixy.mul(Ixy);
    cv::Mat trace = Ixx + Iyy;
    cv::Mat harris_response = det - k * trace.mul(trace);

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

// NMS (Non-Maximum Suppression) 직접 구현 — 지역 최대값 필터링
//
// NMS 알고리즘:
//   각 픽셀 (y, x)에 대해:
//   1. response(y, x) > threshold 확인
//   2. 주변 window × window 영역의 최대값 찾기
//   3. response(y, x) == 최대값이면 → 지역 최대 → 코너로 유지
//   4. 그렇지 않으면 → 더 강한 이웃이 있음 → 제거
//
// 윈도우 크기의 영향:
//   - 작은 윈도우 (3×3): 미세한 차이만 제거 → 많은 점 유지
//   - 큰 윈도우 (11×11): 넓은 범위에서 최강만 유지 → 희소한 결과
//   - 적정 크기: 7×7 ~ 9×9
//
// 경계 처리:
//   - 이미지 경계에서 half_win만큼 안쪽부터 순회
//   - 또는 copyMakeBorder로 패딩 후 처리
//
// ★ FAST의 nonmaxSuppression=true가 이 과정을 내부적으로 수행
//
// TODO: 7×7 윈도우 NMS를 직접 구현하고 전후 개수를 비교하세요
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

    // ✅ 정답: NMS 구현
    int window_size = 7;
    int half_win = window_size / 2;
    int count_after = 0;

    for (int y = half_win; y < response.rows - half_win; y++)
    {
        for (int x = half_win; x < response.cols - half_win; x++)
        {
            float val = response.at<float>(y, x);
            if (val <= thresh_value)
                continue;

            bool is_max = true;
            for (int dy = -half_win; dy <= half_win && is_max; dy++)
            {
                for (int dx = -half_win; dx <= half_win && is_max; dx++)
                {
                    if (dy == 0 && dx == 0)
                        continue;
                    if (response.at<float>(y + dy, x + dx) > val)
                        is_max = false;
                }
            }
            if (is_max)
                count_after++;
        }
    }

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
    std::cout << "Phase 2 Week 3 Quiz - Medium (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_uniform_distribution();
    problem2_adaptive_threshold();
    problem3_multiscale_detection();
    problem4_harris_implementation();
    problem5_nms_implementation();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
