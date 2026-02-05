// Week 6 기초 퀴즈 정답
#include <iostream>
#include <cmath>

int main() {
    std::cout << "Week 6 기초 퀴즈 정답\n" << std::endl;
    
    // 문제 1
    double baseline = 0.12, focal = 600.0;
    double d1 = 60.0, d2 = 30.0;
    std::cout << "문제 1:" << std::endl;
    std::cout << "   시차 " << d1 << " → 깊이 " << (baseline*focal/d1) << " m" << std::endl;
    std::cout << "   시차 " << d2 << " → 깊이 " << (baseline*focal/d2) << " m\n" << std::endl;
    
    std::cout << "문제 2: DLT로 최적 3D 점 찾기" << std::endl;
    std::cout << "문제 3: 재투영 오차 = ||관측 - 투영||" << std::endl;
    std::cout << "문제 4: Baseline 선택은 응용에 따라 다름" << std::endl;
    
    return 0;
}
