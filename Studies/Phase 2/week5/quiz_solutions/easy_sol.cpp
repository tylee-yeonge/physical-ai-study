// Week 5 기초 퀴즈 정답 요약
#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    std::cout << "Week 5 기초 퀴즈 정답\n" << std::endl;

    std::cout << "문제 1: p2^T * E * p1 = 0 (제약식)" << std::endl;
    std::cout << "문제 2:" << std::endl;
    std::cout << "   - E: 캘리브레이션 필요, 정규화 좌표" << std::endl;
    std::cout << "   - F: 캘리브레이션 불필요, 픽셀 좌표\n" << std::endl;

    std::cout << "문제 3: F는 8 자유도 → 8개 점 필요" << std::endl;
    std::cout << "문제 4: Cheirality Check (Z > 0)\n" << std::endl;

    // 문제 5: 에피폴 계산
    std::cout << "문제 5: 에피폴 계산" << std::endl;
    cv::Mat F = (cv::Mat_<double>(3, 3) <<
        1.0e-7, -2.0e-5,  0.004,
        2.0e-5,  3.0e-8, -0.010,
       -0.005,   0.012,   1.0);

    // e1: F의 오른쪽 null space (Fe1 = 0)
    cv::Mat w, u, vt;
    cv::SVD::compute(F, w, u, vt);
    cv::Mat e1 = vt.row(2).t();  // 마지막 행
    e1 = e1 / e1.at<double>(2);  // 동차 좌표 정규화
    std::cout << "   e1 (이미지 1 에피폴): " << e1.t() << std::endl;

    // e2: F^T의 오른쪽 null space (F^T e2 = 0)
    cv::SVD::compute(F.t(), w, u, vt);
    cv::Mat e2 = vt.row(2).t();
    e2 = e2 / e2.at<double>(2);
    std::cout << "   e2 (이미지 2 에피폴): " << e2.t() << std::endl;

    return 0;
}
