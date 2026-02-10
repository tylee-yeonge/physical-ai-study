/**
 * Phase 2 Week 8 - Optical Flow 직접 구현
 *
 * basic.h의 OpticalFlowBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make test_my_basic
 * 테스트: ./test_my_basic
 */
#include "basic.h"
#include <iostream>

double OpticalFlowBasic::lucasKanade(const cv::Mat& prev_img,
                                      const cv::Mat& curr_img,
                                      const std::vector<cv::Point2f>& prev_pts,
                                      std::vector<cv::Point2f>& curr_pts,
                                      std::vector<uchar>& status) {
    // TODO: cv::calcOpticalFlowPyrLK 사용, 추적 성공률 반환
    return 0.0;
}

void OpticalFlowBasic::farneback(const cv::Mat& prev_img,
                                  const cv::Mat& curr_img,
                                  cv::Mat& flow) {
    // TODO: cv::calcOpticalFlowFarneback 사용
}

void OpticalFlowBasic::visualizeFlow(const cv::Mat& flow,
                                      cv::Mat& output,
                                      int step) {
    // TODO: Dense flow 화살표 시각화
}

void OpticalFlowBasic::visualizeSparseFlow(const cv::Mat& img,
                                            const std::vector<cv::Point2f>& prev_pts,
                                            const std::vector<cv::Point2f>& curr_pts,
                                            const std::vector<uchar>& status,
                                            cv::Mat& output) {
    // TODO: Sparse flow 시각화
}

void OpticalFlowBasic::demoFeatureTracking(const std::string& video_path) {
    // TODO: Feature tracking 데모
}

double OpticalFlowBasic::evaluateFlow(const std::vector<cv::Point2f>& prev_pts,
                                       const std::vector<cv::Point2f>& curr_pts,
                                       const std::vector<uchar>& status) {
    // TODO: 성공한 추적점들의 평균 이동 거리 계산
    return 0.0;
}

void OpticalFlowBasic::compareSparseVsDense() {
    // TODO: Sparse vs Dense 비교
}

void OpticalFlowBasic::demoPipeline() {
    // TODO: 전체 파이프라인
}

#ifndef MY_BASIC_LIB_ONLY
int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] Optical Flow 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 생성
    cv::Mat prev = cv::Mat::zeros(240, 320, CV_8UC1);
    cv::Mat curr = cv::Mat::zeros(240, 320, CV_8UC1);
    cv::circle(prev, cv::Point(100, 100), 20, cv::Scalar(255), -1);
    cv::circle(curr, cv::Point(110, 105), 20, cv::Scalar(255), -1);

    // Sparse flow
    std::vector<cv::Point2f> prev_pts = {{100, 100}};
    std::vector<cv::Point2f> curr_pts;
    std::vector<uchar> status;
    double rate = OpticalFlowBasic::lucasKanade(prev, curr, prev_pts, curr_pts, status);
    std::cout << "LK 추적 성공률: " << rate << "%" << std::endl;

    // Dense flow
    cv::Mat flow;
    OpticalFlowBasic::farneback(prev, curr, flow);
    std::cout << "Dense flow 크기: " << flow.cols << "x" << flow.rows << std::endl;

    std::cout << "\n구현 완료 후 테스트: ./test_my_basic" << std::endl;
    return 0;
}
#endif
