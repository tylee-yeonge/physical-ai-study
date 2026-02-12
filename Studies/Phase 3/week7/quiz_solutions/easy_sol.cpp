/**
 * Quiz Solutions - Easy
 */

#include <iostream>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7 Quiz Solutions (Easy)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1. Bundle Adjustment가 최소화하는 것은?\n";
    std::cout << "정답: b) 재투영 오차\n";
    std::cout << "설명: BA는 Σ ||observed - projected||²를 최소화\n\n";

    std::cout << "Q2. g2o에서 Vertex는 무엇?\n";
    std::cout << "정답: a) 최적화 변수\n";
    std::cout << "설명: Vertex = 카메라 포즈, 3D 점 등 최적화할 변수\n\n";

    std::cout << "Q3. Edge는 무엇?\n";
    std::cout << "정답: b) 제약 조건\n";
    std::cout << "설명: Edge = 재투영 제약 등 오차 함수\n\n";

    std::cout << "Q4. Huber Loss의 목적은?\n";
    std::cout << "정답: b) Outlier 영향 감소\n";
    std::cout << "설명: 큰 오차를 선형화하여 outlier 영향 제한\n\n";

    std::cout << "Q5. Schur Complement의 장점은?\n";
    std::cout << "정답: b) 속도 향상\n";
    std::cout << "설명: 카메라만 먼저 최적화 → 차원 감소 → 10-100배 빠름\n";

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
