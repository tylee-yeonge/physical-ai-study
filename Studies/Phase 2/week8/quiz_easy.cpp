/**
 * Phase 2 Week 8 - Optical Flow 기초 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <iostream>

void problem1_optical_flow_assumptions()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Optical Flow 가정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Lucas-Kanade의 3가지 가정:\n" << std::endl;

    std::cout << "1️⃣  Brightness Constancy (밝기 일정)" << std::endl;
    std::cout << "   - 같은 물체는 밝기 동일" << std::endl;
    std::cout << "   - I(x, y, t) = I(x+dx, y+dy, t+dt)\n" << std::endl;

    std::cout << "2️⃣  Small Motion (작은 움직임)" << std::endl;
    std::cout << "   - 프레임 간 작은 이동" << std::endl;
    std::cout << "   - Taylor 근사 가능\n" << std::endl;

    std::cout << "3️⃣  Spatial Coherence (공간 일관성)" << std::endl;
    std::cout << "   - 이웃 픽셀은 비슷하게 움직임" << std::endl;
    std::cout << "   - 윈도우 내에서 flow 일정\n" << std::endl;

    std::cout << "💡 이 가정이 깨지면?" << std::endl;
    std::cout << "   - 조명 변화 → Brightness 깨짐" << std::endl;
    std::cout << "   - 빠른 움직임 → Small Motion 깨짐" << std::endl;
    std::cout << "   - 경계면 → Spatial Coherence 깨짐" << std::endl;
}

void problem2_aperture_problem()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Aperture Problem" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Aperture Problem이란?" << std::endl;
    std::cout << "   - 작은 윈도우에서는 방향 모호" << std::endl;
    std::cout << "   - 에지만 보면 수직 방향 모름\n" << std::endl;

    std::cout << "예시: 수직 에지" << std::endl;
    std::cout << "   - 좌우 이동은 감지 가능" << std::endl;
    std::cout << "   - 상하 이동은 감지 불가\n" << std::endl;

    std::cout << "💡 해결:" << std::endl;
    std::cout << "   - 코너 점 사용 (Harris, FAST)" << std::endl;
    std::cout << "   - 큰 윈도우 사용" << std::endl;
    std::cout << "   - Pyramidal approach" << std::endl;
}

void problem3_pyramidal_flow()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Pyramidal Optical Flow" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "왜 Pyramid가 필요한가?\n" << std::endl;

    std::cout << "문제: 큰 움직임" << std::endl;
    std::cout << "   - Small Motion 가정 깨짐" << std::endl;
    std::cout << "   - 윈도우 밖으로 이동\n" << std::endl;

    std::cout << "해결: Image Pyramid" << std::endl;
    std::cout << "   1. 이미지 다운샘플링 (여러 레벨)" << std::endl;
    std::cout << "   2. 작은 이미지에서 먼저 추정" << std::endl;
    std::cout << "   3. 결과를 큰 이미지로 전파\n" << std::endl;

    std::cout << "💡 효과:" << std::endl;
    std::cout << "   - 큰 움직임 → 작은 이미지에서 작게 보임" << std::endl;
    std::cout << "   - 성공률 ↑, 정확도 ↑" << std::endl;
}

void problem4_slam_application()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: SLAM에서 활용" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Optical Flow in SLAM:\n" << std::endl;

    std::cout << "1️⃣  Direct VO (VINS)" << std::endl;
    std::cout << "   - Optical Flow로 특징점 추적" << std::endl;
    std::cout << "   - 빠른 초기화\n" << std::endl;

    std::cout << "2️⃣  Hybrid Approach" << std::endl;
    std::cout << "   - Flow + 특징점 매칭" << std::endl;
    std::cout << "   - 더 robust\n" << std::endl;

    std::cout << "3️⃣  Dense SLAM (LSD-SLAM)" << std::endl;
    std::cout << "   - Dense flow로 depth 추정" << std::endl;
    std::cout << "   - GPU 가속 필요\n" << std::endl;

    std::cout << "💡 장점:" << std::endl;
    std::cout << "   - 특징 없는 영역도 추적" << std::endl;
    std::cout << "   - 부드러운 trajectory" << std::endl;
    std::cout << "\n단점:" << std::endl;
    std::cout << "   - 조명 변화에 약함" << std::endl;
    std::cout << "   - Drift 누적" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 8 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_optical_flow_assumptions() problem2_aperture_problem() problem3_pyramidal_flow()
            problem4_slam_application()

                std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
