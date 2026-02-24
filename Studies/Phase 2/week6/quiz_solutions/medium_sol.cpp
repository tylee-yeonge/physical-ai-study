/**
 * Phase 2 Week 6 - 삼각측량 중급 퀴즈 (정답)
 *
 * 다루는 개념:
 *   - cv::triangulatePoints 사용법 (투영 행렬, 동차 좌표)
 *   - Bundle Adjustment 개념 (비선형 최적화)
 *   - Stereo Matching 알고리즘 (BM, SGM)
 *   - Essential Matrix 분해 + Cheirality Check
 *   - 포즈 추정 실패 사례 (순수 회전, 작은 이동)
 *
 * 선수 지식: week6 quiz_easy (삼각측량 기초), week5 (에피폴라 기하학)
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

// 삼각측량 구현 — OpenCV triangulatePoints 사용
//
// 투영 행렬 P = K · [R|t]  (3×4)
//   - P1 = K · [I|0]  (카메라 1이 원점일 때)
//   - P2 = K · [R|t]  (카메라 2의 외부 파라미터)
//
// triangulatePoints(P1, P2, pts1, pts2, points4D)
//   - 입력: 2×N 행렬 (각 열이 [u, v]^T)
//   - 출력: 4×N 동차 좌표 → w로 나누어 3D 좌표 복원
//
// 시차(disparity) = 같은 점의 두 이미지 x 좌표 차이
//   Z = (b · f) / d → 시차 60px, b=0.1m, f=600 → Z ≈ 1m
void problem1_implement_triangulation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 삼각측량 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);

    // 두 번째 카메라 위치 (10cm 오른쪽 이동)
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);             // 회전 없음
    cv::Mat t = (cv::Mat_<double>(3, 1) << 0.1, 0, 0);  // 10cm 이동

    // 투영 행렬 구성: P = K · [R|t]
    // P1 = K · [I|0] (카메라 1: 원점)
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));

    // P2 = K · [R|t] (카메라 2)
    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R.copyTo(RT(cv::Rect(0, 0, 3, 3)));
    t.copyTo(RT(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT;

    // 대응점 (이미지 중심에서 관측, 시차 60픽셀)
    std::vector<cv::Point2f> pts1 = {cv::Point2f(400, 300)};
    std::vector<cv::Point2f> pts2 = {cv::Point2f(460, 300)};  // 시차 60픽셀

    // triangulatePoints: 4×N 동차 좌표 반환
    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, points4D);

    // 동차 → 유클리드: [X, Y, Z, W] → [X/W, Y/W, Z/W]
    float w = points4D.at<float>(3, 0);
    cv::Point3f pt3d(points4D.at<float>(0, 0) / w, points4D.at<float>(1, 0) / w,
                     points4D.at<float>(2, 0) / w);

    std::cout << "복원된 3D 점: (" << pt3d.x << ", " << pt3d.y << ", " << pt3d.z << ")"
              << std::endl;
    std::cout << "\n💡 시차 60픽셀 → 깊이 약 1m" << std::endl;
}

// Bundle Adjustment 개념 — 삼각측량의 한계와 비선형 최적화
//
// Linear Triangulation (DLT)의 한계:
//   - 대수적 오차(algebraic error)를 최소화 → 물리적 의미 약함
//   - 재투영 오차(geometric error)를 직접 최소화하지 않음
//
// Bundle Adjustment (BA):
//   min Σᵢ Σⱼ ||pᵢⱼ_observed - π(Rⱼ, tⱼ, Xᵢ)||²
//   - pᵢⱼ: 카메라 j에서 관측한 점 i의 픽셀 좌표
//   - π(): 투영 함수 (3D → 2D)
//   - 최적화 변수: 3D 점(Xᵢ)과 카메라 포즈(Rⱼ, tⱼ) 동시 최적화
//
// 사용 라이브러리: Ceres Solver, g2o
void problem2_optimize_reconstruction()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Bundle Adjustment" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Linear Triangulation의 문제:" << std::endl;
    std::cout << "   - 재투영 오차를 직접 최소화하지 않음" << std::endl;
    std::cout << "   - 노이즈에 민감\n" << std::endl;

    std::cout << "Bundle Adjustment:" << std::endl;
    std::cout << "   - 재투영 오차의 제곱합 최소화" << std::endl;
    std::cout << "   - 비선형 최적화 (Levenberg-Marquardt)" << std::endl;
    std::cout << "   - 3D 점과 카메라 포즈 동시 최적화\n" << std::endl;

    std::cout << "Cost Function:" << std::endl;
    std::cout << "   Σ ||p_observed - π(P, X)||²" << std::endl;
    std::cout << "   (모든 관측에 대한 재투영 오차 합)\n" << std::endl;

    std::cout << "💡 SLAM에서는 Ceres나 g2o 라이브러리 사용" << std::endl;
}

// Stereo Matching 알고리즘 — 밀집 깊이 맵 생성
//
// Rectified Stereo (정렬된 스테레오):
//   - 에피폴라 선이 수평 → 대응점이 같은 y 좌표
//   - 2D 검색 → 1D 수평 검색으로 축소 (속도 ↑)
//
// 알고리즘:
//   1. Block Matching (BM): 윈도우 기반 SAD/SSD, 빠르지만 저품질
//   2. Semi-Global Matching (SGM): 여러 방향에서 비용 집계, 품질/속도 균형
//   3. Deep Learning: PSMNet, RAFT-Stereo, 고품질이지만 GPU 필요
//
// OpenCV: cv::StereoBM, cv::StereoSGBM
void problem3_stereo_matching()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Stereo Matching" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Rectified Stereo에서:" << std::endl;
    std::cout << "   - 대응점이 같은 y 좌표 (수평선)\n" << std::endl;
    std::cout << "   - 1D 검색으로 매칭 속도 ↑" << std::endl;
    std::cout << "   - 시차 맵 생성 가능\n" << std::endl;

    std::cout << "알고리즘:" << std::endl;
    std::cout << "   1. Block Matching (윈도우 기반)" << std::endl;
    std::cout << "   2. Semi-Global Matching (SGM)" << std::endl;
    std::cout << "   3. Deep Learning (PSMNet, RAFT-Stereo)\n" << std::endl;

    std::cout << "💡 OpenCV: cv::StereoBM, cv::StereoSGBM" << std::endl;
}

/**
 * @brief E에서 4가지 (R,t) 해 분해 + Cheirality Check
 *
 * E = U · diag(1,1,0) · V^T 에서 W 행렬을 이용해 4가지 해를 도출하고,
 * 삼각측량 + Cheirality Check로 유효한 해 1개를 선택한다.
 *
 * 4가지 해 도출:
 *   W = [0 -1 0; 1 0 0; 0 0 1]
 *   R1 = U · W^T · V^T,  R2 = U · W · V^T
 *   t1 = +U.col(2),      t2 = -U.col(2)
 *   → (R1,t1), (R1,t2), (R2,t1), (R2,t2)
 *
 * Cheirality Check:
 *   각 해로 삼각측량 수행 → 3D 점이 두 카메라 앞(Z > 0)에 있는 해 선택
 *   물리적으로 유효한 해는 항상 정확히 1개 (퇴화 케이스 제외)
 *
 * OpenCV: cv::recoverPose()가 이 전체 과정을 자동 수행
 */
