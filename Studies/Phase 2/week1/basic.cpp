#include "basic.h"
#include <iostream>
#include <cmath>

// 라디안 → 도(degree) 변환 계수
constexpr double kRadToDeg = 180.0 / CV_PI;

// 생성자 — 카메라 파라미터 저장
//   K: 3×3 내부 행렬 [fx 0 cx; 0 fy cy; 0 0 1]
//   R: 3×3 회전 행렬 (월드→카메라 좌표 변환)
//   t: 3×1 이동 벡터 (월드→카메라 좌표 변환)
//   clone()으로 독립 복사 — 원본이 바뀌어도 이 객체는 영향받지 않음
PinholeProjection::PinholeProjection(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t)
    : K_(K.clone()), R_(R.clone()), t_(t.clone())
{
}

// 투영된 픽셀이 이미지 영역 안에 있는지 확인
//   이미지 밖에 투영되면 실제로 관측 불가능한 점.
//   3D Detection 의 visibility / valid mask 처리에 사용.
bool PinholeProjection::isInImage(const cv::Point2d& pixel, const cv::Size& imageSize)
{
    return pixel.x >= 0 && pixel.x < imageSize.width && pixel.y >= 0 && pixel.y < imageSize.height;
}

// 시야각(Field of View) 계산 — 카메라가 얼마나 넓은 영역을 촬영하는지
//
// 원리: 이미지 센서의 가장자리 픽셀이 광축과 이루는 각도의 2배
//
//       |<-- W/2 -->|
//       +-----------+
//        \     θ   /
//         \   |   /        FOV = 2θ
//          \  |  /
//           \ | /  ← fx (초점거리, 픽셀 단위)
//            \|/
//             카메라 중심
//
//   tan(θ) = (W/2) / fx  →  θ = atan(W / (2·fx))  →  FOV = 2θ
//
// fx가 크면(망원) → FOV 작음 (좁은 영역을 확대)
// fx가 작으면(광각) → FOV 큼 (넓은 영역 촬영)
cv::Size2d PinholeProjection::computeFOV(const cv::Size& imageSize) const
{
    double fx = K_.at<double>(0, 0);
    double fy = K_.at<double>(1, 1);

    double fov_h = 2.0 * std::atan2(imageSize.width, 2.0 * fx) * kRadToDeg;
    double fov_v = 2.0 * std::atan2(imageSize.height, 2.0 * fy) * kRadToDeg;

    return cv::Size2d(fov_h, fov_v);
}

// 3D 월드 좌표 → 2D 픽셀 좌표 투영 (핀홀 카메라 모델의 핵심)
//
// 수식: p = K · [R|t] · P_world
//   이를 3단계로 분해하면:
//
// Step 1: 월드→카메라 좌표 변환 (외부 파라미터)
//   Pc = R · Pw + t
//   R: 3×3 회전 행렬 — 월드 좌표계의 축을 카메라 좌표계로 변환
//   t: 3×1 이동 벡터 — 월드 원점에서 카메라까지의 변위
//
// Step 2: 원근 투영 (정규화 좌표)
//   x = Xc/Zc, y = Yc/Zc
//   3D를 2D로 축소하는 핵심 — 깊이(Zc)로 나누어 원근 효과 생성
//   ★ 이 나눗셈이 "가까운 물체는 크게, 먼 물체는 작게" 만듦
//
// Step 3: 정규화→픽셀 좌표 변환 (내부 파라미터)
//   u = fx·x + cx, v = fy·y + cy
//   fx,fy: 초점거리 (미터→픽셀 스케일링)
//   cx,cy: 주점 오프셋 (원점을 이미지 좌상단으로 이동)
cv::Point2d PinholeProjection::project(const cv::Point3d& P_world) const
{
    // Step 1: 월드 → 카메라 좌표 변환
    cv::Mat Pw = (cv::Mat_<double>(3, 1) << P_world.x, P_world.y, P_world.z);
    cv::Mat Pc = R_ * Pw + t_;

    double Xc = Pc.at<double>(0);
    double Yc = Pc.at<double>(1);
    double Zc = Pc.at<double>(2);

    // Zc ≤ 0 → 카메라 뒤의 점은 투영 불가 (핀홀 모델의 물리적 한계)
    if (Zc <= 0)
    {
        return cv::Point2d(-1, -1);
    }

    // Step 2: 원근 투영 (깊이 Zc로 나누어 정규화 좌표 생성)
    double x_norm = Xc / Zc;
    double y_norm = Yc / Zc;

    // Step 3: K 행렬 적용 — 정규화 좌표를 픽셀 좌표로 변환
    double fx = K_.at<double>(0, 0);
    double fy = K_.at<double>(1, 1);
    double cx = K_.at<double>(0, 2);
    double cy = K_.at<double>(1, 2);

    double u = fx * x_norm + cx;
    double v = fy * y_norm + cy;

    return cv::Point2d(u, v);
}

