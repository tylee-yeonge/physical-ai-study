// Week 7 중급 퀴즈 정답
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "Week 7 중급 퀴즈 정답\n" << std::endl;
    
    // PnP 예제
    std::vector<cv::Point3f> pts3d = {cv::Point3f(0,0,5), cv::Point3f(1,0,5)};
    std::vector<cv::Point2f> pts2d = {cv::Point2f(400,300), cv::Point2f(1000,300)};
    cv::Mat K = (cv::Mat_<double>(3,3) << 600,0,400, 0,600,300, 0,0,1);
    
    cv::Mat rvec, tvec;
    cv::solvePnP(pts3d, pts2d, K, cv::Mat(), rvec, tvec);
    
    std::cout << "추정된 t: " << tvec.t() << std::endl;
    std::cout << "\nRANSAC 반복: Inlier 70% → ~16회" << std::endl;
    std::cout << "최적화: cv::solvePnPRefineLM()" << std::endl;
    
    return 0;
}
