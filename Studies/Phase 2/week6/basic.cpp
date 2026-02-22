#include "basic.h"
#include <iostream>
#include <iomanip>
#include <cmath>

// 시차(disparity)에서 깊이(depth) 계산 — 스테레오 비전의 기본 공식
//
// 원리: 같은 점을 좌/우 카메라로 보면, 가까운 물체일수록 두 이미지에서의
//   위치 차이(시차)가 크고, 먼 물체일수록 시차가 작음.
//
//   depth = (baseline × focal_length) / disparity
//
//   baseline: 두 카메라 간 거리 (미터)
//   focal_length: 초점거리 (픽셀)
//   disparity: 좌/우 이미지에서의 x좌표 차이 (픽셀)
//
//   시차↑ → 가까움, 시차↓ → 멀리, 시차≈0 → 무한대
//   시차가 0이면 나눗셈 불가 → 0.0 반환
double TriangulationBasic::disparityToDepth(double disparity, double baseline, double focal_length)
{
    if (std::abs(disparity) < 1e-6)
        return 0.0;

    return (baseline * focal_length) / disparity;
}

// Cheirality Check — 삼각측량된 3D 점이 카메라 앞에 있는지 확인
//
// 카메라 좌표계에서 Z > 0이면 카메라 앞 (촬영 가능),
// Z ≤ 0이면 카메라 뒤 (물리적으로 불가능한 위치).
// Essential Matrix 분해 시 4가지 해 중 올바른 해를 선택할 때 사용.
bool TriangulationBasic::isInFrontOfCamera(const cv::Point3f& point3d, const cv::Mat& R,
                                           const cv::Mat& t)
{
    cv::Mat pt = (cv::Mat_<double>(3, 1) << point3d.x, point3d.y, point3d.z);
    cv::Mat pt_cam = R * pt + t;

    return pt_cam.at<double>(2) > 0;
}

// 투영 행렬 P를 사용한 재투영 오차 계산
//
// P: 3×4 투영 행렬 = K · [R|t]
// 동차 좌표 사용: P · [X, Y, Z, 1]ᵀ = [u·w, v·w, w]ᵀ → (u, v) = 픽셀 좌표
// w로 나누는 이유: 투영 행렬은 동차 좌표를 반환하므로 정규화 필요
double TriangulationBasic::reprojectionError(const cv::Point3f& point3d, const cv::Point2f& point2d,
                                             const cv::Mat& P)
{
    // 4×1 동차 좌표: [X, Y, Z, 1]
    cv::Mat pt3d = (cv::Mat_<double>(4, 1) << point3d.x, point3d.y, point3d.z, 1.0);
    cv::Mat projected = P * pt3d;  // 3×1 결과: [u·w, v·w, w]

    // 동차 좌표 정규화: w로 나누어 실제 픽셀 좌표 추출
    double w = projected.at<double>(2);
    if (std::abs(w) < 1e-6)
        return 1e6;

    cv::Point2f proj_pt(projected.at<double>(0) / w, projected.at<double>(1) / w);

    double dx = proj_pt.x - point2d.x;
    double dy = proj_pt.y - point2d.y;

    return std::sqrt(dx * dx + dy * dy);
}

