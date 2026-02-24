/**
 * Quiz Easy - Week 12: Monocular 스케일 모호성
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. 단안 카메라의 스케일 불결정성 — 투영의 근본적 한계
 *   2. Essential Matrix에서 ||t||=1 정규화의 의미
 *   3. 스케일 드리프트 — 매 프레임 오차의 누적
 *   4. 절대 스케일을 결정할 수 있는 조건
 *
 * 스케일 모호성의 수학적 근거:
 *
 *   핀홀 투영: λ·[u, v, 1]^T = K·[R|t]·[X, 1]^T
 *   3D 점 X와 λX가 동일한 (u, v)에 투영됨
 *
 *   Essential Matrix: E = [t]x · R = [αt]x · R (α > 0)
 *   → t의 방향만 복원 가능, 크기는 불확정
 *
 * 스케일 복원 방법:
 *   Stereo: Z = f·b/d (baseline b가 알려진 물리량)
 *   IMU: 가속도 적분으로 이동 거리 측정
 *   알려진 물체: 도로 표지판, 차선 폭 등
 *
 * 난이도: ★☆☆ (기본 개념)
 * 선수 지식: Week 2 (Essential Matrix), Week 1 (VO 파이프라인)
 */

#include <iostream>

// 문제 1: 단안 카메라가 스케일을 결정할 수 없는 이유
//
// 핀홀 모델: λ·m = K·(R·X + t)
//   m = [u, v, 1]^T, X = 3D 점, λ = 깊이
//
// X를 αX로 대체하면:
//   K·(R·αX + αt) = α·K·(R·X + t) = α·λ·m
//   → 새로운 깊이 αλ로 동일한 픽셀에 투영
//
// 직관:
//   1m 물체 @ 2m vs 10m 물체 @ 20m → 이미지에서 동일!
//
// ★ 이것이 Monocular VO/SLAM의 근본 한계
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

// 문제 2: Essential Matrix에서 ||t|| = 1 정규화의 의미
//
// 에피폴라 제약: x2^T · E · x1 = 0
// E = [t]x · R 에서 t를 αt로 바꾸면:
//   x2^T · [αt]x · R · x1 = α · (x2^T · [t]x · R · x1) = α · 0 = 0
//
// → α가 무엇이든 제약이 성립
// → t의 크기(norm)는 에피폴라 기하학에서 결정 불가
// → 관례적으로 ||t|| = 1로 정규화하여 방향만 사용
//
// ★ 이것이 Monocular 초기화에서 스케일을 1로 설정하는 이유
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

// 문제 3: 스케일 드리프트의 정의
//
// Monocular VO에서 매 프레임 스케일을 독립적으로 추정:
//   프레임 k: ||t_k|| = s_k (추정된 스케일)
//   실제 스케일: s_k* (알 수 없음)
//   오차: e_k = s_k / s_k*
//
// n프레임 누적 시:
//   총 스케일 = s_1 · s_2 · ... · s_n
//   오차 누적 = e_1 · e_2 · ... · e_n
//   매 프레임 1% 오차 → 100프레임 후: 1.01^100 ≈ 2.7배
//
// 결과: 맵의 크기가 시간에 따라 점점 왜곡됨
//
// ★ Loop Closure나 IMU 없이는 스케일 드리프트를 막을 수 없음
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

// 문제 4: 절대 스케일을 결정할 수 있는 조건
//
// Stereo 카메라:
//   Z = f · b / d (깊이 공식)
//   baseline b가 물리적으로 알려진 값 → 절대 깊이 복원
//
// IMU 융합 (VIO):
//   가속도 이중 적분 → 이동 거리(미터) 직접 측정
//   비전의 방향 + IMU의 스케일 → 상호 보완
//
// 알려진 크기의 물체:
//   도로 표지판, 차선 폭 등 사전 지식으로 스케일 보정
//
// ★ 공통점: 이미지 외부의 물리적 정보가 반드시 필요
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
