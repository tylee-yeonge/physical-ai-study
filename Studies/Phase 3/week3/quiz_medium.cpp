/**
 * Phase 3 Week 3 - PnP 중급 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <iostream>

void problem1_ransac_iterations()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: RANSAC 반복 횟수" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "RANSAC 반복 횟수 계산:\n" << std::endl;
    std::cout << "   N = log(1-p) / log(1-w^s)" << std::endl;
    std::cout << "   p: 신뢰도 (0.99)" << std::endl;
    std::cout << "   w: inlier 비율 (0.5)" << std::endl;
    std::cout << "   s: 샘플 크기 (3)\n" << std::endl;

    double p = 0.99;
    double w = 0.5;
    int s = 3;
    int N = static_cast<int>(std::log(1 - p) / std::log(1 - std::pow(w, s)));

    std::cout << "💡 결과: " << N << " 반복" << std::endl;
}

void problem2_reprojection_error()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 재투영 오차" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "재투영 오차 = ||x - π(R, t, X)||²\n" << std::endl;

    std::cout << "💡 의미:" << std::endl;
    std::cout << "   - 3D 점을 현재 포즈로 투영" << std::endl;
    std::cout << "   - 실제 관측과 차이" << std::endl;
    std::cout << "   - 포즈 정확도 지표" << std::endl;
}

void problem3_vo_tracking()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: VO 추적 전략" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "추적 유지 조건:" << std::endl;
    std::cout << "   1. Inlier ratio > 30%" << std::endl;
    std::cout << "   2. 충분한 특징점 (> 20개)" << std::endl;
    std::cout << "   3. 재투영 오차 < 3px\n" << std::endl;

    std::cout << "💡 실패 시:" << std::endl;
    std::cout << "   - 재초기화 (2D-2D)" << std::endl;
    std::cout << "   - 또는 Relocalization" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 3 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_ransac_iterations() problem2_reprojection_error() problem3_vo_tracking()

            std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
