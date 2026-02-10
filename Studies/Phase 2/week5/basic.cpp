#include "basic.h"
#include <iostream>
#include <iomanip>

int EpipolarGeometryBasic::estimateEssential(const std::vector<cv::Point2f>& points1,
                                             const std::vector<cv::Point2f>& points2,
                                             cv::Mat& essential,
                                             int method) {
    // Essential Matrix는 정규화 좌표에서 동작
    // focal length = 1.0인 가상 카메라 가정
    cv::Mat E = cv::findEssentialMat(points1, points2, 1.0, cv::Point2d(0, 0), method);
    
    if (!E.empty()) {
        essential = E;
        return points1.size();  // 간략화: 실제로는 inlier mask 확인 필요
    }
    
    return 0;
}

int EpipolarGeometryBasic::estimateFundamental(const std::vector<cv::Point2f>& points1,
                                               const std::vector<cv::Point2f>& points2,
                                               cv::Mat& fundamental,
                                               int method) {
    cv::Mat mask;
    cv::Mat F = cv::findFundamentalMat(points1, points2, method, 3.0, 0.99, mask);
    
    if (!F.empty()) {
        fundamental = F;
        
        // Inlier 개수 계산
        int inliers = cv::countNonZero(mask);
        return inliers;
    }
    
    return 0;
}

bool EpipolarGeometryBasic::recoverPose(const cv::Mat& essential,
                                        const std::vector<cv::Point2f>& points1,
                                        const std::vector<cv::Point2f>& points2,
                                        const cv::Mat& K,
                                        cv::Mat& R,
                                        cv::Mat& t) {
    // Essential Matrix에서 R, t 복원
    // focal length와 principal point 지정
    double focal = K.at<double>(0, 0);
    cv::Point2d pp(K.at<double>(0, 2), K.at<double>(1, 2));
    
    int inliers = cv::recoverPose(essential, points1, points2, R, t, focal, pp);
    
    return inliers > 0;
}

cv::Vec3f EpipolarGeometryBasic::computeEpipolarLine(const cv::Point2f& point,
                                                     const cv::Mat& fundamental,
                                                     bool for_image2) {
    // 에피폴라 선: l = F * p (image2) 또는 l = F^T * p (image1)
    cv::Mat p = (cv::Mat_<double>(3, 1) << point.x, point.y, 1.0);
    cv::Mat line;
    
    if (for_image2) {
        line = fundamental * p;  // l' = F * p
    } else {
        line = fundamental.t() * p;  // l = F^T * p'
    }
    
    // 정규화 (거리 계산 편의)
    double norm = std::sqrt(line.at<double>(0) * line.at<double>(0) +
                           line.at<double>(1) * line.at<double>(1));
    
    if (norm > 1e-6) {
        line /= norm;
    }
    
    return cv::Vec3f(line.at<double>(0), line.at<double>(1), line.at<double>(2));
}

double EpipolarGeometryBasic::verifyEpipolarConstraint(const cv::Point2f& point1,
                                                       const cv::Point2f& point2,
                                                       const cv::Mat& essential_or_fundamental) {
    // 에피폴라 제약: p2^T * E * p1 = 0
    cv::Mat p1 = (cv::Mat_<double>(3, 1) << point1.x, point1.y, 1.0);
    cv::Mat p2 = (cv::Mat_<double>(3, 1) << point2.x, point2.y, 1.0);
    
    cv::Mat result = p2.t() * essential_or_fundamental * p1;
    double error = std::abs(result.at<double>(0));
    
    return error;
}

