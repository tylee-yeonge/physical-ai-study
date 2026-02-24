/**
 * Phase 3 Week 2 - 2D-2D 기초 퀴즈 (정답)
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. Essential vs Fundamental Matrix — 좌표계와 캘리브레이션의 차이
 *   2. 5-Point Algorithm — E의 자유도와 최소 점 수
 *   3. Cheirality Check — SVD 분해 후 4개 해 중 올바른 해 선택
 *   4. 스케일 모호성 — E에서 t의 크기를 알 수 없는 이유
 *
 * 에피폴라 기하학 핵심 수식:
 *
 *   x2^T · E · x1 = 0       (정규화 좌표, 캘리브레이션 필요)
 *   p2^T · F · p1 = 0       (픽셀 좌표, 캘리브레이션 불필요)
 *   E = [t]x · R             (Essential = 이동의 반대칭 × 회전)
 *   F = K2^{-T} · E · K1^{-1} (Fundamental = K로 E를 변환)
 *
 *       x1          e1
 *       ↑          /
 *   [카메라1] ────t──── [카메라2]
 *       \                 ↑
 *        \               /
 *         → 3D 점 P ←──
 *
 * 난이도: ★☆☆ (기본 개념, 답이 포함된 학습형)
 * 선수 지식: Phase 2 Week 5 (에피폴라 기하학), Phase 3 Week 1 (VO 개요)
 */

#include <iostream>

// 문제 1: Essential vs Fundamental Matrix
//
// Essential Matrix E:
//   - 입력: 정규화 좌표 x = K⁻¹·p (카메라 내부 파라미터로 보정된 좌표)
//   - 제약: x2^T · E · x1 = 0
//   - 자유도: 5 (회전 3 + 이동 방향 2)
//   - 특이값: [σ, σ, 0] (두 특이값이 동일)
//   - ★ 캘리브레이션 K를 알아야 사용 가능
//
// Fundamental Matrix F:
//   - 입력: 픽셀 좌표 p (원본 이미지 좌표)
//   - 제약: p2^T · F · p1 = 0
//   - 자유도: 7 (9원소 - 스케일 1 - rank2 제약 1)
//   - F = K2^{-T} · E · K1^{-1}
//   - ★ 캘리브레이션 불필요 → 비보정 카메라에서도 사용 가능
//
// SLAM에서의 선택:
//   보통 E 사용 (카메라가 이미 캘리브레이션된 경우가 대부분)
//   F는 캘리브레이션 없이 기하학 관계만 필요할 때
void problem1_essential_vs_fundamental()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Essential vs Fundamental" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: E와 F의 차이는?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "Essential Matrix (E):" << std::endl;
    std::cout << "   - 정규화 좌표 사용" << std::endl;
    std::cout << "   - 캘리브레이션 필요" << std::endl;
    std::cout << "   - p2^T * E * p1 = 0\n" << std::endl;

    std::cout << "Fundamental Matrix (F):" << std::endl;
    std::cout << "   - 픽셀 좌표 사용" << std::endl;
    std::cout << "   - 캘리브레이션 불필요" << std::endl;
    std::cout << "   - F = K2^{-T} * E * K1^{-1}" << std::endl;
}

// 문제 2: 5-Point Algorithm — E의 자유도
//
// E = [t]x · R 에서:
//   R: SO(3) → 3 자유도 (회전)
//   t: 방향만 의미 (||t||=1 정규화) → 2 자유도
//   합계: 5 자유도
//
// 각 대응점 쌍 (x1, x2)는 1개의 제약을 줌:
//   x2^T · E · x1 = 0
//
// → 최소 5개 대응점 = 5개 제약 → E의 5 자유도 결정
//
// ★ 8-Point Algorithm도 있음:
//   E를 3×3 일반 행렬로 취급 → 9원소 - 1(스케일) = 8 자유도
//   → 최소 8개 점 필요 (구현이 더 간단)
//   OpenCV: findEssentialMat(pts1, pts2, K, RANSAC)
void problem2_five_point()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 5-Point Algorithm" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 왜 최소 5개 점이 필요한가?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   E의 자유도 = 5" << std::endl;
    std::cout << "   - 회전: 3 DoF" << std::endl;
    std::cout << "   - 이동 방향: 2 DoF (크기는 모호)" << std::endl;
    std::cout << "   → 최소 5개 제약 필요" << std::endl;
}

// 문제 3: Cheirality Check — 4개 해 중 올바른 해 선택
//
// E의 SVD 분해: E = U·diag(σ,σ,0)·V^T
// R, t 복원 시 4개 조합 발생:
//   (R1, t), (R1, -t), (R2, t), (R2, -t)
//
// 4개 중 물리적으로 유효한 해 = 1개만
//   조건: 삼각측량된 3D 점이 "두 카메라 모두의 앞"에 있어야 함
//   카메라 앞 = 카메라 좌표계에서 Z > 0
//
//   해1: ○ 앞 / ○ 앞 → 정답!
//   해2: ○ 앞 / ✗ 뒤
//   해3: ✗ 뒤 / ○ 앞
//   해4: ✗ 뒤 / ✗ 뒤
//
// ★ OpenCV recoverPose()가 자동으로 Cheirality Check 수행
//   반환값: 유효한 점 수 (가장 많은 점이 앞에 있는 해)
void problem3_cheirality()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Cheirality Check" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 4개 해 중 어떻게 선택하는가?\n" << std::endl;

    std::cout << "💡 답: Cheirality Check" << std::endl;
    std::cout << "   - 3D 점이 두 카메라 앞에 있는지 확인" << std::endl;
    std::cout << "   - depth > 0 (카메라 좌표계)" << std::endl;
    std::cout << "   - 가장 많은 점이 앞에 있는 해 선택" << std::endl;
}

// 문제 4: 스케일 모호성 — E에서 t의 크기를 모르는 이유
//
// 에피폴라 제약: x2^T · E · x1 = 0
// E = [t]x · R = [αt]x · R (α > 0)
//
// x2^T · [αt]x · R · x1 = α · (x2^T · [t]x · R · x1) = α · 0 = 0
//
// → 어떤 α를 곱해도 제약이 성립
// → 에피폴라 기하학에서 t의 크기(norm)는 결정 불가
// → ||t|| = 1로 정규화하여 방향만 사용
//
// 결과:
//   초기화 시 첫 두 프레임의 이동 크기를 1로 설정
//   이후 프레임의 스케일은 이전과의 일관성으로 추정
//   → 매 프레임 스케일 오차 → 드리프트 누적
//
// ★ 이것이 Monocular VO의 근본 한계 (Week 12-13에서 해결 방법 학습)
void problem4_scale_ambiguity()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 스케일 모호성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 왜 t의 크기를 모르는가?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   E = t^ * R" << std::endl;
    std::cout << "   E = (α*t)^ * R  (동일!)" << std::endl;
    std::cout << "   → 투영 방정식에서 α 소거됨" << std::endl;
    std::cout << "   → 초기화 시 ||t|| = 1로 정규화" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 2 Quiz - Easy (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_essential_vs_fundamental();
    problem2_five_point();
    problem3_cheirality();
    problem4_scale_ambiguity();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
