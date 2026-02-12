/**
 * Quiz Easy - Week 8: Ceres Bundle Adjustment
 */

#include <iostream>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 8 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1. Ceres Solver의 가장 큰 장점은?\n";
    std::cout << "   a) 가장 빠른 속도\n";
    std::cout << "   b) 자동 미분\n";
    std::cout << "   c) GPU 지원\n";
    std::cout << "   d) 시각화 기능\n";
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "Q2. AutoDiffCostFunction의 템플릿 파라미터는?\n";
    std::cout << "   <오차함수, ?, ?, ?>\n";
    std::cout << "   빈칸에 들어갈 것은? (순서대로)\n";
    std::cout << "   답: ___________________\n" << std::endl;

    std::cout << "Q3. Ceres에서 자동 미분을 위해 사용하는 타입은?\n";
    std::cout << "   a) double\n";
    std::cout << "   b) float\n";
    std::cout << "   c) template T\n";
    std::cout << "   d) Eigen::VectorXd\n";
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "Q4. HuberLoss(1.0)의 의미는?\n";
    std::cout << "   a) 1초 timeout\n";
    std::cout << "   b) δ=1 픽셀\n";
    std::cout << "   c) 1번 반복\n";
    std::cout << "   d) 1개 변수\n";
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "Q5. g2o 대비 Ceres의 단점은?\n";
    std::cout << "   a) 느린 속도\n";
    std::cout << "   b) 복잡한 API\n";
    std::cout << "   c) Jacobian 필요\n";
    std::cout << "   d) 메모리 많이 사용\n";
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "Q6. Problem::SetParameterBlockConstant의 목적은?\n";
    std::cout << "   a) 메모리 절약\n";
    std::cout << "   b) 속도 향상\n";
    std::cout << "   c) Gauge freedom 제거\n";
    std::cout << "   d) 정확도 향상\n";
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
