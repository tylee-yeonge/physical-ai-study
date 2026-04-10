/**
 * Phase 2 Week 6 - 삼각측량 기초 퀴즈 (정답)
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

    // ✅ 정답: 시차에서 깊이 계산

    double disparity1 = 60.0;  // 픽셀
    double depth1 = baseline * focal / disparity1;

    double disparity2 = 30.0;
    double depth2 = baseline * focal / disparity2;

    std::cout << "시차 " << disparity1 << " 픽셀 → 깊이 " << depth1 << " m" << std::endl;
    std::cout << "시차 " << disparity2 << " 픽셀 → 깊이 " << depth2 << " m\n" << std::endl;

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [실행 결과] Z = baseline × focal / disparity" << std::endl;
    std::cout << "   시차 60px → 깊이 = 0.12 × 600 / 60 = 1.2 m" << std::endl;
    std::cout << "   시차 30px → 깊이 = 0.12 × 600 / 30 = 2.4 m" << std::endl;
    std::cout << std::endl;
    std::cout << "   [핵심 포인트]" << std::endl;
    std::cout << "   시차와 깊이는 반비례: 시차 2배 → 깊이 1/2" << std::endl;
    std::cout << "   깊이 오차 공식: ΔZ = Z²/(b·f) · Δd" << std::endl;
    std::cout << "   → 먼 물체일수록 깊이 오차가 Z²에 비례하여 급증" << std::endl;
    std::cout << "   → 이것이 스테레오 카메라의 유효 거리가 제한되는 이유" << std::endl;
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

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [정답] 3가지 오차 원인으로 두 광선이 정확히 교차하지 않음" << std::endl;
    std::cout << "   1. 픽셀 양자화 오차: 좌표가 정수 단위라 소수점 이하 정보 손실" << std::endl;
    std::cout << "   2. 특징점 검출 오차: ORB/SIFT 검출 위치가 0.5~2px 부정확" << std::endl;
    std::cout << "   3. 카메라 캘리브레이션 오차: K, 왜곡 계수 추정 오차" << std::endl;
    std::cout << "   → 3D 공간에서 두 ray가 skew lines (꼬인 위치)이 됨\n" << std::endl;

    std::cout << "   [해결법] DLT (Direct Linear Transform)" << std::endl;
    std::cout << "   두 광선의 '가장 가까운 점'을 최소제곱법으로 추정" << std::endl;
    std::cout << "   OpenCV: cv::triangulatePoints() 함수가 이를 수행" << std::endl;
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

    // ✅ 정답: 3D 점을 카메라 행렬로 투영하여 픽셀 좌표 계산
    // R=I, t=0 가정이므로: p = K * [X/Z, Y/Z, 1]^T
    double u = K.at<double>(0, 0) * pt3d.x / pt3d.z + K.at<double>(0, 2);
    double v = K.at<double>(1, 1) * pt3d.y / pt3d.z + K.at<double>(1, 2);

    std::cout << "3D 점: (" << pt3d.x << ", " << pt3d.y << ", " << pt3d.z << ")" << std::endl;
    std::cout << "투영된 2D 점: (" << u << ", " << v << ")\n" << std::endl;

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [코드 핵심] 투영 공식: u = fx*X/Z + cx, v = fy*Y/Z + cy" << std::endl;
    std::cout << "   (1.0, 0.5, 3.0) → u = 600*1/3 + 400 = 600, v = 600*0.5/3 + 300 = 400" << std::endl;
    std::cout << std::endl;
    std::cout << "   [재투영 오차란?]" << std::endl;
    std::cout << "   삼각측량으로 복원한 3D 점을 다시 카메라에 투영했을 때" << std::endl;
    std::cout << "   원래 관측된 2D 위치와의 유클리드 거리 (픽셀 단위)" << std::endl;
    std::cout << std::endl;
    std::cout << "   [품질 기준]" << std::endl;
    std::cout << "   < 1px: 매우 좋음 (정밀한 삼각측량)" << std::endl;
    std::cout << "   1~5px: 양호 (일반적인 수준)" << std::endl;
    std::cout << "   > 5px: outlier 의심 → RANSAC으로 제거" << std::endl;
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

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [정답] Baseline이 클수록 깊이 정확도↑, 하지만 트레이드오프 존재" << std::endl;
    std::cout << std::endl;
    std::cout << "   [수학적 근거]" << std::endl;
    std::cout << "   깊이 오차 ΔZ = Z²/(b·f) · Δd" << std::endl;
    std::cout << "   b(baseline)가 클수록 ΔZ 감소 → 정확도 향상" << std::endl;
    std::cout << std::endl;
    std::cout << "   [실전 응용]" << std::endl;
    std::cout << "   실내 (1~5m 거리): baseline 5~10cm (스마트폰, 소형 로봇)" << std::endl;
    std::cout << "   자율주행 (10~100m): baseline 30~60cm (차량 루프)" << std::endl;
    std::cout << "   단안 SLAM (VINS): 카메라 이동 자체가 baseline 역할" << std::endl;
    std::cout << "   → 너무 크면 시야 겹침↓, 매칭 실패 → 적절한 균형 필요" << std::endl;
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

    // ✅ 정답 1: R_valid와 R_noisy 각각에 대해 검증
    double ortho_err_valid = cv::norm(R_valid.t() * R_valid - cv::Mat::eye(3, 3, CV_64F));
    double det_valid = cv::determinant(R_valid);
    std::cout << "\nR_valid 검증:" << std::endl;
    std::cout << "   직교성 오차: " << ortho_err_valid << std::endl;
    std::cout << "   행렬식: " << det_valid << std::endl;

    double ortho_err_noisy = cv::norm(R_noisy.t() * R_noisy - cv::Mat::eye(3, 3, CV_64F));
    double det_noisy = cv::determinant(R_noisy);
    std::cout << "\nR_noisy 검증:" << std::endl;
    std::cout << "   직교성 오차: " << ortho_err_noisy << std::endl;
    std::cout << "   행렬식: " << det_noisy << std::endl;

    // ✅ 정답 2: R_noisy를 SVD로 가장 가까운 회전 행렬로 복구
    cv::Mat w, u, vt;
    cv::SVD::compute(R_noisy, w, u, vt);
    cv::Mat R_fixed = u * vt;
    if (cv::determinant(R_fixed) < 0)
    {
        cv::Mat D = cv::Mat::eye(3, 3, CV_64F);
        D.at<double>(2, 2) = -1;
        R_fixed = u * D * vt;
    }
    double ortho_err_fixed = cv::norm(R_fixed.t() * R_fixed - cv::Mat::eye(3, 3, CV_64F));
    double det_fixed = cv::determinant(R_fixed);
    std::cout << "\nR_fixed (SVD 복구):" << std::endl;
    std::cout << "   직교성 오차: " << ortho_err_fixed << std::endl;
    std::cout << "   행렬식: " << det_fixed << std::endl;

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

    // ✅ 정답: 다양한 스케일에서 E를 구하고, 정규화 후 비교
    std::vector<double> scales = {0.5, 1.0, 2.0, 5.0};
    cv::Mat E_ref;
    std::cout << "\n스케일별 E 비교:" << std::endl;
    for (double s : scales)
    {
        cv::Mat t_s = s * t_base;
        double tx = t_s.at<double>(0), ty = t_s.at<double>(1), tz = t_s.at<double>(2);
        cv::Mat t_skew = (cv::Mat_<double>(3, 3) <<
            0, -tz, ty,
            tz, 0, -tx,
            -ty, tx, 0);
        cv::Mat E_s = t_skew * R;
        cv::Mat E_norm = E_s / cv::norm(E_s);
        if (E_ref.empty())
        {
            E_ref = E_norm.clone();
            std::cout << "   scale=" << s << " (기준)" << std::endl;
        }
        else
        {
            std::cout << "   scale=" << s << " → diff=" << cv::norm(E_norm - E_ref) << std::endl;
        }
    }
    std::cout << "   → 정규화 후 모든 E가 동일!" << std::endl;

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
    std::cout << "Phase 2 Week 6 Quiz - Easy (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_depth_from_disparity();
    problem2_triangulation_geometry();
    problem3_reprojection_error();
    problem4_baseline_vs_accuracy();
    problem5_validate_rotation();
    problem6_scale_ambiguity();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