// 삼각측량 — 두 카메라의 2D 관측으로부터 3D 점 복원
//
// 원리: 두 카메라 각각에서 한 점을 관측하면, 각 카메라로부터 3D 광선이 나옴.
//   이론적으로 두 광선의 교차점이 3D 점의 위치.
//   실제로는 노이즈 때문에 두 광선이 정확히 교차하지 않으므로,
//   DLT(Direct Linear Transform)로 최소제곱 해를 구함.
//
// DLT 삼각측량:
//   각 카메라에서 p = P · X (X: 동차 3D, p: 동차 2D)
//   이를 정리하면 A · X = 0 형태의 연립방정식 (A: 4×4)
//   SVD로 A를 분해하여 최소 노름 해 X를 구함
//
// OpenCV의 triangulatePoints는 이 DLT를 내부적으로 수행.
// 결과는 4×N 동차 좌표 [X, Y, Z, W] → (X/W, Y/W, Z/W)로 변환
bool TriangulationBasic::triangulatePoint(const cv::Point2f& pt1, const cv::Point2f& pt2,
                                          const cv::Mat& P1, const cv::Mat& P2,
                                          cv::Point3f& point3d)
{
    std::vector<cv::Point2f> pts1 = {pt1};
    std::vector<cv::Point2f> pts2 = {pt2};

    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, points4D);

    // 동차 좌표 → 유클리드 좌표: w로 나눔
    float w = points4D.at<float>(3, 0);
    if (std::abs(w) < 1e-6)
        return false;

    point3d.x = points4D.at<float>(0, 0) / w;
    point3d.y = points4D.at<float>(1, 0) / w;
    point3d.z = points4D.at<float>(2, 0) / w;

    return true;
}

// 배치 삼각측량 — 여러 대응점 쌍을 한 번에 삼각측량
//
// triangulatePoint의 배치 버전. OpenCV의 triangulatePoints는
// 내부적으로 모든 점을 DLT로 일괄 처리하므로, 반복 호출보다 효율적.
//
// 결과: 4×N 동차 좌표 행렬 → 각 열을 w로 나누어 유클리드 좌표로 변환
// w ≈ 0인 경우: 무한원점(ideal point) → 유효하지 않으므로 건너뜀
void TriangulationBasic::triangulatePoints(const std::vector<cv::Point2f>& points1,
                                           const std::vector<cv::Point2f>& points2,
                                           const cv::Mat& P1, const cv::Mat& P2,
                                           std::vector<cv::Point3f>& points3d)
{
    points3d.clear();

    // OpenCV triangulatePoints: DLT 기반 삼각측량
    // 입력: P1, P2 (3×4 투영 행렬), 2D 대응점
    // 출력: 4×N 동차 좌표 행렬
    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, points1, points2, points4D);

    // 동차 좌표 [X, Y, Z, W] → 유클리드 좌표 (X/W, Y/W, Z/W)
    for (int i = 0; i < points4D.cols; i++)
    {
        float w = points4D.at<float>(3, i);
        if (std::abs(w) > 1e-6)
        {
            cv::Point3f pt;
            pt.x = points4D.at<float>(0, i) / w;
            pt.y = points4D.at<float>(1, i) / w;
            pt.z = points4D.at<float>(2, i) / w;
            points3d.push_back(pt);
        }
    }
}

// 평균 재투영 오차 — 삼각측량 결과의 품질 지표
//
// 삼각측량으로 복원한 3D 점을 투영 행렬 P로 다시 2D에 투영하여
// 원래 관측된 2D 점과의 거리를 측정.
//
// 좋은 삼각측량이면 < 1.0 픽셀
// BA(Bundle Adjustment)는 이 오차를 최소화하는 최적화
double TriangulationBasic::averageReprojectionError(const std::vector<cv::Point3f>& points3d,
                                                    const std::vector<cv::Point2f>& points2d,
                                                    const cv::Mat& P)
{
    if (points3d.size() != points2d.size() || points3d.empty())
    {
        return -1.0;
    }

    double total_error = 0.0;
    for (size_t i = 0; i < points3d.size(); i++)
    {
        total_error += reprojectionError(points3d[i], points2d[i], P);
    }

    return total_error / points3d.size();
}

