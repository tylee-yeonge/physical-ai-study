// Phase 2 Week 3 - 중급 퀴즈 정답 (핵심 알고리즘)
#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>

// 문제 1: 균등 분포
std::vector<cv::KeyPoint> uniformDistribution(const cv::Mat& image, int grid_rows, int grid_cols,
                                              int max_per_cell)
{
    std::vector<cv::KeyPoint> result;

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

            // response로 정렬
            std::sort(cell_kp.begin(), cell_kp.end(),
                      [](const cv::KeyPoint& a, const cv::KeyPoint& b)
                      { return a.response > b.response; });

            // 상위 N개만
            int count = std::min((int)cell_kp.size(), max_per_cell);
            for (int k = 0; k < count; k++)
            {
                cell_kp[k].pt.x += roi.x;
                cell_kp[k].pt.y += roi.y;
                result.push_back(cell_kp[k]);
            }
        }
    }

    return result;
}

// 문제 2: Adaptive threshold
int adaptiveThreshold(const cv::Mat& image, int target_count)
{
    int min_t = 1, max_t = 100;
    int best_threshold = 20;

    while (min_t <= max_t)
    {
        int threshold = (min_t + max_t) / 2;
        std::vector<cv::KeyPoint> kp;
        cv::FAST(image, kp, threshold, true);

        if (kp.size() > (size_t)target_count)
        {
            min_t = threshold + 1;
        }
        else
        {
            max_t = threshold - 1;
            best_threshold = threshold;
        }
    }

    return best_threshold;
}

// 문제 4: Harris 코너 검출 직접 구현
cv::Mat harrisCornerManual(const cv::Mat& image_f, double k = 0.04)
{
    // 1. Sobel로 그래디언트 계산
    cv::Mat Ix, Iy;
    cv::Sobel(image_f, Ix, CV_32F, 1, 0, 3);
    cv::Sobel(image_f, Iy, CV_32F, 0, 1, 3);

    // 2. Structure Tensor 요소
    cv::Mat Ixx = Ix.mul(Ix);
    cv::Mat Iyy = Iy.mul(Iy);
    cv::Mat Ixy = Ix.mul(Iy);

    // 3. 가우시안 블러
    cv::GaussianBlur(Ixx, Ixx, cv::Size(5, 5), 0);
    cv::GaussianBlur(Iyy, Iyy, cv::Size(5, 5), 0);
    cv::GaussianBlur(Ixy, Ixy, cv::Size(5, 5), 0);

    // 4. Harris 응답
    cv::Mat det = Ixx.mul(Iyy) - Ixy.mul(Ixy);
    cv::Mat trace = Ixx + Iyy;
    cv::Mat R = det - k * trace.mul(trace);

    return R;
}

// 문제 5: NMS 구현
int nonMaximumSuppression(const cv::Mat& response, int window_size, double threshold_ratio)
{
    double r_min, r_max;
    cv::minMaxLoc(response, &r_min, &r_max);
    double thresh_value = threshold_ratio * r_max;

    int half_win = window_size / 2;
    int count = 0;

    for (int y = half_win; y < response.rows - half_win; y++)
    {
        for (int x = half_win; x < response.cols - half_win; x++)
        {
            float val = response.at<float>(y, x);
            if (val <= thresh_value)
                continue;

            // 윈도우 내 최대값 확인
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
                count++;
        }
    }

    return count;
}

int main()
{
    std::cout << "Week 3 중급 퀴즈 핵심 정답\n" << std::endl;

    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC1);

    // 테스트
    auto kp = uniformDistribution(image, 6, 8, 10);
    std::cout << "균등 분포: " << kp.size() << "개\n" << std::endl;

    int threshold = adaptiveThreshold(image, 200);
    std::cout << "적응형 임계값: " << threshold << "\n" << std::endl;

    // 문제 4: Harris 직접 구현
    std::cout << "문제 4: Harris 직접 구현" << std::endl;
    cv::Mat test_img = cv::Mat::zeros(400, 400, CV_8UC1);
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if ((i + j) % 2 == 0)
                cv::rectangle(test_img, cv::Point(j * 50, i * 50),
                              cv::Point((j + 1) * 50, (i + 1) * 50), cv::Scalar(255), -1);

    cv::Mat test_f;
    test_img.convertTo(test_f, CV_32F);
    cv::Mat R = harrisCornerManual(test_f, 0.04);
    double min_val, max_val;
    cv::minMaxLoc(R, &min_val, &max_val);
    std::cout << "   응답 범위: [" << min_val << ", " << max_val << "]\n" << std::endl;

    // 문제 5: NMS
    std::cout << "문제 5: NMS 구현" << std::endl;
    int nms_count = nonMaximumSuppression(R, 7, 0.01);
    std::cout << "   NMS 후: " << nms_count << "개" << std::endl;

    return 0;
}
