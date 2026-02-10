#include "basic.h"
#include <iostream>
#include <iomanip>
#include <cmath>

bool TriangulationBasic::triangulatePoint(const cv::Point2f& pt1,
                                          const cv::Point2f& pt2,
                                          const cv::Mat& P1,
                                          const cv::Mat& P2,
                                          cv::Point3f& point3d) {
    // OpenCV의 triangulatePoints 사용
    std::vector<cv::Point2f> pts1 = {pt1};
    std::vector<cv::Point2f> pts2 = {pt2};
    
    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, points4D);
    
    // 동차 좌표 → 3D 좌표
    float w = points4D.at<float>(3, 0);
    if (std::abs(w) < 1e-6) return false;
    
    point3d.x = points4D.at<float>(0, 0) / w;
    point3d.y = points4D.at<float>(1, 0) / w;
    point3d.z = points4D.at<float>(2, 0) / w;
    
    return true;
}

void TriangulationBasic::triangulatePoints(const std::vector<cv::Point2f>& points1,
                                           const std::vector<cv::Point2f>& points2,
                                           const cv::Mat& P1,
                                           const cv::Mat& P2,
                                           std::vector<cv::Point3f>& points3d) {
    points3d.clear();
    
    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, points1, points2, points4D);
    
    // 동차 좌표 변환
    for (int i = 0; i < points4D.cols; i++) {
        float w = points4D.at<float>(3, i);
        if (std::abs(w) > 1e-6) {
            cv::Point3f pt;
            pt.x = points4D.at<float>(0, i) / w;
            pt.y = points4D.at<float>(1, i) / w;
            pt.z = points4D.at<float>(2, i) / w;
            points3d.push_back(pt);
        }
    }
}

double TriangulationBasic::reprojectionError(const cv::Point3f& point3d,
                                             const cv::Point2f& point2d,
                                             const cv::Mat& P) {
    // 3D 점을 카메라로 투영
    cv::Mat pt3d = (cv::Mat_<double>(4, 1) << point3d.x, point3d.y, point3d.z, 1.0);
    cv::Mat projected = P * pt3d;
    
    // 동차 좌표 정규화
    double w = projected.at<double>(2);
    if (std::abs(w) < 1e-6) return 1e6;
    
    cv::Point2f proj_pt(projected.at<double>(0) / w, projected.at<double>(1) / w);
    
    // 유클리드 거리
    double dx = proj_pt.x - point2d.x;
    double dy = proj_pt.y - point2d.y;
    
    return std::sqrt(dx * dx + dy * dy);
}

double TriangulationBasic::averageReprojectionError(const std::vector<cv::Point3f>& points3d,
                                                    const std::vector<cv::Point2f>& points2d,
                                                    const cv::Mat& P) {
    if (points3d.size() != points2d.size() || points3d.empty()) {
        return -1.0;
    }
    
    double total_error = 0.0;
    for (size_t i = 0; i < points3d.size(); i++) {
        total_error += reprojectionError(points3d[i], points2d[i], P);
    }
    
    return total_error / points3d.size();
}

double TriangulationBasic::disparityToDepth(double disparity,
                                            double baseline,
                                            double focal_length) {
    // Depth = (baseline * focal_length) / disparity
    if (std::abs(disparity) < 1e-6) return 0.0;
    
    return (baseline * focal_length) / disparity;
}

bool TriangulationBasic::isInFrontOfCamera(const cv::Point3f& point3d,
                                           const cv::Mat& R,
                                           const cv::Mat& t) {
    // 카메라 좌표계로 변환
    cv::Mat pt = (cv::Mat_<double>(3, 1) << point3d.x, point3d.y, point3d.z);
    cv::Mat pt_cam = R * pt + t;
    
    // Z > 0이면 카메라 앞
    return pt_cam.at<double>(2) > 0;
}

void TriangulationBasic::evaluateTriangulation(const std::vector<cv::Point3f>& points3d,
                                               const std::vector<cv::Point2f>& points1,
                                               const std::vector<cv::Point2f>& points2,
                                               const cv::Mat& P1,
                                               const cv::Mat& P2) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "삼각측량 품질 평가" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 재투영 오차
    double error1 = averageReprojectionError(points3d, points1, P1);
    double error2 = averageReprojectionError(points3d, points2, P2);
    
    std::cout << "재투영 오차:" << std::endl;
    std::cout << "   카메라 1: " << std::fixed << std::setprecision(3) 
              << error1 << " 픽셀" << std::endl;
    std::cout << "   카메라 2: " << error2 << " 픽셀" << std::endl;
    std::cout << "   평균: " << (error1 + error2) / 2.0 << " 픽셀\n" << std::endl;
    
    // 3D 점 통계
    visualize3DPoints(points3d);
}

