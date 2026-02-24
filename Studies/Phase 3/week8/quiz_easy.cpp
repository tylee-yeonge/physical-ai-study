/**
 * Quiz Easy - Week 8: Ceres Bundle Adjustment
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. Ceres Solver의 핵심 장점 — 자동 미분 (AutoDiff)
 *   2. AutoDiffCostFunction 템플릿 — <Functor, 잔차 차원, 파라미터 블록 크기들>
 *   3. 자동 미분의 핵심 기법 — template<typename T>와 Jet 타입
 *   4. HuberLoss — Outlier에 강건한 비용 함수의 임계값 δ
 *   5. g2o 대비 Ceres의 장단점 비교
 *   6. SetParameterBlockConstant — Gauge freedom 제거
 *
 * Ceres vs g2o 비교:
 *
 *   g2o:  그래프 기반 (Vertex + Edge) — SLAM에 특화
 *         + 빠른 속도, Schur 최적화, marginalization 지원
 *         - Jacobian을 직접 유도해야 함
 *
 *   Ceres: 일반 비선형 최소제곱 — 범용 최적화
 *         + 자동 미분 (Jacobian 유도 불필요)
 *         + 깔끔한 API, 다양한 solver/loss 지원
 *         - g2o 대비 SLAM 특화 기능 부족
 *
 * Ceres의 AutoDiff 원리:
 *
 *   사용자 코드:  template<typename T>
 *                 operator()(const T* pose, const T* point, T* residual)
 *
 *   T = double → 일반 계산 (비용 평가)
 *   T = Jet<double, N> → 자동 미분 (값 + 도함수 동시 계산)
 *
 * 난이도: ★☆☆ (기본 개념)
 * 선수 지식: Week 7 (g2o BA 기초)
 */

#include <iostream>

// Q1: Ceres Solver의 가장 큰 장점
//
// g2o에서 BA를 구현하려면:
//   1. 오차 함수 e(x) 정의
//   2. Jacobian ∂e/∂x를 수학적으로 유도
//   3. linearizeOplus()에 Jacobian 코드 작성
//   → 실수 가능, 디버깅 어려움
//
// Ceres의 자동 미분 (AutoDiff):
//   1. 오차 함수 e(x)만 정의 (operator() 안에 순방향 계산)
//   2. Jacobian은 Ceres가 자동 계산 (Jet 타입 활용)
//   → 구현 간편, 정확한 Jacobian 보장
//
// ★ 자동 미분 ≠ 수치 미분 (finite difference)
//   자동 미분: 정확한 도함수 (기계 정밀도)
//   수치 미분: 근사치 (h 선택에 의존, 느림)
void problem1_ceres_advantage()
{
    std::cout << "Q1. Ceres Solver의 가장 큰 장점은?\n";
    std::cout << "   a) 가장 빠른 속도\n";
    std::cout << "   b) 자동 미분\n";
    std::cout << "   c) GPU 지원\n";
    std::cout << "   d) 시각화 기능\n";
    std::cout << "   답: _____\n" << std::endl;
}

// Q2: AutoDiffCostFunction 템플릿 파라미터
//
// AutoDiffCostFunction<Functor, R, P1, P2, ...>
//   Functor = 오차 함수 구조체 (operator() 정의)
//   R = 잔차(residual) 차원 (BA에서 2: u, v 오차)
//   P1, P2, ... = 각 파라미터 블록의 크기
//
// BA 예시:
//   AutoDiffCostFunction<ReprojectionError, 2, 6, 3>
//     2 = 잔차 차원 (2D 재투영 오차)
//     6 = 첫 번째 파라미터 블록 (카메라 포즈: 6 DoF)
//     3 = 두 번째 파라미터 블록 (3D 점: x, y, z)
//
// ★ 파라미터 블록 수와 크기가 operator()의 인자와 일치해야 함
void problem2_autodiff_template()
{
    std::cout << "Q2. AutoDiffCostFunction의 템플릿 파라미터는?\n";
    std::cout << "   <오차함수, ?, ?, ?>\n";
    std::cout << "   빈칸에 들어갈 것은? (순서대로)\n";
    std::cout << "   답: ___________________\n" << std::endl;
}

