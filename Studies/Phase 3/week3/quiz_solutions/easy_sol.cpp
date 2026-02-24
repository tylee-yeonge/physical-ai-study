/**
 * Phase 3 Week 3 - PnP 기초 퀴즈 (정답)
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. PnP 정의 — 3D-2D 대응으로 카메라 포즈 추정
 *   2. P3P vs EPnP — 최소 점 수와 알고리즘 특성
 *   3. PnP의 스케일 복원 — 2D-2D와의 핵심 차이
 *   4. Inlier Ratio — 추적 품질 지표와 임계값
 *
 * 2D-2D vs 3D-2D 비교:
 *
 *   2D-2D (Essential Matrix):
 *     입력: 2D↔2D 대응 → 출력: R, t (||t||=1, 스케일 모호)
 *     용도: 초기화 (3D 맵이 아직 없을 때)
 *
 *   3D-2D (PnP):
 *     입력: 3D↔2D 대응 → 출력: R, t (절대 스케일!)
 *     용도: 추적 (이미 3D 맵이 있을 때)
 *
 *     알려진 3D 점 X_i ──투영──→ 예측 2D π(R,t,X_i)
 *                                  ↕ 비교
 *     이미지에서 관측된 2D x_i ←── 특징점 매칭
 *
 * 난이도: ★☆☆ (기본 개념, 답이 포함된 학습형)
 * 선수 지식: Week 2 (2D-2D, Essential Matrix)
 */

#include <iostream>

// 문제 1: PnP 정의 — Perspective-n-Point
//
// PnP 문제:
//   주어진 것: n개의 3D-2D 대응 {(X_i, x_i)}
//     X_i = 3D 점 (월드 좌표, 이미 알고 있음 — 맵에서)
//     x_i = 2D 관측 (현재 이미지에서 검출된 특징점)
//
//   구하는 것: 카메라 포즈 [R|t]
//     x_i = π(R·X_i + t) = K·(R·X_i + t) (투영 방정식)
//
// VO/SLAM에서의 역할:
//   1. Tracking: 이전 KF의 맵 포인트를 현재 이미지에서 찾음
//   2. 매칭된 3D-2D 대응으로 현재 포즈 추정
//   3. 매 프레임 반복 → 실시간 포즈 추적
//
// ★ 2D-2D는 초기화용, 3D-2D(PnP)는 추적용
void problem1_pnp_definition()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: PnP 정의" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: PnP가 푸는 문제는?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   주어진 것: {(X_i, x_i)}" << std::endl;
    std::cout << "     - X_i: 3D 점 (월드)" << std::endl;
    std::cout << "     - x_i: 2D 관측 (이미지)\n" << std::endl;
    std::cout << "   구하는 것: [R|t] (카메라 포즈)" << std::endl;
}

// 문제 2: P3P vs EPnP — 알고리즘 비교
//
// P3P (Perspective-3-Point):
//   최소 3개 점으로 포즈 추정 → 최대 4개 해
//   + RANSAC에 적합 (샘플 크기 3 → 적은 반복)
//   - 4개 해 중 선택 필요 (4번째 점으로 검증)
//   복잡도: O(1) per sample
//
// EPnP (Efficient PnP):
//   n개 점 모두 사용 → 1개 해 (직접 풀이)
//   4개 제어점(control point)으로 문제를 축소
//   + 안정적, 빠름 O(n)
//   + OpenCV 기본 PnP 솔버
//   - RANSAC에는 P3P가 더 적합 (샘플 크기 작으니까)
//
// OpenCV에서:
//   solvePnP(objectPoints, imagePoints, K, distCoeffs, rvec, tvec, false, SOLVEPNP_EPNP);
//   solvePnPRansac(..., SOLVEPNP_P3P);  // RANSAC + P3P
//
// ★ 실전: RANSAC + P3P로 outlier 제거 후, EPnP로 정밀 추정
void problem2_p3p_vs_epnp()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: P3P vs EPnP" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "P3P:" << std::endl;
    std::cout << "   - 최소 3개 점" << std::endl;
    std::cout << "   - 4개 해 존재" << std::endl;
    std::cout << "   - RANSAC에 적합\n" << std::endl;

    std::cout << "EPnP:" << std::endl;
    std::cout << "   - n개 점 (O(n))" << std::endl;
    std::cout << "   - 안정적이고 빠름" << std::endl;
    std::cout << "   - OpenCV 기본" << std::endl;
}

// 문제 3: PnP의 스케일 복원 — 2D-2D와의 핵심 차이
//
// 2D-2D (Essential Matrix):
//   E = [t]x · R → t의 방향만 복원 (||t||=1 정규화)
//   → 절대 거리를 알 수 없음 (스케일 모호)
//
// 3D-2D (PnP):
//   x_i = π(R·X_i + t)
//   X_i가 "실제 크기"를 알고 있으므로
//   → t도 실제 단위(미터)로 복원됨!
//
// 직관:
//   1m 떨어진 점과 10m 떨어진 점이 같은 크기로 보인다면?
//   2D-2D: 구분 불가 (투영이 동일)
//   PnP: 3D 점의 실제 좌표를 알므로 구분 가능
//
// ★ 이것이 VO에서 PnP를 사용하는 가장 큰 이유
//   초기화(2D-2D) 후에는 맵이 있으므로 PnP로 추적 → 스케일 유지
void problem3_scale_recovery()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 스케일 복원" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: PnP가 스케일을 복원하는 이유?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   3D 점의 절대 크기를 알기 때문!" << std::endl;
    std::cout << "   - 2D-2D: ||t|| = 1 (정규화)" << std::endl;
    std::cout << "   - 3D-2D: ||t|| = 실제 이동 거리" << std::endl;
}

// 문제 4: Inlier Ratio — 추적 품질 지표
//
// Inlier Ratio = (PnP 후 유효한 점 수) / (전체 매칭 수)
//
// RANSAC PnP 후:
//   Inlier = 재투영 오차 < 임계값 (보통 3~5 픽셀)인 점
//   Outlier = 잘못된 매칭, 동적 물체, 맵 오류 등
//
// 품질 해석:
//   > 70%: 양호 — 안정적 추적, 맵이 정확
//   > 50%: 보통 — 약간의 노이즈, 일부 오매칭
//   < 30%: 위험 — 추적 실패 직전
//            → 재초기화 (2D-2D) 또는 Relocalization 필요
//
// ★ ORB-SLAM2에서:
//   Inlier < 10개 → 추적 실패 → Relocalization 시도
//   Relocalization = BoW로 유사 KF 검색 → PnP로 포즈 복구
void problem4_inlier_ratio()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Inlier Ratio" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Inlier Ratio = inliers / total\n" << std::endl;

    std::cout << "💡 해석:" << std::endl;
    std::cout << "   > 70%: 양호" << std::endl;
    std::cout << "   > 50%: 보통" << std::endl;
    std::cout << "   < 30%: 추적 실패 → 재초기화" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 3 Quiz - Easy (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_pnp_definition();
    problem2_p3p_vs_epnp();
    problem3_scale_recovery();
    problem4_inlier_ratio();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
