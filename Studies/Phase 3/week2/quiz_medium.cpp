/**
 * Phase 3 Week 2 - 2D-2D 중급 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <iostream>

void problem1_pure_rotation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Pure Rotation 문제" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Pure Rotation (t = 0)일 때:" << std::endl;
    std::cout << "   E = t^ * R = 0" << std::endl;
    std::cout << "   에피폴라 구조 사라짐\n" << std::endl;

    std::cout << "💡 해결: 평행이동 충분한 프레임 선택" << std::endl;
    std::cout << "   - 최소 베이스라인 확보" << std::endl;
    std::cout << "   - 회전만 하는 프레임 건너뛰기" << std::endl;
}

void problem2_degenerate_case()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Degenerate Case" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "평면 장면 문제:" << std::endl;
    std::cout << "   - 모든 점이 평면에 있음" << std::endl;
    std::cout << "   - Homography로도 설명 가능\n" << std::endl;

    std::cout << "💡 해결:" << std::endl;
    std::cout << "   - E와 H 동시 추정 (RANSAC)" << std::endl;
    std::cout << "   - Inlier 더 많은 것 선택" << std::endl;
    std::cout << "   - ORB-SLAM 방식" << std::endl;
}

void problem3_implement_check()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Cheirality 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "간단한 체크:" << std::endl;
    std::cout << "   Z = R.row(2) * X + t(2)" << std::endl;
    std::cout << "   if (Z > 0) → 카메라 앞\n" << std::endl;

    std::cout << "💡 OpenCV recoverPose()가 자동 처리!" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 2 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_pure_rotation();
    problem2_degenerate_case();
    problem3_implement_check();

    std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
