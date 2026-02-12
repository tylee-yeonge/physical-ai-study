// Phase 2 Week 4 - 기초 퀴즈 정답
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

int main()
{
    std::cout << "Week 4 기초 퀴즈 핵심 정답\n" << std::endl;

    // 문제 1: 거리 측정
    std::cout << "문제 1:" << std::endl;
    std::cout << "   - ORB (이진) → 해밍 거리 (XOR 연산)" << std::endl;
    std::cout << "   - SIFT (실수) → 유클리드 거리 (L2)" << std::endl;
    std::cout << "   - 해밍 거리가 훨씬 빠름!\n" << std::endl;

    // 문제 2: Ratio Test
    std::cout << "문제 2:" << std::endl;
    std::cout << "   시나리오1: 25/80 = 0.31 < 0.7 → 수락" << std::endl;
    std::cout << "   시나리오2: 60/65 = 0.92 > 0.7 → 거절\n" << std::endl;

    // 문제 3: RANSAC
    std::cout << "문제 3: N = log(1-p) / log(1-w^s)" << std::endl;
    double p = 0.99, s = 4;
    std::cout << "   Inlier 50%: N = " << (int)(log(1 - p) / log(1 - pow(0.5, s))) << "회"
              << std::endl;
    std::cout << "   Inlier 70%: N = " << (int)(log(1 - p) / log(1 - pow(0.7, s))) << "회"
              << std::endl;
    std::cout << "   Inlier 90%: N = " << (int)(log(1 - p) / log(1 - pow(0.9, s))) << "회\n"
              << std::endl;

    // 문제 4: Cross-Check
    std::cout << "문제 4:" << std::endl;
    std::cout << "   A→B: 0→3, B→A: 3→0 → 일치 → 통과\n" << std::endl;

    return 0;
}
