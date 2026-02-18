// Week 8 기초 퀴즈 정답
#include <iostream>

int main()
{
    std::cout << "Week 8 기초 퀴즈 정답\n" << std::endl;

    std::cout << "문제 1: 3가지 가정 - Brightness, Small Motion, Spatial" << std::endl;
    std::cout << "문제 2: Aperture Problem → 코너 점 사용" << std::endl;
    std::cout << "문제 3: Pyramid로 큰 움직임 처리" << std::endl;
    std::cout << "문제 4: SLAM에서 tracking, Dense VO 등 활용" << std::endl;
    std::cout << "문제 5: A^T A가 역행렬 존재 → 두 고유값 모두 커야 함 → 코너에서만 추적" << std::endl;
    std::cout << "문제 6: Harris M = Structure Tensor, goodFeaturesToTrack()이 추적 가능 점 선택" << std::endl;

    return 0;
}
