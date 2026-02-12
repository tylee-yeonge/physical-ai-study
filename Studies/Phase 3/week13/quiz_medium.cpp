/**
 * Quiz Medium - Week 13: 스케일 복구 방법
 *
 * Eigen을 사용한 계산 문제
 */

#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <iomanip>

using namespace Eigen;
using namespace std;

void problem1_stereo_depth_calculation()
{
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 1: Stereo Depth 계산" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "Stereo 카메라의 파라미터:" << endl;
    cout << "  초점 거리 f = 500 pixel" << endl;
    cout << "  베이스라인 b = 0.12 m" << endl;
    cout << endl;

    // 주어진 데이터: 5개 특징점의 좌우 카메라 x좌표
    Eigen::VectorXd left_x(5);   // 좌 카메라 x좌표
    Eigen::VectorXd right_x(5);  // 우 카메라 x좌표

    left_x << 320.0, 250.0, 400.0, 310.0, 350.0;
    right_x << 310.0, 220.0, 395.0, 280.0, 348.0;

    double f = 500.0;  // 초점 거리
    double b = 0.12;   // 베이스라인

    cout << "5개 특징점의 좌우 카메라 x좌표:" << endl;
    cout << "  점 │ 좌 카메라 (pixel) │ 우 카메라 (pixel)" << endl;
    cout << "  ───┼──────────────────┼──────────────────" << endl;
    for (int i = 0; i < 5; i++)
    {
        printf("   %d  │      %6.1f       │      %6.1f\n", i + 1, left_x(i), right_x(i));
    }
    cout << endl;

    cout << "과제: 각 점의 disparity와 depth를 계산하세요.\n" << endl;
    cout << "  disparity = left_x - right_x" << endl;
    cout << "  depth = f * b / disparity\n" << endl;

    // TODO: 여기에 코드를 작성하세요
    // Eigen::VectorXd disparity = ???;
    // Eigen::VectorXd depth = ???;

    cout << "  점 │ Disparity (pixel) │ Depth (m)" << endl;
    cout << "  ───┼──────────────────┼──────────" << endl;

    // TODO: 결과를 출력하세요
    for (int i = 0; i < 5; i++)
    {
        cout << "   " << i + 1 << "  │      _____        │   _____" << endl;
    }

    cout << "\n  💡 힌트: disparity가 작은 점일수록 멀리 있습니다.\n" << endl;
    cout << "  추가 질문: disparity = 0이면 무슨 일이 일어나나요?\n" << endl;
}

void problem2_imu_integration()
{
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 2: IMU 가속도 적분" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "로봇이 x축 방향으로 이동합니다." << endl;
    cout << "IMU에서 측정된 가속도 데이터 (10 스텝, dt = 0.1초):\n" << endl;

    double dt = 0.1;  // 시간 간격 (초)

    // IMU 가속도 측정값 (x축만, 단순화)
    Eigen::VectorXd accel(10);
    accel << 1.0, 1.0, 0.5, 0.0, 0.0, -0.5, -1.0, 0.0, 0.5, 0.0;

    cout << "  스텝 │ 시간 (초) │ 가속도 (m/s²)" << endl;
    cout << "  ─────┼──────────┼──────────────" << endl;
    for (int i = 0; i < 10; i++)
    {
        printf("    %2d  │   %4.1f    │    %5.2f\n", i + 1, (i + 1) * dt, accel(i));
    }

    cout << "\n과제: 가속도를 적분하여 각 스텝의 속도와 위치를 구하세요." << endl;
    cout << "       초기 조건: v(0) = 0 m/s, p(0) = 0 m\n" << endl;

    cout << "  적분 공식:" << endl;
    cout << "    v(k+1) = v(k) + a(k) * dt" << endl;
    cout << "    p(k+1) = p(k) + v(k) * dt + 0.5 * a(k) * dt²\n" << endl;

    // TODO: 여기에 코드를 작성하세요
    // 1. velocity(11), position(11) 벡터를 선언하고 초기값 0으로 설정
    // 2. 10 스텝 반복하며 위 적분 공식으로 속도와 위치를 누적 계산

    cout << "  스텝 │ 시간 (초) │ 속도 (m/s) │ 위치 (m)" << endl;
    cout << "  ─────┼──────────┼────────────┼─────────" << endl;
    cout << "    0   │    0.0    │    0.000   │   0.000" << endl;
    for (int i = 0; i < 10; i++)
    {
        cout << "   " << setw(2) << i + 1 << "   │   " << fixed << setprecision(1) << (i + 1) * dt
             << "    │    _____   │   _____" << endl;
    }

    cout << "\n  💡 힌트: Eigen의 벡터 연산을 활용하세요.\n" << endl;
    cout << "  추가 질문: 가속도에 바이어스 0.01 m/s²가 추가되면\n";
    cout << "  최종 위치가 얼마나 달라지나요?\n" << endl;
}

void problem3_sensor_comparison()
{
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "문제 3: 센서 융합 비교 분석" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    cout << "3가지 방법으로 10m 직선 경로를 추정한 결과입니다.\n" << endl;

    // 실제 경로: 10m 직선 (x축)
    double true_distance = 10.0;

    // 각 방법의 추정 결과 (시뮬레이션 데이터)
    Eigen::Vector3d mono_estimate(12.5, 0.3, -0.1);     // Monocular: 스케일 드리프트
    Eigen::Vector3d stereo_estimate(10.1, 0.05, 0.02);  // Stereo: 안정적
    Eigen::Vector3d vio_estimate(10.3, 0.1, -0.05);     // VIO: 양호

    cout << "  실제 경로: (10.0, 0.0, 0.0) m" << endl;
    cout << "  Monocular 추정: (" << mono_estimate.transpose() << ") m" << endl;
    cout << "  Stereo 추정:    (" << stereo_estimate.transpose() << ") m" << endl;
    cout << "  VIO 추정:       (" << vio_estimate.transpose() << ") m\n" << endl;

    cout << "과제:" << endl;
    cout << "  1. 각 방법의 절대 위치 오차 (Euclidean distance)를 계산하세요." << endl;
    cout << "  2. 각 방법의 스케일 오차를 백분율로 계산하세요." << endl;
    cout << "  3. 어떤 방법이 가장 정확한지, 그 이유를 설명하세요.\n" << endl;

    // TODO: 여기에 코드를 작성하세요
    // Eigen::Vector3d true_position(10.0, 0.0, 0.0);
    // double mono_error = (mono_estimate - true_position).norm();
    // double mono_scale_error = ???;
    // ...

    cout << "  방법       │ 절대 오차 (m) │ 스케일 오차 (%)" << endl;
    cout << "  ───────────┼──────────────┼────────────────" << endl;
    cout << "  Monocular  │    _____     │     _____" << endl;
    cout << "  Stereo     │    _____     │     _____" << endl;
    cout << "  VIO        │    _____     │     _____" << endl;

    cout << "\n  💡 힌트:" << endl;
    cout << "  - 절대 오차 = ||estimate - true||" << endl;
    cout << "  - 스케일 오차 = |estimate_x - true_x| / true_x * 100%\n" << endl;

    cout << "  서술형: Monocular의 오차가 가장 큰 이유를 Week 12에서 배운\n";
    cout << "  스케일 모호성과 연결하여 설명하세요.\n" << endl;
}

int main()
{
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "Week 13 Quiz - Medium" << endl;
    cout << "스케일 복구 방법" << endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << endl;

    problem1_stereo_depth_calculation();
    problem2_imu_integration();
    problem3_sensor_comparison();

    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;

    return 0;
}
