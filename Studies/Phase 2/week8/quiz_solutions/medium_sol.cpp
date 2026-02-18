// Week 8 중급 퀴즈 정답
#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    std::cout << "Week 8 중급 퀴즈 정답\n" << std::endl;

    std::cout << "문제 1: cv::calcOpticalFlowPyrLK() 사용" << std::endl;
    std::cout << "문제 2: status, error, backward check로 품질 평가" << std::endl;
    std::cout << "문제 3: Flow(빠름) + Matching(robust) 조합" << std::endl;
    std::cout << "문제 4: Sobel로 Ix,Iy 계산, (A^T A)^-1 A^T b로 광류, OpenCV와 유사 결과" << std::endl;
    std::cout << "문제 5: 작은 윈도우=정밀+민감, 큰 윈도우=안정+뭉뚱그림, 권장 15~31" << std::endl;

    return 0;
}
