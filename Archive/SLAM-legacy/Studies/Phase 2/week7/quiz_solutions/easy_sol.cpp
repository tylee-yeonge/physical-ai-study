/**
 * Phase 2 Week 7 - PnP (Perspective-n-Point) 기초 퀴즈 (정답)
 *
 * 다루는 개념:
 *   - PnP 문제 정의: 3D-2D 대응에서 카메라 포즈 추정
 *   - PnP vs Essential Matrix 비교 (3D-2D vs 2D-2D)
 *   - RANSAC의 필요성 (outlier 대응)
 *   - Visual Odometry 파이프라인
 *   - DLT 삼각측량 A 행렬의 의미
 *
 * PnP는 이미 알려진 3D 점과 그 2D 투영으로부터 카메라 포즈를 구한다.
 * SLAM에서 초기화 이후 프레임마다 카메라를 추적(tracking)하는 핵심 방법이다.
 *
 *        3D 월드 점 (Xi)
 *            ●  ●  ●
 *           /|\ | /
 *          / | \|/
 *         /  |  ●
 *        /   |
 *       ◎────┘       ← 카메라 (R, t를 구해야 함)
 *      카메라
 *
 *   알려진 것: 3D 점 좌표 Xi, 이미지 투영 pi
 *   구할 것: 카메라 포즈 [R|t] (6 DoF)
 *
 *   투영 모델: pi = K · [R|t] · Xi
 *   → n개의 대응에서 R, t를 추정
 */

#include <opencv2/opencv.hpp>
#include <iostream>

// PnP 문제 정의 — 3D-2D 대응에서 카메라 포즈 추정
//
// Perspective-n-Point (PnP):
//   입력: n개의 3D 월드 점 Xi와 대응하는 2D 이미지 점 pi
//   출력: 카메라 포즈 [R|t] (회전 3 DoF + 이동 3 DoF = 6 DoF)
//
// 최소 점 개수:
//   - 각 대응점은 2개 제약식 제공 (x좌표, y좌표)
//   - 6 DoF ÷ 2 제약/점 = 최소 3점 (P3P)
//   - P3P는 4개 해가 나와 추가 1점으로 disambiguation 필요
//   - 실전에서는 RANSAC + 수십~수백 점 사용
//
// ★ PnP가 SLAM에서 중요한 이유:
//   - 초기화 후 매 프레임 tracking에 사용
//   - 이미 구축된 3D 맵 점을 활용하므로 효율적
//   - 절대 스케일 유지 (Essential Matrix와 달리)
void problem1_pnp_definition()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: PnP란?" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Perspective-n-Point (PnP):" << std::endl;
    std::cout << "   입력: n개의 3D-2D 대응점" << std::endl;
    std::cout << "   출력: 카메라 포즈 (R, t)\n" << std::endl;

    std::cout << "질문: 최소 몇 개 점이 필요한가요?\n" << std::endl;

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [정답] 최소 3개 (P3P), 실전에서는 4개 이상 + RANSAC" << std::endl;
    std::cout << std::endl;
    std::cout << "   [왜 3개면 충분한가?]" << std::endl;
    std::cout << "   포즈 자유도: 회전 3 DoF + 이동 3 DoF = 6 DoF" << std::endl;
    std::cout << "   각 대응점 → 2개 제약 (u, v 좌표)" << std::endl;
    std::cout << "   3개 점 → 6개 제약 = 6 미지수 → 풀 수 있음" << std::endl;
    std::cout << std::endl;
    std::cout << "   [P3P의 한계]" << std::endl;
    std::cout << "   3점으로 풀면 최대 4개 해가 나옴 → 추가 1점으로 disambiguation" << std::endl;
    std::cout << "   실전에서는 수십~수백 점 + RANSAC으로 outlier에 강건하게 추정" << std::endl;
}

