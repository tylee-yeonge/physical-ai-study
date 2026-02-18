/**
 * Phase 2 Week 6 - 삼각측량 중급 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

void problem1_implement_triangulation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 삼각측량 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);

    // 두 번째 카메라 위치
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);             // 회전 없음
    cv::Mat t = (cv::Mat_<double>(3, 1) << 0.1, 0, 0);  // 10cm 이동

    // 투영 행렬
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));

    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R.copyTo(RT(cv::Rect(0, 0, 3, 3)));
    t.copyTo(RT(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT;

    // TODO: 대응점
    std::vector<cv::Point2f> pts1 = {cv::Point2f(400, 300)};
    std::vector<cv::Point2f> pts2 = {cv::Point2f(460, 300)};  // 시차 60픽셀

    // TODO: triangulatePoints 사용
    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, points4D);

    // 동차 좌표 → 3D
    float w = points4D.at<float>(3, 0);
    cv::Point3f pt3d(points4D.at<float>(0, 0) / w, points4D.at<float>(1, 0) / w,
                     points4D.at<float>(2, 0) / w);

    std::cout << "복원된 3D 점: (" << pt3d.x << ", " << pt3d.y << ", " << pt3d.z << ")"
              << std::endl;
    std::cout << "\n💡 시차 60픽셀 → 깊이 약 1m" << std::endl;
}

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
 * E = U * diag(1,1,0) * V^T 에서 W 행렬을 이용해 4가지 해를 도출하고,
 * 삼각측량 + Cheirality Check로 유효한 해 1개를 선택한다.
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
    t_gt = t_gt / cv::norm(t_gt);  // 단위 벡터

    // E = [t]_x * R 계산
    double tx = t_gt.at<double>(0), ty = t_gt.at<double>(1), tz = t_gt.at<double>(2);
    cv::Mat t_skew = (cv::Mat_<double>(3, 3) <<
        0, -tz, ty,
        tz, 0, -tx,
        -ty, tx, 0);
    cv::Mat E = t_skew * R_gt;

    std::cout << "Essential Matrix E:" << std::endl << E << std::endl;

    // TODO 1: E를 SVD 분해
    //   cv::SVD::compute(E, w, u, vt)

    // TODO 2: W 행렬 정의
    //   W = [0 -1 0; 1 0 0; 0 0 1]

    // TODO 3: 4가지 해 계산
    //   R1 = U * W^T * Vt, R2 = U * W * Vt
    //   t1 = +U.col(2),    t2 = -U.col(2)
    //   det(R) < 0 이면 R = -R

    // TODO 4: 3D 점 생성 → 투영 → 삼각측량 → Cheirality Check
    //   각 해에 대해 삼각측량 후 두 카메라 모두에서 Z > 0인 점의 비율 계산
    //   가장 높은 비율의 해가 올바른 해

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
 * 순수 회전(t=0)일 때 E가 0이 되어 분해 불가능한 경우와,
 * 매우 작은 이동일 때 노이즈에 민감해지는 경우를 실험한다.
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

    // Case 1: 순수 회전 (t = 0)
    std::cout << "[Case 1] 순수 회전 (t = 0):" << std::endl;
    cv::Mat t_zero = cv::Mat::zeros(3, 1, CV_64F);
    cv::Mat t_skew_zero = (cv::Mat_<double>(3, 3) <<
        0, 0, 0, 0, 0, 0, 0, 0, 0);
    cv::Mat E_zero = t_skew_zero * R;

    std::cout << "   E = " << std::endl << E_zero << std::endl;
    std::cout << "   E가 0 행렬 → SVD 분해 의미 없음!" << std::endl;

    // Case 2: 매우 작은 이동
    std::cout << "\n[Case 2] 매우 작은 이동 (t ≈ 0):" << std::endl;
    cv::Mat t_small = (cv::Mat_<double>(3, 1) << 1e-6, 0, 0);
    double tsx = t_small.at<double>(0);
    cv::Mat t_skew_small = (cv::Mat_<double>(3, 3) <<
        0, 0, 0,
        0, 0, -tsx,
        0, tsx, 0);
    cv::Mat E_small = t_skew_small * R;

    // TODO: E_small을 SVD 분해하고 특이값 확인
    //   특이값이 매우 작으면 → 수치적으로 불안정

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
    std::cout << "Phase 2 Week 6 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_implement_triangulation();
    problem2_optimize_reconstruction();
    problem3_stereo_matching();
    problem4_decompose_essential();
    problem5_failure_cases();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
