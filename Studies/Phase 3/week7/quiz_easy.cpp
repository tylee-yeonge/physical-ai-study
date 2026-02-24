/**
 * Quiz Easy - Week 7: Bundle Adjustment (g2o)
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. BA의 목적 함수 — 재투영 오차 최소화
 *   2. g2o의 Vertex — 최적화 변수 (포즈, 3D 점)
 *   3. g2o의 Edge — 관측/제약 조건 (재투영 오차)
 *   4. Huber Loss — Outlier에 강건한 비용 함수
 *   5. Schur Complement — BA의 희소 구조를 이용한 속도 향상
 *
 * Bundle Adjustment 목적 함수:
 *
 *   min  Σ  ||z_ij - π(T_i, P_j)||²
 *   T,P  i,j
 *
 *   z_ij = 카메라 i에서 3D 점 j의 2D 관측 (픽셀)
 *   T_i  = 카메라 i의 포즈 (SE(3), 6 DoF)
 *   P_j  = 3D 점 j의 좌표 (3 DoF)
 *   π()  = 투영 함수: P_cam = R·P + t → (fx·X/Z + cx, fy·Y/Z + cy)
 *
 * g2o 그래프 구조:
 *
 *   [Pose 0] ──Edge── [Point 0]
 *   [Pose 0] ──Edge── [Point 1]
 *   [Pose 1] ──Edge── [Point 0]    ← 각 Edge = 하나의 관측 (재투영 오차)
 *   [Pose 1] ──Edge── [Point 2]
 *    (Vertex)          (Vertex)
 *
 * 난이도: ★☆☆ (기본 개념)
 * 선수 지식: Phase 3 Week 1~5 (VO 파이프라인), Week 6 (Keyframe)
 */

#include <iostream>

// Q1: Bundle Adjustment가 최소화하는 것
//
// BA의 목적 함수: min Σ ||z_ij - π(T_i, P_j)||²
//
// "재투영 오차" = 실제 관측(z_ij)과 모델 예측(π(T_i, P_j))의 차이
//   - 관측: 이미지에서 실제로 검출된 2D 특징점 좌표
//   - 예측: 추정된 3D 점을 추정된 포즈로 투영한 2D 좌표
//
// ★ "Bundle" = 3D 점에서 카메라로 향하는 광선 다발(ray bundle)
//   이 광선들이 모든 관측과 일관되도록 "조정(Adjustment)"
//
// 왜 포즈 간 거리가 아닌가?
//   BA는 "관측"에 기반한 최적화 — 이미지에서 본 것과 모델 예측을 맞춤
//   포즈 간 거리 최소화는 Pose Graph Optimization의 영역
void problem1_ba_minimize()
{
    std::cout << "Q1. Bundle Adjustment가 최소화하는 것은?\n";
    std::cout << "   a) 포즈 간 거리\n";
    std::cout << "   b) 재투영 오차\n";
    std::cout << "   c) 3D 점 간 거리\n";
    std::cout << "   d) 시간 지연\n";
    std::cout << "Your answer: ";
    char ans1;
    std::cin >> ans1;
}

// Q2: g2o에서 Vertex란
//
// g2o의 그래프 모델:
//   Vertex(노드) = 최적화 변수 — BA에서 "추정하고 싶은 것"
//     - VertexSE3Expmap: 카메라 포즈 (R, t) — 6 DoF
//     - VertexPointXYZ: 3D 맵 포인트 (x, y, z) — 3 DoF
//
// Vertex의 속성:
//   - setEstimate(): 초기값 설정 (최적화 시작점)
//   - setFixed(true): 이 변수를 고정 (최적화에서 제외)
//   - setMarginalized(true): Schur Complement 대상으로 지정
//
// ★ Vertex ≠ 관측값. Vertex는 "모르는 값"이고, 관측은 Edge에 포함
void problem2_vertex()
{
    std::cout << "\nQ2. g2o에서 Vertex는 무엇?\n";
    std::cout << "   a) 최적화 변수\n";
    std::cout << "   b) 제약 조건\n";
    std::cout << "   c) 관측값\n";
    std::cout << "   d) 오차 함수\n";
    std::cout << "Your answer: ";
    char ans2;
    std::cin >> ans2;
}