void EpipolarGeometryBasic::visualizeEpipolarLines(const cv::Mat& img1,
                                                   const cv::Mat& img2,
                                                   const std::vector<cv::Point2f>& points1,
                                                   const std::vector<cv::Point2f>& points2,
                                                   const cv::Mat& fundamental,
                                                   cv::Mat& output) {
    // 두 이미지를 나란히 배치
    cv::Mat combined;
    cv::hconcat(img1, img2, combined);
    
    if (combined.channels() == 1) {
        cv::cvtColor(combined, output, cv::COLOR_GRAY2BGR);
    } else {
        output = combined.clone();
    }
    
    int img_width = img1.cols;
    
    // 각 점에 대해 에피폴라 선 그리기
    for (size_t i = 0; i < std::min(points1.size(), (size_t)10); i++) {
        // 첫 번째 이미지의 점
        cv::circle(output, points1[i], 5, cv::Scalar(0, 255, 0), -1);
        
        // 두 번째 이미지의 점 (오른쪽으로 이동)
        cv::Point2f pt2_shifted(points2[i].x + img_width, points2[i].y);
        cv::circle(output, pt2_shifted, 5, cv::Scalar(0, 255, 0), -1);
        
        // image2의 에피폴라 선 계산
        cv::Vec3f line = computeEpipolarLine(points1[i], fundamental, true);
        
        // 선 그리기: ax + by + c = 0
        // y = -(ax + c) / b
        float a = line[0], b = line[1], c = line[2];
        
        if (std::abs(b) > 1e-6) {
            int x1 = 0, x2 = img2.cols;
            int y1 = -(a * x1 + c) / b;
            int y2 = -(a * x2 + c) / b;
            
            cv::line(output, 
                    cv::Point(x1 + img_width, y1),
                    cv::Point(x2 + img_width, y2),
                    cv::Scalar(255, 0, 0), 1);
        }
    }
    
    // 정보 추가
    cv::putText(output, "Epipolar Lines", cv::Point(10, 30),
               cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
}

double EpipolarGeometryBasic::verifyEF_Relationship(const cv::Mat& K,
                                                    const cv::Mat& essential,
                                                    const cv::Mat& fundamental) {
    // F = K'^-T * E * K^-1 (같은 카메라면 K' = K)
    cv::Mat K_inv = K.inv();
    cv::Mat F_from_E = K_inv.t() * essential * K_inv;
    
    // 정규화 (F의 스케일은 임의)
    F_from_E /= F_from_E.at<double>(2, 2);
    cv::Mat F_normalized = fundamental / fundamental.at<double>(2, 2);
    
    // 차이 계산
    double diff = cv::norm(F_from_E - F_normalized);
    
    return diff;
}

void EpipolarGeometryBasic::demoPipeline(const cv::Mat& img1, const cv::Mat& img2, const cv::Mat& K) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "에피폴라 기하학 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // Step 1: 특징점 검출 및 매칭
    std::cout << "1️⃣  특징점 검출 및 매칭..." << std::endl;
    
    cv::Ptr<cv::ORB> orb = cv::ORB::create(500);
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat desc1, desc2;
    
    orb->detectAndCompute(img1, cv::noArray(), kp1, desc1);
    orb->detectAndCompute(img2, cv::noArray(), kp2, desc2);
    
    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<std::vector<cv::DMatch>> knn_matches;
    matcher.knnMatch(desc1, desc2, knn_matches, 2);
    
    // Ratio test
    std::vector<cv::DMatch> good_matches;
    for (const auto& m : knn_matches) {
        if (m.size() >= 2 && m[0].distance < 0.7 * m[1].distance) {
            good_matches.push_back(m[0]);
        }
    }
    
    std::cout << "   매칭 개수: " << good_matches.size() << "개\n" << std::endl;
    
    if (good_matches.size() < 8) {
        std::cout << "❌ 매칭이 부족합니다 (최소 8개 필요)" << std::endl;
        return;
    }
    
    // 점 추출
    std::vector<cv::Point2f> points1, points2;
    for (const auto& match : good_matches) {
        points1.push_back(kp1[match.queryIdx].pt);
        points2.push_back(kp2[match.trainIdx].pt);
    }
    
    // Step 2: Essential Matrix 추정
    std::cout << "2️⃣  Essential Matrix 추정..." << std::endl;
    
    double focal = K.at<double>(0, 0);
    cv::Point2d pp(K.at<double>(0, 2), K.at<double>(1, 2));
    
    cv::Mat E, mask_E;
    E = cv::findEssentialMat(points1, points2, focal, pp, cv::RANSAC, 0.999, 1.0, mask_E);
    
    int inliers_E = cv::countNonZero(mask_E);
    std::cout << "   Inliers: " << inliers_E << " / " << points1.size() << std::endl;
    std::cout << "   Essential Matrix:\n" << E << "\n" << std::endl;
    
    // Step 3: Fundamental Matrix 추정 (비교)
    std::cout << "3️⃣  Fundamental Matrix 추정..." << std::endl;
    
    cv::Mat F, mask_F;
    F = cv::findFundamentalMat(points1, points2, cv::FM_RANSAC, 3.0, 0.99, mask_F);
    
    int inliers_F = cv::countNonZero(mask_F);
    std::cout << "   Inliers: " << inliers_F << " / " << points1.size() << std::endl;
    std::cout << "   Fundamental Matrix:\n" << F << "\n" << std::endl;
    
    // E와 F 관계 검증
    double ef_diff = verifyEF_Relationship(K, E, F);
    std::cout << "   E-F 관계 오차: " << ef_diff << "\n" << std::endl;
    
    // Step 4: 포즈 복원
    std::cout << "4️⃣  카메라 포즈 복원..." << std::endl;
    
    cv::Mat R, t;
    int pose_inliers = cv::recoverPose(E, points1, points2, R, t, focal, pp, mask_E);
    
    std::cout << "   회전 행렬 R:\n" << R << std::endl;
    std::cout << "   이동 벡터 t:\n" << t << std::endl;
    std::cout << "   Pose inliers: " << pose_inliers << "\n" << std::endl;
    
    // Step 5: 에피폴라 제약 검증
    std::cout << "5️⃣  에피폴라 제약 검증..." << std::endl;
    
    double total_error = 0.0;
    int count = 0;
    for (size_t i = 0; i < std::min(points1.size(), (size_t)10); i++) {
        double error = verifyEpipolarConstraint(points1[i], points2[i], E);
        total_error += error;
        count++;
    }
    
    std::cout << "   평균 에피폴라 제약 오차: " << total_error / count << "\n" << std::endl;
    
    // Step 6: 시각화
    std::cout << "6️⃣  에피폴라 선 시각화..." << std::endl;
    
    cv::Mat output;
    visualizeEpipolarLines(img1, img2, points1, points2, F, output);
    cv::imwrite("epipolar_lines_demo.png", output);
    
    std::cout << "   💾 저장: epipolar_lines_demo.png\n" << std::endl;
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 파이프라인 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

// 메인 함수
#ifndef BASIC_LIB_ONLY
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  에피폴라 기하학 기본 데모 (Week 5)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 카메라 행렬 (Week 2에서 캘리브레이션 결과)
    cv::Mat K = (cv::Mat_<double>(3, 3) <<
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
    
    // 테스트 이미지 생성
    cv::Mat img1 = cv::Mat::zeros(600, 800, CV_8UC1);
    cv::Mat img2 = cv::Mat::zeros(600, 800, CV_8UC1);
    
    // 이미지 1: 패턴
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 16; j++) {
            if ((i + j) % 2 == 0) {
                cv::rectangle(img1, cv::Point(j*50, i*50),
                            cv::Point((j+1)*50, (i+1)*50),
                            cv::Scalar(255), -1);
            }
        }
    }
    
    // 이미지 2: 카메라 이동 시뮬레이션 (회전 + 이동)
    cv::Mat M = cv::getRotationMatrix2D(cv::Point(400, 300), 10, 1.0);
    M.at<double>(0, 2) += 50;  // X 이동
    M.at<double>(1, 2) += 30;  // Y 이동
    cv::warpAffine(img1, img2, M, img2.size());
    
    std::cout << "📸 테스트 이미지 생성 완료" << std::endl;
    std::cout << "   카메라 이동: 회전 10° + 이동 (50, 30)\n" << std::endl;
    
    // 전체 파이프라인 실행
    demoPipeline(img1, img2, K);
    
    std::cout << "\n💡 다음 단계:" << std::endl;
    std::cout << "   1. quiz_easy.cpp - 에피폴라 개념 확인" << std::endl;
    std::cout << "   2. quiz_medium.cpp - E/F 추정 실습" << std::endl;
    std::cout << "   3. PRACTICE.md - 실제 스테레오 카메라 사용\n" << std::endl;

    return 0;
}
#endif // BASIC_LIB_ONLY
