#include "basic.h"
#include <iostream>
#include <chrono>
#include <random>

// RMS 재투영 오차 품질 등급 임계값
constexpr double kRmsExcellent = 0.3;  // 매우 우수
constexpr double kRmsGood = 0.5;       // 우수
constexpr double kRmsFair = 1.0;       // 양호 (이상이면 불량)

CameraCalibrationBasic::CameraCalibrationBasic(cv::Size boardSize, float squareSize)
    : boardSize_(boardSize), squareSize_(squareSize)
{
}

// RMS 재투영 오차(Re-projection Error)로 캘리브레이션 품질을 평가
//
// RMS 재투영 오차란?
//   calibrateCamera()가 추정한 K, dist, rvec, tvec으로
//   3D 점을 다시 2D에 투영했을 때, 실제 검출된 코너와의 거리 차이.
//   공식: RMS = sqrt( (1/N) · Σ ||p_detected - p_reprojected||² )
//
// 값이 작을수록 추정된 파라미터가 실제 카메라에 가까움.
// 일반적으로 0.5px 이하면 Depth 추정/3D 복원에 충분.
// 1.0px 이상이면 이미지 수 부족, 체커보드 각도 다양성 부족,
// 또는 초점 거리 변화 등의 문제를 의심해야 함.
std::string CameraCalibrationBasic::evaluateQuality(double rms)
{
    if (rms < kRmsExcellent)
    {
        return "✅ 매우 우수 (Excellent)";
    }
    else if (rms < kRmsGood)
    {
        return "✅ 우수 (Good)";
    }
    else if (rms < kRmsFair)
    {
        return "⚠️  양호 (Fair) - 사용 가능";
    }
    else
    {
        return "❌ 불량 (Poor) - 재캘리브레이션 권장";
    }
}

// 체커보드의 3D 월드 좌표를 생성
//
// 캘리브레이션의 핵심 아이디어:
//   "알려진 3D 좌표"와 "검출된 2D 좌표"의 대응 관계로부터
//   카메라 파라미터(K, dist)를 역으로 추정하는 것.
//   이 함수가 "알려진 3D 좌표" 부분을 담당.
//
// 좌표계 설정:
//   체커보드 평면을 Z=0으로 놓고, 좌상단 내부 코너를 원점(0,0,0)으로 설정.
//   squareSize_=30mm, boardSize_=9×6이면:
//     (0,0,0), (30,0,0), (60,0,0), ... (270,0,0)   ← 1행: j=0~8
//     (0,30,0), (30,30,0), ...                       ← 2행: i=1
//     ...
//     (0,150,0), (30,150,0), ... (270,150,0)         ← 6행: i=5
//   총 9×6 = 54개 점, 모두 Z=0 평면 위.
//
// ★ 이 좌표는 모든 이미지에서 동일!
//   체커보드는 물리적으로 고정된 크기이므로, 카메라를 어느 각도에서
//   촬영하든 3D 좌표는 변하지 않음. 변하는 것은 카메라의 위치/자세(rvec, tvec)뿐.
std::vector<cv::Point3f> CameraCalibrationBasic::generateObjectPoints()
{
    std::vector<cv::Point3f> objectPoints;

    for (int i = 0; i < boardSize_.height; i++)
    {
        for (int j = 0; j < boardSize_.width; j++)
        {
            objectPoints.push_back(cv::Point3f(j * squareSize_,  // X 좌표 (열 방향)
                                               i * squareSize_,  // Y 좌표 (행 방향)
                                               0.0f              // Z 좌표 (평면이므로 항상 0)
                                               ));
        }
    }

    return objectPoints;
}

