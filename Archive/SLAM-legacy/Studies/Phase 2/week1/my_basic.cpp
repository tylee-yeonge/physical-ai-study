/**
 * Phase 2 Week 1 - 핀홀 카메라 모델 직접 구현
 *
 * basic.h의 PinholeProjection 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make my_basic
 * 실행: ./my_basic (각 Step 구현 후 실행하여 확인)
 * 전체 테스트: cd build && cmake .. && make test_my_basic && ./test_my_basic
 *
 * ┌──────────────────────────────────────────────────┐
 * │              구현 순서 가이드                         │
 * ├──────┬──────────────────┬────────┬────────────────┤
 * │ Step │ 함수             │ 난이도  │ 검증 방법        │
 * ├──────┼──────────────────┼────────┼────────────────┤
 * │  1   │ isInImage        │ 쉬움   │ ./my_basic     │
 * │  2   │ computeFOV       │ 쉬움   │ ./my_basic     │
 * │  3   │ project          │ 핵심   │ ./my_basic     │
 * │  4   │ backProject      │ 보통   │ ./my_basic     │
 * │  5   │ projectMultiple  │ 보통   │ ./my_basic     │
 * │  6   │ reprojectionError│ 어려움  │ ./my_basic     │
 * └──────┴──────────────────┴────────┴────────────────┘
 *
 * 💡 각 Step 구현 후 ./my_basic을 실행하면 해당 단계의 테스트 결과를 확인할 수 있습니다.
 */
#include "basic.h"
#include <cmath>
#include <iostream>

// 라디안 → 도(degree) 변환 계수
constexpr double kRadToDeg = 180.0 / CV_PI;

PinholeProjection::PinholeProjection(const cv::Mat &K, const cv::Mat &R, const cv::Mat &t)
    : K_(K.clone()), R_(R.clone()), t_(t.clone())
{
}

bool PinholeProjection::isInImage(const cv::Point2d &pixel, const cv::Size &imageSize)
{
    // [Step 1] 픽셀이 이미지 범위 안에 있는지 확인
    // 힌트: 0 <= x < width && 0 <= y < height
    // 참고: basic.cpp의 isInImage()
    return pixel.x >= 0 && pixel.x < imageSize.width && pixel.y >= 0 && pixel.y < imageSize.height;
}

cv::Size2d PinholeProjection::computeFOV(const cv::Size &imageSize) const
{
    // [Step 2] 수평/수직 시야각 계산
    // 공식: FOV = 2 * atan(image_size / (2 * focal_length))
    // 힌트: std::atan2() 사용, 결과를 kRadToDeg로 변환
    // 참고: basic.cpp의 computeFOV()
    // 기대값: fx=600, width=800 → 약 67°
    double fx = K_.at<double>(0, 0);
    double fy = K_.at<double>(1, 1);

    double fov_h = 2.0 * std::atan2(imageSize.width, 2.0 * fx) * kRadToDeg;
    double fov_v = 2.0 * std::atan2(imageSize.height, 2.0 * fy) * kRadToDeg;

    return cv::Size2d(fov_h, fov_v);
}

cv::Point2d PinholeProjection::project(const cv::Point3d &P_world) const
{
    // [Step 3] 3D 월드 좌표 → 2D 픽셀 좌표 투영 (핵심!)
    // 1) Pc = R * Pw + t (월드 → 카메라)
    // 2) 정규화 좌표: x_norm = Xc/Zc, y_norm = Yc/Zc
    // 3) 픽셀 좌표: u = fx * x_norm + cx, v = fy * y_norm + cy
    // Zc <= 0이면 카메라 뒤 → (-1, -1) 반환
    // 참고: basic.cpp의 project(), ./basic 출력의 "투영 수학" 블록
    // 기대값: (0,0,5) → (400,300) = 이미지 중심

    cv::Mat Pw = (cv::Mat_<double>(3, 1) << P_world.x, P_world.y, P_world.z);
    cv::Mat Pc = R_ * Pw + t_;

    double Xc = Pc.at<double>(0, 0);
    double Yc = Pc.at<double>(1, 0);
    double Zc = Pc.at<double>(2, 0);

    if (Zc <= 0)
    {
        return cv::Point2d(-1, -1);
    }

    double x_norm = Xc / Zc;
    double y_norm = Yc / Zc;

    double fx = K_.at<double>(0, 0);
    double fy = K_.at<double>(1, 1);
    double cx = K_.at<double>(0, 2);
    double cy = K_.at<double>(1, 2);

    double u = fx * x_norm + cx;
    double v = fy * y_norm + cy;

    return cv::Point2d(u, v);
}

