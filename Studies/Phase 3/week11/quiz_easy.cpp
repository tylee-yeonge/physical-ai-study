/**
 * Phase 3 Week 11 - Ceres 실습 퀴즈
 */

#include <iostream>

void problem1_ceres_vs_g2o() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Ceres vs g2o" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: Ceres와 g2o의 가장 큰 차이는 무엇인가요?" << std::endl;
    std::cout << "  A) Ceres는 Python만 지원한다" << std::endl;
    std::cout << "  B) Ceres는 자동 미분(Auto-diff)을 지원한다" << std::endl;
    std::cout << "  C) g2o는 비선형 최적화를 지원하지 않는다" << std::endl;
    std::cout << "  D) g2o는 Robust Kernel을 지원하지 않는다\n" << std::endl;

    std::cout << "💡 답: B) Ceres는 자동 미분(Auto-diff)을 지원한다" << std::endl;
    std::cout << "   - Ceres: operator()만 정의하면 Jacobian 자동 계산" << std::endl;
    std::cout << "   - g2o: linearizeOplus()에서 Jacobian을 직접 구현" << std::endl;
    std::cout << "   - 둘 다 비선형 최적화 + Robust Kernel 지원" << std::endl;
}

void problem2_autodiff_template() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: AutoDiffCostFunction 템플릿 파라미터" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 다음 코드에서 2, 9, 3은 각각 무엇을 의미하나요?\n" << std::endl;
    std::cout << "  new ceres::AutoDiffCostFunction<SnavelyError, 2, 9, 3>(...)\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   - 2: 잔차(residual) 차원 = 재투영 오차 (x, y)" << std::endl;
    std::cout << "   - 9: 첫 번째 파라미터 블록 차원 = 카메라 파라미터" << std::endl;
    std::cout << "     (angle-axis 3 + translation 3 + focal 1 + k1,k2 2)" << std::endl;
    std::cout << "   - 3: 두 번째 파라미터 블록 차원 = 3D 점 (x, y, z)" << std::endl;
}

void problem3_dense_schur() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: DENSE_SCHUR" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: Ceres에서 options.linear_solver_type = DENSE_SCHUR;" << std::endl;
    std::cout << "      이 설정은 내부적으로 무엇을 하나요?\n" << std::endl;

    std::cout << "💡 답: Schur Complement를 적용하여 3D 점을 소거" << std::endl;
    std::cout << "   - 원래: (6N+3M) x (6N+3M) 크기의 H 행렬" << std::endl;
    std::cout << "   - Schur 후: 6N x 6N 크기의 S 행렬만 풀면 됨" << std::endl;
    std::cout << "   - 3D 점은 Back-substitution으로 복원" << std::endl;
    std::cout << "   - g2o에서는 setMarginalized(true)와 동일한 효과" << std::endl;
}

void problem4_huber_loss() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: HuberLoss" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: new ceres::HuberLoss(1.0)은 어떤 역할을 하나요?\n" << std::endl;

    std::cout << "💡 답: Outlier의 영향을 줄이는 Robust Loss Function" << std::endl;
    std::cout << "   - 작은 오차 (|e| <= delta): 제곱 유지 (e^2/2)" << std::endl;
    std::cout << "   - 큰 오차 (|e| > delta): 선형으로 완화" << std::endl;
    std::cout << "     → delta(|e| - delta/2)" << std::endl;
    std::cout << "   - delta=1.0: 1픽셀 이상의 오차를 outlier로 간주" << std::endl;
    std::cout << "   - 실제 데이터에서 잘못된 매칭의 영향을 억제" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 11 Quiz - Easy (Ceres 실습)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_ceres_vs_g2o();
    problem2_autodiff_template();
    problem3_dense_schur();
    problem4_huber_loss();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
