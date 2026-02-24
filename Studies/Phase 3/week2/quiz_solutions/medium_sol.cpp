/**
 * Phase 3 Week 2 - 2D-2D 중급 퀴즈 (정답)
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. Pure Rotation 문제 — t=0일 때 에피폴라 구조 붕괴
 *   2. Degenerate Case — 평면 장면에서의 E/H 선택
 *   3. Cheirality 구현 — 카메라 앞/뒤 판별 방법
 *
 * 2D-2D 모션 추정의 실패 조건:
 *
 *   Pure Rotation: t = 0 → E = [0]x · R = 0 → 에피폴라 제약 무의미
 *   평면 장면: 모든 점이 동일 평면 → E와 H 모두 성립 → 자유도 부족
 *   부족한 매칭: 매칭 수 < 8 (8-point) 또는 < 5 (5-point) → 해 불능
 *
 * ORB-SLAM2의 E/H 동시 추정:
 *   1. RANSAC으로 E 추정 → inlier 수 S_E
 *   2. RANSAC으로 H 추정 → inlier 수 S_H
 *   3. R_H = S_H / (S_H + S_E)
 *   4. R_H > 0.45이면 H 선택 (평면 장면), 아니면 E 선택
 *
 * 난이도: ★★☆ (실전 문제, 예외 처리)
 * 선수 지식: quiz_easy (E/F, 5-point, Cheirality, 스케일 모호성)
 */

#include <opencv2/opencv.hpp>
#include <iostream>

// 문제 1: Pure Rotation 문제 — t=0일 때의 붕괴
//
// E = [t]x · R
// t = 0이면: [t]x = [0] (영행렬) → E = 0
//
// 에피폴라 제약: x2^T · 0 · x1 = 0 → 항상 성립 (아무 정보 없음!)
//
// 물리적 의미:
//   카메라가 제자리에서 회전만 하면
//   모든 3D 점의 광선이 같은 투영 중심을 공유
//   → baseline = 0 → 삼각측량 불가 → 깊이 추정 불가
//
// ★ 해결 전략:
//   1. 충분한 평행이동이 있는 프레임 쌍 선택
//   2. 시차(parallax) 기반 KF 선택 (Week 6)
//   3. 회전만 하는 구간 감지 후 건너뛰기
//   4. ORB-SLAM: 초기화 시 충분한 시차 대기
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

// 문제 2: Degenerate Case — 평면 장면에서의 E/H 선택
//
// 평면 장면의 문제:
//   모든 3D 점이 동일 평면에 있으면
//   Homography H도 이 관계를 완벽히 설명 가능
//   E의 5 자유도 > 평면의 4 자유도 → E가 과도 결정(degenerate)
//   → E 추정이 불안정, 노이즈에 민감
//
// ORB-SLAM2의 해결책: E와 H 동시 추정
//   1. RANSAC으로 E 추정 → inlier 수 S_E
//   2. RANSAC으로 H 추정 → inlier 수 S_H
//   3. 비율 R_H = S_H / (S_H + S_E)
//   4. R_H > 0.45 → H 선택 (평면), 아니면 E 선택
//
// H에서 R, t 분해:
//   H = K · (R + t·n^T/d) · K⁻¹
//   여기서 n = 평면 법선, d = 카메라~평면 거리
//
// ★ 실제 환경: 벽, 바닥, 천장만 보이는 경우 → 평면 degenerate
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

// 문제 3: Cheirality 구현 — 카메라 앞/뒤 판별
//
// E의 SVD 분해 후 4개 (R, t) 조합에 대해:
//   각 조합으로 3D 점을 삼각측량 → 카메라 앞인지 확인
//
// 카메라 앞 판별:
//   카메라 1 좌표계에서: Z1 = X_cam1(2) > 0
//   카메라 2 좌표계에서: X_cam2 = R · X + t, Z2 = X_cam2(2) > 0
//
// 간단한 구현:
//   Z = R.row(2) · X_world + t(2)
//   if (Z > 0) → 카메라 앞에 있음
//
// ★ OpenCV recoverPose(E, pts1, pts2, K, R, t, mask):
//   내부적으로 4개 해 모두 시도 → Cheirality Check → 최적 해 반환
//   반환값 = 유효한 점 수 (카메라 앞에 있는 점)
//   mask = 각 점이 유효한지 여부
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
    std::cout << "Week 2 Quiz - Medium (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_pure_rotation();
    problem2_degenerate_case();
    problem3_implement_check();

    std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
