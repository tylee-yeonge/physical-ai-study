/**
 * @file quiz_easy_answer.cpp
 * @brief Week 3 개념 퀴즈 (Easy) - 답안
 *
 * 주제:
 *   - 에피폴라 제약의 의미
 *   - Rectification 의 목적
 *   - Disparity-Depth 공식 (Z = fB/d)
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

// 문제 1: 에피폴라 제약의 의미
void problem1_epipolar_constraint()
{
    std::cout << "\n------------------------------------" << std::endl;
    std::cout << "문제 1: 에피폴라 제약" << std::endl;
    std::cout << "------------------------------------\n" << std::endl;

    cv::Mat F = (cv::Mat_<double>(3, 3) <<
         0.0,    0.0,   -0.002,
         0.0,    0.0,    0.01,
         0.003, -0.008,  1.0);

    cv::Mat p1 = (cv::Mat_<double>(3, 1) << 320.0, 240.0, 1.0);

    // [답] 에피폴라 선 계산
    cv::Mat l = F * p1;

    double a = l.at<double>(0);
    double b = l.at<double>(1);
    double c = l.at<double>(2);
    double slope = -a / b;

    std::cout << "왼쪽 점 p1 = (320, 240)" << std::endl;
    std::cout << "에피폴라 선 l = F * p1 = [" << a << ", " << b << ", " << c << "]" << std::endl;
    std::cout << "에피폴라 선 방정식: " << a << "*x + " << b << "*y + " << c << " = 0" << std::endl;
    std::cout << "기울기 = -a/b = " << slope << std::endl;
    std::cout << std::endl;
    std::cout << "[TIP]" << std::endl;
    std::cout << "   l = [a, b, c] 일 때, 에피폴라 선의 방정식은 ax + by + c = 0" << std::endl;
    std::cout << "   기울기 = -a/b" << std::endl;
    std::cout << "   Rectification 후에는 기울기가 0 (수평선) 이 되어야 함!" << std::endl;
}

// 문제 2: Rectification 의 목적
void problem2_rectification_purpose()
{
    std::cout << "\n------------------------------------" << std::endl;
    std::cout << "문제 2: Rectification 의 목적" << std::endl;
    std::cout << "------------------------------------\n" << std::endl;

    cv::Point2f before_left(200, 150);
    cv::Point2f before_right(180, 155);

    cv::Point2f after_left(200, 150);
    cv::Point2f after_right(180, 150);

    double disparity = after_left.x - after_right.x;

    std::cout << "Rectification 전:" << std::endl;
    std::cout << "   왼쪽: " << before_left << ", 오른쪽: " << before_right << std::endl;
    std::cout << "   y 차이: " << std::abs(before_left.y - before_right.y) << " px" << std::endl;
    std::cout << std::endl;
    std::cout << "Rectification 후:" << std::endl;
    std::cout << "   왼쪽: " << after_left << ", 오른쪽: " << after_right << std::endl;
    std::cout << "   y 차이: " << std::abs(after_left.y - after_right.y) << " px" << std::endl;
    std::cout << "   Disparity: " << disparity << " px" << std::endl;
    std::cout << std::endl;
    std::cout << "[KEY]" << std::endl;
    std::cout << "   Rectification = 두 이미지를 같은 평면에 정렬하여" << std::endl;
    std::cout << "   대응점의 y좌표를 동일하게 만드는 변환." << std::endl;
    std::cout << "   -> Stereo Depth network (HITNet 등) 는 이 전처리를 전제로 작동" << std::endl;
}

// 문제 3: Disparity -> Depth 공식
void problem3_disparity_depth()
{
    std::cout << "\n------------------------------------" << std::endl;
    std::cout << "문제 3: Disparity -> Depth (Z = fB/d)" << std::endl;
    std::cout << "------------------------------------\n" << std::endl;

    double f = 500.0;
    double B = 0.12;

    std::cout << "카메라 설정: f=" << f << " px, B=" << B << " m" << std::endl;
    std::cout << "f*B = " << f * B << " px*m\n" << std::endl;

    std::vector<double> disparities = {60.0, 30.0, 15.0, 6.0, 1.0};

    for (double d : disparities)
    {
        // [답] Z = fB / d
        double Z = f * B / d;

        std::cout << "   d = " << d << " px -> Z = " << Z << " m" << std::endl;
    }

    std::cout << "\n[KEY]" << std::endl;
    std::cout << "   disparity 가 절반이 되면 depth 는 2배가 됩니다 (반비례)" << std::endl;
    std::cout << "   Stereo Depth 모델은 이 원리로 disparity -> 미터 단위 depth 를 변환합니다" << std::endl;
}

int main()
{
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Phase 2 Week 3 Quiz - Easy [ANSWER]" << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    problem1_epipolar_constraint();
    problem2_rectification_purpose();
    problem3_disparity_depth();

    return 0;
}
