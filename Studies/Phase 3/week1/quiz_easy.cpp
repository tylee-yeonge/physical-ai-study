/**
 * Phase 3 Week 1 - VO 기초 퀴즈
 */

#include <iostream>

void problem1_vo_vs_slam()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: VO vs SLAM" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: VO와 SLAM의 가장 큰 차이는?\n" << std::endl;

    std::cout << "💡 답: Loop Closure" << std::endl;
    std::cout << "   - VO: Loop Closure 없음 → 드리프트 누적" << std::endl;
    std::cout << "   - SLAM: Loop Closure로 전역 오차 보정" << std::endl;
}

void problem2_scale_ambiguity()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 스케일 모호성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: Monocular VO의 근본적인 한계는?\n" << std::endl;

    std::cout << "💡 답: 스케일 모호성" << std::endl;
    std::cout << "   - t의 방향은 알지만 크기는 모름" << std::endl;
    std::cout << "   - 1m와 10m 이동이 동일한 이미지" << std::endl;
    std::cout << "   - 해결: IMU, Stereo, 알려진 크기" << std::endl;
}

void problem3_vo_pipeline()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: VO 파이프라인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 가장 계산량이 많은 단계는?\n" << std::endl;

    std::cout << "💡 답: 특징점 검출/매칭" << std::endl;
    std::cout << "   - 매 프레임 수백 개 특징점" << std::endl;
    std::cout << "   - Descriptor 계산 + 매칭" << std::endl;
    std::cout << "   - 해결: GPU 가속, Optical Flow" << std::endl;
}

void problem4_vo_types()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: VO 유형" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 자율주행에 적합한 VO 유형은?\n" << std::endl;

    std::cout << "💡 답: Stereo VO" << std::endl;
    std::cout << "   - 스케일 복원 가능 → 절대 거리" << std::endl;
    std::cout << "   - 실외 환경에 robust" << std::endl;
    std::cout << "   - 예: KITTI 벤치마크" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 1 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_vo_vs_slam();
    problem2_scale_ambiguity();
    problem3_vo_pipeline();
    problem4_vo_types();

    std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
