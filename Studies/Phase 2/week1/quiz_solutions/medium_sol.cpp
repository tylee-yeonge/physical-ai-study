/**
 * Phase 2 Week 1 - 핀홀 카메라 모델 중급 퀴즈 (정답)
 *
 * 다루는 개념:
 *   - 3D 큐브 투영 시각화 (원근감 확인)
 *   - 재투영 오차 (Reprojection Error) 시뮬레이션
 *   - 외부 파라미터 [R|t] 변화 효과 (이동 vs 회전)
 *   - 다중 카메라 가시 영역 분석
 *
 * 선수 지식: week1 easy (K 행렬, 투영, 역투영, FOV, 가시성)
 *
 * 이 퀴즈는 투영의 실전 응용을 다룬다. SLAM에서 매 프레임마다
 * 수행하는 "3D 점 투영 → 관측과 비교 → 오차 계산"의 기초가 된다.
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>

// 큐브 투영 시각화 — 원근 효과를 직접 확인
//
// 3D 큐브 (한 변 2m, 중심 Z=5)의 8개 꼭짓점을 투영하고 선으로 연결한다.
//
// 원근 효과:
//   - 앞면 (Z=4): Zc가 작으므로 크게 투영됨
//   - 뒷면 (Z=6): Zc가 크므로 작게 투영됨
//   → 가까운 면이 크게, 먼 면이 작게 보이는 것이 원근감
//
// 큐브 연결 규칙:
//   앞면: 0-1-2-3-0  (Z=4 사각형)
//   뒷면: 4-5-6-7-4  (Z=6 사각형)
//   앞뒤: 0-4, 1-5, 2-6, 3-7  (깊이 방향 연결)
//
// ★ SLAM에서 3D 맵 점을 현재 카메라에 투영하여 시각화할 때 동일한 원리
//
// TODO: 8개 점을 투영하고 cv::line으로 큐브를 그리세요
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

    // ✅ 정답: 각 3D 점을 2D로 투영
    double fx = K.at<double>(0, 0), fy = K.at<double>(1, 1);
    double cx = K.at<double>(0, 2), cy = K.at<double>(1, 2);

    std::vector<cv::Point> pixels;
    for (const auto& pt : cube)
    {
        double u = fx * pt.x / pt.z + cx;
        double v = fy * pt.y / pt.z + cy;
        pixels.push_back(cv::Point((int)u, (int)v));
        std::cout << "(" << pt.x << ", " << pt.y << ", " << pt.z << ") → ("
                  << (int)u << ", " << (int)v << ")" << std::endl;
    }

    // ✅ 정답: 이미지에 큐브 그리기
    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC3);

    // 앞면 (초록)
    for (int i = 0; i < 4; i++)
        cv::line(image, pixels[i], pixels[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
    // 뒷면 (파랑)
    for (int i = 0; i < 4; i++)
        cv::line(image, pixels[4 + i], pixels[4 + (i + 1) % 4], cv::Scalar(255, 0, 0), 2);
    // 앞뒤 연결 (빨강)
    for (int i = 0; i < 4; i++)
        cv::line(image, pixels[i], pixels[i + 4], cv::Scalar(0, 0, 255), 1);

    std::cout << "\n💡 앞면(Z=4)의 점이 뒷면(Z=6)보다 크게 투영됨 → 원근감!" << std::endl;
    std::cout << "   앞면(Z=4)의 점은 뒷면(Z=6)보다 크게 투영됩니다" << std::endl;
    std::cout << "   이것이 원근감 (perspective)입니다!" << std::endl;
}

// 재투영 오차 시뮬레이션 — SLAM 정확도 평가의 핵심 지표
//
// 재투영 오차 (Reprojection Error):
//   e = ||p_observed - π(K, R, t, X)||
//   = "실제 관측 픽셀"과 "3D 점 투영 결과" 사이의 유클리드 거리
//
// 계산 과정:
//   1. 3D 점 Xi를 카메라에 투영: p_proj = K · [R|t] · Xi
//   2. 관측된 2D 점: p_obs (노이즈 포함)
//   3. 오차: eᵢ = ||p_obs - p_proj||₂
//   4. 전체 평균 오차: mean = Σeᵢ / N
//
// SLAM에서의 기준:
//   < 0.5 px: 매우 좋음 (잘 캘리브레이션된 시스템)
//   < 1.0 px: 좋음 (일반적 Visual SLAM)
//   > 2.0 px: 캘리브레이션 문제 또는 outlier 존재
//
// ★ Bundle Adjustment의 목적함수 = Σ 재투영 오차² 최소화
//
// TODO: 20개 점의 재투영 오차 평균과 표준편차를 계산하세요
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

    // ✅ 정답: 각 점의 재투영 오차 계산
    std::vector<double> errors;
    for (size_t i = 0; i < points_3d.size(); i++)
    {
        double u_proj = fx * points_3d[i].x / points_3d[i].z + cx;
        double v_proj = fy * points_3d[i].y / points_3d[i].z + cy;
        double du = u_proj - observed_2d[i].x;
        double dv = v_proj - observed_2d[i].y;
        errors.push_back(std::sqrt(du * du + dv * dv));
    }

    // ✅ 정답: 평균과 표준편차 계산
    double sum = std::accumulate(errors.begin(), errors.end(), 0.0);
    double mean_error = sum / errors.size();
    double sq_sum = 0.0;
    for (double e : errors)
        sq_sum += (e - mean_error) * (e - mean_error);
    double std_error = std::sqrt(sq_sum / errors.size());

    std::cout << "📊 재투영 오차 결과:" << std::endl;
    std::cout << "   점 개수: " << points_3d.size() << std::endl;
    std::cout << "   평균 오차: " << mean_error << " 픽셀" << std::endl;
    std::cout << "   표준편차: " << std_error << " 픽셀" << std::endl;

    std::cout << "\n💡 SLAM에서의 기준:" << std::endl;
    std::cout << "   < 0.5 픽셀: 매우 좋음" << std::endl;
    std::cout << "   < 1.0 픽셀: 좋음" << std::endl;
    std::cout << "   > 2.0 픽셀: 재캘리브레이션 필요" << std::endl;
}

// 외부 파라미터 변화 효과 — 카메라 움직임이 투영에 미치는 영향
//
// 외부 파라미터 [R|t]는 카메라의 포즈(위치+방향)를 나타낸다.
// K(내부)는 고정이고, [R|t](외부)만 프레임마다 변한다.
//
// 이동 (Translation) 효과:
//   Pc = R · Pw + t
//   카메라가 오른쪽(+X)으로 이동 → 물체가 왼쪽으로 보임
//   → 투영 u좌표가 감소 (왼쪽으로 이동)
//
// 회전 (Rotation) 효과:
//   카메라가 Y축으로 회전 → 물체가 회전 반대 방향으로 이동
//   Y축 회전 행렬 Ry(θ):
//     [ cos(θ)  0  sin(θ)]
//     [   0     1    0   ]
//     [-sin(θ)  0  cos(θ)]
//
// ★ SLAM이 하는 일 = 매 프레임마다 [R|t]를 추정하는 것
//   K는 캘리브레이션으로 한 번만 구하면 됨
//
// TODO: 이동과 회전 시나리오에서 같은 3D 점의 투영 차이를 관찰하세요
void problem3_extrinsic_effect()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 외부 파라미터 변화 효과" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double fx = 600.0, fy = 600.0;
    double cx = 400.0, cy = 300.0;

    cv::Point3d P(0, 0, 5);

    std::cout << "3D 점: (" << P.x << ", " << P.y << ", " << P.z << ")\n" << std::endl;

    // ✅ 정답: 기본 투영 계산
    double u0 = fx * P.x / P.z + cx;
    double v0 = fy * P.y / P.z + cy;
    std::cout << "기본 (R=I, t=0): (" << u0 << ", " << v0 << ")" << std::endl;

    // ✅ 정답: 시나리오 1 - t = [1, 0, 0]
    // Pc = R*P + t = (0,0,5) + (1,0,0) = (1, 0, 5)
    double Xc1 = P.x + 1.0, Yc1 = P.y, Zc1 = P.z;
    double u1 = fx * Xc1 / Zc1 + cx;
    double v1 = fy * Yc1 / Zc1 + cy;
    std::cout << "시나리오 1 (t=[1,0,0]): (" << u1 << ", " << v1 << ")" << std::endl;

    // ✅ 정답: 시나리오 2 - Y축 15° 회전
    double angle_rad = 15.0 * CV_PI / 180.0;
    cv::Mat Ry = (cv::Mat_<double>(3, 3) <<
        std::cos(angle_rad), 0, std::sin(angle_rad),
        0, 1, 0,
        -std::sin(angle_rad), 0, std::cos(angle_rad));
    cv::Mat Pw = (cv::Mat_<double>(3, 1) << P.x, P.y, P.z);
    cv::Mat Pc = Ry * Pw;
    double u2 = fx * Pc.at<double>(0) / Pc.at<double>(2) + cx;
    double v2 = fy * Pc.at<double>(1) / Pc.at<double>(2) + cy;
    std::cout << "시나리오 2 (Y축 15° 회전): (" << u2 << ", " << v2 << ")" << std::endl;

    std::cout << "\n💡 핵심 관찰:" << std::endl;
    std::cout << "   - K는 변하지 않음 (카메라 하드웨어 고유)" << std::endl;
    std::cout << "   - R, t만 변함 (카메라 움직임)" << std::endl;
    std::cout << "   - 이것이 SLAM에서 매 프레임 추정하는 것!" << std::endl;
}

// 다중 카메라 가시 영역 분석 — 넓은 시야를 위한 다중 카메라 시스템
//
// 단일 카메라의 FOV는 제한적이므로, 여러 카메라를 다른 방향으로 배치하면
// 더 넓은 영역을 커버할 수 있다.
//
// 이 문제의 설정:
//   - 정면 카메라: R = I (0도)
//   - 좌측 카메라: Y축 -30도 회전 (왼쪽을 바라봄)
//   - 우측 카메라: Y축 +30도 회전 (오른쪽을 바라봄)
//
// 가시 영역 계산:
//   특정 거리 d에서의 가시 영역:
//   visible_width = 2 · d · tan(FOV_x / 2)
//   visible_height = 2 · d · tan(FOV_y / 2)
//
// ★ 다중 카메라 SLAM의 장점:
//   - 넓은 시야 → 특징점 추적 안정성 향상
//   - 겹치는 영역 → 스테레오 삼각측량 가능
//   - 자율주행에서 360° 커버리지 확보
//
// TODO: 3대 카메라에 대해 10개 점의 가시성을 판별하고,
//       거리별 가시 영역 크기를 계산하세요
void problem4_multi_camera_visibility()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 다중 카메라 가시 영역" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double fx = 500.0, fy = 500.0;
    double cx = 320.0, cy = 240.0;
    int image_width = 640, image_height = 480;

    // 3대 카메라 설정 (정면, 좌측 30도, 우측 30도)
    struct Camera
    {
        std::string name;
        cv::Mat R;
        cv::Mat t;
    };

    double angle_30 = 30.0 * CV_PI / 180.0;

    std::vector<Camera> cameras;

    // ✅ 정답: 3대 카메라 생성
    cameras.push_back({"정면 카메라", cv::Mat::eye(3, 3, CV_64F), cv::Mat::zeros(3, 1, CV_64F)});
    cameras.push_back({"좌측 30도",
        (cv::Mat_<double>(3, 3) << std::cos(-angle_30), 0, std::sin(-angle_30),
         0, 1, 0, -std::sin(-angle_30), 0, std::cos(-angle_30)),
        cv::Mat::zeros(3, 1, CV_64F)});
    cameras.push_back({"우측 30도",
        (cv::Mat_<double>(3, 3) << std::cos(angle_30), 0, std::sin(angle_30),
         0, 1, 0, -std::sin(angle_30), 0, std::cos(angle_30)),
        cv::Mat::zeros(3, 1, CV_64F)});

    // 10개 3D 점
    std::vector<cv::Point3d> points = {
        {0, 0, 5},   {2, 0, 5},   {-2, 0, 5},  {0, 2, 5},   {0, -2, 5},
        {5, 0, 5},   {-5, 0, 5},  {3, 3, 10},  {0, 0, 20},  {10, 0, 10},
    };

    // TODO: 각 카메라에 대해 각 점을 투영하고, 가시 여부를 판별
    // 힌트:
    // 1. 카메라 좌표로 변환
    // 2. Zc > 0 확인
    // 3. 픽셀 좌표로 투영
    // 4. 이미지 경계 내 확인

    for (size_t c = 0; c < cameras.size(); c++)
    {
        std::cout << cameras[c].name << ":" << std::endl;

        for (size_t i = 0; i < points.size(); i++)
        {
            // ✅ 정답: 카메라 좌표 변환 + 가시성 판별
            cv::Mat Pw = (cv::Mat_<double>(3, 1) << points[i].x, points[i].y, points[i].z);
            cv::Mat Pc = cameras[c].R * Pw + cameras[c].t;
            double Xc = Pc.at<double>(0), Yc = Pc.at<double>(1), Zc = Pc.at<double>(2);

            if (Zc <= 0)
            {
                std::cout << "   점 " << i << ": 카메라 뒤 (보이지 않음)" << std::endl;
                continue;
            }
            double u = fx * Xc / Zc + cx;
            double v = fy * Yc / Zc + cy;
            bool visible = (u >= 0 && u < image_width && v >= 0 && v < image_height);
            std::cout << "   점 " << i << " → (" << (int)u << ", " << (int)v << ") "
                      << (visible ? "보임" : "범위 밖") << std::endl;
        }
        std::cout << std::endl;
    }

    // TODO: 특정 거리에서의 가시 영역 크기 계산
    std::cout << "\n거리별 가시 영역 (단일 카메라):" << std::endl;
    std::cout << std::string(45, '-') << std::endl;

    // ✅ 정답: FOV 기반 가시 영역 계산
    double fov_x = 2.0 * std::atan2(image_width, 2.0 * fx);
    double fov_y = 2.0 * std::atan2(image_height, 2.0 * fy);

    std::vector<double> distances = {1, 5, 10, 20, 50};
    for (double d : distances)
    {
        double visible_width = 2.0 * d * std::tan(fov_x / 2.0);
        double visible_height = 2.0 * d * std::tan(fov_y / 2.0);

        std::cout << "   거리 " << d << "m: " << visible_width << " x " << visible_height
                  << " m" << std::endl;
    }

    std::cout << "\n💡 힌트:" << std::endl;
    std::cout << "   다중 카메라는 단일 카메라보다 넓은 영역을 커버합니다." << std::endl;
    std::cout << "   Visual SLAM에서 다중 카메라 시스템이 유리한 이유!" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 1 Quiz - Medium (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_cube_projection();
    problem2_reprojection_error();
    problem3_extrinsic_effect();
    problem4_multi_camera_visibility();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
