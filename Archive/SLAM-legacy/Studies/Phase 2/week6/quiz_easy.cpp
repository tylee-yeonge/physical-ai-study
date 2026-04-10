/**
 * Phase 2 Week 6 - 삼각측량 기초 퀴즈
 *
 * 다루는 개념:
 *   - Stereo Depth: 시차(disparity)에서 깊이 계산
 *   - 삼각측량 기하학: 두 광선의 교차로 3D 점 복원
 *   - 재투영 오차 (reprojection error)
 *   - Baseline과 깊이 정확도의 관계
 *   - 회전 행렬 유효성 검증 (직교성, det=1)
 *   - 스케일 모호성 (단안 SLAM의 근본적 한계)
 *
 * 삼각측량은 2D 관측에서 3D 구조를 복원하는 핵심 기술이다.
 * SLAM에서 새 랜드마크를 생성할 때 반드시 사용된다.
 *
 *     카메라1      baseline(b)     카메라2
 *       O1 ─────────────────────── O2
 *      /|                          |\
 *     / |                          | \
 *    /  |   depth(Z)               |  \
 *   /   |                          |   \
 *  p1   |                          |   p2
 *  ─────┼──                      ──┼─────
 *       |                          |
 *       └───────── X ──────────────┘   ← 3D 점
 *
 *  Z = (b · f) / d   (d = 시차 = p1.x - p2.x)
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

// Stereo Depth 계산 — 시차에서 깊이를 구하는 기본 공식
//
// 공식: Z = (baseline × focal) / disparity
//
//   - baseline (b): 두 카메라 간 거리 (미터)
//   - focal (f): 초점거리 (픽셀 단위)
//   - disparity (d): 같은 점의 좌/우 이미지 x 좌표 차이 (픽셀)
//
// ★ 시차와 깊이는 반비례:
//   시차 크면 → 가까운 물체, 시차 작으면 → 먼 물체
//   시차 = 0 → 무한대 (매칭 불가)
//
// 깊이 오차: ΔZ = Z² / (b·f) · Δd
//   → 깊이가 멀수록 오차가 급격히 증가 (Z²에 비례)
void problem1_depth_from_disparity()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Stereo Depth 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double baseline = 0.12;  // 12cm
    double focal = 600.0;    // 픽셀

    std::cout << "Stereo 카메라 설정:" << std::endl;
    std::cout << "   베이스라인: " << baseline << " m" << std::endl;
    std::cout << "   초점거리: " << focal << " 픽셀\n" << std::endl;

    // TODO: 시차에서 깊이 계산

    double disparity1 = 60.0;  // 픽셀
    double depth1 = 0.0;  // TODO

    double disparity2 = 30.0;
    double depth2 = 0.0;  // TODO

    std::cout << "시차 " << disparity1 << " 픽셀 → 깊이 ??? m" << std::endl;
    std::cout << "시차 " << disparity2 << " 픽셀 → 깊이 ??? m\n" << std::endl;

    std::cout << "💡 관찰:" << std::endl;
    std::cout << "   - 시차 ↑ → 깊이 ↓ (가까움)" << std::endl;
    std::cout << "   - 시차 ↓ → 깊이 ↑ (멀리)" << std::endl;
    std::cout << "   - 시차 = 0 → 무한대 (매칭 불가)" << std::endl;
}

// 삼각측량 기하학 — 두 카메라의 광선 교차로 3D 점 복원
//
// 각 카메라는 2D 픽셀에서 역투영하여 3D 광선을 생성:
//   ray = K^-1 · [u, v, 1]^T  (카메라 좌표계에서의 방향)
//
// 이론적으로 두 광선은 3D 점에서 정확히 교차해야 하지만,
// 실제로는 교차하지 않는다 (skew lines, 꼬인 위치).
//
// 원인:
//   - 픽셀 양자화 오차 (정수 좌표)
//   - 특징점 검출 오차 (서브픽셀 정확도 한계)
//   - 카메라 캘리브레이션 오차
//
// 해결: DLT (Direct Linear Transform)
//   - 두 광선의 "가장 가까운 점"을 최소제곱법으로 구함
//   - 또는 A·X = 0 형태의 선형 시스템을 SVD로 풀음
void problem2_triangulation_geometry()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 삼각측량 기하학" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "두 카메라에서 같은 3D 점을 관측:" << std::endl;
    std::cout << "   카메라 1 방향: ray1" << std::endl;
    std::cout << "   카메라 2 방향: ray2" << std::endl;
    std::cout << "   3D 점 = ray1과 ray2의 교점\n" << std::endl;

    std::cout << "질문: 왜 정확히 교차하지 않나요?\n" << std::endl;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   - 픽셀 양자화 오차" << std::endl;
    std::cout << "   - 특징점 검출 오차" << std::endl;
    std::cout << "   - 카메라 캘리브레이션 오차" << std::endl;
    std::cout << "   → 두 ray가 skew lines (꼬인 위치)\n" << std::endl;

    std::cout << "해결: DLT (Direct Linear Transform)" << std::endl;
    std::cout << "   - 최소제곱법으로 최적 3D 점 찾기" << std::endl;
}

// 재투영 오차 계산 — 삼각측량 결과의 품질 측정
//
// 재투영 오차 = ||p_observed - project(X_3d)||₂
//   - X_3d: 삼각측량으로 복원한 3D 점
//   - project(X_3d): 이 3D 점을 카메라로 다시 투영한 2D 좌표
//   - p_observed: 이미지에서 실제 관측된 2D 좌표
//
// 이 오차가 작을수록 삼각측량이 정확하다는 뜻.
// SLAM에서 outlier 제거, Bundle Adjustment 모두 이 오차를 기준으로 한다.
//
// 기준: < 1px 매우 좋음, < 5px 양호, > 5px outlier 의심
void problem3_reprojection_error()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 재투영 오차" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 3D 점
    cv::Point3f pt3d(1.0, 0.5, 3.0);

    // 카메라 행렬 (R=I, t=0 가정)
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);

    // TODO: 3D 점을 카메라 행렬로 투영하여 픽셀 좌표 계산

    std::cout << "3D 점: (" << pt3d.x << ", " << pt3d.y << ", " << pt3d.z << ")" << std::endl;
    std::cout << "투영된 2D 점: (???, ???)\n" << std::endl;

    std::cout << "💡 재투영 오차 = ||관측 - 투영|| (픽셀)" << std::endl;
    std::cout << "   - 1 픽셀 이하: 매우 좋음" << std::endl;
    std::cout << "   - 5 픽셀 이상: 문제 있음 (outlier)" << std::endl;
}

// Baseline과 깊이 정확도의 관계 — 트레이드오프 분석
//
// Z = (b · f) / d 에서:
//   - b(baseline)가 클수록 같은 깊이에서 시차 d가 커짐
//   - 시차가 클수록 양자화 오차의 상대적 영향이 줄어듦
//   - → 깊이 정확도 향상
//
// 하지만 baseline이 너무 크면:
//   - 시야 겹침(overlap) 감소 → 매칭 실패
//   - 가까운 물체의 시차가 너무 커서 매칭 범위 초과
//
// ★ 실전 기준:
//   - 실내(1~5m): 짧은 baseline (5~10cm)
//   - 자율주행(10~100m): 긴 baseline (30~60cm)
//   - 단안 SLAM(VINS): 연속 프레임 간 이동이 baseline 역할
void problem4_baseline_vs_accuracy()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Baseline과 정확도" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: Baseline이 크면 좋은가요?\n" << std::endl;

    std::cout << "Baseline ↑ (카메라 간격 넓음):" << std::endl;
    std::cout << "   장점: 깊이 정확도 ↑ (시차 커짐)" << std::endl;
    std::cout << "   단점: 매칭 어려움, 시야 겹침 ↓\n" << std::endl;

    std::cout << "Baseline ↓ (카메라 간격 좁음):" << std::endl;
    std::cout << "   장점: 매칭 쉬움, 시야 겹침 ↑" << std::endl;
    std::cout << "   단점: 깊이 정확도 ↓ (시차 작음)\n" << std::endl;

    std::cout << "💡 실제 응용:" << std::endl;
    std::cout << "   - 실내 (가까움): 짧은 baseline (5-10cm)" << std::endl;
    std::cout << "   - 자율주행 (멀리): 긴 baseline (30-60cm)" << std::endl;
    std::cout << "   - VINS: 연속 프레임 (작은 baseline)" << std::endl;
}

/**
 * @brief 회전 행렬 유효성 검증
 *
 * 유효한 회전 행렬의 두 가지 필수 조건:
 *   1. 직교성: R^T · R = I (각 열이 단위 벡터이며 서로 직교)
 *   2. 행렬식: det(R) = 1 (반사가 아닌 순수 회전)
 *      - det(R) = -1이면 "반사 행렬" (거울 뒤집힘)
 *
 * SLAM에서 포즈를 추정하면 수치 오차로 R이 약간 손상될 수 있다.
 * SVD로 "가장 가까운 회전 행렬"로 복구:
 *   R = U·S·V^T → R_fixed = U·V^T
 *   (특이값을 모두 1로 강제하여 직교 행렬 보장)
 *   det(R_fixed) < 0이면 V의 마지막 열 부호 반전
 */