// Q3: 자동 미분을 위한 타입
//
// Ceres AutoDiff의 핵심: template<typename T>
//
//   template<typename T>
//   bool operator()(const T* pose, const T* point, T* residual) const
//   {
//       // T가 double이면 → 일반 계산
//       // T가 Jet이면 → 값 + 도함수 동시 계산
//       residual[0] = observed_x - predicted_x;
//       residual[1] = observed_y - predicted_y;
//   }
//
// ★ 왜 double이 아닌 template T를 사용하는가?
//   Ceres가 T = Jet<double, N>으로 호출하면
//   동일한 코드로 Jacobian이 자동 계산됨 (이중수 산술)
void problem3_template_type()
{
    std::cout << "Q3. Ceres에서 자동 미분을 위해 사용하는 타입은?\n";
    std::cout << "   a) double\n";
    std::cout << "   b) float\n";
    std::cout << "   c) template T\n";
    std::cout << "   d) Eigen::VectorXd\n";
    std::cout << "   답: _____\n" << std::endl;
}

// Q4: HuberLoss(1.0)의 의미
//
// Huber Loss: ρ(s) = { s        if s <= δ²
//                     { 2δ√s - δ²  otherwise
//
// HuberLoss(1.0) → δ = 1.0 픽셀
//   - 재투영 오차가 1 픽셀 이하 → 일반 L2 비용 (Inlier)
//   - 재투영 오차가 1 픽셀 초과 → 선형 증가 (Outlier 영향 감소)
//
// Ceres에서 사용:
//   problem.AddResidualBlock(cost, new ceres::HuberLoss(1.0), ...);
//
// ★ δ 선택: 보통 1~2 픽셀 (재투영 오차의 기대 수준)
//   δ가 너무 크면 Outlier 억제 부족, 너무 작으면 Inlier도 억제
void problem4_huber_loss()
{
    std::cout << "Q4. HuberLoss(1.0)의 의미는?\n";
    std::cout << "   a) 1초 timeout\n";
    std::cout << "   b) δ=1 픽셀\n";
    std::cout << "   c) 1번 반복\n";
    std::cout << "   d) 1개 변수\n";
    std::cout << "   답: _____\n" << std::endl;
}

// Q5: g2o 대비 Ceres의 단점
//
// g2o의 장점 (= Ceres의 약점):
//   1. SLAM 특화: VertexSE3, EdgeSE3 등 SLAM 전용 타입 제공
//   2. Marginalization 지원: setMarginalized(true) → Schur 자동 처리
//   3. 속도: 손으로 유도한 Jacobian은 AutoDiff보다 빠를 수 있음
//
// Ceres의 단점:
//   - SLAM 특화 타입이 없어 직접 구현 필요
//   - AutoDiff의 Jet 연산이 수동 Jacobian보다 느릴 수 있음
//   - 하지만 API가 간결하고 버그 위험이 낮아 실용적
//
// ★ 실제로는 "느린 속도"보다 "SLAM 생태계 통합"이 더 큰 차이
//   ORB-SLAM → g2o, VINS-Mono → Ceres 사용
void problem5_ceres_disadvantage()
{
    std::cout << "Q5. g2o 대비 Ceres의 단점은?\n";
    std::cout << "   a) 느린 속도\n";
    std::cout << "   b) 복잡한 API\n";
    std::cout << "   c) Jacobian 필요\n";
    std::cout << "   d) 메모리 많이 사용\n";
    std::cout << "   답: _____\n" << std::endl;
}

// Q6: SetParameterBlockConstant의 목적
//
// Ceres에서 특정 파라미터 블록을 최적화에서 제외:
//   problem.SetParameterBlockConstant(camera_params[0]);
//
// g2o의 setFixed(true)와 동일한 역할
//
// Gauge Freedom:
//   BA의 비용 함수는 상대적 관계만 정의
//   → 전체 맵을 이동/회전해도 비용이 동일
//   → 해가 유일하지 않음 (특이 헤시안)
//
// 해결: 첫 카메라를 Constant로 설정
//   → 좌표계 원점 고정 → Gauge freedom 제거
//
// ★ Week 7의 setFixed(true)와 완전히 동일한 개념
void problem6_set_constant()
{
    std::cout << "Q6. Problem::SetParameterBlockConstant의 목적은?\n";
    std::cout << "   a) 메모리 절약\n";
    std::cout << "   b) 속도 향상\n";
    std::cout << "   c) Gauge freedom 제거\n";
    std::cout << "   d) 정확도 향상\n";
    std::cout << "   답: _____\n" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 8 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_ceres_advantage();
    problem2_autodiff_template();
    problem3_template_type();
    problem4_huber_loss();
    problem5_ceres_disadvantage();
    problem6_set_constant();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
