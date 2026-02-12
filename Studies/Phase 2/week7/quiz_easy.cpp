/**
 * Phase 2 Week 7 - PnP 기초 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <iostream>

void problem1_pnp_definition()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: PnP란?" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Perspective-n-Point (PnP):" << std::endl;
    std::cout << "   입력: n개의 3D-2D 대응점" << std::endl;
    std::cout << "   출력: 카메라 포즈 (R, t)\n" << std::endl;

    std::cout << "질문: 최소 몇 개 점이 필요한가요?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   - P3P: 최소 3개 (4개 해 → 1개 더로 선택)" << std::endl;
    std::cout << "   - 일반적으로 4개 이상 권장" << std::endl;
    std::cout << "   - 실제로는 수십~수백 개 사용 (RANSAC)\n" << std::endl;

    std::cout << "포즈 자유도:" << std::endl;
    std::cout << "   - 회전 3 DoF + 이동 3 DoF = 6 DoF" << std::endl;
    std::cout << "   - 각 대응점 → 2개 제약 (x, y)" << std::endl;
    std::cout << "   - 3개 점 → 6개 제약 (충분!)" << std::endl;
}

void problem2_pnp_vs_essential()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: PnP vs Essential Matrix" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Essential Matrix (2D-2D):" << std::endl;
    std::cout << "   장점: 3D 정보 불필요" << std::endl;
    std::cout << "   단점: 스케일 모호성 (t의 크기 모름)\n" << std::endl;

    std::cout << "PnP (3D-2D):" << std::endl;
    std::cout << "   장점: 절대 스케일 복원!" << std::endl;
    std::cout << "   단점: 3D 점 필요 (이전 프레임에서 삼각측량)\n" << std::endl;

    std::cout << "💡 SLAM 전략:" << std::endl;
    std::cout << "   1. Frame 0-1: E로 초기화 (스케일 = 1)" << std::endl;
    std::cout << "   2. 삼각측량으로 3D 맵 생성" << std::endl;
    std::cout << "   3. Frame 2~: PnP로 tracking" << std::endl;
}

void problem3_ransac_necessity()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 왜 RANSAC이 필요한가?" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Outlier의 원인:" << std::endl;
    std::cout << "   1. 특징점 매칭 오류" << std::endl;
    std::cout << "   2. 동적 물체 (사람, 자동차)" << std::endl;
    std::cout << "   3. 반복 패턴 (잘못된 대응)\n" << std::endl;

    std::cout << "Outlier의 영향:" << std::endl;
    std::cout << "   - 하나만 있어도 포즈 추정 실패" << std::endl;
    std::cout << "   - 재투영 오차 폭증\n" << std::endl;

    std::cout << "💡 해결: RANSAC" << std::endl;
    std::cout << "   - 랜덤 샘플링으로 가설 생성" << std::endl;
    std::cout << "   - Inlier 개수로 최적 선택" << std::endl;
    std::cout << "   - Outlier에 robust!" << std::endl;
}

void problem4_vo_pipeline()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Visual Odometry 흐름" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Visual Odometry (VO):" << std::endl;
    std::cout << "   - 카메라 이미지로 포즈 추정" << std::endl;
    std::cout << "   - 연속 프레임 간 상대 이동 누적\n" << std::endl;

    std::cout << "파이프라인:" << std::endl;
    std::cout << "   1. 특징점 검출 (ORB)" << std::endl;
    std::cout << "   2. 특징점 매칭 (Ratio Test)" << std::endl;
    std::cout << "   3. PnP로 포즈 추정 (RANSAC)" << std::endl;
    std::cout << "   4. 삼각측량으로 새 3D 점" << std::endl;
    std::cout << "   5. 다음 프레임으로...\n" << std::endl;

    std::cout << "💡 문제점:" << std::endl;
    std::cout << "   - Drift (오차 누적)" << std::endl;
    std::cout << "   - Loop Closure로 보정 필요 → SLAM!" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 7 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_pnp_definition() problem2_pnp_vs_essential() problem3_ransac_necessity()
            problem4_vo_pipeline()

                std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
