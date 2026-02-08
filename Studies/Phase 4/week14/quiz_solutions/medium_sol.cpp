/**
 * Phase 4 Week 14 - Kalibr 실습 중급 퀴즈 풀이
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace Eigen;
using namespace std;

Matrix3d expSO3(const Vector3d& omega) {
    double angle = omega.norm();
    if (angle < 1e-10) return Matrix3d::Identity();
    Vector3d axis = omega / angle;
    Matrix3d K;
    K <<    0, -axis.z(),  axis.y(),
         axis.z(),     0, -axis.x(),
        -axis.y(),  axis.x(),     0;
    return Matrix3d::Identity()
         + sin(angle) * K + (1.0 - cos(angle)) * K * K;
}

void problem1_solution() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 1 풀이: 회전 행렬 검증" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    Matrix4d T_cam_imu;
    T_cam_imu << 0.0148655, -0.999881, 0.00414029, -0.0216401,
                 0.999557,  0.0149672, 0.0257155,  -0.064677,
                -0.0257744, 0.00375619, 0.999661,   0.00981073,
                 0.0,       0.0,        0.0,        1.0;

    cout << "  Step 1: R_ci, t_ci 추출\n" << endl;
    Matrix3d R_ci = T_cam_imu.block<3,3>(0,0);
    Vector3d t_ci = T_cam_imu.block<3,1>(0,3);

    cout << "    R_ci:" << endl;
    cout << R_ci << "\n" << endl;
    cout << "    t_ci = [" << t_ci.transpose() << "]\n" << endl;

    cout << "  Step 2: 회전 행렬 검증\n" << endl;
    double det = R_ci.determinant();
    double ortho = (R_ci * R_ci.transpose() - Matrix3d::Identity()).norm();

    cout << "    det(R) = " << det << endl;
    cout << "    (기대: 1.0, 오차: " << abs(det - 1.0) << ")" << endl;
    cout << "    ||R*R^T - I|| = " << ortho << endl;
    cout << "    (기대: ≈0)" << endl;
    cout << "    → 유효한 회전 행렬 ✓\n" << endl;

    cout << "  Step 3: Angle-axis 변환\n" << endl;
    AngleAxisd aa(R_ci);
    cout << "    회전 각도: " << aa.angle() * 180.0 / M_PI << " 도" << endl;
    cout << "    회전 축: [" << aa.axis().transpose() << "]\n" << endl;
    cout << "    해석: 카메라가 IMU 대비 약 "
         << fixed << setprecision(1) << aa.angle() * 180.0 / M_PI
         << "도 회전" << endl;
    cout << "    (EuRoC: 카메라가 전방, IMU가 상부 → ~90도 회전)\n" << endl;

    cout << "  Step 4: 병진 크기\n" << endl;
    cout << "    ||t_ci|| = " << t_ci.norm() << " m" << endl;
    cout << "    = " << t_ci.norm() * 100 << " cm" << endl;
    cout << "    → 카메라와 IMU가 약 "
         << fixed << setprecision(1) << t_ci.norm() * 100
         << "cm 떨어져 있음" << endl;
    cout << "    (EuRoC MAV의 실제 센서 배치와 일치)" << endl;
}

void problem2_solution() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 2 풀이: Reprojection Error 분석" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    vector<double> errors = {
        0.23, 0.45, 0.31, 0.18, 0.52,
        0.67, 0.29, 0.41, 0.35, 0.48,
        1.23, 0.38, 0.27, 0.33, 0.42
    };
    int n = errors.size();

    cout << "  Step 1: 기본 통계\n" << endl;

    // 평균
    double mean = accumulate(errors.begin(), errors.end(), 0.0) / n;

    // 정렬 및 중앙값
    vector<double> sorted = errors;
    sort(sorted.begin(), sorted.end());
    double median = sorted[n/2];

    // 표준편차
    double var = 0;
    for (double e : errors) var += (e - mean) * (e - mean);
    var /= (n - 1);
    double std_dev = sqrt(var);

    // 최대값
    int max_idx = max_element(errors.begin(), errors.end()) - errors.begin();
    double max_err = errors[max_idx];

    printf("    평균:     %.4f pixel\n", mean);
    printf("    중앙값:   %.4f pixel\n", median);
    printf("    표준편차: %.4f pixel\n", std_dev);
    printf("    최대값:   %.4f pixel (점 %d)\n\n", max_err, max_idx + 1);

    cout << "  Step 2: 이상치 분석\n" << endl;
    double threshold = mean + 2 * std_dev;
    cout << "    이상치 기준: mean + 2σ = " << mean << " + 2×" << std_dev
         << " = " << threshold << endl;
    cout << "    점 11 (1.23) > " << threshold << " → 이상치!\n" << endl;

    cout << "  Step 3: 결과 판정\n" << endl;
    if (mean < 0.3) cout << "    평균 " << mean << " → 우수" << endl;
    else if (mean < 0.5) cout << "    평균 " << mean << " → 양호" << endl;
    else if (mean < 1.0) cout << "    평균 " << mean << " → 보통" << endl;
    else cout << "    평균 " << mean << " → 불량" << endl;
    cout << "    (이상치 포함 시 전체 판정에 주의)\n" << endl;

    cout << "  Step 4: 이상치 제거 후\n" << endl;
    double sum_clean = 0;
    int count_clean = 0;
    for (int i = 0; i < n; i++) {
        if (errors[i] <= threshold) {
            sum_clean += errors[i];
            count_clean++;
        }
    }
    double mean_clean = sum_clean / count_clean;
    printf("    이상치 제거 후 평균: %.4f pixel (%d개)\n", mean_clean, count_clean);
    cout << "    → 이상치 제거 시 양호한 결과\n" << endl;

    cout << "  결론:" << endl;
    cout << "    → 전체적으로 양호하나 점 11은 문제" << endl;
    cout << "    → 해당 프레임의 타겟 검출 확인 필요" << endl;
    cout << "    → 모션 블러 또는 타겟 부분 가림 가능성" << endl;
}

void problem3_solution() {
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 3 풀이: Kalibr → VINS 변환" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    Matrix3d R_ci = expSO3(Vector3d(0.02, -0.01, 1.57));
    Vector3d t_ci(0.05, -0.02, 0.01);

    cout << "  Kalibr 출력 (Camera ← IMU):" << endl;
    cout << "    R_ci:" << endl;
    cout << R_ci << endl;
    cout << "    t_ci = [" << t_ci.transpose() << "]\n" << endl;

    cout << "  Step 1: 역변환 계산\n" << endl;
    Matrix3d R_ic = R_ci.transpose();
    Vector3d t_ic = -R_ci.transpose() * t_ci;

    cout << "    R_ic = R_ci^T:" << endl;
    cout << R_ic << endl;
    cout << "    t_ic = -R_ci^T * t_ci:" << endl;
    cout << "         = [" << t_ic.transpose() << "]\n" << endl;

    cout << "  Step 2: 검증\n" << endl;
    Matrix3d should_be_I = R_ci * R_ic;
    Vector3d should_be_0 = R_ci * t_ic + t_ci;
    cout << "    R_ci * R_ic ≈ I?" << endl;
    cout << "    ||R_ci * R_ic - I|| = "
         << (should_be_I - Matrix3d::Identity()).norm() << endl;
    cout << "    R_ci * t_ic + t_ci ≈ 0?" << endl;
    cout << "    ||R_ci * t_ic + t_ci|| = " << should_be_0.norm() << "\n" << endl;

    cout << "  Step 3: VINS config 형식\n" << endl;
    cout << "    extrinsicRotation: !!opencv-matrix" << endl;
    cout << "       rows: 3" << endl;
    cout << "       cols: 3" << endl;
    cout << "       data: [";
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%.6f", R_ic(i,j));
            if (i * 3 + j < 8) cout << ", ";
        }
    }
    cout << "]" << endl;

    cout << "    extrinsicTranslation: !!opencv-matrix" << endl;
    cout << "       rows: 3" << endl;
    cout << "       cols: 1" << endl;
    printf("       data: [%.6f, %.6f, %.6f]\n\n",
           t_ic(0), t_ic(1), t_ic(2));

    cout << "  주의:" << endl;
    cout << "    → Kalibr는 T_cam_imu (Camera ← IMU)를 출력" << endl;
    cout << "    → VINS는 보통 T_imu_cam (IMU ← Camera)을 사용" << endl;
    cout << "    → 변환 방향을 반드시 확인!" << endl;
    cout << "    → 잘못된 변환 → VIO 완전히 발산" << endl;
}

int main() {
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 14 Quiz Medium - 풀이" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();

    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    return 0;
}
