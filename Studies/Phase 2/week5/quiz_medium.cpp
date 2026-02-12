/**
 * Phase 2 Week 5 - 에피폴라 기하학 중급 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>

void problem1_implement_constraint()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 제약 검증 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // TODO: Essential Matrix 주어짐
    cv::Mat E = cv::Mat::eye(3, 3, CV_64F);
    cv::Point2f p1(0.5, 0.3);
    cv::Point2f p2(0.6, 0.35);

    // TODO: p2^T * E * p1 계산
    cv::Mat pt1 = (cv::Mat_<double>(3, 1) << p1.x, p1.y, 1.0);
    cv::Mat pt2 = (cv::Mat_<double>(3, 1) << p2.x, p2.y, 1.0);

    cv::Mat result = pt2.t() * E * pt1;
    double error = std::abs(result.at<double>(0));

    std::cout << "에피폴라 제약 오차: " << error << std::endl;
    std::cout << "(0에 가까울수록 정확한 대응)" << std::endl;
}

void problem2_estimate_F()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: F Matrix 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // TODO: 대응점 준비 (최소 8개)
    std::vector<cv::Point2f> pts1, pts2;
    for (int i = 0; i < 10; i++)
    {
        pts1.push_back(cv::Point2f(100 + i * 30, 150 + i * 20));
        pts2.push_back(cv::Point2f(110 + i * 32, 160 + i * 21));
    }

    // TODO: findFundamentalMat 사용
    cv::Mat F = cv::findFundamentalMat(pts1, pts2, cv::FM_8POINT);

    std::cout << "Fundamental Matrix:\n" << F << std::endl;
    std::cout << "\n💡 이제 에피폴라 선을 그릴 수 있습니다!" << std::endl;
}

void problem3_e_to_pose()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: E → R, t 복원" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "E를 SVD 분해하여 R, t 추출:\n" << std::endl;
    std::cout << "E = U * diag(1, 1, 0) * V^T" << std::endl;
    std::cout << "R = U * W * V^T  (또는 W^T)" << std::endl;
    std::cout << "t = U의 마지막 열\n" << std::endl;

    std::cout << "W = [0 -1  0]" << std::endl;
    std::cout << "    [1  0  0]" << std::endl;
    std::cout << "    [0  0  1]" << std::endl;

    std::cout << "\n💡 OpenCV: cv::recoverPose()가 자동 수행" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 5 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
                 " << std::endl;

        problem1_implement_constraint() problem2_estimate_F() problem3_e_to_pose()

            std::cout
              << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