// PnP vs Essential Matrix — 두 포즈 추정 방법의 비교
//
// Essential Matrix (2D-2D):
//   - 입력: 두 이미지의 2D 대응점 쌍
//   - 원리: 에피폴라 제약 p2^T · E · p1 = 0
//   - 장점: 3D 정보 없이도 상대 포즈 추정 가능
//   - 단점: 스케일 모호성 — t의 방향만 알 수 있고 크기는 모름
//   - 용도: SLAM 초기화 (첫 두 프레임)
//
// PnP (3D-2D):
//   - 입력: 알려진 3D 점 + 대응 2D 투영점
//   - 원리: 투영 방정식 p = K[R|t]X를 직접 풀기
//   - 장점: 절대 스케일 복원! (3D 점의 실제 거리 정보 사용)
//   - 단점: 사전에 3D 맵 점이 필요
//   - 용도: SLAM tracking (초기화 이후 매 프레임)
//
// ★ SLAM 초기화 전략:
//   Frame 0-1: Essential Matrix → 상대 포즈 (스케일=1)
//   → 삼각측량 → 초기 3D 맵 생성
//   Frame 2~: PnP로 tracking (절대 스케일 유지)
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

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [핵심 차이]" << std::endl;
    std::cout << "   ┌──────────────┬────────────────┬────────────────┐" << std::endl;
    std::cout << "   │              │ Essential (E)  │ PnP            │" << std::endl;
    std::cout << "   ├──────────────┼────────────────┼────────────────┤" << std::endl;
    std::cout << "   │ 입력         │ 2D ↔ 2D       │ 3D ↔ 2D       │" << std::endl;
    std::cout << "   │ 스케일       │ 모호 (방향만)  │ 절대 스케일!   │" << std::endl;
    std::cout << "   │ 3D 맵 필요?  │ 불필요         │ 필요           │" << std::endl;
    std::cout << "   │ 용도         │ 초기화         │ 매 프레임 추적 │" << std::endl;
    std::cout << "   └──────────────┴────────────────┴────────────────┘\n" << std::endl;
    std::cout << "   [SLAM 전략 — 왜 둘 다 필요한가?]" << std::endl;
    std::cout << "   1. Frame 0-1: 3D 점이 없으므로 E로 초기화 (스케일=1)" << std::endl;
    std::cout << "   2. 삼각측량 → 초기 3D 맵 생성" << std::endl;
    std::cout << "   3. Frame 2~: 3D 맵이 있으므로 PnP로 tracking" << std::endl;
    std::cout << "      → 절대 스케일 유지, 3D 점 매칭만으로 빠르게 포즈 추정" << std::endl;
    std::cout << std::endl;
    std::cout << "   [왜 E로만 하면 안 되나?]" << std::endl;
    std::cout << "   E는 매 프레임 스케일이 달라짐 → 궤적이 뒤틀림" << std::endl;
    std::cout << "   PnP는 실제 3D 좌표 기반 → 일관된 스케일 유지" << std::endl;
}

// RANSAC의 필요성 — outlier에 강건한 포즈 추정
//
// 문제: 실제 매칭에는 항상 outlier가 섞여 있다
//   - 특징점 매칭 오류 (유사한 descriptor)
//   - 동적 물체 (사람, 자동차 → rigid body 가정 위반)
//   - 반복 패턴 (벽돌, 타일 → 잘못된 대응)
//
// Outlier가 하나라도 있으면:
//   - 최소자승법(DLT)은 모든 점에 맞추려 해서 결과가 왜곡됨
//   - 재투영 오차가 폭증하여 포즈 추정 실패
//
// RANSAC (RANdom SAmple Consensus):
//   1. 랜덤으로 최소 점 집합 선택 (P3P: 4점)
//   2. 모델(포즈) 추정
//   3. 나머지 점들의 재투영 오차 계산
//   4. 임계값 이내인 점 = inlier 개수 세기
//   5. 반복 → inlier 최다인 모델 선택
//   6. 최종: 모든 inlier로 모델 재추정 (refinement)
//
// ★ cv::solvePnPRansac이 이 전체 과정을 한 번에 수행
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

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [왜 DLT만으로 안 되나?]" << std::endl;
    std::cout << "   DLT(최소자승법)는 모든 점을 동등하게 취급" << std::endl;
    std::cout << "   → outlier 1개가 전체 결과를 크게 왜곡시킴" << std::endl;
    std::cout << "   예: 100개 inlier + 1개 outlier → 포즈 추정 실패 가능\n" << std::endl;
    std::cout << "   [RANSAC 동작 과정]" << std::endl;
    std::cout << "   1. 랜덤으로 최소 점 선택 (P3P: 4점)" << std::endl;
    std::cout << "   2. 해당 점들로 포즈(R,t) 추정" << std::endl;
    std::cout << "   3. 나머지 점들의 재투영 오차 계산" << std::endl;
    std::cout << "   4. 오차 < 임계값 → inlier로 분류" << std::endl;
    std::cout << "   5. 반복 → inlier 최다인 모델 채택" << std::endl;
    std::cout << "   6. 최종: 모든 inlier로 모델 재추정 (refinement)\n" << std::endl;
    std::cout << "   [OpenCV에서의 사용]" << std::endl;
    std::cout << "   cv::solvePnPRansac()이 위 과정을 한 번에 수행" << std::endl;
    std::cout << "   반환값: rvec, tvec (포즈) + inliers (유효한 점 인덱스)" << std::endl;
}

