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

/**
 * @brief 3x3 패치에서 이미지 그래디언트 수동 계산과 LK 방정식 설명
 *
 * Sobel 커널로 Ix, Iy를 구하고 프레임 차이로 It을 계산하는 과정,
 * 그리고 A^T A 행렬(Structure Tensor)을 구성하여 LK 방정식을 세우는
 * 과정을 단계별로 설명한다.
 */
void problem5_gradient_and_lk_equation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: 이미지 그래디언트와 LK 방정식" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "3x3 패치 예제:\n" << std::endl;

    std::cout << "프레임 1:          프레임 2:" << std::endl;
    std::cout << "  [100 100 100]     [100 100 100]" << std::endl;
    std::cout << "  [100 200 100]     [100 100 200]" << std::endl;
    std::cout << "  [100 100 100]     [100 100 100]\n" << std::endl;

    std::cout << "Sobel 커널 (x 방향):" << std::endl;
    std::cout << "   [-1  0  1]" << std::endl;
    std::cout << "   [-2  0  2]  / 8" << std::endl;
    std::cout << "   [-1  0  1]\n" << std::endl;

    std::cout << "Sobel 커널 (y 방향):" << std::endl;
    std::cout << "   [-1 -2 -1]" << std::endl;
    std::cout << "   [ 0  0  0]  / 8" << std::endl;
    std::cout << "   [ 1  2  1]\n" << std::endl;

    std::cout << "그래디언트 계산:" << std::endl;
    std::cout << "   Ix: x 방향 밝기 변화 (Sobel_x * I)" << std::endl;
    std::cout << "   Iy: y 방향 밝기 변화 (Sobel_y * I)" << std::endl;
    std::cout << "   It: 시간 변화 (Frame2 - Frame1)\n" << std::endl;

    std::cout << "LK 방정식:" << std::endl;
    std::cout << "   윈도우 내 N개 픽셀에서:" << std::endl;
    std::cout << "   A = [Ix_1  Iy_1]     b = [-It_1]" << std::endl;
    std::cout << "       [Ix_2  Iy_2]         [-It_2]" << std::endl;
    std::cout << "       [ ...   ... ]         [ ... ]" << std::endl;
    std::cout << "       [Ix_N  Iy_N]         [-It_N]\n" << std::endl;

    std::cout << "A^T A (2x2 Structure Tensor):" << std::endl;
    std::cout << "   [sum(Ix^2)     sum(Ix*Iy)]" << std::endl;
    std::cout << "   [sum(Ix*Iy)    sum(Iy^2) ]\n" << std::endl;

    std::cout << "해: [u, v]^T = (A^T A)^-1 * A^T * b\n" << std::endl;

    std::cout << "질문: 왜 A^T A가 역행렬이 존재해야 하나요?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   A^T A가 특이(singular)하면 해를 구할 수 없음" << std::endl;
    std::cout << "   → 두 고유값이 모두 충분히 커야 함" << std::endl;
    std::cout << "   → 코너(Corner) 영역에서만 안정적 추적 가능" << std::endl;
}

/**
 * @brief Structure Tensor 고유값 기반 추적 가능성 판별 설명
 *
 * 코너/에지/평면 영역에서 고유값 패턴이 다른 것을 설명하고
 * 조리개 문제(Aperture Problem)와의 관계를 다룬다.
 */
void problem6_trackability()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 6: 추적 가능성 판별" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Structure Tensor M = A^T A 의 고유값 분석:\n" << std::endl;

    std::cout << "코너 (Corner):" << std::endl;
    std::cout << "   λ1 >> 0, λ2 >> 0 (둘 다 큼)" << std::endl;
    std::cout << "   → 모든 방향으로 밝기 변화" << std::endl;
    std::cout << "   → 추적 가능!\n" << std::endl;

    std::cout << "에지 (Edge):" << std::endl;
    std::cout << "   λ1 >> 0, λ2 ≈ 0 (하나만 큼)" << std::endl;
    std::cout << "   → 한 방향으로만 밝기 변화" << std::endl;
    std::cout << "   → 조리개 문제 (Aperture Problem)!\n" << std::endl;

    std::cout << "평면 (Flat):" << std::endl;
    std::cout << "   λ1 ≈ 0, λ2 ≈ 0 (둘 다 작음)" << std::endl;
    std::cout << "   → 밝기 변화 없음" << std::endl;
    std::cout << "   → 추적 불가!\n" << std::endl;

    std::cout << "시각화:" << std::endl;
    std::cout << "   ┌──── λ2 ────┐" << std::endl;
    std::cout << "   │ Corner     │" << std::endl;
    std::cout << "   │ (추적 OK)  │" << std::endl;
    std::cout << "   │            │ λ1" << std::endl;
    std::cout << "   │Edge Edge   │" << std::endl;
    std::cout << "   │(불안정)    │" << std::endl;
    std::cout << "   │Flat        │" << std::endl;
    std::cout << "   │(추적 불가) │" << std::endl;
    std::cout << "   └────────────┘\n" << std::endl;

    std::cout << "질문: Harris 코너 검출과 어떤 관계인가요?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   Harris M = Structure Tensor = A^T A" << std::endl;
    std::cout << "   Harris 응답: R = det(M) - k*trace(M)^2" << std::endl;
    std::cout << "   → 코너에서 R 큼 = 추적 가능" << std::endl;
    std::cout << "   → goodFeaturesToTrack()이 바로 이것!" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 8 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_optical_flow_assumptions();
    problem2_aperture_problem();
    problem3_pyramidal_flow();
    problem4_slam_application();
    problem5_gradient_and_lk_equation();
    problem6_trackability();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
