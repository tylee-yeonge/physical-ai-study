/**
 * Phase 2 Week 8 - Optical Flow 기초 퀴즈 (정답)
 *
 * 다루는 개념:
 *   - Lucas-Kanade (LK) Optical Flow의 3가지 가정
 *   - 조리개 문제 (Aperture Problem)
 *   - Pyramidal Optical Flow (큰 움직임 대응)
 *   - SLAM에서의 Optical Flow 활용
 *   - 이미지 그래디언트와 LK 방정식 유도
 *   - Structure Tensor 고유값 기반 추적 가능성 판별
 *
 * Optical Flow는 연속 프레임에서 픽셀의 움직임(motion vector)을 추정한다.
 * 특징점 매칭(descriptor) 없이 직접 밝기 패턴으로 추적하는 방법이다.
 *
 *   프레임 t          프레임 t+1
 *   ┌──────────┐     ┌──────────┐
 *   │    ●     │     │      ●   │
 *   │  ●   ●  │ ──→ │   ●   ● │
 *   │    ●     │     │     ●   │
 *   └──────────┘     └──────────┘
 *        각 점의 이동 벡터 (u, v) = Optical Flow
 *
 * 핵심 방정식 (Brightness Constancy):
 *   I(x, y, t) = I(x+u, y+v, t+1)
 *   Taylor 전개 → Ix·u + Iy·v + It = 0  (Optical Flow Constraint)
 *   1개 방정식, 2개 미지수 → 추가 제약 필요 (Aperture Problem)
 */

#include <opencv2/opencv.hpp>
#include <iostream>

// Lucas-Kanade Optical Flow의 3가지 가정
//
// 1. Brightness Constancy (밝기 일정):
//    같은 물체 점의 밝기는 프레임 간 변하지 않는다.
//    I(x, y, t) = I(x+dx, y+dy, t+dt)
//    → 이 가정으로 Optical Flow Constraint 유도
//    ★ 위반 사례: 조명 변화, 그림자, 반사
//
// 2. Small Motion (작은 움직임):
//    프레임 간 이동이 충분히 작아서 Taylor 1차 근사가 유효하다.
//    I(x+dx, ...) ≈ I(x, ...) + Ix·dx + Iy·dy + It·dt
//    ★ 위반 사례: 빠른 카메라 회전, 빠른 물체 → Pyramid로 해결
//
// 3. Spatial Coherence (공간 일관성):
//    윈도우 내 이웃 픽셀들은 모두 같은 flow (u, v)를 가진다.
//    → 하나의 윈도우에서 N개 방정식, 2개 미지수 → 과잉결정 시스템
//    → (AᵀA)⁻¹Aᵀb 로 최소자승 해 구함
//    ★ 위반 사례: 물체 경계면 (서로 다른 속도)
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

// 조리개 문제 (Aperture Problem) — 1개 방정식, 2개 미지수
//
// Optical Flow Constraint: Ix·u + Iy·v + It = 0
//   - 미지수 2개 (u, v), 방정식 1개 → 해가 무한
//   - 에지 방향(수직)의 flow만 결정 가능, 에지 따라가는 flow는 모호
//
// 시각화:
//   수직 에지 |||||    → 좌우 이동 감지 가능 ✓
//              |||||    → 상하 이동 감지 불가 ✗ (에지가 똑같이 보임)
//
// 해결 방법:
//   1. 코너점 사용: 두 방향 모두 밝기 변화 → 모호성 없음
//      (Harris, Shi-Tomasi goodFeaturesToTrack)
//   2. 윈도우 확대: 더 많은 제약식 → 과잉결정
//   3. Pyramidal approach: 큰 움직임도 small motion으로 환원
//
// ★ Aperture Problem은 LK뿐 아니라 모든 로컬 flow 방법의 근본 한계
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

// Pyramidal Optical Flow — 큰 움직임 대응을 위한 다중 해상도 전략
//
// 문제: LK는 Small Motion 가정 → 큰 움직임에서 실패
//   - 픽셀이 윈도우 밖으로 이동하면 추적 불가
//   - Taylor 근사가 부정확해짐
//
// 해결: Image Pyramid (Coarse-to-Fine)
//
//   Level 3:  [■]           ← 1/8 해상도 (큰 움직임이 작게 보임)
//                ↓ 추정
//   Level 2:  [■■]          ← 1/4 해상도
//                ↓ 전파 (×2 업샘플)
//   Level 1:  [■■■■]        ← 1/2 해상도
//                ↓ 전파 (×2 업샘플)
//   Level 0:  [■■■■■■■■]    ← 원본 해상도 (정밀 보정)
//
// 각 레벨에서:
//   1. 이전 레벨의 flow를 2배로 업샘플
//   2. 그 flow로 이미지를 warp (대략적 정렬)
//   3. 잔여(residual) flow만 LK로 추정
//   4. 누적하여 최종 flow
//
// ★ OpenCV calcOpticalFlowPyrLK에서 maxLevel 파라미터로 피라미드 레벨 설정
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

