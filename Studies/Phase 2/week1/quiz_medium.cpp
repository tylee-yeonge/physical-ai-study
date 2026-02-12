/**
 * Phase 2 Week 1 - 핀홀 카메라 모델 중급 퀴즈
 *
 * 이 퀴즈는 투영/역투영의 실전 응용을 다룹니다.
 *
 * 난이도: ⭐⭐
 * 예상 소요 시간: 30-60분
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>

/**
 * 문제 1: 다중 점 투영 및 시각화
 *
 * 큐브의 8개 꼭짓점을 투영하고 이미지 위에 그리세요.
 *
 * 큐브 정의: 중심 (0, 0, 5), 한 변 길이 2
 *
 * TODO: 큐브를 투영하고 cv::line으로 연결하세요.
 */
void problem1_cube_projection()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 큐브 투영 시각화" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // R = I, t = 0
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t = cv::Mat::zeros(3, 1, CV_64F);

    // 큐브 꼭짓점 (중심 (0,0,5), 한 변 2)
    std::vector<cv::Point3d> cube = {
        {-1, -1, 4}, {1, -1, 4}, {1, 1, 4}, {-1, 1, 4},  // 앞면
        {-1, -1, 6}, {1, -1, 6}, {1, 1, 6}, {-1, 1, 6}   // 뒷면
    };

    // TODO: 각 3D 점을 2D로 투영
    std::vector<cv::Point2d> pixels;

    // TODO: 이미지에 큐브 그리기
    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC3);

    // 힌트: 앞면 연결 (0-1-2-3-0), 뒷면 연결 (4-5-6-7-4)
    // 앞뒤 연결 (0-4, 1-5, 2-6, 3-7)
    // cv::line으로 인접 꼭짓점을 연결하세요

    std::cout << "💡 힌트: cv::imshow로 결과를 확인하세요" << std::endl;
    std::cout << "   앞면(Z=4)의 점은 뒷면(Z=6)보다 크게 투영됩니다" << std::endl;
    std::cout << "   이것이 원근감 (perspective)입니다!" << std::endl;
}

/**
 * 문제 2: 재투영 오차 시뮬레이션
 *
 * 여러 3D 점을 투영한 후 노이즈를 추가하고,
 * 재투영 오차를 계산하세요.
 *
 * TODO: 오차의 평균과 표준편차를 구하세요.
 */
void problem2_reprojection_error()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 재투영 오차 시뮬레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    double fx = 600.0, fy = 600.0;
    double cx = 400.0, cy = 300.0;

    // 3D 점 생성 (Z=3~10 사이 랜덤)
    std::vector<cv::Point3d> points_3d;
    std::vector<cv::Point2d> observed_2d;

    srand(42);
    for (int i = 0; i < 20; i++)
    {
        double x = (rand() % 100 - 50) / 25.0;  // -2 ~ 2
        double y = (rand() % 100 - 50) / 25.0;
        double z = 3.0 + (rand() % 70) / 10.0;  // 3 ~ 10
        points_3d.push_back(cv::Point3d(x, y, z));

        // 투영 (R=I, t=0)
        double u = fx * x / z + cx;
        double v = fy * y / z + cy;

        // 노이즈 추가 (표준편차 1.0 픽셀)
        double noise_u = (rand() % 200 - 100) / 100.0;
        double noise_v = (rand() % 200 - 100) / 100.0;
        observed_2d.push_back(cv::Point2d(u + noise_u, v + noise_v));
    }

    // TODO: 각 점의 재투영 오차 계산
    std::vector<double> errors;
    // 힌트:
    //   1. points_3d 각 점을 투영하여 projected_2d 계산
    //   2. projected_2d와 observed_2d의 유클리드 거리 = error
    //   3. errors 벡터에 누적

    // TODO: 평균과 표준편차 계산
    double mean_error = 0.0;  // TODO
    double std_error = 0.0;   // TODO

    std::cout << "📊 재투영 오차 결과:" << std::endl;
    std::cout << "   점 개수: " << points_3d.size() << std::endl;
    std::cout << "   평균 오차: " << mean_error << " 픽셀" << std::endl;
    std::cout << "   표준편차: " << std_error << " 픽셀" << std::endl;

    std::cout << "\n💡 SLAM에서의 기준:" << std::endl;
    std::cout << "   < 0.5 픽셀: 매우 좋음" << std::endl;
    std::cout << "   < 1.0 픽셀: 좋음" << std::endl;
    std::cout << "   > 2.0 픽셀: 재캘리브레이션 필요" << std::endl;
}

/**
 * 문제 3: 외부 파라미터 변화 효과
 *
 * 카메라가 이동/회전할 때 투영 결과가 어떻게 변하는지 관찰하세요.
 *
 * 시나리오 1: 카메라가 오른쪽으로 1m 이동 (t = [1, 0, 0])
 * 시나리오 2: 카메라가 Y축으로 15° 회전
 *
 * TODO: 각 시나리오에서 점 (0, 0, 5)의 투영 결과를 비교하세요.
 */
void problem3_extrinsic_effect()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 외부 파라미터 변화 효과" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double fx = 600.0, fy = 600.0;
    double cx = 400.0, cy = 300.0;

    cv::Point3d P(0, 0, 5);

    std::cout << "3D 점: (" << P.x << ", " << P.y << ", " << P.z << ")\n" << std::endl;

    // 기본: R=I, t=0
    // TODO: 기본 투영 계산
    double u0 = 0.0, v0 = 0.0;  // TODO
    std::cout << "기본 (R=I, t=0): (" << u0 << ", " << v0 << ")" << std::endl;

    // 시나리오 1: t = [1, 0, 0] (카메라가 오른쪽으로 1m)
    // TODO: Pc = R*P + t = P + [1,0,0] = (1, 0, 5) 투영
    // 힌트: 카메라가 오른쪽으로 이동하면, 물체는 왼쪽으로 보임
    double u1 = 0.0, v1 = 0.0;  // TODO
    std::cout << "시나리오 1 (t=[1,0,0]): (" << u1 << ", " << v1 << ")" << std::endl;

    // 시나리오 2: Y축 15° 회전
    // TODO: 회전 행렬 생성 후 Pc = R*P + t
    double angle_rad = 15.0 * CV_PI / 180.0;
    // Y축 회전 행렬:
    // [ cos(θ)  0  sin(θ)]
    // [   0     1    0   ]
    // [-sin(θ)  0  cos(θ)]
    //
    // 힌트: cv::Mat R = (cv::Mat_<double>(3,3) << ...);
    // Pc = R * [0, 0, 5]^T

    double u2 = 0.0, v2 = 0.0;  // TODO
    std::cout << "시나리오 2 (Y축 15° 회전): (" << u2 << ", " << v2 << ")" << std::endl;

    std::cout << "\n💡 핵심 관찰:" << std::endl;
    std::cout << "   - K는 변하지 않음 (카메라 하드웨어 고유)" << std::endl;
    std::cout << "   - R, t만 변함 (카메라 움직임)" << std::endl;
    std::cout << "   - 이것이 SLAM에서 매 프레임 추정하는 것!" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 1 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_cube_projection();
    problem2_reprojection_error();
    problem3_extrinsic_effect();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
