/**
 * Phase 3 Week 3 - PnP 중급 퀴즈
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. RANSAC 반복 횟수 공식 — 신뢰도, inlier 비율, 샘플 크기
 *   2. 재투영 오차 — 포즈 정확도의 핵심 지표
 *   3. VO 추적 전략 — 추적 유지/실패 조건과 복구 방법
 *
 * RANSAC (Random Sample Consensus):
 *
 *   반복 횟수: N = log(1-p) / log(1-w^s)
 *     p = 원하는 신뢰도 (보통 0.99)
 *     w = inlier 비율 (0.5이면 절반이 outlier)
 *     s = 최소 샘플 크기 (P3P: 3, 8-point: 8)
 *
 *   각 반복:
 *     1. s개 점을 랜덤 샘플링
 *     2. 모델 추정 (PnP/Essential)
 *     3. 전체 점에 대해 inlier/outlier 판별
 *     4. 최다 inlier 모델 저장
 *
 * 난이도: ★★☆ (수치 계산, 시스템 설계)
 * 선수 지식: quiz_easy (PnP 기본, Inlier Ratio)
 */

#include <opencv2/opencv.hpp>
#include <iostream>

// 문제 1: RANSAC 반복 횟수 — 확률론적 최적 반복 수
//
// N = log(1-p) / log(1-w^s)
//
// 매개변수의 의미:
//   p = 0.99: 99% 확률로 올바른 모델을 찾겠다
//   w = 0.5: 데이터의 50%가 inlier (나머지 50% outlier)
//   s = 3: 한 번에 3개 점을 샘플 (P3P)
//
// 계산:
//   w^s = 0.5³ = 0.125 (3개 모두 inlier일 확률)
//   1 - w^s = 0.875 (적어도 1개 outlier 포함 확률)
//   N = log(0.01) / log(0.875) ≈ 35
//
// ★ s가 커지면 N이 급증:
//   s=3, w=0.5 → N≈35
//   s=8, w=0.5 → N≈1177 (8-point는 RANSAC에 비효율)
//   → P3P가 RANSAC에 적합한 이유!
void problem1_ransac_iterations()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: RANSAC 반복 횟수" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "RANSAC 반복 횟수 계산:\n" << std::endl;
    std::cout << "   N = log(1-p) / log(1-w^s)" << std::endl;
    std::cout << "   p: 신뢰도 (0.99)" << std::endl;
    std::cout << "   w: inlier 비율 (0.5)" << std::endl;
    std::cout << "   s: 샘플 크기 (3)\n" << std::endl;

    double p = 0.99;
    double w = 0.5;
    int s = 3;
    int N = static_cast<int>(std::log(1 - p) / std::log(1 - std::pow(w, s)));

    std::cout << "💡 결과: " << N << " 반복" << std::endl;
}

// 문제 2: 재투영 오차 — 포즈 정확도의 핵심 지표
//
// 재투영 오차 = ||x_observed - π(R, t, X)||²
//
//   x_observed: 이미지에서 실제 관측된 2D 좌표
//   π(R, t, X): 3D 점 X를 추정 포즈(R,t)로 투영한 2D 좌표
//
// 투영 함수 π:
//   X_cam = R·X + t
//   u = fx · X_cam(0)/X_cam(2) + cx
//   v = fy · X_cam(1)/X_cam(2) + cy
//
// 재투영 오차의 용도:
//   1. RANSAC inlier 판별: 오차 < 3px → inlier
//   2. PnP 정확도 평가: 평균 오차 < 1px → 좋은 포즈
//   3. BA의 목적 함수: Σ 재투영 오차² 를 최소화
//   4. 맵 포인트 품질: 오차 큰 점 → 삭제 대상
//
// ★ BA에서 가장 중요한 양 — Week 7~9에서 집중적으로 다룸
void problem2_reprojection_error()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 재투영 오차" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "재투영 오차 = ||x - π(R, t, X)||²\n" << std::endl;

    std::cout << "💡 의미:" << std::endl;
    std::cout << "   - 3D 점을 현재 포즈로 투영" << std::endl;
    std::cout << "   - 실제 관측과 차이" << std::endl;
    std::cout << "   - 포즈 정확도 지표" << std::endl;
}

// 문제 3: VO 추적 전략 — 추적 유지/실패 조건
//
// 추적 유지 조건 (ORB-SLAM2 기준):
//   1. Inlier ratio > 30% (충분한 유효 매칭)
//   2. 추적된 특징점 > 20개 (최소한의 제약)
//   3. 재투영 오차 < 3~5px (포즈가 합리적)
//
// 추적 실패 시 복구 전략:
//
//   1단계: Motion Model 기반 추적
//     등속 모델로 포즈 예측 → PnP로 정제
//     빠른 이동이나 블러에 대응
//
//   2단계: Reference KF 기반 추적
//     가장 최근 KF과 매칭 시도
//     Motion Model 실패 시 fallback
//
//   3단계: Relocalization
//     전체 KF 데이터베이스에서 BoW 유사도 검색
//     가장 유사한 KF을 찾아 PnP로 포즈 복구
//     ★ 모든 추적이 실패한 마지막 수단
//
// ★ ORB-SLAM2의 Tracking 상태 머신:
//   OK → RECENTLY_LOST → LOST → (Relocalization) → OK
void problem3_vo_tracking()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: VO 추적 전략" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "추적 유지 조건:" << std::endl;
    std::cout << "   1. Inlier ratio > 30%" << std::endl;
    std::cout << "   2. 충분한 특징점 (> 20개)" << std::endl;
    std::cout << "   3. 재투영 오차 < 3px\n" << std::endl;

    std::cout << "💡 실패 시:" << std::endl;
    std::cout << "   - 재초기화 (2D-2D)" << std::endl;
    std::cout << "   - 또는 Relocalization" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 3 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_ransac_iterations();
    problem2_reprojection_error();
    problem3_vo_tracking();

    std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
