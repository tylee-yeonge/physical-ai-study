#include "mini_vo.h"
#include <iostream>
#include <iomanip>

MiniVO::MiniVO(const cv::Mat& K) : K_(K.clone()) {
    // ORB detector 초기화
    detector_ = cv::ORB::create(500);  // 500개 특징점
}

bool MiniVO::processFrame(const cv::Mat& frame) {
    state_.frame_id++;
    
    if (state_.status == INITIALIZING) {
        // 초기화 시도
        if (prev_frame_.empty()) {
            prev_frame_ = frame.clone();
            return false;
        }
        
        bool success = tryInitialize(prev_frame_, frame);
        if (success) {
            std::cout << "✅ 초기화 성공! Frame " << state_.frame_id << std::endl;
            state_.last_keyframe = frame.clone();
            state_.last_keyframe_id = state_.frame_id;
        }
        prev_frame_ = frame.clone();
        return success;
    }
    
    // 추적
    bool success = track(prev_frame_, frame);
    
    if (!success) {
        std::cout << "⚠️  추적 실패! Frame " << state_.frame_id << std::endl;
        state_.status =LOST;
        return false;
    }
    
    // 궤적 저장
    cv::Point3f pos(state_.t_curr.at<double>(0),
                    state_.t_curr.at<double>(1),
                    state_.t_curr.at<double>(2));
    trajectory_.push_back(pos);
    
    // 맵 업데이트
    if (isKeyframe()) {
        updateMap(frame);
        state_.last_keyframe = frame.clone();
        state_.last_keyframe_id = state_.frame_id;
    }
    
    // 나쁜 점 제거
    cullBadPoints();
    
    prev_frame_ = frame.clone();
    return true;
}

bool MiniVO::tryInitialize(const cv::Mat& frame0, const cv::Mat& frame1) {
    // 1. 특징점 검출
    std::vector<cv::KeyPoint> kp0, kp1;
    cv::Mat desc0, desc1;
    
    detector_->detectAndCompute(frame0, cv::Mat(), kp0, desc0);
    detector_->detectAndCompute(frame1, cv::Mat(), kp1, desc1);
    
    if (kp0.size() < 100 || kp1.size() < 100) {
        return false;
    }
    
    // 2. 매칭
    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<cv::DMatch> matches;
    matcher.match(desc0, desc1, matches);
    
    // 3. 좋은 매칭만 선택
    double min_dist = 100000;
    for (const auto& m : matches) {
        if (m.distance < min_dist) min_dist = m.distance;
    }
    
    std::vector<cv::Point2f> pts0, pts1;
    for (const auto& m : matches) {
        if (m.distance < 3.0 * min_dist) {
            pts0.push_back(kp0[m.queryIdx].pt);
            pts1.push_back(kp1[m.trainIdx].pt);
        }
    }
    
    if (pts0.size() < 50) {
        return false;
    }
    
    // 4. 베이스라인 체크
    if (!checkBaseline(pts0, pts1)) {
        return false;
    }
    
    // 5. 정규화
    std::vector<cv::Point2f> pts0_norm, pts1_norm;
    cv::Mat K_inv = K_.inv();
    for (size_t i = 0; i < pts0.size(); i++) {
        cv::Mat p0 = (cv::Mat_<double>(3,1) << pts0[i].x, pts0[i].y, 1.0);
        cv::Mat p1 = (cv::Mat_<double>(3,1) << pts1[i].x, pts1[i].y, 1.0);
        
        cv::Mat p0_norm = K_inv * p0;
        cv::Mat p1_norm = K_inv * p1;
        
        pts0_norm.push_back(cv::Point2f(p0_norm.at<double>(0), p0_norm.at<double>(1)));
        pts1_norm.push_back(cv::Point2f(p1_norm.at<double>(0), p1_norm.at<double>(1)));
    }
    
    // 6. Essential Matrix
    cv::Mat inliers_e;
    cv::Mat E = cv::findEssentialMat(pts0_norm, pts1_norm,
                                     cv::Mat::eye(3, 3, CV_64F),
                                     cv::RANSAC, 0.999, 0.001, inliers_e);
    
    if (E.empty()) {
        return false;
    }
    
    // 7. R, t 복원
    cv::Mat R, t, inliers_r;
    int good = cv::recoverPose(E, pts0_norm, pts1_norm,
                              cv::Mat::eye(3, 3, CV_64F), R, t, inliers_r);
    
    if (good < 50) {
        return false;
    }
    
    // 8. 초기 3D 맵 생성 (삼각측량)
    cv::Mat P0 = K_ * cv::Mat::eye(3, 4, CV_64F);
    cv::Mat P1(3, 4, CV_64F);
    R.copyTo(P1.rowRange(0, 3).colRange(0, 3));
    t.copyTo(P1.rowRange(0, 3).col(3));
    P1 = K_ * P1;
    
    cv::Mat points4D;
    std::vector<cv::Point2f> pts0_inliers, pts1_inliers;
    for (int i = 0; i < inliers_r.rows; i++) {
        if (inliers_r.at<uchar>(i)) {
            pts0_inliers.push_back(pts0[i]);
            pts1_inliers.push_back(pts1[i]);
        }
    }
    
    cv::triangulatePoints(P0, P1, pts0_inliers, pts1_inliers, points4D);
    
    // 9. 맵 포인트 저장
    for (int i = 0; i < points4D.cols; i++) {
        cv::Mat x = points4D.col(i);
        x /= x.at<float>(3);
        
        float depth = x.at<float>(2);
        if (depth > 0 && depth < 100) {  // 유효한 깊이
            MapPoint mp;
            mp.position = cv::Point3f(x.at<float>(0), x.at<float>(1), x.at<float>(2));
            mp.last_2d = pts1_inliers[i];
            mp.observations = 2;
            state_.map_points.push_back(mp);
        }
    }
    
    if (state_.map_points.size() < 30) {
        state_.map_points.clear();
        return false;
    }
    
    // 10. 상태 업데이트
    state_.status = TRACKING;
    state_.R_curr = R.clone();
    state_.t_curr = t.clone();
    state_.curr_keypoints = pts1_inliers;
    
    return true;
}