// 삼각측량 결과 종합 평가 — 재투영 오차 + 3D 점군 통계
//
// 두 카메라 각각에 대한 재투영 오차를 계산하여 출력하고,
// 3D 점군의 깊이 분포 통계를 표시.
//
// 평가 기준:
//   재투영 오차 < 0.5px: 매우 정확 (노이즈 없는 시뮬레이션)
//   재투영 오차 < 2.0px: 실제 환경에서 양호
//   재투영 오차 > 5.0px: 삼각측량 실패 가능성 높음
void TriangulationBasic::evaluateTriangulation(const std::vector<cv::Point3f>& points3d,
                                               const std::vector<cv::Point2f>& points1,
                                               const std::vector<cv::Point2f>& points2,
                                               const cv::Mat& P1, const cv::Mat& P2)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "삼각측량 품질 평가" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 각 카메라에 대한 재투영 오차
    double error1 = averageReprojectionError(points3d, points1, P1);
    double error2 = averageReprojectionError(points3d, points2, P2);

    std::cout << "재투영 오차:" << std::endl;
    std::cout << "   카메라 1: " << std::fixed << std::setprecision(3) << error1 << " 픽셀"
              << std::endl;
    std::cout << "   카메라 2: " << error2 << " 픽셀" << std::endl;
    std::cout << "   평균: " << (error1 + error2) / 2.0 << " 픽셀\n" << std::endl;

    // 3D 점 통계
    visualize3DPoints(points3d);
}

// 3D 점군 통계 출력 — 깊이(Z) 범위와 평균
//
// 삼각측량 결과의 3D 점들이 합리적인 깊이에 분포하는지 확인.
// Z < 0인 점이 있다면 삼각측량 실패 (카메라 뒤에 복원됨).
// 깊이 범위가 너무 넓으면 일부 outlier 존재 가능.
void TriangulationBasic::visualize3DPoints(const std::vector<cv::Point3f>& points3d)
{
    if (points3d.empty())
        return;

    std::cout << "3D 점군 통계:" << std::endl;
    std::cout << "   점 개수: " << points3d.size() << std::endl;

    // Z(깊이) 값의 최솟값, 최댓값, 평균 계산
    double min_z = 1e9, max_z = -1e9, avg_z = 0.0;
    for (const auto& pt : points3d)
    {
        min_z = std::min(min_z, (double)pt.z);
        max_z = std::max(max_z, (double)pt.z);
        avg_z += pt.z;
    }
    avg_z /= points3d.size();

    std::cout << "   깊이 범위: " << std::fixed << std::setprecision(2) << min_z << " ~ " << max_z
              << " m" << std::endl;
    std::cout << "   평균 깊이: " << avg_z << " m" << std::endl;
}

