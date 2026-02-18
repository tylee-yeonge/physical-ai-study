// Week 6 중급 퀴즈 정답
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

int main()
{
    std::cout << "Week 6 중급 퀴즈 정답\n" << std::endl;

    std::cout << "문제 1: cv::triangulatePoints() 사용" << std::endl;
    std::cout << "문제 2: Bundle Adjustment (Ceres/g2o)" << std::endl;
    std::cout << "문제 3: cv::StereoBM, cv::StereoSGBM\n" << std::endl;

    // 삼각측량 예제
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));

    std::vector<cv::Point2f> pts1 = {cv::Point2f(400, 300)};
    std::vector<cv::Point2f> pts2 = {cv::Point2f(460, 300)};

    cv::Mat P2 = P1.clone();
    P2.at<double>(0, 3) = 0.1 * 600;  // baseline * focal

    cv::Mat pts4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, pts4D);

    std::cout << "복원된 3D 점 (동차): " << pts4D << "\n" << std::endl;

    // 문제 4: E 분해 + Cheirality Check
    std::cout << "문제 4: E 분해 + Cheirality Check" << std::endl;

    double angle = 10.0 * CV_PI / 180.0;
    cv::Mat R_gt = (cv::Mat_<double>(3, 3) <<
        cos(angle), 0, sin(angle), 0, 1, 0, -sin(angle), 0, cos(angle));
    cv::Mat t_gt = (cv::Mat_<double>(3, 1) << 0.5, 0.1, 0.2);
    t_gt = t_gt / cv::norm(t_gt);

    double tx = t_gt.at<double>(0), ty = t_gt.at<double>(1), tz = t_gt.at<double>(2);
    cv::Mat t_skew = (cv::Mat_<double>(3, 3) << 0, -tz, ty, tz, 0, -tx, -ty, tx, 0);
    cv::Mat E = t_skew * R_gt;

    // SVD 분해
    cv::Mat w, u, vt;
    cv::SVD::compute(E, w, u, vt);

    cv::Mat W_mat = (cv::Mat_<double>(3, 3) << 0, -1, 0, 1, 0, 0, 0, 0, 1);

    // 4가지 해
    cv::Mat R1_est = u * W_mat.t() * vt;
    cv::Mat R2_est = u * W_mat * vt;
    cv::Mat t1_est = u.col(2).clone();
    cv::Mat t2_est = -u.col(2);

    // det 보정
    if (cv::determinant(R1_est) < 0) R1_est = -R1_est;
    if (cv::determinant(R2_est) < 0) R2_est = -R2_est;

    std::cout << "   4가지 해 중 올바른 해 선택:" << std::endl;
    std::cout << "   R1 det=" << cv::determinant(R1_est) << std::endl;
    std::cout << "   R2 det=" << cv::determinant(R2_est) << std::endl;
    std::cout << "   Cheirality Check로 Z>0 비율 확인\n" << std::endl;

    // 문제 5: 실패 사례
    std::cout << "문제 5: 포즈 추정 실패 사례" << std::endl;
    std::cout << "   순수 회전 (t=0): E = 0 → 분해 불가" << std::endl;
    std::cout << "   작은 이동: 수치적으로 불안정 → 노이즈에 민감" << std::endl;

    return 0;
}
