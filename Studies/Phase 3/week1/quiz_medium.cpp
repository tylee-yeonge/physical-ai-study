/**
 * Phase 3 Week 1 - VO 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>

void problem1_drift() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 드리프트 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "매 프레임 1% 오차 누적:" << std::endl;
    
    double cum_error = 1.0;
    for (int i = 1; i <= 100; i += 20) {
        cum_error *= std::pow(1.01, 20);
        std::cout << "   Frame " << i << ": " << cum_error << "x" << std::endl;
    }
    
    std::cout << "\n💡 지수적으로 증가!" << std::endl;
}

void problem2_frontend_vs_backend() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Frontend vs Backend" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "Frontend (VO):" << std::endl;
    std::cout << "   - 특징점 추적" << std::endl;
    std::cout << "   - 포즈 추정" << std::endl;
    std::cout << "   - 빠름, 로컬\n" << std::endl;
    
    std::cout << "Backend (Optimization):" << std::endl;
    std::cout << "   - Graph 최적화" << std::endl;
    std::cout << "   - Loop Closure" << std::endl;
    std::cout << "   - 느림, 전역\n" << std::endl;
    
    std::cout << "💡 SLAM = Frontend + Backend" << std::endl;
}

void problem3_vo_realtime() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 실시간 VO" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "30 FPS 실시간 처리:" << std::endl;
    std::cout << "   프레임당 예산: 33ms\n" << std::endl;
    
    std::cout << "시간 분배:" << std::endl;
    std::cout << "   특징점 검출: 10ms" << std::endl;
    std::cout << "   특징점 추적: 5ms" << std::endl;
    std::cout << "   모션 추정: 10ms" << std::endl;
    std::cout << "   기타: 8ms\n" << std::endl;
    
    std::cout << "💡 최적화 필요: 멀티스레드, GPU" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 1 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    problem1_drift()
    problem2_frontend_vs_backend()
    problem3_vo_realtime()
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
