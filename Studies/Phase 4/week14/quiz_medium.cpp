/**
 * Phase 4 Week 14 - Kalibr 실습 중급 퀴즈
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace Eigen;
using namespace std;

Matrix3d expSO3(const Vector3d& omega)
{
    double angle = omega.norm();
    if (angle < 1e-10)
        return Matrix3d::Identity();
    Vector3d axis = omega / angle;
    Matrix3d K;
    K << 0, -axis.z(), axis.y(), axis.z(), 0, -axis.x(), -axis.y(), axis.x(), 0;
    return Matrix3d::Identity() + sin(angle) * K + (1.0 - cos(angle)) * K * K;
}

void problem1_validate_rotation()
{
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 1: 회전 행렬 검증" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "Kalibr에서 다음 T_cam_imu가 출력되었습니다:\n" << endl;

    Matrix4d T_cam_imu;
    T_cam_imu << 0.0148655, -0.999881, 0.00414029, -0.0216401, 0.999557, 0.0149672, 0.0257155,
        -0.064677, -0.0257744, 0.00375619, 0.999661, 0.00981073, 0.0, 0.0, 0.0, 1.0;

    cout << "  T_cam_imu:" << endl;
    cout << T_cam_imu << "\n" << endl;

    cout << "과제:" << endl;
    cout << "  1. R_ci (3x3 회전) 과 t_ci (3x1 병진)를 추출하세요.\n" << endl;
    cout << "  2. R_ci가 유효한 회전 행렬인지 검증하세요:" << endl;
    cout << "     - det(R) = 1?" << endl;
    cout << "     - R * R^T = I?\n" << endl;
    cout << "  3. 회전을 angle-axis로 변환하세요:" << endl;
    cout << "     - 회전 각도는 몇 도?" << endl;
    cout << "     - 회전 축은?\n" << endl;
    cout << "  4. t_ci의 크기 (||t||)를 구하세요." << endl;
    cout << "     → 카메라와 IMU가 약 몇 cm 떨어져 있는가?\n" << endl;

    // TODO: 학생이 구현
    cout << "  R_ci:" << endl;
    cout << "    (3x3 출력)" << endl;
    cout << "  t_ci: [_____, _____, _____]" << endl;
    cout << "  det(R) = _____" << endl;
    cout << "  ||R*R^T - I|| = _____" << endl;
    cout << "  회전 각도: _____ 도" << endl;
    cout << "  ||t|| = _____ m = _____ cm\n" << endl;

    cout << "  힌트: 이 값은 EuRoC MAV 데이터셋의 실제 캘리브레이션 결과입니다." << endl;
    cout << "  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

void problem2_reprojection_analysis()
{
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 2: Reprojection Error 분석" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "캘리브레이션 결과에서 15개 점의 reprojection error:\n" << endl;

    vector<double> errors = {0.23, 0.45, 0.31, 0.18, 0.52, 0.67, 0.29, 0.41,
                             0.35, 0.48, 1.23, 0.38, 0.27, 0.33, 0.42};

    cout << "  점 │ error (pixel)" << endl;
    cout << "  ───┼───────────────" << endl;
    for (int i = 0; i < 15; i++)
    {
        printf("  %2d │   %5.2f\n", i + 1, errors[i]);
    }

    cout << "\n과제:" << endl;
    cout << "  1. 평균, 중앙값, 표준편차, 최대값을 구하세요.\n" << endl;
    cout << "  2. 이상치(outlier)가 있는가? 어느 점?\n" << endl;
    cout << "  3. 이 결과는 '좋음/보통/나쁨' 중 어디에 해당하는가?\n" << endl;
    cout << "  4. 이상치를 제거하면 평균이 어떻게 변하는가?\n" << endl;

    // TODO: 학생이 구현
    cout << "  평균: _____ pixel" << endl;
    cout << "  중앙값: _____ pixel" << endl;
    cout << "  표준편차: _____ pixel" << endl;
    cout << "  최대값: _____ pixel (점 번호: __)" << endl;
    cout << "  이상치 제거 후 평균: _____ pixel\n" << endl;
    cout << "  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

void problem3_extrinsic_to_vins()
{
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 3: Kalibr → VINS 변환" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "Kalibr 결과: T_cam_imu (Camera ← IMU)" << endl;
    cout << "VINS 필요: T_imu_cam (IMU ← Camera)\n" << endl;

    Matrix3d R_ci = expSO3(Vector3d(0.02, -0.01, 1.57));
    Vector3d t_ci(0.05, -0.02, 0.01);

    cout << "  Kalibr 출력:" << endl;
    cout << "    R_ci (Camera ← IMU):" << endl;
    cout << R_ci << endl;
    cout << "    t_ci = [" << t_ci.transpose() << "]\n" << endl;

    cout << "과제:" << endl;
    cout << "  1. T_imu_cam = T_cam_imu의 역변환을 구하세요." << endl;
    cout << "     R_ic = R_ci^T" << endl;
    cout << "     t_ic = -R_ci^T * t_ci\n" << endl;
    cout << "  2. 결과를 VINS config 형식으로 작성하세요:" << endl;
    cout << "     extrinsicRotation: R_ic" << endl;
    cout << "     extrinsicTranslation: t_ic\n" << endl;

    // TODO: 학생이 구현
    cout << "  R_ic:" << endl;
    cout << "    (3x3 출력)" << endl;
    cout << "  t_ic: [_____, _____, _____]\n" << endl;

    cout << "  검증: R_ci * R_ic = I? R_ci * t_ic + t_ci = 0?" << endl;
    cout << "\n  정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
}

int main()
{
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 14 Quiz - Medium" << endl;
    cout << "Kalibr 실습" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    problem1_validate_rotation();
    problem2_reprojection_analysis();
    problem3_extrinsic_to_vins();

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
