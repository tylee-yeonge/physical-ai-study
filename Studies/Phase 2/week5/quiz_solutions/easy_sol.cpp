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
    std::cout << "문제 4: Cheirality Check (Z > 0)" << std::endl;

    return 0;
}
