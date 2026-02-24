/**
 * Phase 2 Week 2 - 카메라 캘리브레이션 기초 퀴즈
 *
 * 다루는 개념:
 *   - K 행렬 (fx, fy, cx, cy)의 물리적 의미
 *   - 렌즈 왜곡: 방사 왜곡 (k1, k2, k3) vs 접선 왜곡 (p1, p2)
 *   - 재투영 오차 (RMS)로 캘리브레이션 품질 평가
 *   - 체커보드 캘리브레이션의 3D-2D 대응점 생성
 *   - 이미지 위치별 왜곡 크기 분포 분석
 *
 * 카메라 캘리브레이션은 실제 카메라의 내부 파라미터(K)와 렌즈 왜곡 계수를
 * 정밀하게 측정하는 과정이다. SLAM의 정확도는 캘리브레이션 품질에 직결된다.
 *
 * 왜곡 모델 (Brown-Conrady):
 *   방사 왜곡: x_dist = x · (1 + k1·r² + k2·r⁴ + k3·r⁶)
 *   접선 왜곡: x_dist += 2·p1·x·y + p2·(r² + 2·x²)
 *   여기서 r² = x² + y² (정규화 좌표 기준)
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

// K 행렬 요소 추출 — 캘리브레이션 결과물의 첫 번째 활용
//
// 캘리브레이션으로 얻는 K 행렬:
//   K = [fx   0  cx]     K.at<double>(0,0) = fx
//       [ 0  fy  cy]     K.at<double>(1,1) = fy
//       [ 0   0   1]     K.at<double>(0,2) = cx, K.at<double>(1,2) = cy
//
// 각 요소의 캘리브레이션 관점 의미:
//   - fx, fy: 렌즈와 센서로 결정됨 (물리적 초점거리 ÷ 픽셀 크기)
//   - cx, cy: 렌즈 광축과 센서 중심의 오프셋 (제조 공차)
//   - fx ≠ fy: 픽셀이 정사각형이 아닌 경우 (드문 경우)
//
// ★ 캘리브레이션 없이 추정: cx ≈ width/2, cy ≈ height/2
//   정밀한 SLAM에서는 반드시 캘리브레이션된 K 사용
//
// TODO: K.at<double>(row, col)로 fx, fy, cx, cy를 추출하세요
void problem1_camera_matrix()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 카메라 행렬 K 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    std::cout << "주어진 카메라 행렬 K:" << std::endl;
    std::cout << K << "\n" << std::endl;

    // TODO: K 행렬에서 값을 추출하세요
    // 힌트: K.at<double>(row, col)로 접근
    double fx = 0.0;  // TODO: fx 값 추출
    double fy = 0.0;  // TODO: fy 값 추출
    double cx = 0.0;  // TODO: cx 값 추출
    double cy = 0.0;  // TODO: cy 값 추출

    std::cout << "📊 추출한 값:" << std::endl;
    std::cout << "   fx (X방향 초점거리): " << fx << " 픽셀" << std::endl;
    std::cout << "   fy (Y방향 초점거리): " << fy << " 픽셀" << std::endl;
    std::cout << "   cx (주점 X좌표): " << cx << " 픽셀" << std::endl;
    std::cout << "   cy (주점 Y좌표): " << cy << " 픽셀" << std::endl;
}

// 왜곡 유형 판별 — k1의 부호로 배럴 vs 핀쿠션 구분
//
// OpenCV 왜곡 계수 순서: dist = [k1, k2, p1, p2, k3]
//   - k1, k2, k3: 방사 왜곡 (radial) — 렌즈 곡률로 인한 왜곡
//   - p1, p2: 접선 왜곡 (tangential) — 렌즈 정렬 오차
//
// 방사 왜곡 공식:
//   r_distorted = r · (1 + k1·r² + k2·r⁴ + k3·r⁶)
//
// k1의 부호에 따른 왜곡 유형:
//   k1 < 0: 배럴 왜곡 (Barrel Distortion)
//     - 이미지가 바깥으로 부풀어 보임
//     - 광각 렌즈에서 흔함 (GoPro, 어안 렌즈)
//     ┌──────┐     ╭──────╮
//     │ □□□□ │ →   │ ╲  ╱ │  (직선이 바깥으로 휘어짐)
//     │ □□□□ │     │ ╱  ╲ │
//     └──────┘     ╰──────╯
//
//   k1 > 0: 핀쿠션 왜곡 (Pincushion Distortion)
//     - 이미지가 안쪽으로 당겨 보임
//     - 망원 렌즈에서 흔함
//
// ★ 대부분의 실제 카메라는 k1이 지배적이고, k2는 보정 역할
//
// TODO: k1의 부호로 왜곡 유형을 판별하세요
void problem2_distortion_type()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 왜곡 유형 판별" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat dist = (cv::Mat_<double>(1, 5) << -0.28, 0.07, 0.0, 0.0, 0.0);

    std::cout << "주어진 왜곡 계수: " << dist << std::endl;

    double k1 = dist.at<double>(0);
    double k2 = dist.at<double>(1);

    std::cout << "   k1 = " << k1 << std::endl;
    std::cout << "   k2 = " << k2 << "\n" << std::endl;

    // TODO: k1의 부호로 왜곡 유형 판별
    std::string distortion_type = "UNKNOWN";  // "BARREL" 또는 "PINCUSHION"으로 변경

    // 힌트:
    // - k1 < 0: 배럴 왜곡 (광각 렌즈, 이미지 바깥으로 밀림)
    // - k1 > 0: 핀쿠션 왜곡 (망원 렌즈, 이미지 안쪽으로 당김)

    std::cout << "📋 판별 결과: " << distortion_type << " 왜곡" << std::endl;
}

// 재투영 오차 (RMS) 해석 — 캘리브레이션 품질의 정량적 지표
//
// RMS (Root Mean Square) 재투영 오차:
//   RMS = √(Σᵢ ||p_obs_i - p_proj_i||² / N)
//   = 모든 캘리브레이션 점에서 "실제 관측"과 "모델 투영"의 평균 차이
//
// 캘리브레이션 품질 기준:
//   RMS < 0.5 px : 우수 (EXCELLENT) — 정밀 측정/3D 복원에 적합
//   0.5 ≤ RMS < 1.0 : 양호 (GOOD) — 일반적 Visual SLAM에 적합
//   RMS ≥ 1.0 : 불량 (POOR) — 재캘리브레이션 필요
//
// RMS가 높은 원인:
//   - 체커보드 검출 오류 (블러, 부분 가려짐)
//   - 왜곡 모델 차수 부족 (어안 렌즈에 5차 모델 사용)
//   - 캘리브레이션 이미지 수 부족 또는 다양성 부족
//   - 체커보드 인쇄 품질 문제
//
// ★ cv::calibrateCamera의 반환값이 바로 RMS
//
// TODO: RMS 값에 따라 품질을 EXCELLENT, GOOD, POOR로 분류하세요
void problem3_rms_evaluation()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: RMS 재투영 오차 평가" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::vector<double> rms_values = {0.35, 0.85, 1.25};
    std::vector<std::string> scenarios = {"시나리오 A", "시나리오 B", "시나리오 C"};

    for (size_t i = 0; i < rms_values.size(); i++)
    {
        double rms = rms_values[i];
        std::cout << scenarios[i] << ": RMS = " << rms << " 픽셀" << std::endl;

        // TODO: RMS 값에 따라 품질 평가
        std::string quality = "UNKNOWN";  // "EXCELLENT", "GOOD", "FAIR", "POOR" 중 선택

        // 힌트:
        // - RMS < 0.5: 우수 (EXCELLENT)
        // - 0.5 <= RMS < 1.0: 양호 (GOOD)
        // - RMS >= 1.0: 재캘리브레이션 권장 (POOR)

        std::cout << "   → 품질: " << quality << "\n" << std::endl;
    }

    std::cout << "💡 일반적인 기준:" << std::endl;
    std::cout << "   - RMS < 0.5: 매우 우수, 대부분의 응용에 적합" << std::endl;
    std::cout << "   - 0.5 ≤ RMS < 1.0: 양호, 일반적인 SLAM에 사용 가능" << std::endl;
    std::cout << "   - RMS ≥ 1.0: 불량, 재캘리브레이션 필요" << std::endl;
}

// 체커보드 3D-2D 대응점 — 캘리브레이션의 입력 데이터 생성
//
// 체커보드 캘리브레이션 원리:
//   1. 체커보드는 평면 (Z=0) 위의 격자 점 → 3D 좌표를 정확히 안다
//   2. 카메라로 촬영 → 2D 이미지에서 코너 자동 검출
//   3. 3D-2D 대응 쌍들로 K와 왜곡 계수를 최적화
//
// 3D 객체 점 생성 규칙:
//   체커보드를 XY 평면에 놓으면:
//   (0, 0, 0), (square_size, 0, 0), (2*square_size, 0, 0), ...
//   (0, square_size, 0), (square_size, square_size, 0), ...
//   Z = 0 고정 (평면 가정)
//
// 9×6 체커보드, 한 칸 30mm일 때:
//   총 9×6 = 54개 내부 코너
//   첫 행: (0,0,0), (30,0,0), (60,0,0), ..., (240,0,0)
//
// ★ 체커보드의 실제 크기가 중요: SLAM에서 절대 스케일을 결정
//   square_size를 mm 대신 m로 설정하면 전체 스케일이 달라짐
//
// TODO: 첫 번째 행(y=0)의 9개 3D 좌표를 생성하세요
void problem4_3d_2d_correspondence()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 3D-2D 대응점 생성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    int board_width = 9;
    int board_height = 6;
    float square_size = 30.0f;  // mm

    std::cout << "체커보드 설정:" << std::endl;
    std::cout << "   - 내부 코너: " << board_width << " × " << board_height << std::endl;
    std::cout << "   - 한 칸 크기: " << square_size << " mm\n" << std::endl;

    // TODO: 첫 번째 행 (y=0)의 3D 점들을 생성하세요
    std::vector<cv::Point3f> first_row;

    // 힌트: (0, 0, 0), (30, 0, 0), (60, 0, 0), ..., (240, 0, 0)
    // for 문을 사용하여 board_width 개의 점 생성

    std::cout << "첫 번째 행의 3D 좌표:" << std::endl;
    for (size_t i = 0; i < first_row.size(); i++)
    {
        std::cout << "   점 " << i << ": (" << first_row[i].x << ", " << first_row[i].y << ", "
                  << first_row[i].z << ") mm" << std::endl;
    }
}

// 왜곡 크기 분포 분석 — 이미지 위치별 왜곡의 크기 비교
//
// 방사 왜곡 계산 과정:
//   1. 픽셀→정규화: x = (u - cx) / fx,  y = (v - cy) / fy
//   2. r² 계산: r² = x² + y²
//   3. 왜곡 계수: radial = 1 + k1·r² + k2·r⁴
//   4. 왜곡된 정규화: x_dist = x · radial,  y_dist = y · radial
//   5. 다시 픽셀로: u_dist = fx · x_dist + cx,  v_dist = fy · y_dist + cy
//   6. 변위: displacement = √((u - u_dist)² + (v - v_dist)²)
//
// 왜곡의 공간적 분포:
//   - 이미지 중심 (cx, cy): r = 0 → 왜곡 = 0
//   - 이미지 가장자리: r 큼 → 왜곡 큼 (수십 px 가능)
//   - 모서리: r 최대 → 왜곡 최대
//
//   ┌───────────────────────┐
//   │ 많이 왜곡   약간   많이   │  ← 왜곡은 중심에서 방사형으로 증가
//   │            왜곡        │
//   │         (0) 중심       │
//   │            왜곡        │
//   │ 많이 왜곡   약간   많이   │
//   └───────────────────────┘
//
// ★ 왜곡 보정(undistortion)을 안 하면 직선이 휘어 보여서
//   에피폴라 제약, PnP 등 모든 기하학적 연산이 부정확해짐
//
// TODO: 7개 격자 점에서 왜곡 변위를 계산하세요
void problem5_distortion_magnitude()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: 왜곡 크기 분포 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    double fx = 500.0, fy = 500.0;
    double cx = 320.0, cy = 240.0;
    double k1 = -0.3, k2 = 0.1;

    // 이미지 격자 점 (5x5)
    struct GridPoint
    {
        double u, v;
        std::string location;
    };

    std::vector<GridPoint> grid_points = {
        {320, 240, "중심"},    {160, 120, "좌상 1/4"},    {480, 360, "우하 1/4"},
        {0, 0, "좌상 모서리"}, {640, 480, "우하 모서리"}, {320, 0, "상단 중앙"},
        {0, 240, "좌측 중앙"},
    };

    std::cout << "왜곡 변위 분석 (k1=" << k1 << ", k2=" << k2 << "):\n" << std::endl;
    std::cout << "  위치              |  (u, v)       | r^2    | 변위(px)" << std::endl;
    std::cout << std::string(65, '-') << std::endl;

    for (const auto& pt : grid_points)
    {
        // TODO: 정규화 좌표 계산
        double x = 0.0;  // TODO
        double y = 0.0;  // TODO

        // TODO: r^2 계산
        double r2 = 0.0;  // TODO

        // TODO: 방사 왜곡 계수 계산
        double radial = 0.0;  // TODO

        // TODO: 왜곡된 정규화 좌표 계산
        double x_dist = 0.0;  // TODO
        double y_dist = 0.0;  // TODO

        // TODO: 왜곡된 픽셀 좌표 계산
        double u_dist = 0.0;  // TODO
        double v_dist = 0.0;  // TODO

        // TODO: 변위 크기 계산 (원본 픽셀과 왜곡 픽셀의 거리)
        double displacement = 0.0;  // TODO

        std::cout << "  " << pt.location;
        // 정렬을 위한 패딩
        for (size_t i = pt.location.size(); i < 18; i++)
        {
            std::cout << " ";
        }
        std::cout << "| (" << (int)pt.u << ", " << (int)pt.v << ")"
                  << "\t| " << r2 << "\t | " << displacement << std::endl;
    }

    std::cout << "\n💡 관찰 포인트:" << std::endl;
    std::cout << "   - 중심(r=0)에서 왜곡은 0" << std::endl;
    std::cout << "   - 모서리로 갈수록 r^2이 커지고 왜곡도 증가" << std::endl;
    std::cout << "   - k1 < 0이므로 배럴 왜곡 (바깥으로 밀림)" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 2 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_camera_matrix();
    problem2_distortion_type();
    problem3_rms_evaluation();
    problem4_3d_2d_correspondence();
    problem5_distortion_magnitude();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
