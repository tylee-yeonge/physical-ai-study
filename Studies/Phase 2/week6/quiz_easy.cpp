/**
 * Phase 2 Week 6 - 삼각측량 기초 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

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
    // Depth = (baseline * focal) / disparity

    double disparity1 = 60.0;  // 픽셀
    double depth1 = 0.0;       // TODO: 계산하기

    double disparity2 = 30.0;
    double depth2 = 0.0;  // TODO: 계산하기

    std::cout << "시차 " << disparity1 << " 픽셀 → 깊이 ??? m" << std::endl;
    std::cout << "시차 " << disparity2 << " 픽셀 → 깊이 ??? m\n" << std::endl;

    std::cout << "💡 관찰:" << std::endl;
    std::cout << "   - 시차 ↑ → 깊이 ↓ (가까움)" << std::endl;
    std::cout << "   - 시차 ↓ → 깊이 ↑ (멀리)" << std::endl;
    std::cout << "   - 시차 = 0 → 무한대 (매칭 불가)" << std::endl;
}

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

void problem3_reprojection_error()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 재투영 오차" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 3D 점
    cv::Point3f pt3d(1.0, 0.5, 3.0);

    // 카메라 행렬
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);

    // TODO: 투영
    // p = K * [X, Y, Z]^T
    // u = p_x / p_z, v = p_y / p_z

    std::cout << "3D 점: (" << pt3d.x << ", " << pt3d.y << ", " << pt3d.z << ")" << std::endl;
    std::cout << "투영된 2D 점: (???, ???)\n" << std::endl;

    std::cout << "💡 재투영 오차 = ||관측 - 투영|| (픽셀)" << std::endl;
    std::cout << "   - 1 픽셀 이하: 매우 좋음" << std::endl;
    std::cout << "   - 5 픽셀 이상: 문제 있음 (outlier)" << std::endl;
}

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
 * 주어진 행렬이 유효한 회전 행렬인지 두 가지 조건으로 판별한다:
 * 1. 직교성: R^T * R = I
 * 2. 행렬식: det(R) = 1
 *
 * 유효하지 않은 R을 SVD로 가장 가까운 회전 행렬로 복구한다.
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

    // TODO 1: R_valid와 R_noisy 각각에 대해 검증
    //   조건 1: R^T * R ≈ I (직교성)
    //     힌트: R의 전치와 R을 곱한 결과가 단위행렬에 얼마나 가까운지 측정하세요
    //
    //   조건 2: det(R) ≈ 1
    //     힌트: 행렬식을 계산하여 1에 가까운지 확인하세요

    // TODO 2: R_noisy를 SVD로 가장 가까운 회전 행렬로 복구
    //   힌트: SVD 분해 후 U와 V^T를 곱하면 가장 가까운 직교 행렬을 얻습니다
    //         행렬식이 -1이면 반사 행렬이므로 부호를 조정하세요

    std::cout << "\n올바른 회전 행렬 조건:" << std::endl;
    std::cout << "   1. R^T * R = I (직교성)" << std::endl;
    std::cout << "   2. det(R) = 1 (반사가 아닌 회전)" << std::endl;
    std::cout << "   SVD 복구: R_fixed = U * Vt (가장 가까운 직교 행렬)" << std::endl;
}

/**
 * @brief 스케일 모호성 이해
 *
 * t를 2배, 0.5배로 바꿔도 E = [t]_x R이 같은 방향인지 확인한다.
 * 단안 SLAM에서 절대 스케일을 알 수 없는 이유를 설명한다.
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
    // [t]_x (skew-symmetric) 구하는 법:
    //   tx, ty, tz = t의 원소
    //   t_skew = [0, -tz, ty; tz, 0, -tx; -ty, tx, 0]
    //
    // E = t_skew * R
    // E_norm = E / cv::norm(E)
    //
    // 모든 스케일에서 E_norm이 같은지 확인!

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
