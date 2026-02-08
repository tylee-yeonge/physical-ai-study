/**
 * Phase 2 Week 1 - 핀홀 카메라 모델 기초 퀴즈
 *
 * 이 퀴즈는 핀홀 카메라 모델의 기본 개념을 확인합니다.
 * 각 문제를 풀고, main() 함수를 실행하여 정답을 확인하세요.
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

/**
 * 문제 1: 카메라 내부 파라미터 K 분석
 *
 * 다음 카메라 행렬 K가 주어졌을 때, 각 요소를 추출하고
 * 이 카메라의 특성을 분석하세요.
 *
 * K = [500.0,   0.0, 320.0]
 *     [  0.0, 500.0, 240.0]
 *     [  0.0,   0.0,   1.0]
 *
 * TODO: 아래 함수를 완성하세요.
 */
void problem1_intrinsic_analysis() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: K 행렬 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat K = (cv::Mat_<double>(3, 3) <<
        500.0, 0.0, 320.0,
        0.0, 500.0, 240.0,
        0.0, 0.0, 1.0);

    std::cout << "주어진 카메라 행렬 K:" << std::endl;
    std::cout << K << "\n" << std::endl;

    // TODO: K 행렬에서 값을 추출하세요
    // 힌트: K.at<double>(row, col)로 접근
    double fx = 0.0;  // TODO: fx 추출
    double fy = 0.0;  // TODO: fy 추출
    double cx = 0.0;  // TODO: cx 추출
    double cy = 0.0;  // TODO: cy 추출

    std::cout << "📊 추출한 값:" << std::endl;
    std::cout << "   fx (X방향 초점거리): " << fx << " 픽셀" << std::endl;
    std::cout << "   fy (Y방향 초점거리): " << fy << " 픽셀" << std::endl;
    std::cout << "   cx (주점 X좌표): " << cx << " 픽셀" << std::endl;
    std::cout << "   cy (주점 Y좌표): " << cy << " 픽셀" << std::endl;

    // TODO: 이 카메라의 이미지 크기를 추정하세요 (cx, cy로부터)
    // 힌트: 주점은 보통 이미지 중심 근처
    int estimated_width = 0;   // TODO
    int estimated_height = 0;  // TODO

    std::cout << "\n   추정 이미지 크기: " << estimated_width << " x "
              << estimated_height << std::endl;
}

/**
 * 문제 2: 3D → 2D 투영 단계별 계산
 *
 * 3D 점 P = (2, 1, 5)을 다음 카메라로 투영하세요.
 *
 * K = [600, 0, 400; 0, 600, 300; 0, 0, 1]
 * R = I (단위 행렬)
 * t = [0, 0, 0]
 *
 * TODO: 각 단계를 직접 계산하세요.
 */
void problem2_projection_steps() {
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
    // TODO: Pc = R * Pw + t 계산
    double Xc = 0.0;  // TODO
    double Yc = 0.0;  // TODO
    double Zc = 0.0;  // TODO

    std::cout << "Step 1 (월드→카메라): Pc = (" << Xc << ", " << Yc << ", " << Zc << ")" << std::endl;

    // Step 2: 원근 투영 (정규화 좌표)
    // TODO: x' = Xc/Zc, y' = Yc/Zc
    double x_norm = 0.0;  // TODO
    double y_norm = 0.0;  // TODO

    std::cout << "Step 2 (원근 투영): x'=" << x_norm << ", y'=" << y_norm << std::endl;

    // Step 3: 정규화 → 픽셀
    // TODO: u = fx * x' + cx, v = fy * y' + cy
    double u = 0.0;  // TODO
    double v = 0.0;  // TODO

    std::cout << "Step 3 (픽셀 변환): pixel = (" << u << ", " << v << ")" << std::endl;
}

/**
 * 문제 3: FOV (시야각) 계산
 *
 * 두 카메라의 FOV를 계산하고 비교하세요.
 *
 * 카메라 A: fx=300, 이미지 너비=640
 * 카메라 B: fx=1200, 이미지 너비=640
 *
 * 어떤 카메라가 광각이고, 어떤 카메라가 망원인가요?
 *
 * TODO: FOV를 계산하세요.
 */
void problem3_fov_calculation() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: FOV 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    int image_width = 640;

    // 카메라 A
    double fx_A = 300.0;
    // TODO: FOV = 2 * arctan(width / (2 * fx)) * 180 / PI
    double fov_A = 0.0;  // TODO

    // 카메라 B
    double fx_B = 1200.0;
    double fov_B = 0.0;  // TODO

    std::cout << "카메라 A (fx=" << fx_A << "): FOV = " << fov_A << "°" << std::endl;
    std::cout << "카메라 B (fx=" << fx_B << "): FOV = " << fov_B << "°" << std::endl;

    // TODO: 어떤 카메라가 광각인지 판별
    std::string camera_A_type = "UNKNOWN";  // "광각" 또는 "망원"
    std::string camera_B_type = "UNKNOWN";  // "광각" 또는 "망원"

    std::cout << "\n📋 판별 결과:" << std::endl;
    std::cout << "   카메라 A: " << camera_A_type << std::endl;
    std::cout << "   카메라 B: " << camera_B_type << std::endl;

    std::cout << "\n💡 힌트:" << std::endl;
    std::cout << "   fx 크다 → FOV 작다 → 망원" << std::endl;
    std::cout << "   fx 작다 → FOV 크다 → 광각" << std::endl;
}

/**
 * 문제 4: 역투영 (Back-projection) 계산
 *
 * 픽셀 (400, 300)에서 3D 광선 방향을 역투영하세요.
 *
 * K = [600, 0, 400; 0, 600, 300; 0, 0, 1]
 *
 * 이 픽셀은 이미지 중심입니다. 어떤 방향을 가리키나요?
 *
 * TODO: 역투영을 계산하세요.
 */
void problem4_back_projection() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 역투영" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double fx = 600.0, fy = 600.0;
    double cx = 400.0, cy = 300.0;

    // 테스트 픽셀 1: 이미지 중심
    double u1 = 400.0, v1 = 300.0;

    // TODO: 정규화 좌표 계산
    // x' = (u - cx) / fx
    // y' = (v - cy) / fy
    double x1_norm = 0.0;  // TODO
    double y1_norm = 0.0;  // TODO

    std::cout << "픽셀 1 (" << u1 << ", " << v1 << ") = 이미지 중심:" << std::endl;
    std::cout << "   정규화 좌표: (" << x1_norm << ", " << y1_norm << ")" << std::endl;
    std::cout << "   광선 방향: [" << x1_norm << ", " << y1_norm << ", 1]" << std::endl;

    // 테스트 픽셀 2: 이미지 좌상단
    double u2 = 0.0, v2 = 0.0;

    double x2_norm = 0.0;  // TODO
    double y2_norm = 0.0;  // TODO

    std::cout << "\n픽셀 2 (" << u2 << ", " << v2 << ") = 좌상단:" << std::endl;
    std::cout << "   정규화 좌표: (" << x2_norm << ", " << y2_norm << ")" << std::endl;
    std::cout << "   광선 방향: [" << x2_norm << ", " << y2_norm << ", 1]" << std::endl;

    std::cout << "\n💡 힌트:" << std::endl;
    std::cout << "   이미지 중심 → 정규화 좌표 (0, 0) → 카메라 정면 [0, 0, 1]" << std::endl;
    std::cout << "   좌상단 → 음의 X, 음의 Y → 왼쪽 위를 가리킴" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 1 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_intrinsic_analysis();
    problem2_projection_steps();
    problem3_fov_calculation();
    problem4_back_projection();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
