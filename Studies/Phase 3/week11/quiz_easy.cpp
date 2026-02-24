/**
 * Phase 3 Week 11 - Ceres 실습 퀴즈
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. Ceres vs g2o — 자동 미분이 핵심 차이
 *   2. AutoDiffCostFunction 템플릿 — BAL(Snavely) 모델의 파라미터 구성
 *   3. DENSE_SCHUR — BA의 희소 구조를 활용한 Schur Complement
 *   4. HuberLoss — Outlier에 강건한 비용 함수
 *
 * BAL (Bundle Adjustment in the Large) 모델:
 *
 *   카메라 파라미터 (9 DoF):
 *     angle-axis (3) + translation (3) + focal (1) + k1, k2 (2)
 *
 *   3D 점 파라미터 (3 DoF):
 *     x, y, z
 *
 *   AutoDiffCostFunction<SnavelyError, 2, 9, 3>
 *     잔차: 2 (재투영 오차 u, v)
 *     블록1: 9 (카메라)
 *     블록2: 3 (3D 점)
 *
 * 난이도: ★☆☆ (기본 개념, 답이 포함된 학습형)
 * 선수 지식: Week 7-8 (g2o/Ceres BA 기초)
 */

#include <iostream>

// 문제 1: Ceres vs g2o — 핵심 차이
//
// g2o에서 BA 구현:
//   1. Edge 클래스 정의
//   2. computeError(): 오차 e = z - π(T, P) 코드 작성
//   3. linearizeOplus(): Jacobian ∂e/∂T, ∂e/∂P 수학적으로 유도 후 코드 작성
//   → Jacobian 유도가 복잡하고 실수 가능
//
// Ceres에서 BA 구현:
//   1. Functor 구조체의 operator() 정의
//   2. 순방향 계산만 작성 (오차 = 관측 - 투영)
//   3. Jacobian은 AutoDiff가 Jet 타입으로 자동 계산
//   → 구현 간편, 정확한 Jacobian 보장
//
// ★ 둘 다 비선형 최적화 + Robust Kernel(Huber 등) 지원
//   차이는 "Jacobian을 어떻게 구하느냐"
void problem1_ceres_vs_g2o()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Ceres vs g2o" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: Ceres와 g2o의 가장 큰 차이는 무엇인가요?" << std::endl;
    std::cout << "  A) Ceres는 Python만 지원한다" << std::endl;
    std::cout << "  B) Ceres는 자동 미분(Auto-diff)을 지원한다" << std::endl;
    std::cout << "  C) g2o는 비선형 최적화를 지원하지 않는다" << std::endl;
    std::cout << "  D) g2o는 Robust Kernel을 지원하지 않는다\n" << std::endl;

    std::cout << "💡 답: B) Ceres는 자동 미분(Auto-diff)을 지원한다" << std::endl;
    std::cout << "   - Ceres: operator()만 정의하면 Jacobian 자동 계산" << std::endl;
    std::cout << "   - g2o: linearizeOplus()에서 Jacobian을 직접 구현" << std::endl;
    std::cout << "   - 둘 다 비선형 최적화 + Robust Kernel 지원" << std::endl;
}

// 문제 2: AutoDiffCostFunction 템플릿 — BAL(Snavely) 모델
//
// new ceres::AutoDiffCostFunction<SnavelyError, 2, 9, 3>(...)
//
// SnavelyError: 오차 함수 구조체 (operator() 정의)
// 2: 잔차 차원 = 2D 재투영 오차 (Δu, Δv)
// 9: 카메라 파라미터 블록 크기
//    = angle-axis(3) + translation(3) + focal(1) + k1,k2(2)
// 3: 3D 점 파라미터 블록 크기 = (x, y, z)
//
// BAL 데이터셋의 투영 모델:
//   1. 카메라 좌표계로 변환: P_cam = R(θ)·P + t
//   2. 정규화 좌표: xp = -P_cam.x/P_cam.z, yp = -P_cam.y/P_cam.z
//   3. 왜곡 보정: r² = xp²+yp², D = 1+k1·r²+k2·r⁴
//   4. 픽셀 변환: u = f·D·xp, v = f·D·yp
//
// ★ 파라미터 수와 operator()의 인자 수가 반드시 일치해야 함
void problem2_autodiff_template()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: AutoDiffCostFunction 템플릿 파라미터" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 다음 코드에서 2, 9, 3은 각각 무엇을 의미하나요?\n" << std::endl;
    std::cout << "  new ceres::AutoDiffCostFunction<SnavelyError, 2, 9, 3>(...)\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   - 2: 잔차(residual) 차원 = 재투영 오차 (x, y)" << std::endl;
    std::cout << "   - 9: 첫 번째 파라미터 블록 차원 = 카메라 파라미터" << std::endl;
    std::cout << "     (angle-axis 3 + translation 3 + focal 1 + k1,k2 2)" << std::endl;
    std::cout << "   - 3: 두 번째 파라미터 블록 차원 = 3D 점 (x, y, z)" << std::endl;
}