// Visual Odometry 파이프라인 — 카메라로 자기 위치 추정
//
// Visual Odometry (VO):
//   연속 프레임의 이미지만으로 카메라의 이동 경로를 추정하는 기술.
//   SLAM의 핵심 구성 요소이며, "프론트엔드"에 해당한다.
//
// 단안 VO 파이프라인:
//
//   프레임 t-1          프레임 t
//   ┌──────────┐       ┌──────────┐
//   │ ● ○ ●    │ ───→  │  ● ○ ●   │
//   │    ○   ● │       │   ○   ●  │
//   └──────────┘       └──────────┘
//        ↓                  ↓
//   1. 특징점 검출 (ORB/FAST)
//        ↓
//   2. 특징점 매칭 (BF/FLANN + Ratio Test)
//        ↓
//   3. 포즈 추정:
//      - 초기화: Essential Matrix → R, t
//      - Tracking: PnP + RANSAC → R, t
//        ↓
//   4. 삼각측량 → 새 3D 점 추가 (맵 확장)
//        ↓
//   5. 다음 프레임으로 반복
//
// ★ VO의 한계: 오차 누적 (drift)
//   → Loop Closure + 전역 최적화로 보정 = SLAM
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

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [VO 파이프라인 5단계 정리]" << std::endl;
    std::cout << "   ① 특징점 검출: ORB/FAST로 각 프레임에서 키포인트 추출" << std::endl;
    std::cout << "   ② 특징점 매칭: BF/FLANN + Ratio Test로 대응점 찾기" << std::endl;
    std::cout << "   ③ 포즈 추정: 초기화=Essential, 이후=PnP+RANSAC" << std::endl;
    std::cout << "   ④ 삼각측량: 새 매칭점 → 3D 맵에 점 추가" << std::endl;
    std::cout << "   ⑤ 반복: 다음 프레임에서 ①부터 반복\n" << std::endl;
    std::cout << "   [VO의 핵심 한계: Drift]" << std::endl;
    std::cout << "   매 프레임 포즈에 미세한 오차 존재" << std::endl;
    std::cout << "   → 오차가 누적되어 궤적이 실제와 점점 어긋남" << std::endl;
    std::cout << "   예: 프레임당 0.1도 오차 → 1000프레임 후 100도 틀어짐\n" << std::endl;
    std::cout << "   [VO → SLAM으로 발전]" << std::endl;
    std::cout << "   VO + Loop Closure(같은 장소 재방문 감지)" << std::endl;
    std::cout << "   + 전역 최적화(Bundle Adjustment/Pose Graph)" << std::endl;
    std::cout << "   = SLAM (drift를 보정하여 일관된 맵 유지)" << std::endl;
}

