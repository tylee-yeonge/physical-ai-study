/**
 * Quiz Easy - Week 6: Keyframe Management
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. Keyframe의 역할과 필요성 — 왜 모든 프레임을 저장하지 않는가?
 *   2. Keyframe 선택 기준 — ORB-SLAM2의 최소 프레임 간격 조건
 *   3. Keyframe Culling — 중복 KF 제거로 맵 크기 유지
 *   4. 90% Rule — ORB-SLAM2의 유명한 중복 판단 기준
 *   5. Covisibility Graph — KF 간 관계를 표현하는 가중 그래프
 *
 * SLAM 파이프라인에서 Keyframe의 위치:
 *
 *   Frame → [Tracking] → Keyframe 선택 → [Local Mapping] → [Loop Closing]
 *                              ↓                ↓
 *                        KF 추가/Culling    Bundle Adjustment
 *                              ↓
 *                       Covisibility Graph
 *
 * ★ 핵심: Keyframe은 SLAM 맵 구축의 기본 단위이며,
 *   "적절한 양"을 유지하는 것이 시스템 성능의 핵심이다.
 *   너무 적으면 → 맵 품질 저하, 추적 불안정
 *   너무 많으면 → BA 느려짐, 메모리 낭비
 *
 * 난이도: ★☆☆ (기본 개념)
 * 선수 지식: Phase 3 Week 1~5 (VO 파이프라인 기본)
 */

#include <iostream>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 6 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ── Q1: Keyframe을 사용하는 주요 이유 ──
    //
    // 30fps 카메라라면 1분에 1800프레임. 전부 저장/처리하면:
    //   - 메모리: 수 GB (이미지 + 특징점 + 디스크립터)
    //   - BA 비용: O(n³) — n이 1800이면 실시간 불가
    //   - 중복 정보: 인접 프레임은 거의 동일한 장면
    //
    // Keyframe으로 "중요한 프레임"만 선별 (보통 100~500개):
    //   1. 맵 구축의 기본 단위 (삼각측량, 맵 포인트 생성)
    //   2. Bundle Adjustment의 최적화 대상 (포즈 + 맵 포인트)
    //   3. Loop Closure의 참조 프레임 (재방문 탐지)
    std::cout << "Q1. Keyframe을 사용하는 주요 이유 3가지는?" << std::endl;
    std::cout << "   답: _____, _____, _____\n" << std::endl;

    // ── Q2: ORB-SLAM에서 새 Keyframe을 만드는 기본 조건 ──
    //
    // ORB-SLAM2의 KF 선택 전제 조건:
    //   "마지막 KF 이후 최소 N프레임이 경과했을 때"
    //
    // 이 조건의 목적:
    //   - 너무 빈번한 KF 생성을 방지 (KF 폭증 → BA 부하)
    //   - Local Mapping 스레드에 처리 여유를 줌
    //   - ORB-SLAM2에서 N = 20 (기본값)
    //
    // 이 전제 + 추가 조건 (추적 품질, 시차, 최대 간격)의 OR로 최종 결정
    std::cout << "Q2. ORB-SLAM에서 새 Keyframe을 만드는 기본 조건은?" << std::endl;
    std::cout << "   a) 10 프레임 경과" << std::endl;
    std::cout << "   b) 20 프레임 경과" << std::endl;
    std::cout << "   c) 30 프레임 경과" << std::endl;
    std::cout << "   d) 50 프레임 경과" << std::endl;
    std::cout << "   답: _____\n" << std::endl;

    // ── Q3: Keyframe Culling의 목적 ──
    //
    // Culling = "가지치기" — 불필요한 KF을 제거하는 과정
    //
    // ORB-SLAM2의 전략: "적극적 생성 + 적극적 Culling"
    //   - 먼저 KF를 자유롭게 생성하여 맵 품질 확보
    //   - 그 후 중복 KF를 Culling하여 맵 크기 유지
    //
    // Culling이 없으면:
    //   1. 메모리 계속 증가 (이미지, 특징점 등)
    //   2. BA 비용 증가 (최적화 변수 수 ∝ KF 수)
    //   3. 중복 정보가 BA를 비효율적으로 만듦
    //
    // ★ 모든 목적(메모리, 계산량, 중복)이 다 해당됨
    std::cout << "Q3. Keyframe Culling의 목적은?" << std::endl;
    std::cout << "   a) 메모리 절약" << std::endl;
    std::cout << "   b) 계산량 감소" << std::endl;
    std::cout << "   c) 중복 제거" << std::endl;
    std::cout << "   d) 위 모두" << std::endl;
    std::cout << "   답: _____\n" << std::endl;

    // ── Q4: 90% Rule ──
    //
    // ORB-SLAM2의 유명한 Keyframe Culling 기준:
    //
    //   "어떤 KF의 맵 포인트 중 90% 이상이
    //    다른 3개 이상의 KF에서도 관측되면 → 이 KF는 중복 → 삭제"
    //
    // 직관:
    //   이 KF에서"만" 볼 수 있는 점이 10% 미만이면,
    //   이 KF을 삭제해도 맵 포인트 대부분이 살아남음 → 정보 손실 최소
    //
    // "3개 이상"의 의미:
    //   맵 포인트가 최소 3개 KF에서 관측되어야 삼각측량이 안정적
    //   (2개만이면 baseline이 하나뿐 → 노이즈에 취약)
    //
    // 예시: KF A가 10개 맵 포인트를 관측
    //   9개가 3+ KF에서도 관측 → 중복 90% → 삭제!
    //   8개가 3+ KF에서도 관측 → 중복 80% → 유지
    std::cout << "Q4. 90% rule이란?" << std::endl;
    std::cout << "   설명: _____________________\n" << std::endl;

    // ── Q5: Covisibility Graph의 용도 ──
    //
    // Covisibility Graph:
    //   노드 = Keyframe
    //   엣지 = 공유하는 맵 포인트 수 (가중치)
    //   임계값 = 15 (ORB-SLAM2 기본값, 15개 이상 공유 시 연결)
    //
    //   KF0 ──25── KF1 ──20── KF3
    //     \         /
    //     15      10
    //       \   /
    //        KF2
    //
    // 활용 1: Local BA 범위 결정
    //   → 현재 KF과 covisibility 높은 KF만 함께 최적화
    //   → 전체 맵 BA보다 훨씬 빠름 (실시간 가능)
    //
    // 활용 2: Loop Closure 후보 탐색
    //   → covisibility가 높은 KF의 BoW 벡터를 우선 비교
    //
    // 활용 3: Relocalization
    //   → 추적 실패 시 covisibility 그래프를 따라 재위치 결정
    //
    // ★ 모든 용도에 사용됨 — Covisibility Graph는 SLAM의 "백본"
    std::cout << "Q5. Covisibility graph의 용도는?" << std::endl;
    std::cout << "   a) Local BA 범위 결정" << std::endl;
    std::cout << "   b) Loop Closure 후보 탐색" << std::endl;
    std::cout << "   c) Relocalization" << std::endl;
    std::cout << "   d) 위 모두" << std::endl;
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
