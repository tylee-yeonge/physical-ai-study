/**
 * Quiz Solutions - Medium
 */

#include <iostream>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7 Quiz Solutions (Medium)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1. Schur Complement에서 먼저 최적화하는 것은?\n";
    std::cout << "정답: b) 카메라 포즈\n";
    std::cout << "설명: Schur complement는 카메라를 먼저 최적화한 후\n";
    std::cout << "      3D 점들을 closed-form으로 업데이트합니다.\n\n";

    std::cout << "Q2. setMarginalized(true)를 설정하는 Vertex는?\n";
    std::cout << "정답: b) 3D 점\n";
    std::cout << "설명: 3D 점들을 marginalized로 설정하면\n";
    std::cout << "      Schur complement가 적용되어 속도가 빨라집니다.\n\n";

    std::cout << "Q3. 첫 카메라를 setFixed(true)하는 이유는?\n";
    std::cout << "정답: b) Gauge freedom 제거\n";
    std::cout << "설명: 전역 좌표계를 고정하여 unique solution을 얻기 위함.\n";
    std::cout << "      이를 gauge freedom 제거라고 합니다.\n";

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
