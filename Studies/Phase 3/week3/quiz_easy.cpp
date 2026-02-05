/**
 * Phase 3 Week 3 - PnP 기초 퀴즈
 */

#include <iostream>

void problem1_pnp_definition() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: PnP 정의" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "질문: PnP가 푸는 문제는?\n" << std::endl;
    
    std::cout << "💡 답:" << std::endl;
    std::cout << "   주어진 것: {(X_i, x_i)}" << std::endl;
    std::cout << "     - X_i: 3D 점 (월드)" << std::endl;
    std::cout << "     - x_i: 2D 관측 (이미지)\n" << std::endl;
    std::cout << "   구하는 것: [R|t] (카메라 포즈)" << std::endl;
}

void problem2_p3p_vs_epnp() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: P3P vs EPnP" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "💡 답:" << std::endl;
    std::cout << "P3P:" << std::endl;
    std::cout << "   - 최소 3개 점" << std::endl;
    std::cout << "   - 4개 해 존재" << std::endl;
    std::cout << "   - RANSAC에 적합\n" << std::endl;
    
    std::cout << "EPnP:" << std::endl;
    std::cout << "   - n개 점 (O(n))" << std::endl;
    std::cout << "   - 안정적이고 빠름" << std::endl;
    std::cout << "   - OpenCV 기본" << std::endl;
}

void problem3_scale_recovery() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 스케일 복원" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "질문: PnP가 스케일을 복원하는 이유?\n" << std::endl;
    
    std::cout << "💡 답:" << std::endl;
    std::cout << "   3D 점의 절대 크기를 알기 때문!" << std::endl;
    std::cout << "   - 2D-2D: ||t|| = 1 (정규화)" << std::endl;
    std::cout << "   - 3D-2D: ||t|| = 실제 이동 거리" << std::endl;
}

void problem4_inlier_ratio() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Inlier Ratio" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "Inlier Ratio = inliers / total\n" << std::endl;
    
    std::cout << "💡 해석:" << std::endl;
    std::cout << "   > 70%: 양호" << std::endl;
    std::cout << "   > 50%: 보통" << std::endl;
    std::cout << "   < 30%: 추적 실패 → 재초기화" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 3 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    problem1_pnp_definition()
    problem2_p3p_vs_epnp()
    problem3_scale_recovery()
    problem4_inlier_ratio()
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