bool MiniVO::track(const cv::Mat& prev_frame, const cv::Mat& curr_frame) {
    // 1. Optical Flow로 추적
    std::vector<cv::Point2f> prev_pts, curr_pts;
    std::vector<uchar> status;
    std::vector<float> err;
    
    for (const auto& mp : state_.map_points) {
        prev_pts.push_back(mp.last_2d);
    }
    
    cv::calcOpticalFlowPyrLK(prev_frame, curr_frame, prev_pts, curr_pts,
                            status, err, cv::Size(21,21), 3);
    
    // 2. 3D-2D 대응 구성
    std::vector<cv::Point3f> points3d;
    std::vector<cv::Point2f> points2d;
    std::vector<int> valid_indices;
    
    for (size_t i = 0; i < status.size(); i++) {
        if (status[i]) {
            points3d.push_back(state_.map_points[i].position);
            points2d.push_back(curr_pts[i]);
            valid_indices.push_back(i);
        }
    }
    
    if (points3d.size() < 20) {
        return false;
    }
    
    // 3. PnP
    cv::Mat rvec, tvec;
    std::vector<int> inliers;
    
    bool success = cv::solvePnPRansac(points3d, points2d, K_, cv::Mat(),
                                     rvec, tvec, false, 100, 8.0, 0.99, inliers);
    
    if (!success || inliers.size() < 15) {
        return false;
    }
    
    // 4. Inlier ratio 체크
    double inlier_ratio = (double)inliers.size() / points3d.size();
    if (inlier_ratio < 0.3) {
        return false;
    }
    
    // 5. 재투영 오차 계산
    std::vector<cv::Point2f> projected;
    cv::projectPoints(points3d, rvec, tvec, K_, cv::Mat(), projected);
    
    double sum_error = 0.0;
    for (size_t i = 0; i < points2d.size(); i++) {
        double dx = projected[i].x - points2d[i].x;
        double dy = projected[i].y - points2d[i].y;
        sum_error += std::sqrt(dx*dx + dy*dy);
    }
    double rms_error = sum_error / points2d.size();
    
    // 6. 상태 업데이트
    cv::Mat R;
    cv::Rodrigues(rvec, R);
    state_.R_curr = R.clone();
    state_.t_curr = tvec.clone();
    state_.num_inliers = inliers.size();
    state_.reprojection_error = rms_error;
    state_.inlier_ratio = inlier_ratio;
    
    // 7. 맵 포인트 업데이트
    for (auto idx : inliers) {
        int map_idx = valid_indices[idx];
        state_.map_points[map_idx].last_2d = curr_pts[valid_indices[idx]];
        state_.map_points[map_idx].observations++;
    }
    
    // 8. 나이 증가
    for (auto& mp : state_.map_points) {
        mp.age++;
    }
    
    return true;
}

