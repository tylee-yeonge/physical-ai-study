/**
 * Phase 3 Week 11 - Ceres 실습 퀴즈 (Easy) 정답 및 해설
 */

#include <iostream>

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 11 Easy Quiz - 정답 및 해설" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 문제 1 해설
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Ceres vs g2o" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    std::cout << "정답: B) Ceres는 자동 미분(Auto-diff)을 지원한다\n" << std::endl;
    std::cout << "해설:" << std::endl;
    std::cout << "  Ceres의 가장 큰 장점은 자동 미분(Auto-differentiation)입니다." << std::endl;
    std::cout << "  operator()에서 순방향 계산만 정의하면, Ceres가 내부적으로" << std::endl;
    std::cout << "  Jet<double, N> 타입을 사용하여 정확한 Jacobian을 자동 계산합니다.\n" << std::endl;
    std::cout << "  g2o에서는 linearizeOplus()에서 Jacobian을 직접 계산해야 합니다." << std::endl;
    std::cout << "  다만, g2o도 표준 SLAM 문제(EdgeProjectXYZ2UV 등)에 대해서는" << std::endl;
    std::cout << "  이미 Jacobian이 구현되어 있으므로 직접 작성할 일은 적습니다.\n" << std::endl;
    std::cout << "  A, C, D는 모두 틀린 설명입니다:" << std::endl;
    std::cout << "  - Ceres는 C++ 라이브러리입니다 (Python 바인딩도 있음)" << std::endl;
    std::cout << "  - g2o도 비선형 최적화를 지원합니다 (GN, LM)" << std::endl;
    std::cout << "  - g2o도 Robust Kernel을 지원합니다 (Huber, Cauchy 등)" << std::endl;

    // 문제 2 해설
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: AutoDiffCostFunction 템플릿 파라미터" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    std::cout << "정답:" << std::endl;
    std::cout << "  AutoDiffCostFunction<SnavelyError, 2, 9, 3>" << std::endl;
    std::cout << "                                     ^  ^  ^" << std::endl;
    std::cout << "                            잔차 차원 |  |  3D 점 차원" << std::endl;
    std::cout << "                                      |  카메라 파라미터 차원" << std::endl;
    std::cout << "\n해설:" << std::endl;
    std::cout << "  - 2 (잔차): 재투영 오차 = (predicted_x - observed_x," << std::endl;
    std::cout << "                             predicted_y - observed_y)" << std::endl;
    std::cout << "  - 9 (카메라): Snavely 카메라 모델" << std::endl;
    std::cout << "    [0-2] angle-axis 회전 (3)" << std::endl;
    std::cout << "    [3-5] 평행이동 tx, ty, tz (3)" << std::endl;
    std::cout << "    [6]   focal length (1)" << std::endl;
    std::cout << "    [7-8] 방사 왜곡 k1, k2 (2)" << std::endl;
    std::cout << "  - 3 (점): 3D 점 좌표 (x, y, z)\n" << std::endl;
    std::cout << "  Week 8에서는 <ReprojError, 2, 6, 3>이었습니다." << std::endl;
    std::cout << "  BAL은 focal + distortion을 추가하여 9차원입니다." << std::endl;

    // 문제 3 해설
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: DENSE_SCHUR" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    std::cout << "정답: Schur Complement를 적용하여 3D 점을 소거\n" << std::endl;
    std::cout << "해설:" << std::endl;
    std::cout << "  원래 정규 방정식: H * dx = b" << std::endl;
    std::cout << "  [Hcc  Hcp] [dc]   [bc]" << std::endl;
    std::cout << "  [Hpc  Hpp] [dp] = [bp]\n" << std::endl;
    std::cout << "  Schur Complement 적용:" << std::endl;
    std::cout << "  S = Hcc - Hcp * Hpp^(-1) * Hpc" << std::endl;
    std::cout << "  S * dc = bc - Hcp * Hpp^(-1) * bp\n" << std::endl;
    std::cout << "  Hpp는 블록 대각 행렬이므로 역행렬 계산이 빠릅니다." << std::endl;
    std::cout << "  DENSE_SCHUR: S를 dense 행렬로 풀기 (소규모)" << std::endl;
    std::cout << "  SPARSE_SCHUR: S를 sparse 행렬로 풀기 (대규모)" << std::endl;
    std::cout << "  g2o에서는 setMarginalized(true)가 동일한 역할입니다." << std::endl;

    // 문제 4 해설
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: HuberLoss" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    std::cout << "정답: Outlier의 영향을 줄이는 Robust Loss Function\n" << std::endl;
    std::cout << "해설:" << std::endl;
    std::cout << "  일반 최소제곱: loss(e) = e^2" << std::endl;
    std::cout << "  → outlier(큰 오차)가 전체 최적화를 지배!\n" << std::endl;
    std::cout << "  Huber Loss:" << std::endl;
    std::cout << "  rho(e) = { e^2 / 2            if |e| <= delta" << std::endl;
    std::cout << "           { delta(|e| - delta/2) if |e| > delta\n" << std::endl;
    std::cout << "  작은 오차: 제곱 (정상 관측)" << std::endl;
    std::cout << "  큰 오차: 선형 (outlier 영향 감소)\n" << std::endl;
    std::cout << "  delta=1.0은 '1픽셀 이상이면 outlier로 간주'를 의미합니다." << std::endl;
    std::cout << "  실제 SLAM에서는 잘못된 특징점 매칭이 빈번하므로" << std::endl;
    std::cout << "  Robust Loss 적용이 필수적입니다." << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Easy Quiz 해설 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