// 삼각측량 전체 파이프라인 데모
//
// 6단계 파이프라인:
//   ① 투영 행렬 P1, P2 생성 (K와 R, t로부터)
//   ② 시뮬레이션 3D 점 생성 → 두 카메라에 투영
//   ③ 2D 관측으로부터 삼각측량으로 3D 복원
//   ④ GT와 비교하여 3D 복원 오차 평가
//   ⑤ 재투영 오차 확인
//   ⑥ Stereo Depth 공식 데모 (시차→깊이)
//
// 투영 행렬 P (3×4):
//   P = K · [R|t]
//   카메라1: 원점에 위치 → P1 = K · [I|0]
//   카메라2: R, t만큼 이동 → P2 = K · [R|t]
void TriangulationBasic::demoPipeline(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "삼각측량 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Step 1: 투영 행렬 생성
    std::cout << "1️⃣  투영 행렬 생성..." << std::endl;

    // 카메라1: 월드 원점에 위치 → P1 = K · [I|0]
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));

    // 카메라2: R, t만큼 이동 → P2 = K · [R|t]
    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R.copyTo(RT(cv::Rect(0, 0, 3, 3)));
    t.copyTo(RT(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT;

    std::cout << "   P1 (기준 카메라):\n" << P1 << std::endl;
    std::cout << "   P2 (이동 카메라):\n" << P2 << "\n" << std::endl;

    // Step 2: GT 3D 점 생성 → 두 카메라에 투영
    // 카메라 앞 1~5m에 20개 점을 랜덤 배치하고 P1, P2로 투영
    std::cout << "2️⃣  3D 점 생성 및 투영..." << std::endl;

    std::vector<cv::Point3f> ground_truth_3d;
    std::vector<cv::Point2f> projected_pts1, projected_pts2;

    for (int i = 0; i < 20; i++)
    {
        cv::Point3f pt3d(-2.0 + (rand() % 40) / 10.0,  // X: -2 ~ 2
                         -1.5 + (rand() % 30) / 10.0,  // Y: -1.5 ~ 1.5
                         1.0 + (rand() % 40) / 10.0    // Z: 1 ~ 5
        );
        ground_truth_3d.push_back(pt3d);

        // P·[X,Y,Z,1]ᵀ = [u·w, v·w, w]ᵀ → (u,v) = 픽셀 좌표
        cv::Mat pt = (cv::Mat_<double>(4, 1) << pt3d.x, pt3d.y, pt3d.z, 1.0);
        cv::Mat proj1 = P1 * pt;
        projected_pts1.push_back(cv::Point2f(proj1.at<double>(0) / proj1.at<double>(2),
                                             proj1.at<double>(1) / proj1.at<double>(2)));

        cv::Mat proj2 = P2 * pt;
        projected_pts2.push_back(cv::Point2f(proj2.at<double>(0) / proj2.at<double>(2),
                                             proj2.at<double>(1) / proj2.at<double>(2)));
    }

    std::cout << "   생성된 3D 점: " << ground_truth_3d.size() << "개\n" << std::endl;

    // Step 3: DLT 삼각측량으로 3D 복원
    // 노이즈 없는 시뮬레이션이므로 GT와 거의 동일하게 복원되어야 함
    std::cout << "3️⃣  삼각측량으로 3D 복원..." << std::endl;

    std::vector<cv::Point3f> reconstructed_3d;
    triangulatePoints(projected_pts1, projected_pts2, P1, P2, reconstructed_3d);

    std::cout << "   복원된 3D 점: " << reconstructed_3d.size() << "개\n" << std::endl;

    // Step 4: GT와 복원된 3D 점 사이의 유클리드 거리 비교
    // 이 오차는 삼각측량의 3D 정확도를 직접 측정
    std::cout << "4️⃣  복원 정확도 평가..." << std::endl;

    double total_error = 0.0;
    for (size_t i = 0; i < std::min(ground_truth_3d.size(), reconstructed_3d.size()); i++)
    {
        cv::Point3f diff = ground_truth_3d[i] - reconstructed_3d[i];
        double error = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        total_error += error;
    }
    double avg_error = total_error / ground_truth_3d.size();

    std::cout << "   평균 3D 복원 오차: " << std::fixed << std::setprecision(4) << avg_error
              << " m\n"
              << std::endl;

    // Step 5: 재투영 오차 확인
    std::cout << "5️⃣  재투영 오차 확인..." << std::endl;

    evaluateTriangulation(reconstructed_3d, projected_pts1, projected_pts2, P1, P2);

    // Step 6: Stereo Depth 공식 데모
    // depth = baseline × focal / disparity
    // 다양한 시차 값에 대해 깊이를 계산하여 역비례 관계를 확인
    std::cout << "\n6️⃣  Stereo Depth 계산 예제..." << std::endl;

    double baseline = cv::norm(t);      // 두 카메라 간 거리 (미터)
    double focal = K.at<double>(0, 0);  // 초점거리 (픽셀)

    std::cout << "   베이스라인: " << baseline << " m" << std::endl;
    std::cout << "   초점거리: " << focal << " 픽셀\n" << std::endl;

    // 시차에서 깊이 계산 예제
    std::vector<double> disparities = {10.0, 20.0, 50.0};
    for (double disp : disparities)
    {
        double depth = disparityToDepth(disp, baseline, focal);
        std::cout << "   시차 " << disp << " 픽셀 → 깊이 " << std::setprecision(2) << depth << " m"
                  << std::endl;
    }

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 파이프라인 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

// 메인 함수 — 삼각측량 학습 데모 전체 실행
//
// 실행 흐름:
//   ① 카메라 K + 상대 포즈(R, t) 설정
//   ② 삼각측량, Stereo Depth, Baseline 교육 출력
//   ③ demoPipeline: 6단계 전체 파이프라인 실행
//
// 시뮬레이션 설정:
//   R: 약 1° 회전 (cos1° ≈ 0.9998, sin1° ≈ 0.0175)
//   t: X방향 10cm 이동 (스테레오 카메라의 작은 베이스라인)
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  삼각측량 기본 데모 (Week 6)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 내부 파라미터 K
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // 상대 포즈 (Week 5에서 Essential Matrix로부터 복원한 R, t)
    // cos(1°) ≈ 0.9998, sin(1°) ≈ 0.0175 → Z축 기준 약 1° 회전
    cv::Mat R = (cv::Mat_<double>(3, 3) << 0.9998, -0.0175, 0.0000, 0.0175, 0.9998, 0.0000, 0.0000,
                 0.0000, 1.0000);

    // 베이스라인 0.1m (10cm) — 실내 스테레오에 적합한 크기
    cv::Mat t = (cv::Mat_<double>(3, 1) << 0.1, 0.0, 0.0);

    std::cout << "카메라 설정:" << std::endl;
    std::cout << "   초점거리: " << K.at<double>(0, 0) << " 픽셀" << std::endl;
    std::cout << "   베이스라인: " << cv::norm(t) << " m\n" << std::endl;

    // 💡 [교육] 삼각측량이란?
    std::cout << "💡 [교육] 삼각측량이란? (quiz 문제 2에서 사용!)" << std::endl;
    std::cout << "   두 카메라에서 같은 3D 점을 관측 → 각 카메라의 광선 교차점 = 3D 점" << std::endl;
    std::cout << "   실제로는 노이즈 때문에 교차 안 함 → DLT로 최적 3D 점 추정\n" << std::endl;

    // 💡 [교육] Stereo Depth: 시차 → 깊이
    std::cout << "💡 [교육] Stereo Depth (quiz 문제 1에서 사용!):" << std::endl;
    std::cout << "   depth = baseline × focal / disparity" << std::endl;
    double demo_depth = (cv::norm(t) * K.at<double>(0, 0)) / 30.0;
    std::cout << "   예: baseline=" << cv::norm(t) << "m, focal=" << K.at<double>(0, 0)
              << ", disparity=30px → depth=" << std::fixed << std::setprecision(2) << demo_depth
              << "m" << std::endl;
    std::cout << "   시차↑ → 가까움, 시차↓ → 멀리, 시차=0 → 무한대\n" << std::endl;

    // 💡 [교육] Baseline과 정확도
    std::cout << "💡 [교육] Baseline과 정확도 (quiz 문제 4에서 사용!):" << std::endl;
    std::cout << "   Baseline ↑ → 시차 커짐 → 깊이 정확도 ↑ (but 매칭 어려워짐)" << std::endl;
    std::cout << "   Baseline ↓ → 시차 작음 → 깊이 정확도 ↓ (but 매칭 쉬움)" << std::endl;
    std::cout << "   💡 실내: 5-10cm, 자율주행: 30-60cm\n" << std::endl;

    // 전체 파이프라인 실행
    TriangulationBasic::demoPipeline(K, R, t);

    std::cout << "\n💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md 이론 읽기 (삼각측량, DLT, 재투영 오차)" << std::endl;
    std::cout << "   2. quiz_easy.cpp — Stereo Depth, 삼각측량, Baseline" << std::endl;
    std::cout << "   3. my_basic.cpp — Step 1~9 순서대로 직접 구현" << std::endl;
    std::cout << "   4. quiz_medium.cpp — 삼각측량 구현, E 분해, Bundle Adjustment" << std::endl;
    std::cout << "   5. PRACTICE.md — 삼각측량 실습\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