void CameraCalibrationBasic::undistortImage(const cv::Mat& distorted, cv::Mat& undistorted,
                                            const cv::Mat& K, const cv::Mat& dist)
{
    // OpenCV의 왜곡 보정 함수 — 왜곡된 이미지를 왜곡 없는 이미지로 복원
    //
    // ★ 핵심: "역방향 매핑" 방식으로 동작
    //   순방향(왜곡→보정)은 출력에 빈 픽셀(hole)이 생기므로,
    //   출력 픽셀에서 역으로 입력 이미지를 찾아가는 역방향을 사용
    //
    // 출력 이미지의 모든 픽셀 (u, v)에 대해 다음 4단계를 수행:
    //
    // ① 정규화 좌표 변환:
    //    x = (u - cx) / fx,  y = (v - cy) / fy
    //
    // ② 왜곡 모델 적용 (radial + tangential):
    //    r² = x² + y²
    //    x_d = x·(1 + k1·r² + k2·r⁴ + k3·r⁶) + 2·p1·xy + p2·(r² + 2x²)
    //    y_d = y·(1 + k1·r² + k2·r⁴ + k3·r⁶) + p1·(r² + 2y²) + 2·p2·xy
    //    - 방사 왜곡(radial): 중심에서 멀수록 커짐 (k1, k2, k3)
    //    - 접선 왜곡(tangential): 렌즈-센서 비평행으로 발생 (p1, p2)
    //
    // ③ 왜곡된 픽셀 좌표 복원:
    //    u_src = fx · x_d + cx,  v_src = fy · y_d + cy
    //    → 원본(왜곡된) 이미지에서 읽어올 위치
    //
    // ④ 보간(interpolation):
    //    (u_src, v_src)는 소수점 좌표이므로 주변 픽셀을 bilinear 보간
    //
    // 내부적으로는 다음 두 함수를 순차 호출하는 것과 동일:
    //   initUndistortRectifyMap(K, dist, ...) → 매핑 테이블(map1, map2) 생성
    //   remap(distorted, undistorted, map1, map2) → 테이블로 이미지 변환
    // 같은 카메라로 여러 이미지를 보정할 때는 map을 한 번만 만들고
    // remap을 반복하는 것이 효율적 (undistort는 매번 map을 새로 계산)
    cv::undistort(distorted, undistorted, K, dist);
}

// 이미지에서 체커보드 내부 코너를 검출 — 캘리브레이션의 "2D 점" 획득 단계
//
// 전체 흐름:
//   ① 그레이스케일 변환 (코너 검출은 밝기 정보만 사용)
//   ② findChessboardCorners로 코너 대략적 위치 검출 (정수 픽셀 수준)
//   ③ cornerSubPix로 서브픽셀 정밀도까지 개선 (0.001 픽셀 수준)
//
// 반환: 검출 성공 여부 (false면 이 이미지는 캘리브레이션에서 제외)
bool CameraCalibrationBasic::detectChessboard(const cv::Mat& image,
                                              std::vector<cv::Point2f>& corners)
{
    // ① 그레이스케일 변환
    //    코너 검출 알고리즘은 밝기 그래디언트만 사용하므로 컬러 불필요.
    //    이미 그레이스케일이면 변환 생략.
    cv::Mat gray;
    if (image.channels() == 3)
    {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        gray = image.clone();
    }

    // ② 체커보드 코너 검출 — 흑백 사각형이 만나는 내부 교차점을 찾음
    //
    // 내부 동작:
    //   1. 적응형 이진화(adaptive threshold)로 흑/백 영역 분리
    //   2. 윤곽선(contour) 검출 → 사각형 후보 추출
    //   3. 사각형들의 연결 관계를 분석하여 체커보드 그리드 구성
    //   4. boardSize_(9×6)와 일치하는 그리드를 찾으면 성공
    //
    // 플래그 설명:
    //   ADAPTIVE_THRESH: 영역별로 다른 임계값 사용
    //     → 그림자나 조명 차이가 큰 환경에서 검출률 향상
    //   NORMALIZE_IMAGE: 이미지 밝기/대비를 정규화한 후 검출
    //     → 너무 어둡거나 밝은 이미지에서도 안정적으로 검출
    //   FAST_CHECK: 체커보드가 없는 이미지를 빠르게 걸러냄
    //     → 전체 알고리즘 실행 전 간단한 사전 체크로 속도 향상
    bool found = cv::findChessboardCorners(
        gray, boardSize_, corners,
        cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK);

    if (found)
    {
        // ③ 서브픽셀 정밀도로 코너 위치 개선
        //
        // findChessboardCorners는 정수 픽셀 수준의 대략적 위치만 반환.
        // 캘리브레이션 정확도를 위해 0.001 픽셀 수준까지 정밀화 필요.
        //
        // 원리: 코너 주변의 밝기 그래디언트(gradient) 방향 분석.
        //   진짜 코너에서는 그래디언트가 코너 점을 향해 수렴하는 성질 이용.
        //   수학적으로: 코너 q에 대해 주변 점 p에서의 그래디언트 ∇I(p)는
        //   (q-p)와 직교 → ∇I(p)ᵀ · (q-p) = 0 을 만족하는 q를 반복 탐색.
        //
        // 파라미터:
        //   Size(11,11): 탐색 윈도우 크기 (코너 주변 11×11 픽셀 영역 분석)
        //     → 너무 크면 인접 코너의 영향, 너무 작으면 정밀도 저하
        //   Size(-1,-1): dead zone 없음 (윈도우 중심부를 제외하지 않음)
        //   TermCriteria: 반복 종료 조건
        //     → 최대 30회 반복 또는 위치 변화가 0.001 픽셀 미만이면 중단
        cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER,
                                  30,     // 최대 반복 횟수
                                  0.001   // 정확도 epsilon (픽셀)
        );

        cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), criteria);
    }

    return found;
}

