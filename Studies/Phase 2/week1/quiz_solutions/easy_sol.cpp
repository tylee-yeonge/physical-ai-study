/**
 * Phase 2 Week 1 - 핀홀 카메라 모델 기초 퀴즈 (정답)
 *
 * 다루는 개념:
 *   - 카메라 내부 파라미터 K 행렬의 구조와 물리적 의미
 *   - 3D → 2D 투영 (World → Camera → Normalized → Pixel)
 *   - FOV (시야각) 계산과 렌즈 특성 (광각 vs 망원)
 *   - 역투영 (Back-projection): 2D 픽셀 → 3D 광선
 *   - 가시성 판별 (Visibility Check): Z>0, 경계 내, FOV 이내
 *
 * 핀홀 카메라 모델은 SLAM의 가장 기본이 되는 수학적 모델이다.
 * 3D 세계를 2D 이미지로 변환하는 전체 과정을 이해해야
 * 이후 에피폴라 기하학, PnP, 삼각측량을 제대로 이해할 수 있다.
 *
 * 투영 과정:
 *   3D 월드 좌표 [Xw, Yw, Zw]
 *        ↓  [R|t] (외부 파라미터)
 *   3D 카메라 좌표 [Xc, Yc, Zc]
 *        ↓  원근 투영 (÷ Zc)
 *   정규화 좌표 [x', y'] = [Xc/Zc, Yc/Zc]
 *        ↓  K (내부 파라미터)
 *   2D 픽셀 좌표 [u, v] = [fx·x'+cx, fy·y'+cy]
 *
 * K 행렬 구조:
 *   K = [fx   0  cx]     fx, fy: 초점거리 (픽셀 단위)
 *       [ 0  fy  cy]     cx, cy: 주점 (이미지 중심)
 *       [ 0   0   1]
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

// 라디안 → 도(degree) 변환 계수
constexpr double kRadToDeg = 180.0 / CV_PI;

// K 행렬 분석 — 카메라 내부 파라미터의 물리적 의미
//
// K 행렬의 각 요소:
//   K = [fx   0  cx]
//       [ 0  fy  cy]
//       [ 0   0   1]
//
//   fx, fy: 초점거리 (focal length, 픽셀 단위)
//     - 물리적 초점거리 f와 센서 픽셀 크기 s의 관계: fx = f / sx
//     - fx = fy이면 정사각 픽셀 (대부분의 현대 카메라)
//     - 큰 fx → 좁은 시야각(망원), 작은 fx → 넓은 시야각(광각)
//
//   cx, cy: 주점 (principal point)
//     - 광축이 이미지 평면과 만나는 점
//     - 이상적으로는 이미지 중심 (width/2, height/2)
//     - 캘리브레이션으로 정확한 값 측정
//
// ★ 이미지 크기 추정: cx ≈ width/2, cy ≈ height/2
//   → width ≈ 2*cx, height ≈ 2*cy
//
// TODO: K.at<double>(row, col)로 각 요소를 추출하세요
void problem1_intrinsic_analysis()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: K 행렬 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat K = (cv::Mat_<double>(3, 3) << 500.0, 0.0, 320.0, 0.0, 500.0, 240.0, 0.0, 0.0, 1.0);

    std::cout << "주어진 카메라 행렬 K:" << std::endl;
    std::cout << K << "\n" << std::endl;

    // ✅ 정답: K 행렬에서 값 추출
    double fx = K.at<double>(0, 0);  // 500.0
    double fy = K.at<double>(1, 1);  // 500.0
    double cx = K.at<double>(0, 2);  // 320.0
    double cy = K.at<double>(1, 2);  // 240.0

    std::cout << "📊 추출한 값:" << std::endl;
    std::cout << "   fx (X방향 초점거리): " << fx << " 픽셀" << std::endl;
    std::cout << "   fy (Y방향 초점거리): " << fy << " 픽셀" << std::endl;
    std::cout << "   cx (주점 X좌표): " << cx << " 픽셀" << std::endl;
    std::cout << "   cy (주점 Y좌표): " << cy << " 픽셀" << std::endl;

    // ✅ 정답: 이미지 크기 추정 (주점은 보통 이미지 중심 근처)
    int estimated_width = static_cast<int>(cx * 2);    // 640
    int estimated_height = static_cast<int>(cy * 2);   // 480

    std::cout << "\n   추정 이미지 크기: " << estimated_width << " x " << estimated_height
              << std::endl;
}

// 3D → 2D 투영 단계별 계산 — 핀홀 카메라의 핵심 연산
//
// 투영 과정 3단계:
//
//   Step 1: 월드 → 카메라 좌표 변환
//     Pc = R · Pw + t
//     (R=I, t=0일 때 카메라 좌표 = 월드 좌표)
//
//   Step 2: 원근 투영 (Perspective Projection)
//     정규화 좌표: x' = Xc / Zc,  y' = Yc / Zc
//     ★ Zc로 나누는 것이 원근 효과의 핵심 — 먼 물체가 작게 보임
//
//   Step 3: 정규화 → 픽셀 좌표
//     u = fx · x' + cx
//     v = fy · y' + cy
//
// 행렬 형태로 한 번에:
//   λ · [u, v, 1]ᵀ = K · [R|t] · [Xw, Yw, Zw, 1]ᵀ
//   (λ = Zc, 동차 좌표의 스케일 팩터)
//
// TODO: P = (2, 1, 5)를 각 단계별로 투영하세요
void problem2_projection_steps()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 투영 단계별 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 3D 점
    double Xw = 2.0, Yw = 1.0, Zw = 5.0;
    std::cout << "3D 점 P = (" << Xw << ", " << Yw << ", " << Zw << ")\n" << std::endl;

    // K 파라미터
    double fx = 600.0, fy = 600.0;
    double cx = 400.0, cy = 300.0;

    // Step 1: 월드 → 카메라 (R=I, t=0이므로 동일)
    // ✅ 정답: R=I, t=0이므로 카메라 좌표 = 월드 좌표
    double Xc = Xw;  // 2.0
    double Yc = Yw;  // 1.0
    double Zc = Zw;  // 5.0

    std::cout << "Step 1 (월드→카메라): Pc = (" << Xc << ", " << Yc << ", " << Zc << ")"
              << std::endl;

    // Step 2: 원근 투영 (정규화 좌표)
    // ✅ 정답: Zc로 나누기
    double x_norm = Xc / Zc;  // 2/5 = 0.4
    double y_norm = Yc / Zc;  // 1/5 = 0.2

    std::cout << "Step 2 (원근 투영): x'=" << x_norm << ", y'=" << y_norm << std::endl;

    // Step 3: 정규화 → 픽셀
    // ✅ 정답: u = fx * x' + cx, v = fy * y' + cy
    double u = fx * x_norm + cx;  // 600*0.4 + 400 = 640
    double v = fy * y_norm + cy;  // 600*0.2 + 300 = 420

    std::cout << "Step 3 (픽셀 변환): pixel = (" << u << ", " << v << ")" << std::endl;
}

// FOV (시야각) 계산 — 초점거리와 시야각의 반비례 관계
//
// FOV 공식:
//   FOV_x = 2 · arctan(width / (2 · fx))
//
// 기하학적 이해:
//                     width/2
//                  ┌───────────┐
//                  │           │
//          fx      │  이미지   │
//   ◎──────────────┤  평면    │
//   카메라 중심     │           │
//                  └───────────┘
//                  ←───FOV/2───→
//
//   tan(FOV/2) = (width/2) / fx
//   → FOV = 2 · arctan(width / (2·fx))
//
// fx와 FOV의 관계:
//   - fx ↑ (초점거리 길다) → FOV ↓ (시야 좁다) → 망원 렌즈
//   - fx ↓ (초점거리 짧다) → FOV ↑ (시야 넓다) → 광각 렌즈
//
// ★ SLAM에서:
//   - 광각(FOV>90°): 넓은 시야, 특징점 많이 보임, 왜곡 큼
//   - 망원(FOV<30°): 좁은 시야, 먼 물체 상세, 빠른 움직임에 약함
//
// TODO: 두 카메라의 FOV를 계산하고 광각/망원을 판별하세요
void problem3_fov_calculation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: FOV 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    int image_width = 640;

    // 카메라 A
    double fx_A = 300.0;
    // ✅ 정답: FOV = 2 * arctan(width / (2*fx)) * 180/π
    double fov_A = 2.0 * std::atan2(image_width, 2.0 * fx_A) * kRadToDeg;

    // 카메라 B
    double fx_B = 1200.0;
    double fov_B = 2.0 * std::atan2(image_width, 2.0 * fx_B) * kRadToDeg;

    std::cout << "카메라 A (fx=" << fx_A << "): FOV = " << fov_A << "°" << std::endl;
    std::cout << "카메라 B (fx=" << fx_B << "): FOV = " << fov_B << "°" << std::endl;

    // ✅ 정답: FOV > 90° → 광각, FOV < 30° → 망원
    std::string camera_A_type = "광각";   // FOV ≈ 93.7°
    std::string camera_B_type = "망원";   // FOV ≈ 29.9°

    std::cout << "\n📋 판별 결과:" << std::endl;
    std::cout << "   카메라 A: " << camera_A_type << std::endl;
    std::cout << "   카메라 B: " << camera_B_type << std::endl;

    std::cout << "\n💡 힌트:" << std::endl;
    std::cout << "   fx 크다 → FOV 작다 → 망원" << std::endl;
    std::cout << "   fx 작다 → FOV 크다 → 광각" << std::endl;
}

// 역투영 (Back-projection) — 2D 픽셀에서 3D 광선 방향 복원
//
// 투영의 역연산:
//   픽셀 (u, v) → 정규화 좌표 → 3D 광선 방향
//
//   x' = (u - cx) / fx
//   y' = (v - cy) / fy
//   광선 방향: d = [x', y', 1]ᵀ (정규화)
//
// 행렬 형태:
//   d = K⁻¹ · [u, v, 1]ᵀ
//
// 기하학적 의미:
//   - 2D 픽셀은 깊이 정보를 잃은 것
//   - 역투영은 "이 픽셀이 바라보는 방향"만 복원
//   - 실제 3D 점은 광선 위 어딘가: P = O + λ·d (λ = 깊이)
//
// 특수 케이스:
//   - 이미지 중심 (cx, cy) → 정규화 (0, 0) → 카메라 정면 [0, 0, 1]
//   - 좌상단 (0, 0) → 음의 x', y' → 왼쪽 위 방향
//
// ★ SLAM에서 삼각측량은 두 카메라의 역투영 광선의 교점을 구하는 것
//
// TODO: 두 픽셀의 정규화 좌표와 광선 방향을 계산하세요
void problem4_back_projection()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 역투영" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double fx = 600.0, fy = 600.0;
    double cx = 400.0, cy = 300.0;

    // 테스트 픽셀 1: 이미지 중심
    double u1 = 400.0, v1 = 300.0;

    // ✅ 정답: x' = (u - cx) / fx, y' = (v - cy) / fy
    double x1_norm = (u1 - cx) / fx;  // 0.0
    double y1_norm = (v1 - cy) / fy;  // 0.0

    std::cout << "픽셀 1 (" << u1 << ", " << v1 << ") = 이미지 중심:" << std::endl;
    std::cout << "   정규화 좌표: (" << x1_norm << ", " << y1_norm << ")" << std::endl;
    std::cout << "   광선 방향: [" << x1_norm << ", " << y1_norm << ", 1]" << std::endl;

    // 테스트 픽셀 2: 이미지 좌상단
    double u2 = 0.0, v2 = 0.0;

    // ✅ 정답
    double x2_norm = (u2 - cx) / fx;  // -0.667
    double y2_norm = (v2 - cy) / fy;  // -0.5

    std::cout << "\n픽셀 2 (" << u2 << ", " << v2 << ") = 좌상단:" << std::endl;
    std::cout << "   정규화 좌표: (" << x2_norm << ", " << y2_norm << ")" << std::endl;
    std::cout << "   광선 방향: [" << x2_norm << ", " << y2_norm << ", 1]" << std::endl;

    std::cout << "\n💡 힌트:" << std::endl;
    std::cout << "   이미지 중심 → 정규화 좌표 (0, 0) → 카메라 정면 [0, 0, 1]" << std::endl;
    std::cout << "   좌상단 → 음의 X, 음의 Y → 왼쪽 위를 가리킴" << std::endl;
}

// 가시성 판별 (Visibility Check) — SLAM에서 3D 점 관리의 기초
//
// 3D 점이 카메라 이미지에 보이려면 3가지 조건을 모두 만족해야 한다:
//
//   조건 1: 카메라 앞에 있어야 함 (Zc > 0)
//     - 카메라 뒤의 점은 물리적으로 볼 수 없음
//     - Zc ≤ 0이면 투영 자체가 의미 없음
//
//   조건 2: 투영된 픽셀이 이미지 경계 내
//     - 0 ≤ u < width && 0 ≤ v < height
//     - 이미지 밖으로 투영되면 관측 불가
//
//   조건 3: FOV 범위 이내
//     - atan2(|Xc|, Zc) < half_fov_x
//     - atan2(|Yc|, Zc) < half_fov_y
//     - 극단적 각도에서는 렌즈 왜곡이 심해 신뢰 불가
//
// ★ SLAM에서의 활용:
//   - 맵 포인트 추적 시 현재 카메라에서 보이는 점만 선별
//   - 가시성 실패 점은 매칭 후보에서 제외 → 효율성 향상
//   - 키프레임 선택 시 공유 가시 점 수로 공동 관측 판단
//
// TODO: 5개 테스트 점에 대해 3가지 조건의 통과/실패를 확인하세요
void problem5_visibility_check()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: 가시성 판별 (Visibility)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double fx = 500.0, fy = 500.0;
    double cx = 320.0, cy = 240.0;
    int image_width = 640, image_height = 480;

    // FOV 계산
    double fov_x = 2.0 * std::atan2(image_width, 2.0 * fx) * kRadToDeg;
    double fov_y = 2.0 * std::atan2(image_height, 2.0 * fy) * kRadToDeg;
    double half_fov_x_rad = std::atan2(image_width, 2.0 * fx);
    double half_fov_y_rad = std::atan2(image_height, 2.0 * fy);

    std::cout << "카메라 설정:" << std::endl;
    std::cout << "   이미지 크기: " << image_width << " x " << image_height << std::endl;
    std::cout << "   FOV: " << fov_x << "° x " << fov_y << "°\n" << std::endl;

    // 테스트 점 (R=I, t=0이므로 카메라 좌표 = 월드 좌표)
    struct TestPoint
    {
        double x, y, z;
        std::string description;
    };

    std::vector<TestPoint> test_points = {
        {0, 0, 5, "정면 중앙"},
        {3, 0, 5, "오른쪽"},
        {10, 0, 5, "멀리 오른쪽"},
        {0, 0, -5, "카메라 뒤"},
        {0, 5, 5, "위쪽 멀리"},
    };

    std::cout << "가시성 테스트:" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (const auto& pt : test_points)
    {
        std::cout << "점 (" << pt.x << ", " << pt.y << ", " << pt.z
                  << ") - " << pt.description << ":" << std::endl;

        // ✅ 정답: 조건 1 - Zc > 0
        bool cond1 = pt.z > 0;

        // ✅ 정답: 조건 2 - 이미지 경계 내
        double u = 0.0, v = 0.0;
        bool cond2 = false;
        if (cond1)
        {
            u = fx * pt.x / pt.z + cx;
            v = fy * pt.y / pt.z + cy;
            cond2 = (u >= 0 && u < image_width && v >= 0 && v < image_height);
        }

        // ✅ 정답: 조건 3 - FOV 범위 이내
        bool cond3 = false;
        if (cond1)
        {
            double angle_x = std::atan2(std::abs(pt.x), pt.z);
            double angle_y = std::atan2(std::abs(pt.y), pt.z);
            cond3 = (angle_x < half_fov_x_rad && angle_y < half_fov_y_rad);
        }

        std::cout << "   조건1 (Zc>0):     " << (cond1 ? "PASS" : "FAIL") << std::endl;
        std::cout << "   조건2 (경계 내):   " << (cond2 ? "PASS" : "FAIL");
        if (cond1)
        {
            std::cout << "  (u=" << u << ", v=" << v << ")";
        }
        std::cout << std::endl;
        std::cout << "   조건3 (FOV 이내): " << (cond3 ? "PASS" : "FAIL") << std::endl;

        bool visible = cond1 && cond2 && cond3;
        std::cout << "   → 결과: " << (visible ? "보임" : "보이지 않음") << "\n" << std::endl;
    }

    std::cout << "💡 힌트:" << std::endl;
    std::cout << "   SLAM에서 가시성 체크는 특징점 추적 시 필수!" << std::endl;
    std::cout << "   3가지 조건 중 하나라도 실패하면 해당 점은 무시" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 1 Quiz - Easy (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_intrinsic_analysis();
    problem2_projection_steps();
    problem3_fov_calculation();
    problem4_back_projection();
    problem5_visibility_check();

    return 0;
}