void problem4_decompose_essential()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: E 분해 + Cheirality" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    // GT 포즈: 10도 Y축 회전 + 이동
    double angle = 10.0 * CV_PI / 180.0;
    cv::Mat R_gt = (cv::Mat_<double>(3, 3) <<
        cos(angle), 0, sin(angle),
        0, 1, 0,
        -sin(angle), 0, cos(angle));
    cv::Mat t_gt = (cv::Mat_<double>(3, 1) << 0.5, 0.1, 0.2);
    t_gt = t_gt / cv::norm(t_gt);  // 단위 벡터 (스케일 모호성)

    // E = [t]_x * R 계산
    //   [t]_x: t의 skew-symmetric 행렬
    //   이 행렬은 외적 t × v = [t]_x · v 를 행렬곱으로 표현
    double tx = t_gt.at<double>(0), ty = t_gt.at<double>(1), tz = t_gt.at<double>(2);
    cv::Mat t_skew = (cv::Mat_<double>(3, 3) <<
        0, -tz, ty,
        tz, 0, -tx,
        -ty, tx, 0);
    cv::Mat E = t_skew * R_gt;

    std::cout << "Essential Matrix E:" << std::endl << E << std::endl;

    // ✅ 정답 1: E를 SVD 분해
    cv::Mat w, u, vt;
    cv::SVD::compute(E, w, u, vt);
    std::cout << "\n특이값: " << w.t() << std::endl;

    // ✅ 정답 2: W 행렬 정의
    cv::Mat W_mat = (cv::Mat_<double>(3, 3) << 0, -1, 0, 1, 0, 0, 0, 0, 1);

    // ✅ 정답 3: 4가지 (R, t) 해 계산
    cv::Mat R1_est = u * W_mat.t() * vt;
    cv::Mat R2_est = u * W_mat * vt;
    cv::Mat t1_est = u.col(2);
    cv::Mat t2_est = -u.col(2);

    if (cv::determinant(R1_est) < 0) R1_est = -R1_est;
    if (cv::determinant(R2_est) < 0) R2_est = -R2_est;

    std::cout << "\nR1 det=" << cv::determinant(R1_est) << std::endl;
    std::cout << "R2 det=" << cv::determinant(R2_est) << std::endl;

    // ✅ 정답 4: Cheirality Check (개념 + GT 비교)
    std::cout << "\nGT와 비교:" << std::endl;
    std::cout << "  R1-R_gt diff: " << cv::norm(R1_est - R_gt) << std::endl;
    std::cout << "  R2-R_gt diff: " << cv::norm(R2_est - R_gt) << std::endl;

    std::cout << "\n4가지 해에서 올바른 해 선택 방법:" << std::endl;
    std::cout << "   1. 각 (R, t) 해로 3D 점 삼각측량" << std::endl;
    std::cout << "   2. 카메라 1에서 Z > 0 확인" << std::endl;
    std::cout << "   3. 카메라 2에서 X_cam2 = R*X + t의 Z > 0 확인" << std::endl;
    std::cout << "   4. 대부분의 점이 Z > 0인 해 = 올바른 해" << std::endl;
    std::cout << "\nOpenCV: cv::recoverPose()가 자동 수행" << std::endl;
}