// 캘리브레이션 결과를 YAML 파일로 저장
//
// 저장하는 이유:
//   캘리브레이션은 시간이 오래 걸리는 작업이므로 한 번 수행 후
//   결과(K, dist)를 파일로 저장해두고, 이후 프로그램에서 불러와 사용.
//   Perception 파이프라인, 3D 복원, AR 등 모든 후속 작업에서 이 파일을 참조.
//
// YAML 형식 예시:
//   camera_matrix: !!opencv-matrix
//     rows: 3
//     cols: 3
//     data: [ 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0 ]
//   distortion_coefficients: !!opencv-matrix
//     data: [ -0.2, 0.05, 0.0, 0.0, 0.0 ]
//
// 불러올 때:
//   cv::FileStorage fs("calibration.yaml", cv::FileStorage::READ);
//   fs["camera_matrix"] >> K;
//   fs["distortion_coefficients"] >> dist;
void CameraCalibrationBasic::saveCalibration(const std::string& filename, const cv::Mat& K,
                                             const cv::Mat& dist, cv::Size imageSize)
{
    // OpenCV FileStorage — cv::Mat를 포함한 다양한 타입을 YAML/XML로 직렬화
    // WRITE 모드: 파일이 존재하면 덮어쓰기, 없으면 새로 생성
    cv::FileStorage fs(filename, cv::FileStorage::WRITE);

    if (!fs.isOpened())
    {
        std::cerr << "❌ 파일 저장 실패: " << filename << std::endl;
        return;
    }

    // 캘리브레이션 시점 기록 (재현성을 위해)
    time_t rawtime;
    time(&rawtime);

    // << 연산자로 키-값 쌍을 순서대로 기록
    // cv::Mat는 자동으로 rows, cols, type, data 구조로 직렬화됨
    fs << "calibration_time" << asctime(localtime(&rawtime));
    fs << "camera_matrix" << K;                    // 3×3 내부 행렬
    fs << "distortion_coefficients" << dist;       // 1×5 왜곡 계수
    fs << "image_width" << imageSize.width;        // 캘리브레이션에 사용된 해상도
    fs << "image_height" << imageSize.height;      // (다른 해상도에서는 K 스케일링 필요)

    fs.release();

    std::cout << "💾 캘리브레이션 결과 저장: " << filename << std::endl;
}

