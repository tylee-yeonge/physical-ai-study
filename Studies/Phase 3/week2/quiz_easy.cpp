/**
 * Phase 3 Week 2 - 2D-2D 기초 퀴즈
 */

#include <iostream>

void problem1_essential_vs_fundamental() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Essential vs Fundamental" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "질문: E와 F의 차이는?\n" << std::endl;
    
    std::cout << "💡 답:" << std::endl;
    std::cout << "Essential Matrix (E):" << std::endl;
    std::cout << "   - 정규화 좌표 사용" << std::endl;
    std::cout << "   - 캘리브레이션 필요" << std::endl;
    std::cout << "   - p2^T * E * p1 = 0\n" << std::endl;
    
    std::cout << "Fundamental Matrix (F):" << std::endl;
    std::cout << "   - 픽셀 좌표 사용" << std::endl;
    std::cout << "   - 캘리브레이션 불필요" << std::endl;
    std::cout << "   - F = K2^{-T} * E * K1^{-1}" << std::endl;
}

void problem2_five_point() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 5-Point Algorithm" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "질문: 왜 최소 5개 점이 필요한가?\n" << std::endl;
    
    std::cout << "💡 답:" << std::endl;
    std::cout << "   E의 자유도 = 5" << std::endl;
    std::cout << "   - 회전: 3 DoF" << std::endl;
    std::cout << "   - 이동 방향: 2 DoF (크기는 모호)" << std::endl;
    std::cout << "   → 최소 5개 제약 필요" << std::endl;
}

void problem3_cheirality() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Cheirality Check" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "질문: 4개 해 중 어떻게 선택하는가?\n" << std::endl;
    
    std::cout << "💡 답: Cheirality Check" << std::endl;
    std::cout << "   - 3D 점이 두 카메라 앞에 있는지 확인" << std::endl;
    std::cout << "   - depth > 0 (카메라 좌표계)" << std::endl;
    std::cout << "   - 가장 많은 점이 앞에 있는 해 선택" << std::endl;
}

void problem4_scale_ambiguity() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 스케일 모호성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "질문: 왜 t의 크기를 모르는가?\n" << std::endl;
    
    std::cout << "💡 답:" << std::endl;
    std::cout << "   E = t^ * R" << std::endl;
    std::cout << "   E = (α*t)^ * R  (동일!)" << std::endl;
    std::cout << "   → 투영 방정식에서 α 소거됨" << std::endl;
    std::cout << "   → 초기화 시 ||t|| = 1로 정규화" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 2 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    problem1_essential_vs_fundamental()
    problem2_five_point()
    problem3_cheirality()
    problem4_scale_ambiguity()
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
