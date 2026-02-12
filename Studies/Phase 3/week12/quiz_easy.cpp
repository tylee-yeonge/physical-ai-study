/**
 * Quiz Easy - Week 12: Monocular 스케일 모호성
 *
 * 4개의 기본 개념 문제
 * 스케일 모호성의 원인과 결과를 이해했는지 점검합니다.
 */

#include <iostream>

void problem1_why_no_scale()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Q1. 단안 카메라가 스케일을 결정할 수 없는 이유" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "단안 카메라로 촬영한 이미지에서\n";
    std::cout << "3D 점의 실제 크기를 알 수 없는 근본적인 이유는?\n" << std::endl;

    std::cout << "  a) 카메라 해상도가 부족하기 때문\n";
    std::cout << "  b) 3D 점 X와 λX가 같은 2D 픽셀에 투영되기 때문\n";
    std::cout << "  c) 렌즈 왜곡 때문\n";
    std::cout << "  d) 노출 시간이 짧기 때문\n";
    std::cout << "  답: _____\n" << std::endl;

    std::cout << "💡 힌트: 핀홀 모델의 투영 방정식을 생각해보세요.\n";
    std::cout << "   λ * [u, v, 1]^T = K * [R|t] * [X, 1]^T\n" << std::endl;
}

void problem2_t_normalization()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Q2. Essential Matrix에서 ||t|| = 1 정규화의 의미" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Essential Matrix E = [t]x R 에서\n";
    std::cout << "t를 ||t|| = 1로 정규화하는 이유는?\n" << std::endl;

    std::cout << "  a) 계산 속도를 높이기 위해\n";
    std::cout << "  b) t의 크기가 결정 불가하므로 방향만 사용하기 위해\n";
    std::cout << "  c) 회전 행렬 R과 크기를 맞추기 위해\n";
    std::cout << "  d) 수치적 안정성을 위해\n";
    std::cout << "  답: _____\n" << std::endl;

    std::cout << "💡 힌트: x2^T [αt]x R x1 = α * (x2^T [t]x R x1) = 0\n";
    std::cout << "   어떤 α를 곱해도 에피폴라 제약이 성립합니다.\n" << std::endl;
}

void problem3_scale_drift()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Q3. 스케일 드리프트의 정의" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "스케일 드리프트(Scale Drift)란 무엇인가?\n" << std::endl;

    std::cout << "  a) 카메라 초점거리가 변하는 현상\n";
    std::cout << "  b) 매 프레임 스케일 오차가 누적되어 맵이 왜곡되는 현상\n";
    std::cout << "  c) 이미지 해상도가 점점 낮아지는 현상\n";
    std::cout << "  d) 특징점 수가 줄어드는 현상\n";
    std::cout << "  답: _____\n" << std::endl;

    std::cout << "💡 힌트: 매 프레임 1%의 스케일 오차가 100프레임 누적되면?\n" << std::endl;
}

void problem4_when_scale_known()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Q4. 스케일을 알 수 있는 경우" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "다음 중 절대 스케일을 결정할 수 있는 경우는?\n" << std::endl;

    std::cout << "  a) 단안 카메라만 사용\n";
    std::cout << "  b) 스테레오 카메라 (baseline 알려짐)\n";
    std::cout << "  c) 더 높은 해상도 카메라 사용\n";
    std::cout << "  d) 더 많은 특징점 추출\n";
    std::cout << "  답: _____\n" << std::endl;

    std::cout << "💡 힌트: 깊이 Z = f * b / d (스테레오 깊이 공식)\n";
    std::cout << "   baseline b가 물리적으로 알려진 값입니다.\n" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 12 Quiz - Easy" << std::endl;
    std::cout << "Monocular 스케일 모호성 기본 개념" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_why_no_scale();
    problem2_t_normalization();
    problem3_scale_drift();
    problem4_when_scale_known();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
