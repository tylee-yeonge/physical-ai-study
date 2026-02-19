#include "basic.h"
#include <iostream>
#include <iomanip>
#include <cmath>

double TriangulationBasic::disparityToDepth(double disparity, double baseline, double focal_length)
{
    // Depth = (baseline * focal_length) / disparity
    if (std::abs(disparity) < 1e-6)
        return 0.0;

    return (baseline * focal_length) / disparity;
}

bool TriangulationBasic::isInFrontOfCamera(const cv::Point3f& point3d, const cv::Mat& R,
                                           const cv::Mat& t)
{
    // 카메라 좌표계로 변환
    cv::Mat pt = (cv::Mat_<double>(3, 1) << point3d.x, point3d.y, point3d.z);
    cv::Mat pt_cam = R * pt + t;

    // Z > 0이면 카메라 앞
    return pt_cam.at<double>(2) > 0;
}

double TriangulationBasic::reprojectionError(const cv::Point3f& point3d, const cv::Point2f& point2d,
                                             const cv::Mat& P)
{
    // 3D 점을 카메라로 투영
    cv::Mat pt3d = (cv::Mat_<double>(4, 1) << point3d.x, point3d.y, point3d.z, 1.0);
    cv::Mat projected = P * pt3d;

    // 동차 좌표 정규화
    double w = projected.at<double>(2);
    if (std::abs(w) < 1e-6)
        return 1e6;

    cv::Point2f proj_pt(projected.at<double>(0) / w, projected.at<double>(1) / w);

    // 유클리드 거리
    double dx = proj_pt.x - point2d.x;
    double dy = proj_pt.y - point2d.y;

    return std::sqrt(dx * dx + dy * dy);
}

bool TriangulationBasic::triangulatePoint(const cv::Point2f& pt1, const cv::Point2f& pt2,
                                          const cv::Mat& P1, const cv::Mat& P2,
                                          cv::Point3f& point3d)
{
    // OpenCV의 triangulatePoints 사용
    std::vector<cv::Point2f> pts1 = {pt1};
    std::vector<cv::Point2f> pts2 = {pt2};

    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, points4D);

    // 동차 좌표 → 3D 좌표
    float w = points4D.at<float>(3, 0);
    if (std::abs(w) < 1e-6)
        return false;

    point3d.x = points4D.at<float>(0, 0) / w;
    point3d.y = points4D.at<float>(1, 0) / w;
    point3d.z = points4D.at<float>(2, 0) / w;

    return true;
}

void TriangulationBasic::triangulatePoints(const std::vector<cv::Point2f>& points1,
                                           const std::vector<cv::Point2f>& points2,
                                           const cv::Mat& P1, const cv::Mat& P2,
                                           std::vector<cv::Point3f>& points3d)
{
    points3d.clear();

    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, points1, points2, points4D);

    // 동차 좌표 변환
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

void TriangulationBasic::evaluateTriangulation(const std::vector<cv::Point3f>& points3d,
                                               const std::vector<cv::Point2f>& points1,
                                               const std::vector<cv::Point2f>& points2,
                                               const cv::Mat& P1, const cv::Mat& P2)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "삼각측량 품질 평가" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 재투영 오차
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

void TriangulationBasic::visualize3DPoints(const std::vector<cv::Point3f>& points3d)
{
    if (points3d.empty())
        return;

    std::cout << "3D 점군 통계:" << std::endl;
    std::cout << "   점 개수: " << points3d.size() << std::endl;

    // 깊이(Z) 통계
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

void TriangulationBasic::demoPipeline(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "삼각측량 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Step 1: 투영 행렬 생성
    std::cout << "1️⃣  투영 행렬 생성..." << std::endl;

    // 첫 번째 카메라: [I | 0]
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));

    // 두 번째 카메라: K * [R | t]
    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R.copyTo(RT(cv::Rect(0, 0, 3, 3)));
    t.copyTo(RT(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT;

    std::cout << "   P1 (기준 카메라):\n" << P1 << std::endl;
    std::cout << "   P2 (이동 카메라):\n" << P2 << "\n" << std::endl;

    // Step 2: 시뮬레이션 3D 점 생성
    std::cout << "2️⃣  3D 점 생성 및 투영..." << std::endl;

    std::vector<cv::Point3f> ground_truth_3d;
    std::vector<cv::Point2f> projected_pts1, projected_pts2;

    for (int i = 0; i < 20; i++)
    {
        // 랜덤 3D 점 (카메라 앞 1~5m)
        cv::Point3f pt3d(-2.0 + (rand() % 40) / 10.0,  // X: -2 ~ 2
                         -1.5 + (rand() % 30) / 10.0,  // Y: -1.5 ~ 1.5
                         1.0 + (rand() % 40) / 10.0    // Z: 1 ~ 5
        );
        ground_truth_3d.push_back(pt3d);

        // 카메라 1에 투영
        cv::Mat pt = (cv::Mat_<double>(4, 1) << pt3d.x, pt3d.y, pt3d.z, 1.0);
        cv::Mat proj1 = P1 * pt;
        projected_pts1.push_back(cv::Point2f(proj1.at<double>(0) / proj1.at<double>(2),
                                             proj1.at<double>(1) / proj1.at<double>(2)));

        // 카메라 2에 투영
        cv::Mat proj2 = P2 * pt;
        projected_pts2.push_back(cv::Point2f(proj2.at<double>(0) / proj2.at<double>(2),
                                             proj2.at<double>(1) / proj2.at<double>(2)));
    }

    std::cout << "   생성된 3D 점: " << ground_truth_3d.size() << "개\n" << std::endl;

    // Step 3: 삼각측량으로 복원
    std::cout << "3️⃣  삼각측량으로 3D 복원..." << std::endl;

    std::vector<cv::Point3f> reconstructed_3d;
    triangulatePoints(projected_pts1, projected_pts2, P1, P2, reconstructed_3d);

    std::cout << "   복원된 3D 점: " << reconstructed_3d.size() << "개\n" << std::endl;

    // Step 4: 복원 정확도 평가
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

    // Step 6: Stereo Depth 예제
    std::cout << "\n6️⃣  Stereo Depth 계산 예제..." << std::endl;

    double baseline = cv::norm(t);      // 베이스라인
    double focal = K.at<double>(0, 0);  // 초점거리

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

// 메인 함수
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  삼각측량 기본 데모 (Week 6)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // 상대 포즈 (Week 5에서 복원한 R, t)
    cv::Mat R = (cv::Mat_<double>(3, 3) << 0.9998, -0.0175, 0.0000, 0.0175, 0.9998, 0.0000, 0.0000,
                 0.0000, 1.0000);  // 약 1도 회전

    cv::Mat t = (cv::Mat_<double>(3, 1) << 0.1, 0.0, 0.0);  // 10cm 이동

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