// 문제 3: DENSE_SCHUR — Ceres에서의 Schur Complement
//
// options.linear_solver_type = ceres::DENSE_SCHUR;
//
// 내부 동작:
//   1. 3D 점 파라미터를 "소거 대상"으로 자동 인식
//      (AddResidualBlock에서 마지막 파라미터 블록이 점)
//   2. Hll (점-점 블록)의 역행렬 계산 (블록 대각 → O(M))
//   3. Schur complement S = Hpp - Hpl·Hll⁻¹·Hpl' 구성
//   4. S·Δp = bp' 풀기 (Dense 풀이)
//   5. Δl = Hll⁻¹(bl - Hpl'·Δp) 역대입
//
// g2o와의 대응:
//   Ceres DENSE_SCHUR ↔ g2o setMarginalized(true)
//   둘 다 동일한 Schur Complement 수행
//
// ★ 포즈 수가 적으면(~100) DENSE_SCHUR가 최적
//   포즈 수가 많으면(1000+) → SPARSE_SCHUR 또는 ITERATIVE_SCHUR
void problem3_dense_schur()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: DENSE_SCHUR" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: Ceres에서 options.linear_solver_type = DENSE_SCHUR;" << std::endl;
    std::cout << "      이 설정은 내부적으로 무엇을 하나요?\n" << std::endl;

    std::cout << "💡 답: Schur Complement를 적용하여 3D 점을 소거" << std::endl;
    std::cout << "   - 원래: (6N+3M) x (6N+3M) 크기의 H 행렬" << std::endl;
    std::cout << "   - Schur 후: 6N x 6N 크기의 S 행렬만 풀면 됨" << std::endl;
    std::cout << "   - 3D 점은 Back-substitution으로 복원" << std::endl;
    std::cout << "   - g2o에서는 setMarginalized(true)와 동일한 효과" << std::endl;
}

// 문제 4: HuberLoss — Outlier에 강건한 비용 함수
//
// new ceres::HuberLoss(1.0)  → δ = 1.0 픽셀
//
// Huber Loss 정의:
//   ρ(s) = { s                    if s <= δ²    (Inlier: L2 비용)
//          { 2δ√s - δ²            otherwise     (Outlier: 선형 증가)
//
// δ=1.0의 의미:
//   재투영 오차가 1 픽셀 이하 → 정상적인 관측 (L2 비용)
//   재투영 오차가 1 픽셀 초과 → Outlier로 간주 (비용 완화)
//
// Ceres 코드:
//   problem.AddResidualBlock(cost_function,
//                            new ceres::HuberLoss(1.0),  // Robust Loss
//                            camera_params, point_params);
//
// ★ Outlier 원인: 잘못된 매칭, 동적 물체, 맵 오류
//   Huber Loss 없이 L2만 사용하면 Outlier가 최적화를 지배
void problem4_huber_loss()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: HuberLoss" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: new ceres::HuberLoss(1.0)은 어떤 역할을 하나요?\n" << std::endl;

    std::cout << "💡 답: Outlier의 영향을 줄이는 Robust Loss Function" << std::endl;
    std::cout << "   - 작은 오차 (|e| <= delta): 제곱 유지 (e^2/2)" << std::endl;
    std::cout << "   - 큰 오차 (|e| > delta): 선형으로 완화" << std::endl;
    std::cout << "     → delta(|e| - delta/2)" << std::endl;
    std::cout << "   - delta=1.0: 1픽셀 이상의 오차를 outlier로 간주" << std::endl;
    std::cout << "   - 실제 데이터에서 잘못된 매칭의 영향을 억제" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 11 Quiz - Easy (Ceres 실습)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_ceres_vs_g2o();
    problem2_autodiff_template();
    problem3_dense_schur();
    problem4_huber_loss();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