cv::Vec3d PinholeProjection::backProject(const cv::Point2d &pixel) const
{
    // [Step 4] 2D 픽셀 → 3D 광선 방향 (정규화된 단위 벡터)
    // 공식: x_norm = (u - cx) / fx, y_norm = (v - cy) / fy
    //       ray = normalize([x_norm, y_norm, 1])
    // 참고: basic.cpp의 backProject(), ./basic 출력의 "역투영 핵심" 블록
    // 기대값: 중심(400,300) → [0,0,1] (카메라 정면)
    double fx = K_.at<double>(0, 0);
    double fy = K_.at<double>(1, 1);
    double cx = K_.at<double>(0, 2);
    double cy = K_.at<double>(1, 2);

    double x_norm = (pixel.x - cx) / fx;
    double y_norm = (pixel.y - cy) / fy;

    cv::Vec3d ray(x_norm, y_norm, 1.0);

    double norm = cv::norm(ray);
    return ray / norm;
}

std::vector<cv::Point2d> PinholeProjection::projectMultiple(
    const std::vector<cv::Point3d> &points_3d) const
{
    // [Step 5] 여러 점을 한번에 투영
    // 힌트: project()를 반복 호출
    // 참고: basic.cpp의 projectMultiple()
    std::vector<cv::Point2d> pixels;
    pixels.reserve(points_3d.size());

    for (const auto &pt : points_3d)
    {
        pixels.push_back(project(pt));
    }
    return pixels;
}