// Q3: g2o에서 Edge란
//
// Edge = 두 Vertex 사이의 제약 조건
//   BA에서: "카메라 i에서 3D 점 j를 관측했다"는 정보
//
//   [Pose_i] ──Edge(z_ij)── [Point_j]
//
//   Edge가 담는 정보:
//     - setMeasurement(z_ij): 실제 관측값 (2D 픽셀 좌표)
//     - setInformation(Ω): 관측의 신뢰도 (공분산 역행렬)
//     - computeError(): z_ij - π(T_i, P_j) 계산
//
// ★ Edge의 개수 = 총 관측 수
//   예: 카메라 10대, 3D 점 100개, 각 점이 평균 3대에서 관측
//   → Edge 수 ≈ 100 × 3 = 300
void problem3_edge()
{
    std::cout << "\nQ3. Edge는 무엇?\n";
    std::cout << "   a) 최적화 변수\n";
    std::cout << "   b) 제약 조건\n";
    std::cout << "   c) 카메라 포즈\n";
    std::cout << "   d) 3D 점\n";
    std::cout << "Your answer: ";
    char ans3;
    std::cin >> ans3;
}

// Q4: Huber Loss의 목적
//
// 일반 L2 비용: ρ(e) = e²
//   → Outlier(큰 오차)가 비용을 지배 → 최적화 결과 왜곡
//
// Huber Loss:
//   ρ(e) = { e²/2           if |e| <= δ     (Inlier: 일반 L2)
//          { δ(|e| - δ/2)   if |e| > δ      (Outlier: 선형 증가)
//
//   비용
//   ↑
//   │      L2 (급격히 증가)
//   │     /
//   │    /  . . Huber (선형으로 완만)
//   │   / .
//   │  /.
//   │ /
//   └──────────→ 오차
//
// ★ SLAM에서 Outlier = 잘못된 특징점 매칭, 동적 물체
//   Huber Loss로 이들의 영향을 줄여 강건한 최적화 가능
void problem4_huber_loss()
{
    std::cout << "\nQ4. Huber Loss의 목적은?\n";
    std::cout << "   a) 속도 향상\n";
    std::cout << "   b) Outlier 영향 감소\n";
    std::cout << "   c) 메모리 절약\n";
    std::cout << "   d) 정확도 향상\n";
    std::cout << "Your answer: ";
    char ans4;
    std::cin >> ans4;
}

// Q5: Schur Complement의 장점
//
// BA의 헤시안(H) 구조:
//   [Hpp  Hpl] [Δp]   [bp]
//   [Hpl' Hll] [Δl] = [bl]
//
//   p = 포즈 변수, l = 맵 포인트 변수
//   Hll은 블록 대각 행렬 (3D 점은 서로 직접 연결 안 됨)
//
// Schur Complement:
//   3D 점(l)을 "소거"하여 포즈(p)만의 작은 시스템으로 축소
//   (Hpp - Hpl·Hll⁻¹·Hpl')·Δp = bp - Hpl·Hll⁻¹·bl
//
//   3D 점이 수만 개여도, 포즈는 수십~수백 개
//   → 큰 시스템을 작은 시스템으로 변환 → 속도 향상
//
// ★ BA에서 가장 중요한 최적화 기법
//   Hll이 블록 대각이므로 역행렬을 O(n)에 계산 가능
void problem5_schur()
{
    std::cout << "\nQ5. Schur Complement의 장점은?\n";
    std::cout << "   a) 정확도 향상\n";
    std::cout << "   b) 속도 향상\n";
    std::cout << "   c) 메모리 절약\n";
    std::cout << "   d) Outlier 제거\n";
    std::cout << "Your answer: ";
    char ans5;
    std::cin >> ans5;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 7 Quiz (Easy)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_ba_minimize();
    problem2_vertex();
    problem3_edge();
    problem4_huber_loss();
    problem5_schur();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "답안 제출 완료!" << std::endl;
    std::cout << "quiz_solutions/easy_sol.cpp에서 정답을 확인하세요." << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