// SLAM에서의 Optical Flow 활용 — 특징점 추적과 직접법
//
// Optical Flow는 SLAM에서 두 가지 방식으로 활용된다:
//
// 1. Sparse Flow (특징점 추적):
//    - goodFeaturesToTrack으로 코너점 검출
//    - calcOpticalFlowPyrLK로 프레임 간 추적
//    - 장점: descriptor 계산 불필요 → 빠름
//    - 대표: VINS-Mono (시각-관성 SLAM)
//
// 2. Dense Flow (밀집 추정):
//    - 모든 픽셀의 flow 추정 (Farneback, DeepFlow)
//    - 장점: 텍스처 없는 영역도 추적
//    - 단점: 계산량 많음 (GPU 필요)
//    - 대표: LSD-SLAM (직접법 SLAM)
//
// 3. Hybrid (병합):
//    - Flow + descriptor 매칭 함께 사용
//    - Flow로 빠른 초기 추적 → 매칭으로 검증
//
// ★ Feature 기반 vs Flow 기반 트레이드오프:
//   Feature: 조명 변화에 강건, 넓은 베이스라인 OK, 계산량 중간
//   Flow: 빠름, 부드러운 궤적, 좁은 베이스라인만 OK, 조명에 약함
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

// 이미지 그래디언트와 LK 방정식 — Optical Flow의 수학적 기초
//
// 1단계: 이미지 그래디언트 계산
//   Ix = ∂I/∂x : Sobel x 커널로 x 방향 밝기 변화 계산
//   Iy = ∂I/∂y : Sobel y 커널로 y 방향 밝기 변화 계산
//   It = ∂I/∂t : 프레임 차이 I(t+1) - I(t)
//
// 2단계: Optical Flow Constraint
//   Ix·u + Iy·v + It = 0  (각 픽셀마다 1개 방정식)
//
// 3단계: Lucas-Kanade (윈도우 내 N개 픽셀)
//   ┌ Ix₁  Iy₁ ┐       ┌ -It₁ ┐
//   │ Ix₂  Iy₂ │ [u] = │ -It₂ │
//   │  ⋮    ⋮  │ [v]   │  ⋮   │
//   └ IxN  IyN ┘       └ -ItN ┘
//       A (N×2)   d(2×1)   b(N×1)
//
// 최소자승 해: d = (AᵀA)⁻¹ · Aᵀb
//
// AᵀA = Structure Tensor M (2×2):
//   M = [ ΣIx²    ΣIxIy ]
//       [ ΣIxIy   ΣIy²  ]
//
// ★ M이 역행렬이 존재하려면 두 고유값이 모두 충분히 커야 함
//   → 코너 영역에서만 안정적 추적 가능 (= goodFeaturesToTrack의 원리)
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

// Structure Tensor 고유값 기반 추적 가능성 판별
//
// Structure Tensor M = AᵀA 의 고유값 (λ₁, λ₂)로 영역 분류:
//
//   λ₂ ↑
//      │  ┌─────────────┐
//      │  │  Corner     │  λ₁ >> 0, λ₂ >> 0
//      │  │  (추적 OK)  │  → 두 방향 모두 밝기 변화
//      │  └─────────────┘
//      │
//      │  Edge           λ₁ >> 0, λ₂ ≈ 0
//      │  (Aperture!)   → 한 방향만 밝기 변화
//      │
//      │  Flat           λ₁ ≈ 0, λ₂ ≈ 0
//      │  (추적 불가)   → 밝기 변화 없음
//      └────────────────→ λ₁
//
// Harris Corner Detection과의 관계:
//   Harris M = Structure Tensor (동일한 행렬!)
//   R = det(M) - k·trace(M)² = λ₁λ₂ - k(λ₁+λ₂)²
//   → 코너: R 큼 (양수) → 추적 가능
//   → 에지: R 음수 → Aperture Problem
//   → 평면: R ≈ 0 → 추적 불가
//
// ★ Shi-Tomasi (goodFeaturesToTrack): min(λ₁, λ₂) > threshold
//   → LK Optical Flow에 최적화된 특징점 선택 기준
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
    std::cout << "Phase 2 Week 8 Quiz - Easy (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_optical_flow_assumptions();
    problem2_aperture_problem();
    problem3_pyramidal_flow();
    problem4_slam_application();
    problem5_gradient_and_lk_equation();
    problem6_trackability();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
