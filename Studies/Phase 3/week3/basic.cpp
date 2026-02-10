#include "motion_3d2d.h"
#include <iostream>
#include <iomanip>

bool Motion3D2D::solvePnP(const std::vector<cv::Point3f>& points3d,
                          const std::vector<cv::Point2f>& points2d,
                          const cv::Mat& K,
                          cv::Mat& rvec,
                          cv::Mat& tvec) {
    if (points3d.size() < 4 || points2d.size() < 4) {
        return false;
    }
    
    return cv::solvePnP(points3d, points2d, K, cv::Mat(), 
                       rvec, tvec, false, cv::SOLVEPNP_EPNP);
}

int Motion3D2D::solvePnPRansac(const std::vector<cv::Point3f>& points3d,
                               const std::vector<cv::Point2f>& points2d,
                               const cv::Mat& K,
                               cv::Mat& rvec,
                               cv::Mat& tvec,
                               std::vector<int>& inliers) {
    if (points3d.size() < 4 || points2d.size() < 4) {
        return 0;
    }
    
    cv::solvePnPRansac(
        points3d, points2d,
        K, cv::Mat(),
        rvec, tvec,
        false,        // useExtrinsicGuess
        100,          // iterationsCount
        8.0,          // reprojectionError
        0.99,         // confidence
        inliers       // output inliers
    );
    
    return inliers.size();
}

double Motion3D2D::computeReprojectionError(const std::vector<cv::Point3f>& points3d,
                                            const std::vector<cv::Point2f>& points2d,
                                            const cv::Mat& K,
                                            const cv::Mat& rvec,
                                            const cv::Mat& tvec) {
    std::vector<cv::Point2f> projected;
    projectPoints(points3d, K, rvec, tvec, projected);
    
    double sum_error = 0.0;
    for (size_t i = 0; i < points2d.size(); i++) {
        double dx = projected[i].x - points2d[i].x;
        double dy = projected[i].y - points2d[i].y;
        sum_error += dx*dx + dy*dy;
    }
    
    return std::sqrt(sum_error / points2d.size());
}

void Motion3D2D::projectPoints(const std::vector<cv::Point3f>& points3d,
                               const cv::Mat& K,
                               const cv::Mat& rvec,
                               const cv::Mat& tvec,
                               std::vector<cv::Point2f>& projected) {
    cv::projectPoints(points3d, rvec, tvec, K, cv::Mat(), projected);
}

void Motion3D2D::simulateTracking(const std::vector<cv::Point3f>& map_points,
                                  const cv::Mat& K,
                                  int num_frames) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "VO 추적 시뮬레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "초기 맵: " << map_points.size() << "개 3D 점\n" << std::endl;
    
    for (int frame = 0; frame < num_frames; frame++) {
        // Ground truth 포즈 (일정하게 이동)
        cv::Mat rvec_gt = (cv::Mat_<double>(3,1) << 0.01*frame, 0.005*frame, 0.0);
        cv::Mat tvec_gt = (cv::Mat_<double>(3,1) << 0.1*frame, 0.02*frame, 0.0);
        
        // 투영 → 2D 관측
        std::vector<cv::Point2f> observations;
        projectPoints(map_points, K, rvec_gt, tvec_gt, observations);
        
        // 노이즈 추가
        for (auto& pt : observations) {
            pt.x += (rand() % 20 - 10) / 10.0;
            pt.y += (rand() % 20 - 10) / 10.0;
        }
        
        // PnP 추정
        cv::Mat rvec_est, tvec_est;
        std::vector<int> inliers;
        int inlier_count = solvePnPRansac(map_points, observations, K, 
                                         rvec_est, tvec_est, inliers);
        
        double inlier_ratio = (double)inlier_count / map_points.size();
        double reproj_error = computeReprojectionError(map_points, observations, 
                                                       K, rvec_est, tvec_est);
        
        std::cout << "Frame " << frame << ":" << std::endl;
        std::cout << "   Inliers: " << inlier_count << "/" << map_points.size()
                  << " (" << std::fixed << std::setprecision(1) 
                  << inlier_ratio * 100 << "%)" << std::endl;
        std::cout << "   Reproj error: " << std::setprecision(2) 
                  << reproj_error << " px" << std::endl;
        std::cout << "   Est t: [" << std::setprecision(3)
                  << tvec_est.at<double>(0) << ", "
                  << tvec_est.at<double>(1) << ", "
                  << tvec_est.at<double>(2) << "]" << std::endl;
        
        if (inlier_ratio < 0.3) {
            std::cout << "   ⚠️  추적 실패! 재초기화 필요\n" << std::endl;
            break;
        }
        std::cout << std::endl;
    }
}