/**
 * @brief 포즈 추정 실패 사례 분석
 *
 * 에피폴라 기하학 기반 포즈 추정이 실패하는 두 가지 대표적 케이스:
 *
 * Case 1: 순수 회전 (t = 0)
 *   - E = [t]_× · R = [0]_× · R = 0 (영행렬)
 *   - SVD 분해 자체가 의미 없음
 *   - 이유: 이동이 없으면 에피폴라 기하학이 퇴화 (degenerate)
 *   - 모든 에피폴라 선이 점으로 수축
 *
 * Case 2: 매우 작은 이동 (t ≈ 0)
 *   - E의 특이값이 매우 작음 → 수치적으로 불안정
 *   - 노이즈가 신호보다 커서 R, t 추정이 부정확
 *
 * SLAM에서의 대응:
 *   - 충분한 baseline(이동)이 있는 프레임 쌍만 사용
 *   - 키프레임 선택 시 최소 이동량 임계값 적용
 */
void problem5_failure_cases()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: 포즈 추정 실패 사례" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 회전 행렬 (30도 Z축)
    double angle = 30.0 * CV_PI / 180.0;
    cv::Mat R = (cv::Mat_<double>(3, 3) <<
        cos(angle), -sin(angle), 0,
        sin(angle), cos(angle), 0,
        0, 0, 1);

    // Case 1: 순수 회전 (t = 0) → E = 영행렬
    std::cout << "[Case 1] 순수 회전 (t = 0):" << std::endl;
    cv::Mat t_zero = cv::Mat::zeros(3, 1, CV_64F);
    cv::Mat t_skew_zero = (cv::Mat_<double>(3, 3) <<
        0, 0, 0, 0, 0, 0, 0, 0, 0);
    cv::Mat E_zero = t_skew_zero * R;

    std::cout << "   E = " << std::endl << E_zero << std::endl;
    std::cout << "   E가 0 행렬 → SVD 분해 의미 없음!" << std::endl;

    // Case 2: 매우 작은 이동 → 특이값 ≈ 0, 노이즈에 민감
    std::cout << "\n[Case 2] 매우 작은 이동 (t ≈ 0):" << std::endl;
    cv::Mat t_small = (cv::Mat_<double>(3, 1) << 1e-6, 0, 0);
    double tsx = t_small.at<double>(0);
    cv::Mat t_skew_small = (cv::Mat_<double>(3, 3) <<
        0, 0, 0,
        0, 0, -tsx,
        0, tsx, 0);
    cv::Mat E_small = t_skew_small * R;

    // ✅ 정답: E_small을 SVD 분해하고 특이값 확인
    cv::Mat w_small, u_small, vt_small;
    cv::SVD::compute(E_small, w_small, u_small, vt_small);
    std::cout << "   E_small 특이값: " << w_small.t() << std::endl;

    std::cout << "   t = " << t_small.t() << std::endl;
    std::cout << "   E의 특이값이 매우 작음 → 노이즈에 민감" << std::endl;
    std::cout << "\n실패 사례 정리:" << std::endl;
    std::cout << "   1. 순수 회전 (t=0): E = 0 → 분해 불가" << std::endl;
    std::cout << "   2. 작은 이동: 특이값 ≈ 0 → 수치 불안정" << std::endl;
    std::cout << "   해결: 충분한 baseline이 있는 프레임 쌍 사용" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 6 Quiz - Medium (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_implement_triangulation();
    problem2_optimize_reconstruction();
    problem3_stereo_matching();
    problem4_decompose_essential();
    problem5_failure_cases();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