double PinholeProjection::reprojectionError(const cv::Point3d &P_world,
                                            const cv::Point2d &observed_pixel) const
{
    // [Step 6] 재투영 오차 = ||project(P) - observed||
    // 힌트: project() 호출 후 유클리드 거리 계산
    // 투영 실패(x<0)이면 -1.0 반환
    // 참고: basic.cpp의 reprojectionError()
    // 기대값: 정확한 투영이면 0.0
    cv::Point2d projected = project(P_world);

    if (projected.x < 0)
        return -1.0;

    double dx = projected.x - observed_pixel.x;
    double dy = projected.y - observed_pixel.y;

    return std::sqrt(dx * dx + dy * dy);
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 핀홀 카메라 - 단계별 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 공통 설정
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t = cv::Mat::zeros(3, 1, CV_64F);
    cv::Size imageSize(800, 600);

    PinholeProjection camera(K, R, t);

    // ── Step 1: isInImage ──
    std::cout << "Step 1: isInImage" << std::endl;
    bool in1 = PinholeProjection::isInImage(cv::Point2d(320, 240), imageSize);
    bool in2 = PinholeProjection::isInImage(cv::Point2d(-1, 0), imageSize);
    bool in3 = PinholeProjection::isInImage(cv::Point2d(800, 300), imageSize);
    std::cout << "   (320,240) in 800x600 → " << (in1 ? "true" : "false")
              << (in1 ? " ✅" : " ❌ 기대: true") << std::endl;
    std::cout << "   (-1, 0)   in 800x600 → " << (in2 ? "true" : "false")
              << (!in2 ? " ✅" : " ❌ 기대: false") << std::endl;
    std::cout << "   (800,300) in 800x600 → " << (in3 ? "true" : "false")
              << (!in3 ? " ✅" : " ❌ 기대: false (경계 밖)") << std::endl;

    // ── Step 2: computeFOV ──
    std::cout << "\nStep 2: computeFOV" << std::endl;
    cv::Size2d fov = camera.computeFOV(imageSize);
    std::cout << "   수평 FOV: " << fov.width << "°" << std::endl;
    std::cout << "   수직 FOV: " << fov.height << "°" << std::endl;
    bool fov_ok = (fov.width > 60 && fov.width < 75);
    std::cout << "   " << (fov_ok ? "✅ 범위 정상 (60~75°)" : "❌ 기대 범위: 60~75°")
              << std::endl;

    // ── Step 3: project ──
    std::cout << "\nStep 3: project" << std::endl;
    cv::Point2d p1 = camera.project(cv::Point3d(0, 0, 5));
    cv::Point2d p2 = camera.project(cv::Point3d(0, 0, -1));
    std::cout << "   (0,0,5) → (" << p1.x << ", " << p1.y << ")"
              << (std::abs(p1.x - 400) < 1 && std::abs(p1.y - 300) < 1
                      ? " ✅ 중심 근처"
                      : " ❌ 기대: (400,300)")
              << std::endl;
    std::cout << "   (0,0,-1) → (" << p2.x << ", " << p2.y << ")"
              << (p2.x < 0 ? " ✅ 카메라 뒤 → (-1,-1)" : " ❌ 기대: (-1,-1)") << std::endl;

    // ── Step 4: backProject ──
    std::cout << "\nStep 4: backProject" << std::endl;
    cv::Vec3d ray_center = camera.backProject(cv::Point2d(400, 300));
    std::cout << "   (400,300) → [" << ray_center[0] << ", " << ray_center[1] << ", "
              << ray_center[2] << "]" << std::endl;
    bool bp_ok = (std::abs(ray_center[0]) < 0.001 && std::abs(ray_center[1]) < 0.001 &&
                  std::abs(ray_center[2] - 1.0) < 0.001);
    std::cout << "   " << (bp_ok ? "✅ 중심 → [0,0,1] 정면" : "❌ 기대: [0,0,1]") << std::endl;

    // 투영→역투영 일관성 검증
    cv::Point3d test_3d(2, 1, 5);
    cv::Point2d test_px = camera.project(test_3d);
    cv::Vec3d ray_test = camera.backProject(test_px);
    cv::Vec3d dir_orig = cv::Vec3d(test_3d.x, test_3d.y, test_3d.z);
    dir_orig /= cv::norm(dir_orig);
    double dir_err = cv::norm(ray_test - dir_orig);
    std::cout << "   투영→역투영 방향 일치: 오차 = " << dir_err
              << (dir_err < 0.01 ? " ✅" : " ❌") << std::endl;

    // ── Step 5: projectMultiple ──
    std::cout << "\nStep 5: projectMultiple" << std::endl;
    std::vector<cv::Point3d> cube = {{-1, -1, 5}, {1, -1, 5}, {1, 1, 5}, {-1, 1, 5},
                                     {-1, -1, 7}, {1, -1, 7}, {1, 1, 7}, {-1, 1, 7}};
    auto pixels = camera.projectMultiple(cube);
    std::cout << "   큐브 8점 투영: " << pixels.size() << "개"
              << (pixels.size() == 8 ? " ✅" : " ❌ 기대: 8") << std::endl;
    if (pixels.size() == 8)
    {
        std::cout << "   첫 점 (-1,-1,5) → (" << (int)pixels[0].x << "," << (int)pixels[0].y
                  << ")" << std::endl;
        std::cout << "   뒷면은 앞면보다 중심에 가까움 (원근감)" << std::endl;
    }

    // ── Step 6: reprojectionError ──
    std::cout << "\nStep 6: reprojectionError" << std::endl;
    cv::Point3d pt(2, 1, 5);
    cv::Point2d exact_px = camera.project(pt);
    double err_exact = camera.reprojectionError(pt, exact_px);
    cv::Point2d noisy_px(exact_px.x + 3, exact_px.y + 4);
    double err_noisy = camera.reprojectionError(pt, noisy_px);
    std::cout << "   정확한 관측: 오차 = " << err_exact << " px"
              << (err_exact < 0.001 ? " ✅" : " ❌ 기대: ~0") << std::endl;
    std::cout << "   노이즈(+3,+4): 오차 = " << err_noisy << " px"
              << (std::abs(err_noisy - 5.0) < 0.01 ? " ✅ = 5.0" : " ❌ 기대: 5.0")
              << std::endl;

    // ── 요약 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  ✅ 전체 확인 완료 후: ./test_my_basic" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
