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

    // 문제 5: Homography 변환
    std::cout << "문제 5: Homography 점 변환" << std::endl;
    std::cout << "   동차좌표 변환 후 정규화:" << std::endl;
    std::cout << "   [u']       [x]" << std::endl;
    std::cout << "   [v'] = H * [y]" << std::endl;
    std::cout << "   [w ]       [1]" << std::endl;
    std::cout << "   결과: (u'/w, v'/w)\n" << std::endl;

    double theta = 15.0 * M_PI / 180.0;
    double scale = 1.1;
    double tx = 50.0, ty = 30.0;

    // 점 (100, 100) 변환 예시
    double x = 100.0, y = 100.0;
    double u = scale * cos(theta) * x + (-scale * sin(theta)) * y + tx;
    double v = scale * sin(theta) * x + scale * cos(theta) * y + ty;
    // w = 1 (마지막 행이 [0, 0, 1])
    std::cout << "   (100, 100) → (" << u << ", " << v << ")" << std::endl;

    x = 200.0; y = 200.0;
    u = scale * cos(theta) * x + (-scale * sin(theta)) * y + tx;
    v = scale * sin(theta) * x + scale * cos(theta) * y + ty;
    std::cout << "   (200, 200) → (" << u << ", " << v << ")\n" << std::endl;

    std::cout << "   핵심: h31=h32=0, h33=1이면 w=1 → 정규화 불필요" << std::endl;
    std::cout << "   일반적인 H에서는 w≠1이므로 반드시 정규화 필요!" << std::endl;

    return 0;
}
