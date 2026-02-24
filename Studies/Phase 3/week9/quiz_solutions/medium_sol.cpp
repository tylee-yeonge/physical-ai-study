/**
 * Quiz Medium - Week 9: BA 최적화 기법 (Schur Complement) (정답)
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. Schur Complement의 계산량 절감 원리
 *   2. Local BA의 장단점 — 실시간성 vs 전역 일관성
 *   3. VINS Sliding Window — Local BA + IMU 융합
 *
 * Schur Complement 계산량 비교:
 *
 *   Full BA: N_p개 포즈, N_l개 3D 점
 *     전체 시스템 크기: (6·N_p + 3·N_l) × (6·N_p + 3·N_l)
 *     예: 100 포즈, 10000 점 → 30600 × 30600 → O(30600³) ≈ 불가능
 *
 *   Schur 소거 후:
 *     reduced 크기: 6·N_p × 6·N_p = 600 × 600 → O(600³) ≈ 실시간 가능
 *     + Hll⁻¹ 계산: O(N_l) (블록 대각이므로 3×3 역행렬 N_l번)
 *
 * Local BA vs Global BA:
 *
 *   Local BA:  현재 KF + covisible KF (~20개) → 수십 ms
 *   Global BA: 모든 KF + 모든 점 → 수 초~분
 *
 *   ORB-SLAM2 구조:
 *     Tracking → Local Mapping (Local BA) → Loop Closing (Global BA)
 *                     ↑ 매 KF마다               ↑ Loop 발견 시에만
 *
 * 난이도: ★★☆ (서술형)
 * 선수 지식: quiz_easy (Schur, Local/Global BA 개념)
 */

#include <iostream>

// Q1: Schur Complement의 계산량 절감 원리
//
// 핵심 포인트:
//
// 1. Hll이 블록 대각:
//    3D 점끼리 직접 연결 안 됨 → Hll = diag(H_l1, ..., H_ln)
//    → Hll⁻¹ = diag(H_l1⁻¹, ..., H_ln⁻¹) — O(N_l)
//
// 2. 문제 크기 축소:
//    원래: (6N_p + 3N_l) 크기의 선형 시스템
//    Schur 후: 6N_p 크기의 reduced system
//    N_l >> N_p이므로 극적인 크기 감소
//
// 3. 풀이 순서:
//    ① Hll⁻¹ 계산 (블록 대각 → O(N_l))
//    ② reduced system 구성 및 풀이 (Δp)
//    ③ 역대입으로 Δl 계산
//
// ★ 수치 예시:
//    100 포즈, 10000 점:
//    원래: 30600 × 30600 → 축소: 600 × 600 (51배 축소)
//    O(n³) 기준: 51³ ≈ 130000배 속도 향상
void problem1_schur_savings()
{
    std::cout << "Q1. Schur Complement가 계산량을 줄이는 원리를 설명하세요.\n";
    std::cout << "   힌트: Hpp의 구조, 문제 크기 변화\n";
    // ✅ 정답: Hll이 블록 대각 → O(N_l)로 역행렬, 문제 크기 극적 축소
    std::cout << "   답: Hll이 블록 대각이라 O(N_l)로 역행렬 계산 가능\n";
    std::cout << "   (6N_p+3N_l) → 6N_p로 축소 (예: 30600→600, 51배)\n" << std::endl;
}

// Q2: Local BA의 장단점
//
// 장점:
//   1. 실시간 가능: covisible KF ~20개만 최적화 → 수십 ms
//   2. 메모리 효율: 전체 맵을 메모리에 올릴 필요 없음
//   3. 점진적 개선: 매 KF마다 주변 맵을 정제
//   4. 멀티스레드 호환: Tracking과 병렬 실행 가능
//
// 단점:
//   1. 전역 일관성 부재: 먼 KF 간 오차 보정 불가
//   2. 드리프트 누적: Local 영역 밖의 오차가 남음
//      → Loop Closure + Global BA로 해결
//   3. 경계 효과: Local 영역 경계의 KF은 제약 부족
//
// ★ ORB-SLAM2의 해법:
//   Local BA (실시간 정밀화) + Global BA (Loop 후 전역 보정)
void problem2_local_ba()
{
    std::cout << "Q2. Local BA의 장단점을 시스템 관점에서 설명하세요.\n";
    std::cout << "   힌트: 실시간성, 정확도, 드리프트\n";
    // ✅ 정답:
    std::cout << "   장점: 실시간 가능(~20 KF, 수십ms), 메모리 효율, 멀티스레드 호환\n";
    std::cout << "   단점: 전역 일관성 부재, 드리프트 누적 → Loop+Global BA로 해결\n" << std::endl;
}

// Q3: VINS Sliding Window와 Local BA
//
// VINS-Mono의 Sliding Window:
//   고정 크기(~10개 KF)의 윈도우를 유지
//   새 KF 추가 시 → 가장 오래된 KF를 marginalize하여 제거
//
// Local BA를 사용하는 이유:
//
//   1. 실시간 요구:
//      VIO는 200Hz+ IMU + 30Hz 카메라 → 실시간 처리 필수
//      Global BA는 너무 느림
//
//   2. 메모리 제한:
//      모바일/드론 환경 → 제한된 메모리
//      고정 크기 윈도우로 메모리 사용량 일정하게 유지
//
//   3. IMU 융합:
//      IMU pre-integration이 인접 KF 간 제약 제공
//      → Local 영역에서 IMU + Vision 동시 최적화
//      → 멀리 떨어진 KF과의 관계는 IMU가 아닌 Loop Closure로 처리
//
// ★ VINS의 marginalization:
//   제거되는 KF의 정보를 "사전 정보(prior)"로 변환하여 보존
//   → 정보 손실 최소화 (단순 삭제와 다름)
void problem3_vins_sliding_window()
{
    std::cout << "Q3. VINS의 sliding window optimization이 Local BA를 사용하는 이유는?\n";
    std::cout << "   힌트: 실시간, 메모리, IMU\n";
    // ✅ 정답: 실시간 요구 + 메모리 제한 + IMU가 인접 KF 간 제약 제공
    std::cout << "   답: 1) VIO 실시간 요구, 2) 모바일/드론 메모리 제한,\n";
    std::cout << "   3) IMU pre-integration이 인접 KF 간 제약 → Local이면 충분\n" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 9 Quiz - Medium (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_schur_savings();
    problem2_local_ba();
    problem3_vins_sliding_window();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
