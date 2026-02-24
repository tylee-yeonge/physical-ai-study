/**
 * Phase 3 Week 1 - VO 중급 퀴즈
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. 드리프트 누적 분석 — 프레임 단위 오차의 지수적 증가
 *   2. Frontend vs Backend — SLAM 시스템의 이중 구조
 *   3. 실시간 VO의 시간 제약 — 30fps = 33ms 예산 관리
 *
 * 드리프트의 수학적 모델:
 *
 *   매 프레임 오차율 ε = 1% (곱셈 오차):
 *   n프레임 후 누적 오차 = (1 + ε)^n
 *   100프레임: (1.01)^100 ≈ 2.7 → 170% 오차!
 *
 *   이것이 VO만으로는 장거리 주행이 불가능한 이유
 *   → Backend (Loop Closure + BA)가 필수
 *
 * SLAM 시스템 구조:
 *
 *   [Frontend (VO)]          [Backend (Optimization)]
 *   특징점 추적 ──────────→ Graph 최적화
 *   포즈 추정               Loop Closure
 *   빠름, 로컬             느림, 전역
 *   매 프레임 실행          비동기 실행
 *
 * 난이도: ★★☆ (수치 분석, 시스템 이해)
 * 선수 지식: quiz_easy (VO 기본 개념)
 */

#include <iostream>
#include <Eigen/Dense>

// 문제 1: 드리프트 분석 — 오차의 지수적 누적
//
// 프레임 단위 오차가 1%인 경우:
//   T_estimated = T_true × (1 + ε)  (매 프레임)
//   n프레임 후: (1.01)^n
//
//   n=20:  (1.01)^20 ≈ 1.22 → 22% 오차
//   n=60:  (1.01)^60 ≈ 1.82 → 82% 오차
//   n=100: (1.01)^100 ≈ 2.70 → 170% 오차!
//
// ★ 핵심 인사이트:
//   작은 오차도 곱셈으로 누적되면 지수적으로 증가
//   이것이 VO의 근본 한계 → Backend 최적화가 필수인 이유
//   Loop Closure: 누적 드리프트를 한번에 보정
void problem1_drift()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 드리프트 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "매 프레임 1% 오차 누적:" << std::endl;

    double cum_error = 1.0;
    for (int i = 1; i <= 100; i += 20)
    {
        cum_error *= std::pow(1.01, 20);
        std::cout << "   Frame " << i << ": " << cum_error << "x" << std::endl;
    }

    std::cout << "\n💡 지수적으로 증가!" << std::endl;
}

// 문제 2: Frontend vs Backend — SLAM의 이중 구조
//
// Frontend (VO/Tracking):
//   역할: 매 프레임 카메라 포즈 추정
//   방법: 특징점 추적 → 모션 추정 (PnP, Essential)
//   특징: 빠름 (수 ms), 로컬 (현재 + 직전 프레임만)
//   ★ 실시간성이 핵심 — 프레임 드롭 불가
//
// Backend (Optimization):
//   역할: 전체 맵의 일관성 유지
//   방법: Bundle Adjustment, Pose Graph Optimization
//   특징: 느림 (수십 ms ~ 초), 전역 (모든 KF/맵 포인트)
//   ★ 정확성이 핵심 — 드리프트 보정
//
// SLAM = Frontend + Backend:
//   Frontend → 초기 포즈 추정 (빠르지만 부정확)
//   Backend → 초기 추정 정제 (느리지만 정확)
//   보통 멀티스레드로 병렬 실행
void problem2_frontend_vs_backend()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Frontend vs Backend" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Frontend (VO):" << std::endl;
    std::cout << "   - 특징점 추적" << std::endl;
    std::cout << "   - 포즈 추정" << std::endl;
    std::cout << "   - 빠름, 로컬\n" << std::endl;

    std::cout << "Backend (Optimization):" << std::endl;
    std::cout << "   - Graph 최적화" << std::endl;
    std::cout << "   - Loop Closure" << std::endl;
    std::cout << "   - 느림, 전역\n" << std::endl;

    std::cout << "💡 SLAM = Frontend + Backend" << std::endl;
}

// 문제 3: 실시간 VO의 시간 제약
//
// 카메라 FPS별 프레임 예산:
//   30 FPS → 33.3ms
//   60 FPS → 16.7ms
//
// 일반적인 VO 시간 분배 (30fps 기준):
//   특징점 검출 (ORB): ~10ms
//   특징점 추적/매칭:  ~5ms
//   모션 추정 (PnP):   ~10ms
//   기타 (시각화 등):   ~8ms
//   합계:              ~33ms (빡빡!)
//
// ★ 실시간 달성 전략:
//   1. 멀티스레드: Tracking/Mapping/Loop Closing 분리 (ORB-SLAM)
//   2. GPU 가속: 특징점 검출/매칭을 GPU로 (ORB-CUDA)
//   3. Optical Flow: 매칭 대신 추적 (LK Optical Flow)
//   4. 해상도 조절: 320×240 vs 640×480
//   5. 특징점 수 제한: 500개 이하로 제한
void problem3_vo_realtime()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 실시간 VO" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "30 FPS 실시간 처리:" << std::endl;
    std::cout << "   프레임당 예산: 33ms\n" << std::endl;

    std::cout << "시간 분배:" << std::endl;
    std::cout << "   특징점 검출: 10ms" << std::endl;
    std::cout << "   특징점 추적: 5ms" << std::endl;
    std::cout << "   모션 추정: 10ms" << std::endl;
    std::cout << "   기타: 8ms\n" << std::endl;

    std::cout << "💡 최적화 필요: 멀티스레드, GPU" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 1 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_drift();
    problem2_frontend_vs_backend();
    problem3_vo_realtime();

    std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