// 2D 픽셀 좌표 → 3D 광선 방향 (역투영, back-projection)
//
// 투영의 역과정이지만, 깊이 정보를 복원할 수 없다는 점이 핵심 차이.
// 투영 시 Zc로 나누면서 깊이 정보가 소실되기 때문.
// → 복원 가능한 것은 "방향"뿐, "거리"는 알 수 없음.
//
// 수식: K⁻¹ · [u, v, 1]ᵀ = [x_norm, y_norm, 1]ᵀ
//   x_norm = (u - cx) / fx
//   y_norm = (v - cy) / fy
//   → 카메라 좌표계에서 (x_norm, y_norm, 1) 방향의 광선
//
// 활용:
//   - 삼각측량: 두 카메라의 광선 교차점 = 3D 점
//   - Depth 모델 출력 결합: 픽셀의 광선 방향 × 추정 depth = 3D 점
//   - 3D 복원: 광선 + 깊이 값 → 3D 점 복원
cv::Vec3d PinholeProjection::backProject(const cv::Point2d& pixel) const
{
    double fx = K_.at<double>(0, 0);
    double fy = K_.at<double>(1, 1);
    double cx = K_.at<double>(0, 2);
    double cy = K_.at<double>(1, 2);

    // K⁻¹ 적용: 주점을 빼고 초점거리로 나누어 정규화 좌표로 변환
    double x_norm = (pixel.x - cx) / fx;
    double y_norm = (pixel.y - cy) / fy;

    // 카메라 원점에서 해당 픽셀 방향으로 뻗어나가는 3D 광선
    //   z=1 평면 위의 점 (x_norm, y_norm, 1)을 가리키는 벡터
    //   중심 픽셀(cx,cy) → (0, 0, 1) = 카메라 정면 방향
    cv::Vec3d ray(x_norm, y_norm, 1.0);

    // 단위 벡터로 정규화 → 방향만 남기고 크기를 1로
    double norm = cv::norm(ray);
    return ray / norm;
}

// 여러 3D 점을 한 번에 투영 — project()를 반복 호출
//   reserve()로 메모리를 미리 확보하여 push_back 시 재할당 방지
std::vector<cv::Point2d> PinholeProjection::projectMultiple(
    const std::vector<cv::Point3d>& points_3d) const
{
    std::vector<cv::Point2d> pixels;
    pixels.reserve(points_3d.size());

    for (const auto& pt : points_3d)
    {
        pixels.push_back(project(pt));
    }

    return pixels;
}

// 재투영 오차 계산 — 3D 점을 투영한 결과와 실제 관측 픽셀의 거리
//
// 재투영 오차(reprojection error)란?
//   "추정된 3D 점을 카메라 모델로 다시 투영했을 때,
//    실제 이미지에서 관측된 위치와 얼마나 차이나는가"
//
//   error = ||project(P_world) - observed_pixel||₂  (유클리드 거리, 픽셀 단위)
//
// 이 값이 작을수록:
//   - 카메라 파라미터(K, R, t)가 정확
//   - 3D 점 위치가 정확
//   - 캘리브레이션, 삼각측량, PnP, Bundle Adjustment 모든 곳에서
//     최적화 목표(cost function)로 사용
double PinholeProjection::reprojectionError(const cv::Point3d& P_world,
                                            const cv::Point2d& observed_pixel) const
{
    cv::Point2d projected = project(P_world);

    // project()가 카메라 뒤의 점에 대해 (-1,-1)을 반환하므로 실패 감지
    if (projected.x < 0)
        return -1.0;

    double dx = projected.x - observed_pixel.x;
    double dy = projected.y - observed_pixel.y;

    return std::sqrt(dx * dx + dy * dy);
}