// DLT 삼각측량 개념 — 외적 소거법으로 3D 점 복원
//
// 투영 방정식 (동차 좌표):
//   λ·p = P · X
//   여기서 p = [u, v, 1]^T, P = K[R|t] (3×4), X = [X, Y, Z, 1]^T
//
// 외적 소거법 (cross product elimination):
//   p와 P·X는 같은 방향 (스케일 λ만 다름)
//   → p × (P·X) = 0  (같은 방향 벡터의 외적 = 0)
//
// 전개하면 각 뷰에서 2개 독립 방정식:
//   u·(P₃ᵀ·X) - (P₁ᵀ·X) = 0   ← x좌표 제약
//   v·(P₃ᵀ·X) - (P₂ᵀ·X) = 0   ← y좌표 제약
//   (Pᵢ는 P의 i번째 행벡터)
//
// 2개 뷰 → 4개 방정식 → A·X = 0 (4×4 선형 시스템):
//
//       ┌ u₁·P₁₃ᵀ - P₁₁ᵀ ┐
//   A = │ v₁·P₁₃ᵀ - P₁₂ᵀ │   (4×4 행렬)
//       │ u₂·P₂₃ᵀ - P₂₁ᵀ │
//       └ v₂·P₂₃ᵀ - P₂₂ᵀ ┘
//
// SVD로 풀기: A = U·S·Vᵀ → X = V의 마지막 열
// 동차→유클리드 변환: X₃ₐ = X[:3] / X[3]
//
// ★ 뷰가 많을수록 A의 행이 증가 → 과잉결정 시스템 → 노이즈에 더 강건
void problem5_dlt_triangulation_concept()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: DLT 삼각측량 개념" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "DLT 삼각측량 원리:\n" << std::endl;

    std::cout << "투영 방정식: p = P * X  (동차 좌표)" << std::endl;
    std::cout << "   p: [u, v, 1]^T  (2D 이미지 좌표)" << std::endl;
    std::cout << "   P: 3x4 투영 행렬 = K[R|t]" << std::endl;
    std::cout << "   X: [X, Y, Z, 1]^T  (3D 동차 좌표)\n" << std::endl;

    std::cout << "외적 소거법: p x (P*X) = 0" << std::endl;
    std::cout << "   동차 좌표에서 p와 P*X는 같은 방향" << std::endl;
    std::cout << "   → 외적 = 0\n" << std::endl;

    std::cout << "전개하면 각 뷰에서 2개 독립 방정식:" << std::endl;
    std::cout << "   u*(P3^T*X) - (P1^T*X) = 0" << std::endl;
    std::cout << "   v*(P3^T*X) - (P2^T*X) = 0\n" << std::endl;

    std::cout << "2개 뷰 → 4개 방정식, 미지수 4개 (동차 X):" << std::endl;
    std::cout << "        [ u1*P1_3^T - P1_1^T ]" << std::endl;
    std::cout << "   A =  [ v1*P1_3^T - P1_2^T ]   (4x4)" << std::endl;
    std::cout << "        [ u2*P2_3^T - P2_1^T ]" << std::endl;
    std::cout << "        [ v2*P2_3^T - P2_2^T ]\n" << std::endl;

    std::cout << "SVD로 해 구하기:" << std::endl;
    std::cout << "   A = U * S * V^T" << std::endl;
    std::cout << "   X = V의 마지막 열 (가장 작은 특이값)" << std::endl;
    std::cout << "   X_3d = X[:3] / X[3]  (동차→유클리드)\n" << std::endl;

    // 간단한 수치 예제
    std::cout << "수치 예제:" << std::endl;
    std::cout << "   K = [500  0  320]" << std::endl;
    std::cout << "       [  0 500 240]" << std::endl;
    std::cout << "       [  0   0   1]\n" << std::endl;

    std::cout << "   카메라 1: P1 = K*[I|0]  (원점)" << std::endl;
    std::cout << "   카메라 2: P2 = K*[R|t]  (오른쪽 0.5m)" << std::endl;
    std::cout << "   3D 점: [0.3, -0.2, 5.0]\n" << std::endl;

    std::cout << "질문: A 행렬의 각 행이 무엇을 의미하나요?\n" << std::endl;

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [A 행렬의 각 행 의미]" << std::endl;
    std::cout << "   행 1: 카메라1의 u좌표 제약 → u₁(P₁₃ᵀX) - P₁₁ᵀX = 0" << std::endl;
    std::cout << "   행 2: 카메라1의 v좌표 제약 → v₁(P₁₃ᵀX) - P₁₂ᵀX = 0" << std::endl;
    std::cout << "   행 3: 카메라2의 u좌표 제약 → u₂(P₂₃ᵀX) - P₂₁ᵀX = 0" << std::endl;
    std::cout << "   행 4: 카메라2의 v좌표 제약 → v₂(P₂₃ᵀX) - P₂₂ᵀX = 0\n" << std::endl;
    std::cout << "   [핵심 정리]" << std::endl;
    std::cout << "   - 1개 뷰 → 2개 방정식 (u, v 각각 1개씩)" << std::endl;
    std::cout << "   - 2개 뷰 → 4개 방정식 → 미지수 4개(동차 X) → 풀 수 있음" << std::endl;
    std::cout << "   - 3개 뷰 → 6개 방정식 → 과잉결정 → 노이즈에 더 강건\n" << std::endl;
    std::cout << "   [SVD로 푸는 이유]" << std::endl;
    std::cout << "   AX = 0에서 X ≠ 0인 해가 필요" << std::endl;
    std::cout << "   → SVD의 마지막 열(가장 작은 특이값) = 최소 오차 해" << std::endl;
    std::cout << "   → X[3]으로 나누어 동차→유클리드 변환: [X,Y,Z]" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 7 Quiz - Easy (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_pnp_definition();
    problem2_pnp_vs_essential();
    problem3_ransac_necessity();
    problem4_vo_pipeline();
    problem5_dlt_triangulation_concept();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
