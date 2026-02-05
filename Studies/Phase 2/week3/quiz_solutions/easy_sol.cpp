// Phase 2 Week 3 - 기초 퀴즈 정답 (요약)
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "Week 3 기초 퀴즈 핵심 정답\n" << std::endl;
    
    cv::Mat image = cv::Mat::zeros(400, 600, CV_8UC1);
    
    // 문제 1: FAST threshold
    std::cout << "문제 1: 임계값 ↑ → 특징점 개수 ↓" << std::endl;
    std::cout << "   - 더 강한 코너만 검출됨\n" << std::endl;
    
    // 문제 2: ORB descriptor
    std::cout << "문제 2: ORB = 256 bits = 32 bytes" << std::endl;
    std::cout << "   - 이진 디스크립터 → 해밍거리로 빠른 매칭\n" << std::endl;
    
    // 문제 3: NMS
    std::cout << "문제 3: NMS로 중복 제거" << std::endl;
    std::cout << "   - 더 균등한 분포 → 더 나은 포즈 추정\n" << std::endl;
    
    // 문제 4: Speed
    std::cout << "문제 4: FAST가 ORB보다 빠름" << std::endl;
    std::cout << "   - FAST: 코너만, ORB: 코너 + 디스크립터" << std::endl;
    
    return 0;
}