bool MiniVO::isKeyframe() const {
    int frames_since_last = state_.frame_id - state_.last_keyframe_id;
    
    // 조건 1: 일정 시간 경과
    if (frames_since_last > 10) {
        return true;
    }
    
    // 조건 2: Inlier ratio 낮음
    if (state_.inlier_ratio < 0.7) {
        return true;
    }
    
    return false;
}

void MiniVO::updateMap(const cv::Mat& curr_frame) {
    // TODO: 새로운 점 추가 (삼각측량)
    // 간단히 생략
}

void MiniVO::cullBadPoints() {
    for (int i = state_.map_points.size()-1; i >= 0; i--) {
        bool should_remove = false;
        
        // 나이가 너무 많음
        if (state_.map_points[i].age > 30) {
            should_remove = true;
        }
        
        // 관측 횟수 적음
        if (state_.map_points[i].observations < 3 && state_.map_points[i].age > 5) {
            should_remove = true;
        }
        
        if (should_remove) {
            state_.map_points.erase(state_.map_points.begin() + i);
        }
    }
}

bool MiniVO::checkBaseline(const std::vector<cv::Point2f>& pts0,
                          const std::vector<cv::Point2f>& pts1) const {
    std::vector<double> parallax;
    for (size_t i = 0; i < pts0.size(); i++) {
        double dx = pts1[i].x - pts0[i].x;
        double dy = pts1[i].y - pts0[i].y;
        parallax.push_back(std::sqrt(dx*dx + dy*dy));
    }
    
    std::sort(parallax.begin(), parallax.end());
    double median = parallax[parallax.size()/2];
    
    return median > 1.0;  // 중앙값 > 1 픽셀
}

// ============================================
// 데모 함수
// ============================================

void demoMiniVO() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Mini VO 데모 (합성 데이터)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3,3) <<
        600.0, 0.0, 400.0,
        0.0, 600.0, 300.0,
        0.0, 0.0, 1.0);
    
    MiniVO vo(K);
    
    // 합성 이미지 시퀀스 (실제로는 파일에서 읽기)
    std::cout << "합성 데이터 생성 중...\n" << std::endl;
    
    for (int i = 0; i < 20; i++) {
        cv::Mat frame = cv::Mat::zeros(600, 800, CV_8UC1);
        
        // 랜덤 특징점 그리기 (간단한 시뮬레이션)
        for (int j = 0; j < 100; j++) {
            int x = 100 + rand() % 600;
            int y = 100 + rand() % 400;
            cv::circle(frame, cv::Point(x + i*2, y), 3, cv::Scalar(255), -1);
        }
        
        bool success = vo.processFrame(frame);
        
        VOState state = vo.getState();
        
        if (state.status == TRACKING) {
            std::cout << "Frame " << std::setw(3) << i << ": "
                      << "맵 포인트 " << std::setw(4) << state.map_points.size()
                      << ", Inliers " << std::setw(3) << state.num_inliers
                      << " (" << std::fixed << std::setprecision(0)
                      << state.inlier_ratio * 100 << "%)"
                      << ", 오차 " << std::setprecision(2)
                      << state.reprojection_error << "px" << std::endl;
        }
    }
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 5: Mini VO 프로젝트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    demoMiniVO();
    
    std::cout << "\n💡 핵심 내용:" << std::endl;
    std::cout << "   - Week 2 (Essential) + Week 3 (PnP) 통합" << std::endl;
    std::cout << "   - 초기화 → 추적 → 맵 관리" << std::endl;
    std::cout << "   - 실제 VO 시스템 완성!" << std::endl;
    std::cout << "   - 드리프트 관찰 가능\n" << std::endl;
    
    std::cout << "다음: Week 6 - Keyframe Management\n" << std::endl;
    
    return 0;
}