// 카메라 캘리브레이션 수행 — 3D↔2D 대응으로부터 카메라 파라미터 추정
//
// 캘리브레이션이 푸는 문제:
//   "여러 각도에서 촬영한 체커보드 이미지들로부터
//    카메라의 내부 파라미터(K)와 렌즈 왜곡 계수(dist)를 추정"
//
// 입력:
//   - imagePoints: 각 이미지에서 검출된 2D 코너 좌표들 (detectChessboard 결과)
//   - imageSize: 이미지 해상도 (K의 cx, cy 초기값 추정에 사용)
//
// 출력:
//   - cameraMatrix: 3×3 카메라 내부 행렬 K = [fx 0 cx; 0 fy cy; 0 0 1]
//   - distCoeffs: 왜곡 계수 [k1, k2, p1, p2, k3]
//   - 반환값: RMS 재투영 오차 (캘리브레이션 품질 지표)
//
// 내부 알고리즘 (Zhang's Method):
//   ① 호모그래피 추정: 각 이미지에서 3D 평면↔2D 대응으로 호모그래피 H 계산
//   ② 초기 K 추정: 여러 H로부터 연립방정식을 세워 fx, fy, cx, cy 초기값 추정
//   ③ 초기 외부 파라미터: K와 H로부터 각 이미지의 rvec, tvec 계산
//   ④ 비선형 최적화 (Levenberg-Marquardt):
//      K, dist, rvec, tvec 전체를 동시에 조정하여
//      재투영 오차 Σ||p_detected - project(P_3d, K, dist, rvec, tvec)||² 를 최소화
//
// ★ 좋은 캘리브레이션을 위한 조건:
//   - 이미지 수: 최소 10장 이상 (다양한 각도)
//   - 체커보드가 이미지 전체를 고르게 커버
//   - 다양한 기울기 (정면만 X, 30~45도 기울인 것도 포함)
//   - 초점 고정 (줌 변경 시 fx, fy가 달라지므로 재캘리브 필요)
double CameraCalibrationBasic::calibrate(const std::vector<std::vector<cv::Point2f>>& imagePoints,
                                         cv::Size imageSize, cv::Mat& cameraMatrix,
                                         cv::Mat& distCoeffs)
{
    auto start = std::chrono::high_resolution_clock::now();

    // 3D 객체 점 생성 — 체커보드의 물리적 좌표 (모든 이미지에서 동일)
    std::vector<cv::Point3f> objp = generateObjectPoints();

    // objectPoints[i] = i번째 이미지의 3D 점 (모두 같은 objp)
    // imagePoints[i] = i번째 이미지의 2D 점 (이미지마다 다름)
    // → objectPoints[i][j] ↔ imagePoints[i][j] 가 3D↔2D 대응 쌍
    std::vector<std::vector<cv::Point3f>> objectPoints;
    for (size_t i = 0; i < imagePoints.size(); i++)
    {
        objectPoints.push_back(objp);
    }

    // calibrateCamera가 출력하는 외부 파라미터:
    //   rvecs[i]: i번째 이미지에서의 회전 벡터 (3×1, Rodrigues 표현)
    //     → cv::Rodrigues(rvec, R)로 3×3 회전 행렬 R 변환 가능
    //   tvecs[i]: i번째 이미지에서의 이동 벡터 (3×1)
    //     → 월드 좌표계 원점(체커보드)에서 카메라까지의 위치
    //   이 값들은 캘리브레이션 후 보통 버리지만,
    //   재투영 오차 검증이나 카메라 포즈 시각화에 활용 가능.
    std::vector<cv::Mat> rvecs, tvecs;

    std::cout << "\n⚙️  캘리브레이션 수행 중..." << std::endl;
    std::cout << "   - 이미지 개수: " << imagePoints.size() << std::endl;
    std::cout << "   - 이미지 크기: " << imageSize << std::endl;

    // cv::calibrateCamera — Zhang's method + LM 최적화
    //
    // 반환값: RMS 재투영 오차 (픽셀 단위)
    //   = sqrt( (1/N) · Σ ||p_detected - p_reprojected||² )
    //   N = 전체 이미지의 전체 코너 수 합계
    //   p_reprojected = projectPoints(objp, rvec[i], tvec[i], K, dist)
    double rms = cv::calibrateCamera(objectPoints,  // 입력: 3D 점들 (이미지 수 × 코너 수)
                                     imagePoints,   // 입력: 2D 점들 (이미지 수 × 코너 수)
                                     imageSize,     // 입력: 이미지 해상도
                                     cameraMatrix,  // 출력: 카메라 행렬 K (3×3)
                                     distCoeffs,    // 출력: 왜곡 계수 (1×5)
                                     rvecs,         // 출력: 각 이미지의 회전 벡터
                                     tvecs          // 출력: 각 이미지의 이동 벡터
    );

    // 성능 측정 종료
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "   ✅ 완료! (소요시간: " << duration.count() << " ms)" << std::endl;
    std::cout << "\n📊 RMS 재투영 오차: " << rms << " 픽셀" << std::endl;
    std::cout << "   → " << evaluateQuality(rms) << std::endl;

    return rms;
}

