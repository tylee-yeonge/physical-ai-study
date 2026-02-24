/**
 * Phase 3 Week 1 - VO 기초 퀴즈 (정답)
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. VO vs SLAM — Loop Closure 유무가 핵심 차이
 *   2. 스케일 모호성 — 단안 카메라의 근본 한계
 *   3. VO 파이프라인 — 병목 구간과 해결 방법
 *   4. VO 유형 비교 — Mono/Stereo/RGB-D 각각의 특징
 *
 * Visual Odometry 파이프라인:
 *
 *   이미지 입력 → 특징점 검출 → 특징점 매칭/추적 → 모션 추정 → 포즈 출력
 *                  (FAST/ORB)    (BF/Optical Flow)   (E/PnP)
 *
 * VO vs SLAM:
 *   VO:   Frontend만 → 로컬 추정 → 드리프트 누적
 *   SLAM: Frontend + Backend → Loop Closure → 전역 보정
 *
 * 난이도: ★☆☆ (기본 개념, 답이 포함된 학습형)
 * 선수 지식: Phase 2 (카메라 모델, 특징점, 에피폴라 기하학)
 */

#include <iostream>

// 문제 1: VO vs SLAM — Loop Closure의 역할
//
// VO (Visual Odometry):
//   인접 프레임 간 상대 모션만 추정 → 오차가 누적(드리프트)
//   100프레임에서 1% 오차 → 최종 위치 ~2.7배 오차 (지수 증가)
//
// SLAM (Simultaneous Localization and Mapping):
//   VO + Loop Closure + Global Optimization
//   이전에 방문한 장소를 재인식 → 전역 오차를 한번에 보정
//
// ★ Loop Closure = "같은 장소를 다시 왔다"를 인식하는 기능
//   BoW(Bag of Words)로 이미지 유사도 비교 → 재방문 탐지
void problem1_vo_vs_slam()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: VO vs SLAM" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: VO와 SLAM의 가장 큰 차이는?\n" << std::endl;

    std::cout << "💡 답: Loop Closure" << std::endl;
    std::cout << "   - VO: Loop Closure 없음 → 드리프트 누적" << std::endl;
    std::cout << "   - SLAM: Loop Closure로 전역 오차 보정" << std::endl;
}

// 문제 2: 스케일 모호성 — 단안 카메라의 근본 한계
//
// 핀홀 모델 투영: λ·[u, v, 1]^T = K·[R|t]·[X, 1]^T
//
// 3D 점 X와 λX가 동일한 픽셀 (u, v)에 투영됨
//   → 이미지만으로는 실제 크기/거리를 알 수 없음
//   → Essential Matrix에서 t의 방향만 복원 (||t|| = 1로 정규화)
//
// 해결 방법:
//   - Stereo 카메라: baseline b를 알면 Z = f·b / disparity
//   - IMU 융합 (VIO): 가속도 적분으로 절대 스케일 복원
//   - 알려진 크기의 물체: 도로 표지판, 차선 폭 등
//
// ★ Week 12-13에서 이 문제를 깊이 다룸
void problem2_scale_ambiguity()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 스케일 모호성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: Monocular VO의 근본적인 한계는?\n" << std::endl;

    std::cout << "💡 답: 스케일 모호성" << std::endl;
    std::cout << "   - t의 방향은 알지만 크기는 모름" << std::endl;
    std::cout << "   - 1m와 10m 이동이 동일한 이미지" << std::endl;
    std::cout << "   - 해결: IMU, Stereo, 알려진 크기" << std::endl;
}

// 문제 3: VO 파이프라인 병목 — 특징점 검출/매칭
//
// VO 파이프라인 시간 분배 (30fps = 33ms 예산):
//   특징점 검출: ~10ms (FAST: 빠름, ORB: 중간)
//   매칭/추적: ~10ms (BF: 느림, Optical Flow: 빠름)
//   모션 추정: ~5ms (E matrix 분해 or PnP)
//   기타: ~8ms
//
// 특징점 검출/매칭이 가장 시간 소모적인 이유:
//   - 매 프레임 수백~수천 개 특징점 검출 필요
//   - 각 특징점의 디스크립터(256bit ORB) 계산
//   - N×M 매칭 (brute-force: O(N·M·256))
//
// ★ 해결 전략:
//   GPU 가속 (ORB-CUDA), Optical Flow (매칭 대체), 다운샘플링
void problem3_vo_pipeline()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: VO 파이프라인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 가장 계산량이 많은 단계는?\n" << std::endl;

    std::cout << "💡 답: 특징점 검출/매칭" << std::endl;
    std::cout << "   - 매 프레임 수백 개 특징점" << std::endl;
    std::cout << "   - Descriptor 계산 + 매칭" << std::endl;
    std::cout << "   - 해결: GPU 가속, Optical Flow" << std::endl;
}

// 문제 4: VO 유형 비교 — Mono/Stereo/RGB-D
//
// Monocular VO:
//   + 하드웨어 단순 (카메라 1대)
//   - 스케일 모호성, 초기화 필요 (Pure Rotation 문제)
//   대표: ORB-SLAM (Mono), DSO
//
// Stereo VO:
//   + 스케일 복원 가능 (Z = f·b/d)
//   + 초기화 불필요 (첫 프레임부터 3D 점 생성)
//   - 넓은 baseline 필요, 실외 적합
//   대표: ORB-SLAM2 (Stereo), KITTI 벤치마크
//
// RGB-D VO:
//   + 직접 깊이 측정 (초기화/삼각측량 불필요)
//   - 실내 전용 (적외선 센서 한계), 측정 범위 ~5m
//   대표: ElasticFusion, KinectFusion
//
// ★ 자율주행: 실외 + 절대 거리 필요 → Stereo VO가 적합
void problem4_vo_types()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: VO 유형" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 자율주행에 적합한 VO 유형은?\n" << std::endl;

    std::cout << "💡 답: Stereo VO" << std::endl;
    std::cout << "   - 스케일 복원 가능 → 절대 거리" << std::endl;
    std::cout << "   - 실외 환경에 robust" << std::endl;
    std::cout << "   - 예: KITTI 벤치마크" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 1 Quiz - Easy (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_vo_vs_slam();
    problem2_scale_ambiguity();
    problem3_vo_pipeline();
    problem4_vo_types();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
