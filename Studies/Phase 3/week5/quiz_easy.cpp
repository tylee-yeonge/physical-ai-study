/**
 * Quiz Easy - Week 5: Mini VO 프로젝트
 * 
 * 기본 개념 이해 확인
 */

#include <iostream>
#include <string>

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 5 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "Q1. Mini VO 시스템의 두 가지 주요 단계는?" << std::endl;
    std::cout << "   힌트: Week 2와 Week 3" << std::endl;
    std::cout << "   답: _________와 _________\n" << std::endl;
    
    std::cout << "Q2. 초기화(Initialization)에서 사용하는 방법은?" << std::endl;
    std::cout << "   a) PnP" << std::endl;
    std::cout << "   b) Essential Matrix" << std::endl;
    std::cout << "   c) ICP" << std::endl;
    std::cout << "   d) Homography" << std::endl;
    std::cout << "   답: _____\n" << std::endl;
    
    std::cout << "Q3. 추적(Tracking)에서 사용하는 방법은?" << std::endl;
    std::cout << "   a) Essential Matrix" << std::endl;
    std::cout << "   b) PnP + RANSAC" << std::endl;
    std::cout << "   c) ICP" << std::endl;
    std::cout << "   d) Optical Flow만" << std::endl;
    std::cout << "   답: _____\n" << std::endl;
    
    std::cout << "Q4. 초기화를 위한 베이스라인 체크는 왜 필요한가?" << std::endl;
    std::cout << "   a) 계산 속도 향상" << std::endl;
    std::cout << "   b) 삼각측량 안정성" << std::endl;
    std::cout << "   c) 메모리 절약" << std::endl;
    std::cout << "   d) 특징점 증가" << std::endl;
    std::cout << "   답: _____\n" << std::endl;
    
    std::cout << "Q5. Keyframe을 사용하는 이유는?" << std::endl;
    std::cout << "   a) 맵 크기 관리" << std::endl;
    std::cout << "   b) 계산량 감소" << std::endl;
    std::cout << "   c) 중요한 프레임 선택" << std::endl;
    std::cout << "   d) 위 모두" << std::endl;
    std::cout << "   답: _____\n" << std::endl;
    
    std::cout << "Q6. Inlier ratio가 25%일 때 어떻게 해야 하나?" << std::endl;
    std::cout << "   a) 계속 추적" << std::endl;
    std::cout << "   b) 재초기화" << std::endl;
    std::cout << "   c) 프로그램 종료" << std::endl;
    std::cout << "   d) 아무것도 안 함" << std::endl;
    std::cout << "   답: _____\n" << std::endl;
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