// 메인 함수 — 실제 카메라 없이 합성 데이터로 캘리브레이션 과정을 시연
//
// 시뮬레이션 전략:
//   ① 가상의 "정답" 카메라 파라미터(K_true, dist_true) 설정
//   ② projectPoints로 3D→2D 투영하여 "가짜 이미지 점" 생성
//   ③ 노이즈를 추가하여 실제 검출 오차 모사
//   ④ calibrateCamera로 K, dist를 추정
//   ⑤ 추정값과 정답을 비교하여 캘리브레이션 정확도 확인
//
// 실제 캘리브레이션과의 차이:
//   실제: 카메라로 체커보드 촬영 → detectChessboard → calibrate
//   시뮬: projectPoints + noise로 가짜 2D 점 생성 → calibrate
//   → 정답을 알고 있으므로 추정 오차를 정량적으로 검증 가능
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  카메라 캘리브레이션 기본 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 체커보드 설정
    //   9×6 = 내부 코너 개수 (사각형 수는 10×7)
    //   30.0f = 각 사각형의 한 변 길이 (mm)
    //   → 체커보드를 인쇄할 때 정확한 크기가 중요!
    //     실제 크기와 다르면 tvec(거리)이 틀려짐 (K의 fx, fy에는 영향 없음)
    CameraCalibrationBasic calib(cv::Size(9, 6), 30.0f);

    std::cout << "📸 시뮬레이션 데이터 생성 중..." << std::endl;

    // 가상의 "정답" 카메라 파라미터 — 캘리브레이션 결과와 비교할 기준
    //
    // K_true (카메라 내부 행렬):
    //   [fx  0  cx]   [600   0  400]
    //   [ 0 fy  cy] = [  0 600  300]
    //   [ 0  0   1]   [  0   0    1]
    //   - fx=fy=600: 초점거리 (픽셀 단위). 같으면 정사각 픽셀
    //   - cx=400, cy=300: 주점(principal point). 이미지 중심(800/2, 600/2)과 가까움
    //     → 주점이 중심에서 크게 벗어나면 렌즈 조립 불량 의심
    cv::Mat K_true =
        (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // dist_true (왜곡 계수): [k1, k2, p1, p2, k3]
    //   k1=-0.2: 배럴 왜곡 (음수 → 이미지 가장자리가 바깥으로 볼록)
    //   k2=+0.05: k1의 고차 보정항 (r⁴ 항)
    //   p1=p2=0: 접선 왜곡 없음 (렌즈-센서 완벽 정렬)
    //   k3=0: 6차 방사 왜곡 없음
    cv::Mat dist_true = (cv::Mat_<double>(1, 5) << -0.2, 0.05, 0.0, 0.0, 0.0);

    std::vector<std::vector<cv::Point2f>> imagePoints;
    cv::Size imageSize(800, 600);

    std::vector<cv::Point3f> objectPoints = calib.generateObjectPoints();

    // 💡 objectPoints = 체커보드의 물리적 3D 좌표 (모든 이미지에서 동일)
    std::cout << "\n💡 [교육] 3D 객체 점 (objectPoints):" << std::endl;
    std::cout << "   체커보드 9×6 = " << objectPoints.size() << "개 코너" << std::endl;
    std::cout << "   첫 점: (" << objectPoints[0].x << ", " << objectPoints[0].y
              << ", " << objectPoints[0].z << ") mm" << std::endl;
    std::cout << "   마지막: (" << objectPoints.back().x << ", " << objectPoints.back().y
              << ", " << objectPoints.back().z << ") mm" << std::endl;
    std::cout << "   → 모든 이미지에서 동일! (체커보드는 물리적으로 고정)" << std::endl;
    std::cout << "   → 변하는 건 카메라 위치(tvec)와 자세(rvec) 뿐" << std::endl;
    std::cout << "   💡 quiz 문제 4에서 이 개념이 나옵니다!\n" << std::endl;

    // 노이즈 생성 — 실제 코너 검출의 측정 오차를 시뮬레이션
    //   σ=0.5 픽셀: cornerSubPix의 전형적인 검출 오차 수준
    //   seed=42: 고정 시드로 실행할 때마다 동일한 결과 보장 (재현성)
    //   정규분포: 실제 측정 오차는 대부분 가우시안 분포를 따름
    constexpr double kNoiseStdDev = 0.5;
    std::mt19937 rng(42);
    std::normal_distribution<double> noise(0.0, kNoiseStdDev);

    // 15개 가상 이미지 생성 — 각각 다른 각도/거리에서 촬영한 것처럼 시뮬레이션
    for (int i = 0; i < 15; i++)
    {
        std::vector<cv::Point2f> projected;

        // 각 이미지마다 다른 카메라 포즈 (회전/이동) 설정
        //   rvec: i에 따라 X, Y, Z축 회전이 변함 → 다양한 기울기
        //   tvec: Z값 500~1250mm → 체커보드까지의 거리가 점점 멀어짐
        //         X, Y값도 변함 → 체커보드가 이미지의 다양한 위치에 나타남
        //   ★ 실제 캘리브레이션에서도 다양한 각도/거리/위치가 핵심!
        cv::Mat rvec = (cv::Mat_<double>(3, 1) << 0.1 * (i - 7), 0.05 * (i - 7), 0.02 * i);
        cv::Mat tvec = (cv::Mat_<double>(3, 1) << 10 * (i - 7), 10 * (i - 7), 500 + 50 * i);

        // projectPoints: 3D→2D 투영 (카메라 모델의 순방향 계산)
        //   P_world → [R|t] → P_camera → K·왜곡 → p_pixel
        //   = 실제 카메라가 촬영하는 과정을 수식으로 재현
        cv::projectPoints(objectPoints, rvec, tvec, K_true, dist_true, projected);

        // 가우시안 노이즈 추가 — 완벽한 투영에 실제 검출 오차를 더함
        //   노이즈가 없으면 RMS=0이 되어 비현실적
        //   σ=0.5px → 대략 RMS ≈ 0.5px 수준의 결과 예상
        for (auto& point : projected)
        {
            point.x += static_cast<float>(noise(rng));
            point.y += static_cast<float>(noise(rng));
        }

        imagePoints.push_back(projected);
    }

    std::cout << "   ✅ " << imagePoints.size() << "개 이미지 생성 완료\n" << std::endl;

    // 💡 rvec/tvec = 각 이미지에서의 카메라 포즈
    std::cout << "💡 [교육] rvec/tvec의 의미:" << std::endl;
    std::cout << "   rvec = 회전 벡터 (3×1), Rodrigues로 3×3 회전행렬 변환 가능" << std::endl;
    std::cout << "   tvec = 이동 벡터 (3×1), 카메라→체커보드 거리" << std::endl;
    std::cout << "   → 이미지마다 다른 rvec/tvec = 다른 각도에서 촬영" << std::endl;
    std::cout << "   → calibrateCamera()가 K, dist와 함께 이것도 출력!\n" << std::endl;

    // 캘리브레이션 수행
    cv::Mat K_estimated, dist_estimated;
    double rms = calib.calibrate(imagePoints, imageSize, K_estimated, dist_estimated);

    // 결과 출력
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  캘리브레이션 결과" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "📐 카메라 행렬 K:" << std::endl;
    std::cout << K_estimated << "\n" << std::endl;

    // 💡 K 행렬 요소별 접근 (quiz 문제 1에서 사용!)
    std::cout << "💡 [교육] K 행렬 요소별 접근:" << std::endl;
    std::cout << "   K.at<double>(0,0) = fx = " << K_estimated.at<double>(0, 0) << std::endl;
    std::cout << "   K.at<double>(1,1) = fy = " << K_estimated.at<double>(1, 1) << std::endl;
    std::cout << "   K.at<double>(0,2) = cx = " << K_estimated.at<double>(0, 2) << std::endl;
    std::cout << "   K.at<double>(1,2) = cy = " << K_estimated.at<double>(1, 2) << std::endl;
    std::cout << "   → fx≈fy이면 픽셀이 정사각형 (보통의 카메라)\n" << std::endl;

    std::cout << "📉 왜곡 계수 [k1, k2, p1, p2, k3]:" << std::endl;
    std::cout << dist_estimated << "\n" << std::endl;

    // 💡 왜곡 계수 부호의 의미 (quiz 문제 2에서 사용!)
    {
        double k1_est = dist_estimated.at<double>(0);
        double k2_est = dist_estimated.at<double>(1);
        std::cout << "💡 [교육] 왜곡 계수 해석:" << std::endl;
        std::cout << "   k1 = " << k1_est
                  << (k1_est < 0 ? " → 배럴(Barrel) 왜곡: 가장자리가 바깥으로"
                                 : " → 핀쿠션(Pincushion) 왜곡: 가장자리가 안쪽으로")
                  << std::endl;
        std::cout << "   k2 = " << k2_est << " → k1의 고차 보정항" << std::endl;
        std::cout << "   p1, p2 ≈ 0 → 접선 왜곡 작음 (렌즈-센서 정렬 양호)\n" << std::endl;
    }

    // 💡 왜곡 수학 단계별 시연 (quiz medium 문제 2에서 직접 구현!)
    //
    // cv::undistort 내부에서 모든 픽셀에 대해 수행하는 계산을
    // 좌상단 구석 (0,0) 한 점에 대해 수동으로 재현하여 원리 이해.
    // (0,0)을 선택한 이유: 중심(cx,cy)에서 가장 멀어 왜곡이 최대인 점.
    {
        double fx_k = K_estimated.at<double>(0, 0);
        double fy_k = K_estimated.at<double>(1, 1);
        double cx_k = K_estimated.at<double>(0, 2);
        double cy_k = K_estimated.at<double>(1, 2);
        double k1_k = dist_estimated.at<double>(0);
        double k2_k = dist_estimated.at<double>(1);

        // ① 정규화: 픽셀 좌표 → 카메라 좌표 (K의 역변환)
        //    이미지 중심(cx,cy)을 원점으로, fx/fy로 스케일링
        double x_n = (0 - cx_k) / fx_k;
        double y_n = (0 - cy_k) / fy_k;

        // ② r² 계산: 중심으로부터의 거리 제곱
        //    r²이 클수록 왜곡이 커짐 — 이것이 "방사 왜곡"의 핵심
        double r_sq = x_n * x_n + y_n * y_n;

        // ③ 방사 왜곡 계수: 1 + k1·r² + k2·r⁴
        //    radial < 1이면 안쪽으로 수축 (배럴 왜곡)
        //    radial > 1이면 바깥으로 팽창 (핀쿠션 왜곡)
        double radial = 1 + k1_k * r_sq + k2_k * r_sq * r_sq;

        // ④ 왜곡된 픽셀 좌표 복원: 정규화 좌표에 radial 곱한 뒤 다시 픽셀로
        double u_d = fx_k * (x_n * radial) + cx_k;
        double v_d = fy_k * (y_n * radial) + cy_k;

        std::cout << "💡 [교육] 왜곡 수학 시연 — 좌상단 구석 (0,0):" << std::endl;
        std::cout << "   ① 정규화: x=(0-cx)/fx=" << x_n
                  << ", y=(0-cy)/fy=" << y_n << std::endl;
        std::cout << "   ② r² = x²+y² = " << r_sq
                  << " (중심에서 먼 점 → r² 큼)" << std::endl;
        std::cout << "   ③ radial = 1 + k1·r² + k2·r⁴ = " << radial << std::endl;
        std::cout << "   ④ 픽셀 복원: (" << u_d << ", " << v_d << ")" << std::endl;
        std::cout << "   → 변위: Δu=" << (u_d - 0) << ", Δv=" << (v_d - 0)
                  << " 픽셀" << std::endl;
        std::cout << "   → 중심(cx,cy)에서는 왜곡≈0, 가장자리일수록 ↑\n" << std::endl;
    }

    // 참값과 비교 — 시뮬레이션이므로 정답을 알고 있어 정량적 검증 가능
    //   실제 캘리브레이션에서는 정답을 모르므로 RMS만으로 품질 판단
    std::cout << "🎯 실제 값 (참조):" << std::endl;
    std::cout << "   K_true:" << std::endl;
    std::cout << K_true << "\n" << std::endl;
    std::cout << "   dist_true:" << std::endl;
    std::cout << dist_true << "\n" << std::endl;

    // 추정 오차 — |추정값 - 정답|
    //   fx 오차가 1픽셀 이내면 매우 정확한 추정
    //   k1 오차가 0.01 이내면 왜곡 모델도 정확하게 추정된 것
    double fx_error = std::abs(K_estimated.at<double>(0, 0) - K_true.at<double>(0, 0));
    double k1_error = std::abs(dist_estimated.at<double>(0) - dist_true.at<double>(0));

    std::cout << "📊 추정 오차:" << std::endl;
    std::cout << "   fx 오차: " << fx_error << " 픽셀" << std::endl;
    std::cout << "   k1 오차: " << k1_error << std::endl;

    // 💡 RMS 체감 기준 (quiz 문제 3에서 사용!)
    std::cout << "\n💡 [교육] RMS 재투영 오차 체감:" << std::endl;
    std::cout << "   < 0.3 px: 매우 우수 — 서브픽셀 수준" << std::endl;
    std::cout << "   < 0.5 px: 우수 — Depth 추정 / 3D Detection 에 충분" << std::endl;
    std::cout << "   < 1.0 px: 양호 — 사용 가능하나 주의" << std::endl;
    std::cout << "   ≥ 1.0 px: 불량 — 재캘리브레이션 필요" << std::endl;

    // 결과 저장
    calib.saveCalibration("calibration_result.yaml", K_estimated, dist_estimated, imageSize);

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  ✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md 이론 읽기 (왜곡 수학, 캘리브레이션 과정)" << std::endl;
    std::cout << "   2. quiz_easy.cpp — K 분석, 왜곡 판별, RMS 평가" << std::endl;
    std::cout << "   3. my_basic.cpp — Step 1~7 순서대로 직접 구현" << std::endl;
    std::cout << "   4. quiz_medium.cpp — 왜곡 보정 구현, 캘리브 시뮬레이션" << std::endl;
    std::cout << "   5. PRACTICE.md — 실제 카메라 캘리브레이션 실습\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
