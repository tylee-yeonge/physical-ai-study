/**
 * Quiz Medium - Week 6: Keyframe Management
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. ORB-SLAM vs VINS의 Keyframe 선택 전략 차이
 *   2. Keyframe Culling 시점 — 언제 수행하는 것이 효율적인가?
 *   3. Covisibility의 정의와 의미 — 공유 맵 포인트 기반 KF 관계
 *   4. 첫 Keyframe 보호 — 좌표계 원점으로서의 역할
 *
 * 두 가지 대표적 KF 관리 전략:
 *
 *   ORB-SLAM2: 적극적 생성 + 적극적 Culling
 *     Frame → [4가지 조건 OR] → KF 생성 → [90% Rule] → 중복 제거
 *     특징: KF를 자유롭게 만들고, 나중에 중복을 정리
 *
 *   VINS-Mono: 보수적 생성 + Sliding Window
 *     Frame → [시차 조건] → KF 생성 → [Window 크기 초과] → 오래된 KF 제거
 *     특징: 고정 크기 윈도우 유지, IMU 덕분에 KF가 적어도 안정적
 *
 * 난이도: ★★☆ (비교 분석, 서술)
 * 선수 지식: quiz_easy (Keyframe 기본 개념), basic.cpp (KF 선택/Culling 구현)
 */

#include <iostream>

// Q1: ORB-SLAM vs VINS의 Keyframe 선택 차이
//
// ORB-SLAM2의 KF 선택 기준 (4가지 OR):
//   전제: num_frames >= 20 AND Local Mapping이 idle
//   조건A: num_frames >= 30 (최대 간격)
//   조건B: tracked < 50% (추적 품질 저하)
//   조건C: parallax > 10px (삼각측량 가능)
//
// VINS-Mono의 KF 선택 기준:
//   - 시차 기반: avg_parallax > 임계값
//   - Sliding Window: 고정 크기(보통 10~15개) 윈도우 유지
//   - 오래된 KF를 marginalize하여 제거
//
// ★ 핵심 차이:
//   ORB-SLAM: Vision-only → KF가 많아야 맵 품질 유지 → 적극 생성
//   VINS: Vision+IMU → IMU가 프레임 간 모션 보간 → KF 적어도 안정
//   ORB-SLAM: 90% Rule로 중복 제거 (크기 가변)
//   VINS: Sliding Window로 크기 고정 (오래된 것 제거)
void problem1_orb_vs_vins()
{
    std::cout << "Q1. ORB-SLAM vs VINS의 주요 Keyframe 선택 차이는?" << std::endl;
    std::cout << "   설명:\n" << std::endl;
}

// Q2: Keyframe Culling 시점
//
// 선택지별 분석:
//   a) 매 프레임마다 — 비용 과다. Culling 자체가 O(KF × MP) 연산
//   b) 새 KF 추가 시 — ORB-SLAM2의 실제 전략! Local Mapping에서 수행
//      새 KF이 추가되면 → 기존 KF 중 중복 발생 가능 → 바로 Culling
//   c) 주기적으로 — 차선책. 구현이 단순하지만 최적 타이밍을 놓칠 수 있음
//   d) 메모리 부족 시 — 너무 늦음. 이미 시스템이 느려진 후
//
// ★ ORB-SLAM2에서 Culling은 Local Mapping 스레드에서 수행:
//   1. 새 KF 삽입
//   2. 맵 포인트 Culling (관측 부족한 MP 제거)
//   3. 새 맵 포인트 생성 (삼각측량)
//   4. Local BA
//   5. KF Culling ← 여기서 90% Rule 적용
void problem2_culling_timing()
{
    std::cout << "Q2. Keyframe Culling을 언제 하면 좋을까요?" << std::endl;
    std::cout << "   a) 매 프레임마다" << std::endl;
    std::cout << "   b) 새 Keyframe 추가 시" << std::endl;
    std::cout << "   c) 주기적으로 (예: 50 프레임마다)" << std::endl;
    std::cout << "   d) 메모리 부족 시" << std::endl;
    std::cout << "   가장 좋은 답: _____\n" << std::endl;
}

// Q3: Covisibility가 높은 Keyframe의 의미
//
// Covisibility = 두 KF이 공유하는 맵 포인트 수
//
//   KF_A가 관측하는 맵 포인트: {P1, P2, P3, P4, P5}
//   KF_B가 관측하는 맵 포인트: {P1, P2, P3, P6, P7}
//   → 공유 = {P1, P2, P3} = 3개 → covisibility = 3
//
// covisibility가 높다 = 두 KF이 비슷한 장면을 보고 있다
//   → 같은 공간을 다른 시점에서 관측 (인접 프레임 or 재방문)
//   → 함께 최적화하면 효과적 (Local BA)
//   → Loop Closure 후보가 될 수 있음
//
// ★ ORB-SLAM2: covisibility >= 15이면 그래프 엣지 생성
void problem3_covisibility()
{
    std::cout << "Q3. Covisibility가 높은 Keyframe이란?" << std::endl;
    std::cout << "   설명: _____________________\n" << std::endl;
}

// Q4: 첫 Keyframe을 제거하지 않는 이유
//
// SLAM에서 첫 KF의 특별한 역할:
//
//   1. 좌표계 원점: 모든 포즈와 맵 포인트의 기준점
//      T_0 = [I | 0] (단위 회전, 영 이동)
//      → 삭제하면 전체 맵의 좌표 기준이 사라짐
//
//   2. Gauge Freedom 고정:
//      BA에서는 전체 맵을 자유롭게 이동/회전할 수 있음 (7 DoF)
//      첫 KF를 고정(fixed)하여 이 자유도를 제거
//      → 삭제하면 BA가 발산하거나 맵이 표류(drift)
//
//   3. 초기 맵의 anchor:
//      첫 KF에서 생성된 맵 포인트가 전체 맵의 기반
//
// ★ ORB-SLAM2에서 Culling 루프: i > 0 (인덱스 0 = 첫 KF 보호)
void problem4_first_keyframe()
{
    std::cout << "Q4. 첫 Keyframe을 제거하지 않는 이유는?" << std::endl;
    std::cout << "   설명:\n" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 6 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_orb_vs_vins();
    problem2_culling_timing();
    problem3_covisibility();
    problem4_first_keyframe();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