void TriangulationBasic::visualize3DPoints(const std::vector<cv::Point3f>& points3d) {
    if (points3d.empty()) return;
    
    std::cout << "3D 점군 통계:" << std::endl;
    std::cout << "   점 개수: " << points3d.size() << std::endl;
    
    // 깊이(Z) 통계
    double min_z = 1e9, max_z = -1e9, avg_z = 0.0;
    for (const auto& pt : points3d) {
        min_z = std::min(min_z, (double)pt.z);
        max_z = std::max(max_z, (double)pt.z);
        avg_z += pt.z;
    }
    avg_z /= points3d.size();
    
    std::cout << "   깊이 범위: " << std::fixed << std::setprecision(2) 
              << min_z << " ~ " << max_z << " m" << std::endl;
    std::cout << "   평균 깊이: " << avg_z << " m" << std::endl;
}

void TriangulationBasic::demoPipeline(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t) {
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
    
    for (int i = 0; i < 20; i++) {
        // 랜덤 3D 점 (카메라 앞 1~5m)
        cv::Point3f pt3d(
            -2.0 + (rand() % 40) / 10.0,  // X: -2 ~ 2
            -1.5 + (rand() % 30) / 10.0,  // Y: -1.5 ~ 1.5
            1.0 + (rand() % 40) / 10.0    // Z: 1 ~ 5
        );
        ground_truth_3d.push_back(pt3d);
        
        // 카메라 1에 투영
        cv::Mat pt = (cv::Mat_<double>(4, 1) << pt3d.x, pt3d.y, pt3d.z, 1.0);
        cv::Mat proj1 = P1 * pt;
        projected_pts1.push_back(cv::Point2f(
            proj1.at<double>(0) / proj1.at<double>(2),
            proj1.at<double>(1) / proj1.at<double>(2)
        ));
        
        // 카메라 2에 투영
        cv::Mat proj2 = P2 * pt;
        projected_pts2.push_back(cv::Point2f(
            proj2.at<double>(0) / proj2.at<double>(2),
            proj2.at<double>(1) / proj2.at<double>(2)
        ));
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
    for (size_t i = 0; i < std::min(ground_truth_3d.size(), reconstructed_3d.size()); i++) {
        cv::Point3f diff = ground_truth_3d[i] - reconstructed_3d[i];
        double error = std::sqrt(diff.x*diff.x + diff.y*diff.y + diff.z*diff.z);
        total_error += error;
    }
    double avg_error = total_error / ground_truth_3d.size();
    
    std::cout << "   평균 3D 복원 오차: " << std::fixed << std::setprecision(4) 
              << avg_error << " m\n" << std::endl;
    
    // Step 5: 재투영 오차 확인
    std::cout << "5️⃣  재투영 오차 확인..." << std::endl;
    
    evaluateTriangulation(reconstructed_3d, projected_pts1, projected_pts2, P1, P2);
    
    // Step 6: Stereo Depth 예제
    std::cout << "\n6️⃣  Stereo Depth 계산 예제..." << std::endl;
    
    double baseline = cv::norm(t);  // 베이스라인
    double focal = K.at<double>(0, 0);  // 초점거리
    
    std::cout << "   베이스라인: " << baseline << " m" << std::endl;
    std::cout << "   초점거리: " << focal << " 픽셀\n" << std::endl;
    
    // 시차에서 깊이 계산 예제
    std::vector<double> disparities = {10.0, 20.0, 50.0};
    for (double disp : disparities) {
        double depth = disparityToDepth(disp, baseline, focal);
        std::cout << "   시차 " << disp << " 픽셀 → 깊이 " 
                  << std::setprecision(2) << depth << " m" << std::endl;
    }
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 파이프라인 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

// 메인 함수
#ifndef BASIC_LIB_ONLY
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  삼각측량 기본 데모 (Week 6)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) <<
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
    
    // 상대 포즈 (Week 5에서 복원한 R, t)
    cv::Mat R = (cv::Mat_<double>(3, 3) <<
        0.9998, -0.0175, 0.0000,
        0.0175,  0.9998, 0.0000,
        0.0000,  0.0000, 1.0000);  // 약 1도 회전
    
    cv::Mat t = (cv::Mat_<double>(3, 1) << 0.1, 0.0, 0.0);  // 10cm 이동
    
    std::cout << "카메라 설정:" << std::endl;
    std::cout << "   초점거리: " << K.at<double>(0,0) << " 픽셀" << std::endl;
    std::cout << "   베이스라인: " << cv::norm(t) << " m\n" << std::endl;
    
    // 전체 파이프라인 실행
    demoPipeline(K, R, t);
    
    std::cout << "\n💡 다음 단계:" << std::endl;
    std::cout << "   1. quiz_easy.cpp - 삼각측량 개념 확인" << std::endl;
    std::cout << "   2. quiz_medium.cpp - 재투영 오차 최소화" << std::endl;
    std::cout << "   3. Week 7 - PnP로 카메라 포즈 추정\n" << std::endl;

    return 0;
}
#endif // BASIC_LIB_ONLY
