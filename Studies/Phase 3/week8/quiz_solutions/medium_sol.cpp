/**
 * Quiz Solutions - Medium
 */

#include <iostream>

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 8 Quiz Solutions (Medium)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "Q1. Ceres의 Jet 타입은 무엇을 저장하나요?\n";
    std::cout << "정답: 값(value)과 미분(derivative)을 동시에 저장\n";
    std::cout << "설명:\n";
    std::cout << "  Jet<T, N>는 다음을 저장:\n";
    std::cout << "  - a: 함수의 값\n";
    std::cout << "  - v[N]: N개 변수에 대한 편미분\n";
    std::cout << "  \n";
    std::cout << "  예: f(x,y) = x² + y를 계산하면\n";
    std::cout << "    Jet.a = x² + y (값)\n";
    std::cout << "    Jet.v[0] = 2x (∂f/∂x)\n";
    std::cout << "    Jet.v[1] = 1  (∂f/∂y)\n\n";
    
    std::cout << "Q2. DENSE_SCHUR solver를 사용하는 이유는?\n";
    std::cout << "정답: BA의 구조를 활용한 효율적 해법\n";
    std::cout << "설명:\n";
    std::cout << "  BA 문제의 Hessian 구조:\n";
    std::cout << "    [Hcc  Hcp]  = 카메라-카메라, 카메라-점\n";
    std::cout << "    [Hpc  Hpp]    점-카메라, 점-점\n";
    std::cout << "  \n";
    std::cout << "  Schur complement:\n";
    std::cout << "    (Hcc - Hcp·Hpp⁻¹·Hpc)Δc = bc - Hcp·Hpp⁻¹·bp\n";
    std::cout << "  \n";
    std::cout << "  장점:\n";
    std::cout << "  - 카메라 변수만 직접 최적화 (차원 감소)\n";
    std::cout << "  - 점들은 closed-form 업데이트\n";
    std::cout << "  - 10-100배 빠름!\n\n";
    
    std::cout << "Q3. Ceres vs g2o를 각각 언제 사용할지:\n";
    std::cout << "정답:\n";
    std::cout << "  Ceres 권장:\n";
    std::cout << "  - 빠른 프로토타이핑이 필요할 때\n";
    std::cout << "  - 새로운 오차 함수를 실험할 때\n";
    std::cout << "  - SLAM 외 최적화 문제 (curve fitting 등)\n";
    std::cout << "  - 코드 간결성이 중요할 때\n";
    std::cout << "  - Jacobian 유도가 복잡할 때\n";
    std::cout << "  \n";
    std::cout << "  g2o 권장:\n";
    std::cout << "  - 최고 성능이 필요할 때\n";
    std::cout << "  - 표준 SLAM 문제 (Pose graph, BA)\n";
    std::cout << "  - 대규모 최적화 (10,000+ 변수)\n";
    std::cout << "  - 기존 g2o 코드베이스가 있을 때\n";
    std::cout << "  - 실시간 처리가 중요할 때\n";
    std::cout << "  \n";
    std::cout << "  실무 조언:\n";
    std::cout << "  - 개발 초기: Ceres로 빠르게 구현\n";
    std::cout << "  - 성능 최적화 단계: g2o로 전환 고려\n";
    std::cout << "  - 혼용도 가능: 각 모듈에 적합한 것 선택\n";
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
