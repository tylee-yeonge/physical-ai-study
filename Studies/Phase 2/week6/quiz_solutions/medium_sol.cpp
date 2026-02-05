// Week 6 중급 퀴즈 정답
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "Week 6 중급 퀴즈 정답\n" << std::endl;
    
    std::cout << "문제 1: cv::triangulatePoints() 사용" << std::endl;
    std::cout << "문제 2: Bundle Adjustment (Ceres/g2o)" << std::endl;
    std::cout << "문제 3: cv::StereoBM, cv::StereoSGBM\n" << std::endl;
    
    // 삼각측량 예제
    cv::Mat K = (cv::Mat_<double>(3,3) << 600,0,400, 0,600,300, 0,0,1);
    cv::Mat P1 = cv::Mat::zeros(3,4,CV_64F);
    K.copyTo(P1(cv::Rect(0,0,3,3)));
    
    std::vector<cv::Point2f> pts1 = {cv::Point2f(400,300)};
    std::vector<cv::Point2f> pts2 = {cv::Point2f(460,300)};
    
    cv::Mat P2 = P1.clone();
    P2.at<double>(0,3) = 0.1 * 600;  // baseline * focal
    
    cv::Mat pts4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, pts4D);
    
    std::cout << "복원된 3D 점 (동차): " << pts4D << std::endl;
    
    return 0;
}
