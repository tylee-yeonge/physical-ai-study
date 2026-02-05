/**
 * Phase 2 Week 6 - 삼각측량 중급 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <iostream>

void problem1_implement_triangulation() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 삼각측량 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3,3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);
    
    // 두 번째 카메라 위치
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);  // 회전 없음
    cv::Mat t = (cv::Mat_<double>(3,1) << 0.1, 0, 0);  // 10cm 이동
    
    // 투영 행렬
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0,0,3,3)));
    
    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R.copyTo(RT(cv::Rect(0,0,3,3)));
    t.copyTo(RT(cv::Rect(3,0,1,3)));
    cv::Mat P2 = K * RT;
    
    // TODO: 대응점
    std::vector<cv::Point2f> pts1 = {cv::Point2f(400, 300)};
    std::vector<cv::Point2f> pts2 = {cv::Point2f(460, 300)};  // 시차 60픽셀
    
    // TODO: triangulatePoints 사용
    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, points4D);
    
    // 동차 좌표 → 3D
    float w = points4D.at<float>(3, 0);
    cv::Point3f pt3d(
        points4D.at<float>(0,0)/w,
        points4D.at<float>(1,0)/w,
        points4D.at<float>(2,0)/w
    );
    
    std::cout << "복원된 3D 점: (" << pt3d.x << ", " << pt3d.y << ", " << pt3d.z << ")" << std::endl;
    std::cout << "\n💡 시차 60픽셀 → 깊이 약 1m" << std::endl;
}

void problem2_optimize_reconstruction() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Bundle Adjustment" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "Linear Triangulation의 문제:" << std::endl;
    std::cout << "   - 재투영 오차를 직접 최소화하지 않음" << std::endl;
    std::cout << "   - 노이즈에 민감\n" << std::endl;
    
    std::cout << "Bundle Adjustment:" << std::endl;
    std::cout << "   - 재투영 오차의 제곱합 최소화" << std::endl;
    std::cout << "   - 비선형 최적화 (Levenberg-Marquardt)" << std::endl;
    std::cout << "   - 3D 점과 카메라 포즈 동시 최적화\n" << std::endl;
    
    std::cout << "Cost Function:" << std::endl;
    std::cout << "   Σ ||p_observed - π(P, X)||²" << std::endl;
    std::cout << "   (모든 관측에 대한 재투영 오차 합)\n" << std::endl;
    
    std::cout << "💡 SLAM에서는 Ceres나 g2o 라이브러리 사용" << std::endl;
}

void problem3_stereo_matching() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Stereo Matching" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "Rectified Stereo에서:" << std::endl;
    std::cout << "   - 대응점이 같은 y 좌표 (수평선)\n" << std::endl;
    std::cout << "   - 1D 검색으로 매칭 속도 ↑" << std::endl;
    std::cout << "   - 시차 맵 생성 가능\n" << std::endl;
    
    std::cout << "알고리즘:" << std::endl;
    std::cout << "   1. Block Matching (윈도우 기반)" << std::endl;
    std::cout << "   2. Semi-Global Matching (SGM)" << std::endl;
    std::cout << "   3. Deep Learning (PSMNet, RAFT-Stereo)\n" << std::endl;
    
    std::cout << "💡 OpenCV: cv::StereoBM, cv::StereoSGBM" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 6 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    problem1_implement_triangulation()
    problem2_optimize_reconstruction()
    problem3_stereo_matching()
    
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    return 0;
}
