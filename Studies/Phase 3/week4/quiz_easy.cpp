/**
 * Phase 3 Week 4 - ICP 기초 퀴즈
 */

#include <iostream>

void problem1_icp_definition() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: ICP 정의" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "질문: ICP가 푸는 문제는?\n" << std::endl;
    
    std::cout << "💡 답:" << std::endl;
    std::cout << "   주어진 것: 두 Point Cloud" << std::endl;
    std::cout << "     P = {p_i} (소스)" << std::endl;
    std::cout << "     Q = {q_i} (타겟)\n" << std::endl;
    std::cout << "   구하는 것: [R|t] (정합 변환)" << std::endl;
    std::cout << "   최소화: Σ ||q_i - (R*p_i + t)||²" << std::endl;
}

void problem2_icp_steps() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: ICP 알고리즘 단계" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "💡 답 (반복):" << std::endl;
    std::cout << "   1. 대응점 찾기 (Correspondence)" << std::endl;
    std::cout << "      → K-D Tree로 최근접 이웃\n" << std::endl;
    std::cout << "   2. 변환 추정 (Alignment)" << std::endl;
    std::cout << "      → SVD로 R, t 계산\n" << std::endl;
    std::cout << "   3. 점 변환 (Transform)" << std::endl;
    std::cout << "      → p' = R*p + t\n" << std::endl;
    std::cout << "   4. 수렴 체크 (Convergence)" << std::endl;
    std::cout << "      → 오차 감소 < threshold" << std::endl;
}

void problem3_point_to_point_vs_plane() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Point-to-Point vs Point-to-Plane" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "💡 답:" << std::endl;
    std::cout << "Point-to-Point:" << std::endl;
    std::cout << "   - min Σ ||q_i - (R*p_i + t)||²" << std::endl;
    std::cout << "   - 점 간 거리 최소화" << std::endl;
    std::cout << "   - SVD로 풀이\n" << std::endl;
    
    std::cout << "Point-to-Plane:" << std::endl;
    std::cout << "   - min Σ (n_i · (R*p_i + t - q_i))²" << std::endl;
    std::cout << "   - 평면 제약 (법선 활용)" << std::endl;
    std::cout << "   - 더 빠른 수렴" << std::endl;
}

void problem4_icp_limitations() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: ICP 한계점" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "💡 답:" << std::endl;
    std::cout << "   1. Local minimum" << std::endl;
    std::cout << "      → 초기값이 중요!\n" << std::endl;
    std::cout << "   2. Outlier에 민감" << std::endl;
    std::cout << "      → Robust ICP 필요\n" << std::endl;
    std::cout << "   3. 반복 계산" << std::endl;
    std::cout << "      → K-D Tree, Downsampling" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 4 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    problem1_icp_definition()
    problem2_icp_steps()
    problem3_point_to_point_vs_plane()
    problem4_icp_limitations()
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