void Motion3D2D::demo() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "3D-2D 모션 추정 (PnP) 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) <<
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
    
    // 3D 맵 (Week 2에서 생성했다고 가정)
    std::vector<cv::Point3f> map_points;
    for (int i = 0; i < 100; i++) {
        map_points.push_back(cv::Point3f(
            -3.0 + (rand() % 60) / 10.0,
            -2.0 + (rand() % 40) / 10.0,
            3.0 + (rand() % 50) / 10.0
        ));
    }
    
    std::cout << "3D 맵: " << map_points.size() << "개 점\n" << std::endl;
    
    // Ground truth 포즈
    cv::Mat rvec_gt = (cv::Mat_<double>(3,1) << 0.05, 0.02, 0.0);
    cv::Mat tvec_gt = (cv::Mat_<double>(3,1) << 0.5, 0.1, 0.05);
    
    std::cout << "Ground Truth 포즈:" << std::endl;
    std::cout << "   t = " << tvec_gt.t() << "\n" << std::endl;
    
    // 투영
    std::vector<cv::Point2f> observations;
    projectPoints(map_points, K, rvec_gt, tvec_gt, observations);
    
    // 노이즈 추가
    for (auto& pt : observations) {
        pt.x += (rand() % 20 - 10) / 10.0;
        pt.y += (rand() % 20 - 10) / 10.0;
    }
    
    // PnP 추정 (RANSAC)
    cv::Mat rvec_est, tvec_est;
    std::vector<int> inliers;
    int inlier_count = solvePnPRansac(map_points, observations, K,
                                     rvec_est, tvec_est, inliers);
    
    std::cout << "PnP 추정 결과:" << std::endl;
    std::cout << "   Inliers: " << inlier_count << "/" << map_points.size() << std::endl;
    std::cout << "   Est t: " << tvec_est.t() << std::endl;
    
    double t_error = cv::norm(tvec_gt - tvec_est);
    std::cout << "   이동 오차: " << std::fixed << std::setprecision(4) 
              << t_error << "\n" << std::endl;
    
    // 재투영 오차
    double reproj_error = computeReprojectionError(map_points, observations,
                                                   K, rvec_est, tvec_est);
    std::cout << "재투영 오차: " << std::setprecision(2) 
              << reproj_error << " px\n" << std::endl;
    
    // VO 추적 시뮬레이션
    simulateTracking(map_points, K, 5);
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

#ifndef BASIC_LIB_ONLY
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 3: 3D-2D 모션 추정 (PnP)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    Motion3D2D::demo();
    
    std::cout << "\n💡 핵심 내용:" << std::endl;
    std::cout << "   - PnP로 3D-2D 모션 추정" << std::endl;
    std::cout << "   - RANSAC으로 Outlier 제거" << std::endl;
    std::cout << "   - ✅ 절대 스케일 복원!" << std::endl;
    std::cout << "   - VO 추적의 핵심 방법\n" << std::endl;
    
    std::cout << "다음: Week 4 - 3D-3D 모션 추정 (ICP)\n" << std::endl;

    return 0;
}
#endif // BASIC_LIB_ONLY