void problem5_validate_rotation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: 회전 행렬 유효성 검증" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 유효한 회전 행렬 (Y축 15도)
    double angle = 15.0 * CV_PI / 180.0;
    cv::Mat R_valid = (cv::Mat_<double>(3, 3) <<
        cos(angle), 0, sin(angle),
        0, 1, 0,
        -sin(angle), 0, cos(angle));

    // 노이즈로 손상된 행렬
    cv::Mat noise = cv::Mat::zeros(3, 3, CV_64F);
    cv::RNG rng(42);
    rng.fill(noise, cv::RNG::NORMAL, 0, 0.01);
    cv::Mat R_noisy = R_valid + noise;

    std::cout << "R_valid:" << std::endl << R_valid << std::endl;
    std::cout << "\nR_noisy (노이즈 추가):" << std::endl << R_noisy << std::endl;

    // TODO 1: R_valid와 R_noisy 각각에 대해 두 가지 조건을 검증하세요
    //   힌트: Doxygen 주석의 직교성 조건과 행렬식 조건을 확인하세요

    // TODO 2: R_noisy를 SVD로 가장 가까운 회전 행렬로 복구하세요
    //   힌트: 위 Doxygen 주석의 SVD 복구 공식을 참고하세요

    std::cout << "\n올바른 회전 행렬 조건:" << std::endl;
    std::cout << "   1. R^T * R = I (직교성)" << std::endl;
    std::cout << "   2. det(R) = 1 (반사가 아닌 회전)" << std::endl;
    std::cout << "   SVD 복구: R_fixed = U * Vt (가장 가까운 직교 행렬)" << std::endl;
}

