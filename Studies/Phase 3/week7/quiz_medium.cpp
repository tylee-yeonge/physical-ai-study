/**
 * Quiz Medium - Week 7: Bundle Adjustment
 */

#include <iostream>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7 Quiz (Medium)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1. Schur Complement에서 먼저 최적화하는 것은?\n";
    std::cout << "   a) 3D 점\n";
    std::cout << "   b) 카메라 포즈\n";
    std::cout << "   c) 동시에\n";
    std::cout << "   d) 순서 상관 없음\n";
    std::cout << "Your answer: ";
    char ans1;
    std::cin >> ans1;

    std::cout << "\nQ2. setMarginalized(true)를 설정하는 Vertex는?\n";
    std::cout << "   a) 카메라 포즈\n";
    std::cout << "   b) 3D 점\n";
    std::cout << "   c) 둘 다\n";
    std::cout << "   d) 없음\n";
    std::cout << "Your answer: ";
    char ans2;
    std::cin >> ans2;

    std::cout << "\nQ3. 첫 카메라를 setFixed(true)하는 이유는?\n";
    std::cout << "   a) 속도 향상\n";
    std::cout << "   b) Gauge freedom 제거\n";
    std::cout << "   c) 메모리 절약\n";
    std::cout << "   d) Outlier 제거\n";
    std::cout << "Your answer: ";
    char ans3;
    std::cin >> ans3;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "답안 제출 완료!" << std::endl;
    std::cout << "quiz_solutions/medium_sol.cpp에서 정답을 확인하세요." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
