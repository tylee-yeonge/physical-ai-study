/**
 * Quiz Solutions - Easy
 */

#include <iostream>

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 9 Quiz Solutions (Easy)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1. BA에서 자코비안 행렬이 희소(sparse)한 이유는?\n";
    std::cout << "정답: b) 각 관측이 하나의 카메라와 하나의 점에만 연결되기 때문\n";
    std::cout << "설명:\n";
    std::cout << "  - 하나의 관측 = (카메라 i, 점 j) 쌍\n";
    std::cout << "  - 자코비안에서 해당 카메라와 점 블록만 값이 있음\n";
    std::cout << "  - 다른 카메라/점에 대한 미분은 0\n";
    std::cout << "  - 따라서 대부분이 0인 희소 행렬\n\n";

    std::cout << "Q2. Schur Complement에서 먼저 소거하는 변수는?\n";
    std::cout << "정답: b) 3D 점\n";
    std::cout << "설명:\n";
    std::cout << "  - 점의 수 >> 카메라 수 (보통 100배 이상)\n";
    std::cout << "  - 점을 소거하면 작은 시스템만 풀면 됨\n";
    std::cout << "  - Hpp가 블록 대각이라 역행렬 계산이 쉬움\n";
    std::cout << "  - 30,600 차원 → 600 차원으로 감소!\n\n";

    std::cout << "Q3. Local BA와 Global BA의 가장 큰 차이점은?\n";
    std::cout << "정답: b) 최적화하는 변수의 범위\n";
    std::cout << "설명:\n";
    std::cout << "  - Local BA: 최근 N개 키프레임만 (실시간 가능)\n";
    std::cout << "  - Global BA: 전체 맵의 모든 변수 (느리지만 정확)\n";
    std::cout << "  - 알고리즘 자체는 동일 (Gauss-Newton/LM)\n\n";

    std::cout << "Q4. Hessian 행렬의 블록 구조에서 Hpp (점-점 블록)의 특징은?\n";
    std::cout << "정답: b) 블록 대각 행렬\n";
    std::cout << "설명:\n";
    std::cout << "  - 각 점은 다른 점과 직접 연결되지 않음\n";
    std::cout << "  - 점 i와 점 j 사이에는 엣지가 없음\n";
    std::cout << "  - 따라서 Hpp는 3x3 블록들이 대각선에만 있음\n";
    std::cout << "  - 이 구조 덕분에 역행렬 계산이 빠름!\n\n";

    std::cout << "Q5. g2o에서 Schur Complement를 적용하려면 어떤 함수를 호출?\n";
    std::cout << "정답: b) setMarginalized(true)\n";
    std::cout << "설명:\n";
    std::cout << "  - 점 Vertex에 setMarginalized(true) 호출\n";
    std::cout << "  - g2o가 내부적으로 Schur Complement 적용\n";
    std::cout << "  - 카메라는 setMarginalized(false) (기본값)\n\n";

    std::cout << "Q6. BA에서 Gauge freedom이란?\n";
    std::cout << "정답: b) 전역 좌표계를 고정할 자유도\n";
    std::cout << "설명:\n";
    std::cout << "  - BA는 상대적인 구조만 복원\n";
    std::cout << "  - 전체를 이동/회전해도 재투영 오차 동일\n";
    std::cout << "  - 유일한 해를 위해 첫 카메라를 고정\n";
    std::cout << "  - g2o: v->setFixed(true)\n";

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