#ifndef PINHOLE_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  핀홀 카메라 투영 기본 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 내부 파라미터 K
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // 카메라 외부 파라미터 (단위 행렬 = 원점에서 정면)
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t = cv::Mat::zeros(3, 1, CV_64F);

    PinholeProjection camera(K, R, t);

    // 1. 기본 투영
    std::cout << "📐 카메라 파라미터:" << std::endl;
    std::cout << "   fx = " << camera.getFx() << ", fy = " << camera.getFy() << std::endl;
    std::cout << "   cx = " << camera.getCx() << ", cy = " << camera.getCy() << "\n" << std::endl;

    // 💡 K 행렬 요소 접근법 (quiz에서 이렇게 접근!)
    std::cout << "💡 [K 행렬 접근법] quiz에서 이렇게 접근합니다:" << std::endl;
    std::cout << "   K.at<double>(0,0) = fx = " << K.at<double>(0, 0) << std::endl;
    std::cout << "   K.at<double>(1,1) = fy = " << K.at<double>(1, 1) << std::endl;
    std::cout << "   K.at<double>(0,2) = cx = " << K.at<double>(0, 2) << std::endl;
    std::cout << "   K.at<double>(1,2) = cy = " << K.at<double>(1, 2) << "\n" << std::endl;

    // 3D 점 정의 (큐브 꼭짓점)
    std::vector<cv::Point3d> cube_points = {
        {-1, -1, 5}, {1, -1, 5}, {1, 1, 5}, {-1, 1, 5},  // 앞면
        {-1, -1, 7}, {1, -1, 7}, {1, 1, 7}, {-1, 1, 7}   // 뒷면
    };

    std::cout << "📊 3D 큐브 투영 결과:" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    auto pixels = camera.projectMultiple(cube_points);
    cv::Size imageSize(800, 600);

    for (size_t i = 0; i < cube_points.size(); i++)
    {
        const auto& p3d = cube_points[i];
        const auto& p2d = pixels[i];
        bool visible = PinholeProjection::isInImage(p2d, imageSize);

        std::cout << "   (" << p3d.x << ", " << p3d.y << ", " << p3d.z << ")"
                  << " → (" << (int)p2d.x << ", " << (int)p2d.y << ")"
                  << (visible ? " ✅" : " ❌ 이미지 밖") << std::endl;
    }

    // 💡 투영 수학 단계별 시연
    {
        cv::Point3d demo_pt = cube_points[0];  // (-1, -1, 5)
        // R=I, t=0 이므로 카메라 좌표 = 월드 좌표
        double Xc = demo_pt.x, Yc = demo_pt.y, Zc = demo_pt.z;
        double x_norm = Xc / Zc;
        double y_norm = Yc / Zc;
        double fx = K.at<double>(0, 0), fy = K.at<double>(1, 1);
        double cx = K.at<double>(0, 2), cy = K.at<double>(1, 2);
        double u = fx * x_norm + cx;
        double v = fy * y_norm + cy;

        std::cout << "\n💡 [투영 수학] 점 (" << demo_pt.x << "," << demo_pt.y << "," << demo_pt.z
                  << ")의 투영 과정:" << std::endl;
        std::cout << "   Step 1. 월드→카메라: Pc = R*Pw + t = (" << Xc << "," << Yc << "," << Zc
                  << ")  (R=I, t=0)" << std::endl;
        std::cout << "   Step 2. 정규화: x = Xc/Zc = " << Xc << "/" << Zc << " = " << x_norm
                  << std::endl;
        std::cout << "                   y = Yc/Zc = " << Yc << "/" << Zc << " = " << y_norm
                  << std::endl;
        std::cout << "   Step 3. 픽셀:  u = fx*x + cx = " << fx << "*" << x_norm << " + " << cx
                  << " = " << u << std::endl;
        std::cout << "                  v = fy*y + cy = " << fy << "*" << y_norm << " + " << cy
                  << " = " << v << std::endl;
        std::cout << "   → 위 테이블에서 (" << demo_pt.x << "," << demo_pt.y << "," << demo_pt.z
                  << ") → (" << (int)u << "," << (int)v << ") 확인!" << std::endl;
        std::cout << "   💡 이 3단계가 quiz_easy 문제 2!" << std::endl;
    }

    // 2. FOV 계산
    std::cout << "\n📐 시야각 (FOV):" << std::endl;
    cv::Size2d fov = camera.computeFOV(imageSize);
    std::cout << "   수평 FOV: " << fov.width << "°" << std::endl;
    std::cout << "   수직 FOV: " << fov.height << "°" << std::endl;

    // 💡 FOV ↔ 초점거리 관계
    std::cout << "\n💡 [FOV 공식] FOV = 2 * atan(이미지크기 / (2*초점거리))" << std::endl;
    std::cout << "   수평: 2 * atan(" << imageSize.width << " / (2*" << K.at<double>(0, 0)
              << ")) = " << fov.width << "°" << std::endl;
    std::cout << "   fx 크면 → FOV 작음 (망원), fx 작으면 → FOV 큼 (광각)" << std::endl;
    std::cout << "   💡 이 계산이 quiz_easy 문제 3!" << std::endl;

    // 3. 역투영
    std::cout << "\n🔄 역투영 테스트:" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    // 이미지 중심 역투영
    cv::Point2d center(camera.getCx(), camera.getCy());
    cv::Vec3d ray_center = camera.backProject(center);
    std::cout << "   중심 (" << center.x << ", " << center.y << ")"
              << " → ray [" << ray_center[0] << ", " << ray_center[1] << ", " << ray_center[2]
              << "]" << std::endl;

    // 모서리 역투영
    cv::Point2d corner(0, 0);
    cv::Vec3d ray_corner = camera.backProject(corner);
    std::cout << "   좌상단 (0, 0)"
              << " → ray [" << ray_corner[0] << ", " << ray_corner[1] << ", " << ray_corner[2]
              << "]" << std::endl;

    // 💡 역투영 의미 설명
    std::cout << "\n💡 [역투영 핵심] 픽셀 → 3D 광선 방향 (깊이는 알 수 없음!)" << std::endl;
    std::cout << "   공식: x' = (u-cx)/fx, y' = (v-cy)/fy, ray = [x',y',1]" << std::endl;
    std::cout << "   중심 픽셀 (" << camera.getCx() << "," << camera.getCy()
              << ") → ray [0,0,1] = 카메라 정면" << std::endl;
    std::cout << "   💡 이 계산이 quiz_easy 문제 4!" << std::endl;

    // 4. 재투영 오차
    std::cout << "\n📏 재투영 오차:" << std::endl;
    cv::Point3d test_point(2.0, 1.0, 5.0);
    cv::Point2d projected = camera.project(test_point);
    cv::Point2d noisy_obs(projected.x + 1.5, projected.y - 0.8);

    double error = camera.reprojectionError(test_point, noisy_obs);
    std::cout << "   투영: (" << projected.x << ", " << projected.y << ")" << std::endl;
    std::cout << "   관측: (" << noisy_obs.x << ", " << noisy_obs.y << ")" << std::endl;
    std::cout << "   오차: " << error << " 픽셀" << std::endl;

    // 5. 카메라 위치 변경
    std::cout << "\n📷 카메라 위치 변경 테스트:" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    cv::Mat t2 = (cv::Mat_<double>(3, 1) << 2.0, 0.0, 0.0);  // 오른쪽으로 2m 이동
    PinholeProjection camera2(K, R, t2);

    cv::Point3d P(0, 0, 5);
    cv::Point2d pix1 = camera.project(P);
    cv::Point2d pix2 = camera2.project(P);

    std::cout << "   점 (0,0,5):" << std::endl;
    std::cout << "   카메라1 (원점): (" << pix1.x << ", " << pix1.y << ")" << std::endl;
    std::cout << "   카메라2 (x+2m): (" << pix2.x << ", " << pix2.y << ")" << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  ✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md에서 K 행렬/투영 이론 읽기 → quiz_easy 문제 1~3" << std::endl;
    std::cout << "   2. my_basic.cpp Step 1~4 구현 → quiz_easy 문제 4~5" << std::endl;
    std::cout << "   3. my_basic.cpp Step 5~6 구현 → quiz_medium 문제 1~5" << std::endl;
    std::cout << "   4. PRACTICE.md에서 실제 카메라 실습\n" << std::endl;

    return 0;
}
#endif  // PINHOLE_LIB_ONLY
