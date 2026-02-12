/**
 * Quiz Easy - Week 7: Bundle Adjustment
 */

#include <iostream>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7 Quiz (Easy)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1. Bundle Adjustment가 최소화하는 것은?\n";
    std::cout << "   a) 포즈 간 거리\n";
    std::cout << "   b) 재투영 오차\n";
    std::cout << "   c) 3D 점 간 거리\n";
    std::cout << "   d) 시간 지연\n";
    std::cout << "Your answer: ";
    char ans1;
    std::cin >> ans1;

    std::cout << "\nQ2. g2o에서 Vertex는 무엇?\n";
    std::cout << "   a) 최적화 변수\n";
    std::cout << "   b) 제약 조건\n";
    std::cout << "   c) 관측값\n";
    std::cout << "   d) 오차 함수\n";
    std::cout << "Your answer: ";
    char ans2;
    std::cin >> ans2;

    std::cout << "\nQ3. Edge는 무엇?\n";
    std::cout << "   a) 최적화 변수\n";
    std::cout << "   b) 제약 조건\n";
    std::cout << "   c) 카메라 포즈\n";
    std::cout << "   d) 3D 점\n";
    std::cout << "Your answer: ";
    char ans3;
    std::cin >> ans3;

    std::cout << "\nQ4. Huber Loss의 목적은?\n";
    std::cout << "   a) 속도 향상\n";
    std::cout << "   b) Outlier 영향 감소\n";
    std::cout << "   c) 메모리 절약\n";
    std::cout << "   d) 정확도 향상\n";
    std::cout << "Your answer: ";
    char ans4;
    std::cin >> ans4;

    std::cout << "\nQ5. Schur Complement의 장점은?\n";
    std::cout << "   a) 정확도 향상\n";
    std::cout << "   b) 속도 향상\n";
    std::cout << "   c) 메모리 절약\n";
    std::cout << "   d) Outlier 제거\n";
    std::cout << "Your answer: ";
    char ans5;
    std::cin >> ans5;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "답안 제출 완료!" << std::endl;
    std::cout << "quiz_solutions/easy_sol.cpp에서 정답을 확인하세요." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
