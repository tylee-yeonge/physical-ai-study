/**
 * Phase 2 Week 7 - PnP (Perspective-n-Point) 중급 퀴즈 (정답)
 *
 * 다루는 개념:
 *   - cv::solvePnP 사용법 (기본 PnP 구현)
 *   - RANSAC 반복 횟수 공식: N = log(1-p) / log(1-w^s)
 *   - 포즈 최적화: Linear PnP → Levenberg-Marquardt refinement
 *   - DLT 삼각측량 직접 구현 (A 행렬 구성, SVD, 베이스라인 실험)
 *   - DLT PnP 직접 구현 (2N×12 행렬, outlier 영향 실험)
 *
 * 선수 지식: week7 easy (PnP 이론), week5 (에피폴라 기하학), week6 (삼각측량)
 *
 * PnP DLT의 핵심 아이디어:
 *   투영 방정식 λ·[u, v, 1]ᵀ = P · [X, Y, Z, 1]ᵀ 에서
 *   외적 소거로 λ를 제거하면 각 대응점마다 2개 선형 방정식 생성.
 *   n개 점 → 2n×12 시스템 → SVD로 P의 12개 원소 추정.
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

// cv::solvePnP 기본 사용 — 3D-2D 대응에서 카메라 포즈 추정
//
// 흐름:
//   1. 3D 월드 점과 카메라 내부 파라미터 준비
//   2. projectPoints로 Ground Truth 2D 투영 생성
//   3. solvePnP로 포즈 (rvec, tvec) 추정
//   4. Ground Truth와 비교하여 정확도 확인
//
// cv::solvePnP(objectPoints, imagePoints, K, distCoeffs, rvec, tvec)
//   - objectPoints: 3D 월드 좌표 (vector<Point3f>)
//   - imagePoints: 대응 2D 이미지 좌표 (vector<Point2f>)
//   - K: 3×3 카메라 내부 행렬
//   - distCoeffs: 왜곡 계수 (없으면 cv::Mat())
//   - rvec: 출력 — Rodrigues 회전 벡터 (3×1)
//   - tvec: 출력 — 이동 벡터 (3×1)
//
// ★ 노이즈 없는 이상적 조건에서는 정확히 복원됨
void problem1_implement_pnp()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: PnP 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 3D 점들 (월드 좌표계)
    std::vector<cv::Point3f> pts3d = {cv::Point3f(0, 0, 5), cv::Point3f(1, 0, 5),
                                      cv::Point3f(0, 1, 5), cv::Point3f(1, 1, 5)};

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);

    // Ground truth 포즈
    cv::Mat rvec_gt = (cv::Mat_<double>(3, 1) << 0, 0, 0);    // 회전 없음
    cv::Mat tvec_gt = (cv::Mat_<double>(3, 1) << 0.5, 0, 0);  // X 이동

    // 2D 관측 생성
    std::vector<cv::Point2f> pts2d;
    cv::projectPoints(pts3d, rvec_gt, tvec_gt, K, cv::Mat(), pts2d);

    // TODO: solvePnP로 포즈 추정
    cv::Mat rvec, tvec;
    cv::solvePnP(pts3d, pts2d, K, cv::Mat(), rvec, tvec);

    std::cout << "Ground Truth t: " << tvec_gt.t() << std::endl;
    std::cout << "Estimated t:    " << tvec.t() << std::endl;
    std::cout << "Error: " << cv::norm(tvec_gt - tvec) << " m\n" << std::endl;

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [코드 핵심]" << std::endl;
    std::cout << "   1. projectPoints: GT 포즈로 3D→2D 투영 (관측값 생성)" << std::endl;
    std::cout << "   2. solvePnP: 3D-2D 대응에서 포즈(rvec, tvec) 역추정" << std::endl;
    std::cout << "   3. 노이즈 없는 이상적 조건 → 오차 ≈ 0 (정확히 복원)\n" << std::endl;
    std::cout << "   [solvePnP 인자 설명]" << std::endl;
    std::cout << "   - objectPoints: 3D 월드 좌표 (vector<Point3f>)" << std::endl;
    std::cout << "   - imagePoints: 2D 이미지 좌표 (vector<Point2f>)" << std::endl;
    std::cout << "   - K: 카메라 내부 행렬, distCoeffs: 왜곡 계수" << std::endl;
    std::cout << "   - rvec: Rodrigues 회전 벡터 (3x1), tvec: 이동 벡터 (3x1)" << std::endl;
}

// RANSAC 반복 횟수 공식 — 성공 확률 p를 보장하는 최소 반복 수
//
// 공식: N = log(1 - p) / log(1 - w^s)
//   - p: 원하는 성공 확률 (보통 0.99)
//   - w: inlier 비율 (0~1)
//   - s: 모델 추정에 필요한 최소 점 수 (PnP: s=4)
//   - N: 필요한 반복 횟수
//
// 직관:
//   - w^s = 한 번 샘플링에서 모든 점이 inlier일 확률
//   - 1 - w^s = 한 번 샘플링이 실패할 확률
//   - (1 - w^s)^N = N번 모두 실패할 확률
//   - 1 - (1 - w^s)^N ≥ p → N 도출
//
// 예시 (s=4, p=0.99):
//   w=50% → w^s=6.25% → N≈72회  (outlier 많으면 반복 많이 필요)
//   w=90% → w^s=65.6% → N≈6회   (inlier 많으면 빠르게 수렴)
//
// ★ RANSAC의 반복 횟수는 outlier 비율에 지수적으로 증가한다
void problem2_ransac_iterations()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: RANSAC 반복 횟수" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "PnP는 최소 4개 점 필요 (s=4)\n" << std::endl;

    // N = log(1-p) / log(1-w^s)
    double p = 0.99;
    int s = 4;

    std::vector<double> inlier_ratios = {0.5, 0.7, 0.9};

    std::cout << "Inlier 비율  |  필요 반복" << std::endl;
    std::cout << "-------------+-----------" << std::endl;

    for (double w : inlier_ratios)
    {
        int N = (int)(log(1 - p) / log(1 - pow(w, s)));
        std::cout << "   " << (int)(w * 100) << "%       |    " << N << "회" << std::endl;
    }

    std::cout << "\n💡 정답 해설:" << std::endl;
    std::cout << "   [공식] N = log(1-p) / log(1-w^s)" << std::endl;
    std::cout << "   p=0.99(성공 확률), s=4(PnP 최소 점), w=inlier 비율\n" << std::endl;
    std::cout << "   [실행 결과 해석]" << std::endl;
    std::cout << "   w=50% → w^4=6.25% → N≈72회  (outlier 많으면 반복 많이 필요)" << std::endl;
    std::cout << "   w=70% → w^4=24.0% → N≈16회  (중간)" << std::endl;
    std::cout << "   w=90% → w^4=65.6% → N≈6회   (inlier 많으면 빠르게 수렴)\n" << std::endl;
    std::cout << "   [직관적 이해]" << std::endl;
    std::cout << "   w^s = 랜덤 4점이 모두 inlier일 확률" << std::endl;
    std::cout << "   → outlier가 많으면 '좋은 4점'을 뽑기 어려움" << std::endl;
    std::cout << "   → 반복 횟수가 지수적으로 증가" << std::endl;
}

// 포즈 최적화 — Linear PnP의 한계와 비선형 refinement
//
// Linear PnP (DLT)의 한계:
//   - A·x = 0 형태의 대수적(algebraic) 오차를 최소화
//   - 기하학적으로 의미 있는 재투영 오차를 직접 최소화하지 않음
//   - 노이즈에 민감하고, 최적해가 아닐 수 있음
//
// 비선형 최적화 (Refinement):
//   - 목적함수: min Σᵢ ||pᵢ_obs - π(R, t, Xᵢ)||²
//     (π: 3D→2D 투영 함수, ||·||: 유클리드 거리)
//   - 재투영 오차(reprojection error)를 직접 최소화
//   - Levenberg-Marquardt (LM) 알고리즘으로 반복 최적화
//   - DLT 결과를 초기값으로 사용하여 수렴 보장
//
// OpenCV 함수:
//   - cv::solvePnPRefineLM(): LM으로 rvec, tvec 정밀화
//   - cv::solvePnP(..., SOLVEPNP_ITERATIVE): 내부적으로 LM 적용
//
// ★ SLAM에서의 활용:
//   PnP → 초기 포즈 추정 (빠름)
//   BA (Bundle Adjustment) → 전체 포즈+맵 동시 최적화 (정확)
void problem3_pose_optimization()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 포즈 최적화" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Linear PnP의 한계:" << std::endl;
    std::cout << "   - 재투영 오차를 직접 최소화 안 함" << std::endl;
    std::cout << "   - 노이즈에 민감\n" << std::endl;

    std::cout << "비선형 최적화 (Refinement):" << std::endl;
    std::cout << "   - Minimize: Σ ||p_obs - π(R, t, X)||²" << std::endl;
    std::cout << "   - Levenberg-Marquardt" << std::endl;
    std::cout << "   - OpenCV: cv::solvePnPRefineLM()\n" << std::endl;

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [Linear PnP(DLT)가 부정확한 이유]" << std::endl;
    std::cout << "   DLT는 '대수적 오차(algebraic error)'를 최소화" << std::endl;
    std::cout << "   → 이미지 상의 거리(픽셀)와 무관한 수학적 잔차" << std::endl;
    std::cout << "   → 실제 재투영 오차(기하학적 의미)를 직접 줄이지 않음\n" << std::endl;
    std::cout << "   [비선형 최적화(Refinement)란?]" << std::endl;
    std::cout << "   목적함수: min Σ ||p_obs - π(R,t,X)||²" << std::endl;
    std::cout << "   → 재투영 오차(픽셀 단위)를 직접 최소화" << std::endl;
    std::cout << "   → Levenberg-Marquardt 알고리즘으로 반복 수렴" << std::endl;
    std::cout << "   → DLT 결과를 초기값으로 사용 (수렴 보장)\n" << std::endl;
    std::cout << "   [SLAM에서의 2단계 전략]" << std::endl;
    std::cout << "   ① PnP(DLT) → 빠르게 초기 포즈 추정 (실시간성)" << std::endl;
    std::cout << "   ② BA → 모든 포즈+3D점을 동시에 최적화 (정확성)" << std::endl;
    std::cout << "   OpenCV: cv::solvePnPRefineLM()으로 ①→② 가능" << std::endl;
}

// DLT 삼각측량 직접 구현 — A 행렬 구성부터 SVD까지
//
// 알고리즘:
//   1. 투영 행렬 P₁, P₂ 계산: Pᵢ = K · [Rᵢ|tᵢ]
//   2. Ground Truth 3D 점을 두 카메라에 투영 → (u₁,v₁), (u₂,v₂)
//   3. A 행렬 구성 (4×4):
//        A[0] = u₁·P₁₃ᵀ - P₁₁ᵀ    ← 카메라1의 x 제약
//        A[1] = v₁·P₁₃ᵀ - P₁₂ᵀ    ← 카메라1의 y 제약
//        A[2] = u₂·P₂₃ᵀ - P₂₁ᵀ    ← 카메라2의 x 제약
//        A[3] = v₂·P₂₃ᵀ - P₂₂ᵀ    ← 카메라2의 y 제약
//   4. SVD(A) → V의 마지막 열 = 3D 점 (동차 좌표)
//   5. 동차→유클리드 변환: X₃ₐ = X[:3] / X[3]
//
// 추가 실험: 베이스라인 크기별 정확도 비교
//   - 좁은 베이스라인: 삼각형 꼭지각 작음 → 깊이 불확실성 증가
//   - 넓은 베이스라인: 교차각 큼 → 깊이 정확도 향상
//   - 노이즈 없이는 차이 없지만, 노이즈 시 좁은 베이스라인에서 오차 급증
//
// ★ OpenCV cv::triangulatePoints와 동일한 원리
void problem4_dlt_triangulation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: DLT 삼각측량 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 내부 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    // 카메라 1: 원점 [I|0]
    cv::Mat R1 = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t1 = cv::Mat::zeros(3, 1, CV_64F);

    // 카메라 2: 오른쪽으로 0.5m 이동 + 약간 회전
    double angle = 10.0 * CV_PI / 180.0;
    cv::Mat R2 = (cv::Mat_<double>(3, 3) <<
        cos(angle), 0, sin(angle),
        0, 1, 0,
        -sin(angle), 0, cos(angle));
    cv::Mat t2 = (cv::Mat_<double>(3, 1) << 0.5, 0, 0);

    // 투영 행렬 P = K * [R|t]
    cv::Mat Rt1, Rt2;
    cv::hconcat(R1, t1, Rt1);
    cv::hconcat(R2, t2, Rt2);
    cv::Mat P1 = K * Rt1;
    cv::Mat P2 = K * Rt2;

    // Ground Truth 3D 점
    cv::Point3d X_gt(0.3, -0.2, 5.0);

    // 두 카메라에 투영
    cv::Mat X_h = (cv::Mat_<double>(4, 1) << X_gt.x, X_gt.y, X_gt.z, 1.0);
    cv::Mat p1_h = P1 * X_h;
    cv::Mat p2_h = P2 * X_h;
    double u1 = p1_h.at<double>(0) / p1_h.at<double>(2);
    double v1 = p1_h.at<double>(1) / p1_h.at<double>(2);
    double u2 = p2_h.at<double>(0) / p2_h.at<double>(2);
    double v2 = p2_h.at<double>(1) / p2_h.at<double>(2);

    std::cout << "Ground Truth: [" << X_gt.x << ", " << X_gt.y << ", " << X_gt.z << "]" << std::endl;
    std::cout << "카메라 1 투영: (" << u1 << ", " << v1 << ")" << std::endl;
    std::cout << "카메라 2 투영: (" << u2 << ", " << v2 << ")\n" << std::endl;

    // DLT 삼각측량: A 행렬 구성
    // A = [u1*P1_3^T - P1_1^T]
    //     [v1*P1_3^T - P1_2^T]
    //     [u2*P2_3^T - P2_1^T]
    //     [v2*P2_3^T - P2_2^T]
    cv::Mat A(4, 4, CV_64F);
    for (int j = 0; j < 4; j++)
    {
        A.at<double>(0, j) = u1 * P1.at<double>(2, j) - P1.at<double>(0, j);
        A.at<double>(1, j) = v1 * P1.at<double>(2, j) - P1.at<double>(1, j);
        A.at<double>(2, j) = u2 * P2.at<double>(2, j) - P2.at<double>(0, j);
        A.at<double>(3, j) = v2 * P2.at<double>(2, j) - P2.at<double>(1, j);
    }

    // SVD → V의 마지막 열
    cv::Mat W, U, Vt;
    cv::SVD::compute(A, W, U, Vt);
    cv::Mat X_homo = Vt.row(Vt.rows - 1).t();

    // 동차 → 유클리드
    double x_est = X_homo.at<double>(0) / X_homo.at<double>(3);
    double y_est = X_homo.at<double>(1) / X_homo.at<double>(3);
    double z_est = X_homo.at<double>(2) / X_homo.at<double>(3);

    std::cout << "DLT 삼각측량 결과: [" << x_est << ", " << y_est << ", " << z_est << "]" << std::endl;

    double error = sqrt(pow(x_est - X_gt.x, 2) + pow(y_est - X_gt.y, 2) + pow(z_est - X_gt.z, 2));
    std::cout << "3D 오차: " << error << " m\n" << std::endl;

    // OpenCV triangulatePoints 비교
    cv::Mat pts1_mat = (cv::Mat_<double>(2, 1) << u1, v1);
    cv::Mat pts2_mat = (cv::Mat_<double>(2, 1) << u2, v2);
    cv::Mat pts4d;
    cv::triangulatePoints(P1, P2, pts1_mat, pts2_mat, pts4d);

    double x_cv = pts4d.at<double>(0) / pts4d.at<double>(3);
    double y_cv = pts4d.at<double>(1) / pts4d.at<double>(3);
    double z_cv = pts4d.at<double>(2) / pts4d.at<double>(3);

    std::cout << "OpenCV 결과:      [" << x_cv << ", " << y_cv << ", " << z_cv << "]" << std::endl;
    double error_cv = sqrt(pow(x_cv - X_gt.x, 2) + pow(y_cv - X_gt.y, 2) + pow(z_cv - X_gt.z, 2));
    std::cout << "OpenCV 오차:      " << error_cv << " m\n" << std::endl;

    // 베이스라인 크기별 정확도
    std::cout << "베이스라인 크기별 정확도:" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Baseline(m) | 3D Error(m)" << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    std::vector<double> baselines = {0.1, 0.5, 1.0, 2.0};
    for (double b : baselines)
    {
        cv::Mat t2_b = (cv::Mat_<double>(3, 1) << b, 0, 0);
        cv::Mat Rt2_b;
        cv::hconcat(R2, t2_b, Rt2_b);
        cv::Mat P2_b = K * Rt2_b;

        // 재투영
        cv::Mat p2_b_h = P2_b * X_h;
        double u2_b = p2_b_h.at<double>(0) / p2_b_h.at<double>(2);
        double v2_b = p2_b_h.at<double>(1) / p2_b_h.at<double>(2);

        // DLT
        cv::Mat A_b(4, 4, CV_64F);
        for (int j = 0; j < 4; j++)
        {
            A_b.at<double>(0, j) = u1 * P1.at<double>(2, j) - P1.at<double>(0, j);
            A_b.at<double>(1, j) = v1 * P1.at<double>(2, j) - P1.at<double>(1, j);
            A_b.at<double>(2, j) = u2_b * P2_b.at<double>(2, j) - P2_b.at<double>(0, j);
            A_b.at<double>(3, j) = v2_b * P2_b.at<double>(2, j) - P2_b.at<double>(1, j);
        }

        cv::Mat W_b, U_b, Vt_b;
        cv::SVD::compute(A_b, W_b, U_b, Vt_b);
        cv::Mat X_b = Vt_b.row(Vt_b.rows - 1).t();

        double x_b = X_b.at<double>(0) / X_b.at<double>(3);
        double y_b = X_b.at<double>(1) / X_b.at<double>(3);
        double z_b = X_b.at<double>(2) / X_b.at<double>(3);

        double err_b = sqrt(pow(x_b - X_gt.x, 2) + pow(y_b - X_gt.y, 2) + pow(z_b - X_gt.z, 2));
        std::cout << "   " << b << "        |  " << err_b << std::endl;
    }

    std::cout << "\n💡 정답 해설:" << std::endl;
    std::cout << "   [코드 핵심]" << std::endl;
    std::cout << "   1. A 행렬 구성: 각 뷰의 u,v 좌표에서 2개 방정식씩" << std::endl;
    std::cout << "   2. SVD(A) → V 마지막 열 = 3D 점 (동차 좌표)" << std::endl;
    std::cout << "   3. X[:3]/X[3] → 유클리드 좌표로 변환\n" << std::endl;
    std::cout << "   [베이스라인과 정확도 관계]" << std::endl;
    std::cout << "   노이즈 없음: 모든 베이스라인에서 오차 ≈ 0" << std::endl;
    std::cout << "   노이즈 있을 때:" << std::endl;
    std::cout << "   - 좁은 베이스라인(0.1m): 삼각형 꼭지각 작음 → 깊이 불확실성 ↑↑" << std::endl;
    std::cout << "   - 넓은 베이스라인(2.0m): 교차각 큼 → 깊이 정확도 ↑\n" << std::endl;
    std::cout << "   [실전 가이드]" << std::endl;
    std::cout << "   깊이 오차 ∝ Z² / (f × baseline)" << std::endl;
    std::cout << "   → 멀리 있는 점일수록, 베이스라인이 좁을수록 오차 급증" << std::endl;
    std::cout << "   → SLAM에서 삼각측량 시 충분한 카메라 이동(베이스라인) 필요" << std::endl;
}

// PnP DLT 직접 구현 — 투영 행렬 P를 SVD로 추정
//
// PnP DLT 알고리즘:
//   투영: λ·[u, v, 1]ᵀ = P · [X, Y, Z, 1]ᵀ
//   외적 소거로 각 대응점에서 2개 방정식:
//
//   행 2i:   [X Y Z 1  0 0 0 0  -uX -uY -uZ -u] · p = 0
//   행 2i+1: [0 0 0 0  X Y Z 1  -vX -vY -vZ -v] · p = 0
//
//   여기서 p = vec(P) (P의 12개 원소를 벡터로)
//   n개 점 → 2n×12 행렬 A 구성 → SVD → V의 마지막 열 = p
//   p를 3×4로 reshape → P
//
// P에서 [R|t] 분리:
//   [R|t] = K⁻¹ · P
//   R이 정확한 회전 행렬이 아닐 수 있음 → SVD로 직교화:
//     SVD(R_est) = U·S·Vᵀ → R = U·Vᵀ
//     det(R) < 0이면 부호 반전 (반사 방지)
//     t는 S의 평균 스케일로 나눠 정규화
//
// Outlier 실험:
//   - 20개 점 중 5개에 100px 노이즈 추가
//   - DLT는 모든 점에 동일 가중치 → outlier에 매우 취약
//   - ★ 실전에서는 반드시 RANSAC (cv::solvePnPRansac) 사용
void problem5_pnp_dlt()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: PnP DLT 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    // Ground truth 포즈
    cv::Mat rvec_gt = (cv::Mat_<double>(3, 1) << 0.1, 0.2, 0.05);
    cv::Mat R_gt;
    cv::Rodrigues(rvec_gt, R_gt);
    cv::Mat t_gt = (cv::Mat_<double>(3, 1) << 1.0, 0.5, 0.2);

    std::cout << "Ground Truth:" << std::endl;
    std::cout << "  rvec: " << rvec_gt.t() << std::endl;
    std::cout << "  t:    " << t_gt.t() << "\n" << std::endl;

    // 3D 점 생성 (20개)
    std::vector<cv::Point3f> pts3d;
    cv::RNG rng(42);
    for (int i = 0; i < 20; i++)
    {
        float x = rng.uniform(-2.0f, 2.0f);
        float y = rng.uniform(-2.0f, 2.0f);
        float z = rng.uniform(4.0f, 8.0f);
        pts3d.push_back(cv::Point3f(x, y, z));
    }

    // 2D 투영
    std::vector<cv::Point2f> pts2d;
    cv::projectPoints(pts3d, rvec_gt, t_gt, K, cv::Mat(), pts2d);

    // DLT PnP: A 행렬 구성 (2N x 12)
    int n = (int)pts3d.size();
    cv::Mat A_pnp(2 * n, 12, CV_64F, cv::Scalar(0));

    for (int i = 0; i < n; i++)
    {
        double X = pts3d[i].x, Y = pts3d[i].y, Z = pts3d[i].z;
        double u = pts2d[i].x, v = pts2d[i].y;

        // 행 2i: [X Y Z 1 0 0 0 0 -uX -uY -uZ -u]
        A_pnp.at<double>(2 * i, 0) = X;
        A_pnp.at<double>(2 * i, 1) = Y;
        A_pnp.at<double>(2 * i, 2) = Z;
        A_pnp.at<double>(2 * i, 3) = 1;
        A_pnp.at<double>(2 * i, 8) = -u * X;
        A_pnp.at<double>(2 * i, 9) = -u * Y;
        A_pnp.at<double>(2 * i, 10) = -u * Z;
        A_pnp.at<double>(2 * i, 11) = -u;

        // 행 2i+1: [0 0 0 0 X Y Z 1 -vX -vY -vZ -v]
        A_pnp.at<double>(2 * i + 1, 4) = X;
        A_pnp.at<double>(2 * i + 1, 5) = Y;
        A_pnp.at<double>(2 * i + 1, 6) = Z;
        A_pnp.at<double>(2 * i + 1, 7) = 1;
        A_pnp.at<double>(2 * i + 1, 8) = -v * X;
        A_pnp.at<double>(2 * i + 1, 9) = -v * Y;
        A_pnp.at<double>(2 * i + 1, 10) = -v * Z;
        A_pnp.at<double>(2 * i + 1, 11) = -v;
    }

    // SVD → P (3x4)
    cv::Mat W_pnp, U_pnp, Vt_pnp;
    cv::SVD::compute(A_pnp, W_pnp, U_pnp, Vt_pnp);
    cv::Mat P_vec = Vt_pnp.row(Vt_pnp.rows - 1);
    cv::Mat P = P_vec.reshape(1, 3);  // 3x4

    // K^-1 * P = [R|t]
    cv::Mat K_inv = K.inv();
    cv::Mat M = K_inv * P;

    cv::Mat R_est = M(cv::Rect(0, 0, 3, 3)).clone();
    cv::Mat t_est = M.col(3).clone();

    // R 직교화 (SVD)
    cv::Mat W_r, U_r, Vt_r;
    cv::SVD::compute(R_est, W_r, U_r, Vt_r);
    R_est = U_r * Vt_r;

    // det(R) < 0이면 부호 반전
    if (cv::determinant(R_est) < 0)
    {
        R_est = -R_est;
        t_est = -t_est;
    }

    // 스케일 조정
    double scale = (W_r.at<double>(0) + W_r.at<double>(1) + W_r.at<double>(2)) / 3.0;
    t_est = t_est / scale;

    // DLT 결과 출력
    cv::Mat rvec_est;
    cv::Rodrigues(R_est, rvec_est);

    std::cout << "DLT PnP 결과:" << std::endl;
    std::cout << "  rvec: " << rvec_est.t() << std::endl;
    std::cout << "  t:    " << t_est.t() << std::endl;

    double r_error = cv::norm(rvec_est - rvec_gt);
    double t_error = cv::norm(t_est - t_gt);
    std::cout << "  R 오차: " << r_error << std::endl;
    std::cout << "  t 오차: " << t_error << "\n" << std::endl;

    // OpenCV solvePnP 비교
    cv::Mat rvec_cv, tvec_cv;
    cv::solvePnP(pts3d, pts2d, K, cv::Mat(), rvec_cv, tvec_cv);

    std::cout << "OpenCV solvePnP 결과:" << std::endl;
    std::cout << "  rvec: " << rvec_cv.t() << std::endl;
    std::cout << "  t:    " << tvec_cv.t() << std::endl;
    std::cout << "  R 오차: " << cv::norm(rvec_cv - rvec_gt) << std::endl;
    std::cout << "  t 오차: " << cv::norm(tvec_cv - t_gt) << "\n" << std::endl;

    // Outlier 추가 시 정확도
    std::cout << "Outlier 추가 시 DLT 정확도:" << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    std::vector<cv::Point2f> pts2d_outlier = pts2d;
    // 5개 점에 큰 노이즈 추가
    for (int i = 0; i < 5; i++)
    {
        pts2d_outlier[i].x += rng.uniform(-100.0f, 100.0f);
        pts2d_outlier[i].y += rng.uniform(-100.0f, 100.0f);
    }

    // DLT with outliers (A 행렬 재구성)
    cv::Mat A_out(2 * n, 12, CV_64F, cv::Scalar(0));
    for (int i = 0; i < n; i++)
    {
        double X = pts3d[i].x, Y = pts3d[i].y, Z = pts3d[i].z;
        double u = pts2d_outlier[i].x, v = pts2d_outlier[i].y;

        A_out.at<double>(2 * i, 0) = X;
        A_out.at<double>(2 * i, 1) = Y;
        A_out.at<double>(2 * i, 2) = Z;
        A_out.at<double>(2 * i, 3) = 1;
        A_out.at<double>(2 * i, 8) = -u * X;
        A_out.at<double>(2 * i, 9) = -u * Y;
        A_out.at<double>(2 * i, 10) = -u * Z;
        A_out.at<double>(2 * i, 11) = -u;

        A_out.at<double>(2 * i + 1, 4) = X;
        A_out.at<double>(2 * i + 1, 5) = Y;
        A_out.at<double>(2 * i + 1, 6) = Z;
        A_out.at<double>(2 * i + 1, 7) = 1;
        A_out.at<double>(2 * i + 1, 8) = -v * X;
        A_out.at<double>(2 * i + 1, 9) = -v * Y;
        A_out.at<double>(2 * i + 1, 10) = -v * Z;
        A_out.at<double>(2 * i + 1, 11) = -v;
    }

    cv::Mat W_out, U_out, Vt_out;
    cv::SVD::compute(A_out, W_out, U_out, Vt_out);
    cv::Mat P_out_vec = Vt_out.row(Vt_out.rows - 1);
    cv::Mat P_out = P_out_vec.reshape(1, 3);
    cv::Mat M_out = K_inv * P_out;

    cv::Mat R_out = M_out(cv::Rect(0, 0, 3, 3)).clone();
    cv::Mat t_out_est = M_out.col(3).clone();

    cv::SVD::compute(R_out, W_r, U_r, Vt_r);
    R_out = U_r * Vt_r;
    if (cv::determinant(R_out) < 0)
    {
        R_out = -R_out;
        t_out_est = -t_out_est;
    }
    double scale_out = (W_r.at<double>(0) + W_r.at<double>(1) + W_r.at<double>(2)) / 3.0;
    t_out_est = t_out_est / scale_out;

    cv::Mat rvec_out;
    cv::Rodrigues(R_out, rvec_out);

    std::cout << "  정상 데이터 - R 오차: " << r_error << ", t 오차: " << t_error << std::endl;
    std::cout << "  Outlier 포함 - R 오차: " << cv::norm(rvec_out - rvec_gt)
              << ", t 오차: " << cv::norm(t_out_est - t_gt) << std::endl;

    std::cout << "\n💡 정답 해설:" << std::endl;
    std::cout << "   [PnP DLT 코드 핵심]" << std::endl;
    std::cout << "   1. 2N×12 A행렬: 각 점에서 2개 방정식 (u, v 제약)" << std::endl;
    std::cout << "   2. SVD → V의 마지막 열 = P의 12개 원소" << std::endl;
    std::cout << "   3. reshape(3,4) → 투영 행렬 P" << std::endl;
    std::cout << "   4. K⁻¹·P = [R|t] → R 직교화(SVD) + det 체크\n" << std::endl;
    std::cout << "   [왜 DLT는 outlier에 취약한가?]" << std::endl;
    std::cout << "   DLT = 모든 점에 동일 가중치로 최소자승 풀이" << std::endl;
    std::cout << "   → outlier(100px 오차)도 inlier(0px 오차)와 동등하게 취급" << std::endl;
    std::cout << "   → 20개 중 5개(25%) outlier만으로도 결과 완전히 왜곡\n" << std::endl;
    std::cout << "   [해결책: RANSAC]" << std::endl;
    std::cout << "   solvePnPRansac(): 랜덤 샘플로 가설 생성 → inlier만 사용" << std::endl;
    std::cout << "   → outlier 25%여도 안정적으로 정확한 포즈 추정 가능" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 7 Quiz - Medium (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_implement_pnp();
    problem2_ransac_iterations();
    problem3_pose_optimization();
    problem4_dlt_triangulation();
    problem5_pnp_dlt();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