/**
 * @brief 스케일 모호성 이해
 *
 * 단안 카메라에서 E = [t]_× · R 을 분해하면 t의 "방향"만 알 수 있고
 * "크기"는 알 수 없다. 이것이 스케일 모호성(scale ambiguity)이다.
 *
 * 수학적 증명:
 *   E(t) = [t]_× · R
 *   E(2t) = [2t]_× · R = 2·[t]_× · R = 2·E(t)
 *   → E를 정규화하면 E(t)/||E(t)|| = E(2t)/||E(2t)||
 *   → t와 2t는 같은 정규화된 E를 생성 → 스케일 구분 불가
 *
 * 해결 방법:
 *   - Stereo 카메라: baseline(물리적 거리)을 알고 있으므로 절대 스케일 복원
 *   - IMU 융합 (VINS): 가속도계의 중력 벡터로 스케일 추정
 *   - 알려진 물체: 물체의 실제 크기로 추정
 */
void problem6_scale_ambiguity()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 6: 스케일 모호성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 회전 행렬 (10도 Y축)
    double angle = 10.0 * CV_PI / 180.0;
    cv::Mat R = (cv::Mat_<double>(3, 3) <<
        cos(angle), 0, sin(angle),
        0, 1, 0,
        -sin(angle), 0, cos(angle));

    // 기본 이동 벡터
    cv::Mat t_base = (cv::Mat_<double>(3, 1) << 1.0, 0.0, 0.0);

    std::cout << "t_base: " << t_base.t() << std::endl;

    // TODO: 다양한 스케일에서 E를 구하고, 정규화 후 비교
    // 스케일: 0.5, 1.0, 2.0, 5.0
    //
    // 힌트: E = [t]_× · R 공식을 사용하세요
    //       정규화 후 E가 스케일에 무관하게 동일한지 확인하세요
    //
    // ★ 모든 스케일에서 정규화된 E가 동일한지 확인!
    //   → 이것이 스케일 모호성의 수학적 증거

    std::cout << "\n스케일 모호성의 의미:" << std::endl;
    std::cout << "   - t와 2t는 같은 방향의 E를 생성 (크기만 다름)" << std::endl;
    std::cout << "   - 정규화 후 E는 동일 → 스케일 구분 불가" << std::endl;
    std::cout << "   - 단안 SLAM에서 절대 스케일을 알 수 없는 이유!" << std::endl;
    std::cout << "\n해결 방법:" << std::endl;
    std::cout << "   - Stereo: 베이스라인 알려짐" << std::endl;
    std::cout << "   - IMU 융합: 가속도로 스케일 추정 (VINS)" << std::endl;
    std::cout << "   - 알려진 물체: 물체 크기로 추정" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 6 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_depth_from_disparity();
    problem2_triangulation_geometry();
    problem3_reprojection_error();
    problem4_baseline_vs_accuracy();
    problem5_validate_rotation();
    problem6_scale_ambiguity();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
