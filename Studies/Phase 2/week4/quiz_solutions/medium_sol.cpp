// Phase 2 Week 4 - 중급 퀴즈 정답 (핵심)
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>

// 문제 2: Essential Matrix 예제
void essentialMatrixExample()
{
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);

    std::vector<cv::Point2f> pts1 = {cv::Point2f(100, 150), cv::Point2f(200, 250),
                                     cv::Point2f(300, 180), cv::Point2f(450, 320)};

    std::vector<cv::Point2f> pts2 = {cv::Point2f(120, 160), cv::Point2f(210, 240),
                                     cv::Point2f(310, 190), cv::Point2f(460, 310)};

    // Essential Matrix 추정
    cv::Mat E = cv::findEssentialMat(pts1, pts2, K, cv::RANSAC, 0.999, 1.0);

    std::cout << "Essential Matrix:\n" << E << std::endl;

    // R, t 복원
    cv::Mat R, t;
    cv::recoverPose(E, pts1, pts2, K, R, t);

    std::cout << "\nRotation:\n" << R << std::endl;
    std::cout << "\nTranslation:\n" << t << std::endl;
}

int main()
{
    std::cout << "Week 4 중급 퀴즈 핵심 정답\n" << std::endl;

    std::cout << "문제 1: Ratio 0.7은 좋은 균형점" << std::endl;
    std::cout << "   - 낮으면: precise but few" << std::endl;
    std::cout << "   - 높으면: many but noisy\n" << std::endl;

    std::cout << "문제 2: Essential Matrix" << std::endl;
    essentialMatrixExample();

    std::cout << "\n문제 3: FLANN이 BF보다 10-100x 빠름" << std::endl;
    std::cout << "   - 특징점 많을수록 차이 커짐" << std::endl;

    return 0;
}
