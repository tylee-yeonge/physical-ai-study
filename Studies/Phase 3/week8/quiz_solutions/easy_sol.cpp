/**
 * Quiz Solutions - Easy
 */

#include <iostream>

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 8 Quiz Solutions (Easy)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "Q1. Ceres Solver의 가장 큰 장점은?\n";
    std::cout << "정답: b) 자동 미분\n";
    std::cout << "설명: Ceres는 template과 Jet 타입으로 자동 미분을 수행하여\n";
    std::cout << "      Jacobian을 직접 계산할 필요가 없습니다.\n\n";
    
    std::cout << "Q2. AutoDiffCostFunction의 템플릿 파라미터는?\n";
    std::cout << "정답: <오차함수, 출력차원, 입력1차원, 입력2차원, ...>\n";
    std::cout << "예시: <ReprojectionError, 2, 6, 3>\n";
    std::cout << "      출력 2차원(x,y 오차), 입력1 6차원(카메라), 입력2 3차원(점)\n\n";
    
    std::cout << "Q3. Ceres에서 자동 미분을 위해 사용하는 타입은?\n";
    std::cout << "정답: c) template T\n";
    std::cout << "설명: operator()를 template으로 정의하면 Ceres가\n";
    std::cout << "      T를 ceres::Jet으로 인스턴스화하여 자동 미분합니다.\n\n";
    
    std::cout << "Q4. HuberLoss(1.0)의 의미는?\n";
    std::cout << "정답: b) δ=1 픽셀\n";
    std::cout << "설명: |e| ≤ 1이면 e²/2, |e| > 1이면 선형으로 처리하여\n";
    std::cout << "      outlier의 영향을 감소시킵니다.\n\n";
    
    std::cout << "Q5. g2o 대비 Ceres의 단점은?\n";
    std::cout << "정답: a) 느린 속도\n";
    std::cout << "설명: 자동 미분은 편리하지만 수동 Jacobian보다 약간 느립니다.\n";
    std::cout << "      하지만 개발 속도는 훨씬 빠릅니다!\n\n";
    
    std::cout << "Q6. Problem::SetParameterBlockConstant의 목적은?\n";
    std::cout << "정답: c) Gauge freedom 제거\n";
    std::cout << "설명: 첫 카메라를 고정하여 전역 좌표계를 결정하고\n";
    std::cout << "      unique solution을 얻습니다.\n";
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
