// Week 5 중급 퀴즈 정답
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "Week 5 중급 퀴즈 정답\n" << std::endl;
    
    std::vector<cv::Point2f> pts1 = {cv::Point2f(100,150), cv::Point2f(200,250)};
    std::vector<cv::Point2f> pts2 = {cv::Point2f(110,160), cv::Point2f(210,260)};
    
    cv::Mat F = cv::findFundamentalMat(pts1, pts2, cv::FM_8POINT);
    std::cout << "Fundamental Matrix:\n" << F << std::endl;
    
    std::cout << "\nE에서 R,t 복원은 cv::recoverPose() 사용" << std::endl;
    
    return 0;
}
