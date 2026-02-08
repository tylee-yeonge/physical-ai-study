/**
 * Quiz Easy - Week 9: BA 최적화 기법
 */

#include <iostream>

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 9 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1. BA에서 자코비안 행렬이 희소(sparse)한 이유는?\n";
    std::cout << "   a) 카메라가 모든 점을 관측하기 때문\n";
    std::cout << "   b) 각 관측이 하나의 카메라와 하나의 점에만 연결되기 때문\n";
    std::cout << "   c) 3D 점이 모든 카메라에 보이기 때문\n";
    std::cout << "   d) Hessian이 대칭이기 때문\n";
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "Q2. Schur Complement에서 먼저 소거하는 변수는?\n";
    std::cout << "   a) 카메라 포즈\n";
    std::cout << "   b) 3D 점\n";
    std::cout << "   c) 관측값\n";
    std::cout << "   d) 잔차\n";
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "Q3. Local BA와 Global BA의 가장 큰 차이점은?\n";
    std::cout << "   a) 사용하는 알고리즘\n";
    std::cout << "   b) 최적화하는 변수의 범위\n";
    std::cout << "   c) 수렴 속도\n";
    std::cout << "   d) 정확도\n";
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "Q4. Hessian 행렬의 블록 구조에서 Hpp (점-점 블록)의 특징은?\n";
    std::cout << "   a) 밀집(dense) 행렬\n";
    std::cout << "   b) 블록 대각 행렬\n";
    std::cout << "   c) 영행렬\n";
    std::cout << "   d) 단위행렬\n";
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "Q5. g2o에서 Schur Complement를 적용하려면 어떤 함수를 호출?\n";
    std::cout << "   a) setFixed(true)\n";
    std::cout << "   b) setMarginalized(true)\n";
    std::cout << "   c) setOptimized(true)\n";
    std::cout << "   d) setSchur(true)\n";
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "Q6. BA에서 Gauge freedom이란?\n";
    std::cout << "   a) 카메라 해상도 자유도\n";
    std::cout << "   b) 전역 좌표계를 고정할 자유도\n";
    std::cout << "   c) 점의 색상 자유도\n";
    std::cout << "   d) 최적화 반복 횟수\n";
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
