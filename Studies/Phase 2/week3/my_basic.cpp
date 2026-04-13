/**
 * @file my_basic.cpp
 * @brief Week 3: 사용자 구현 뼈대 — Step 1~6
 *
 * basic.cpp 의 데모를 참고하여 각 Step 을 직접 구현하세요.
 * TODO 주석이 있는 곳을 채우면 됩니다.
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 3: My Implementation" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Size image_size(640, 480);

    // ── Step 1: 카메라 파라미터 설정 ──
    // TODO: fx=500, fy=500, cx=320, cy=240 으로 K 행렬 생성
    //       baseline=0.12m 로 T 벡터 생성
    //       R = 단위 행렬, dist = 영벡터
    cv::Mat K1;  // TODO
    cv::Mat K2;  // TODO
    cv::Mat dist1;  // TODO
    cv::Mat dist2;  // TODO
    cv::Mat R;  // TODO
    cv::Mat T;  // TODO

    std::cout << "[Step 1] 카메라 파라미터 설정 완료" << std::endl;

    // ── Step 2: 합성 스테레오 이미지 생성 ──
    // TODO: 3D 점 50개를 랜덤 생성 (x: -3~3, y: -2~2, z: 3~10)
    cv::Mat left = cv::Mat::zeros(image_size, CV_8UC1);
    cv::Mat right = cv::Mat::zeros(image_size, CV_8UC1);
    // TODO: 위에서 만든 3D 점들을 2D 이미지 평면에 투영한 뒤, cv::circle 로 그린다.
    //       1) 투영 행렬 만들기: P1 = K * [I|0] (왼쪽), P2 = K * [R|t] (오른쪽)
    //       2) 각 3D 점을 동차좌표 (x, y, z, 1)로 만들어 P * Pw 계산
    //       3) 원근 나누기: u = result(0)/w, v = result(1)/w (w는 result(2))
    //       4) w > 0 이고 이미지 범위 안이면 cv::circle 로 원 그리기

    std::cout << "[Step 2] 스테레오 쌍 생성 완료" << std::endl;

    // ── Step 3: 특징점 매칭 + Fundamental Matrix ──
    // TODO: ORB 특징점 추출 + BFMatcher 매칭
    //       cv::findFundamentalMat(pts1, pts2, cv::FM_RANSAC) 으로 F 계산
    //       F 행렬 출력
    std::cout << "[Step 3] TODO: Fundamental Matrix 계산" << std::endl;

    // ── Step 4: Stereo Rectification ──
    // TODO: cv::stereoRectify 로 R1, R2, P1, P2, Q 계산
    //       cv::initUndistortRectifyMap 으로 각 카메라의 remap 맵 생성
    //       cv::remap 으로 rectified 이미지 생성
    std::cout << "[Step 4] TODO: Stereo Rectification" << std::endl;

    // ── Step 5: Disparity 계산 ──
    // TODO: cv::StereoBM::create(64, 15) 로 disparity 계산
    //       출력을 CV_32F 로 변환 (/ 16.0)
    std::cout << "[Step 5] TODO: Disparity 계산" << std::endl;

    // ── Step 6: Depth 변환 ──
    // TODO: Z = fB / d 공식으로 depth map 생성
    //       유효한 disparity (> 1.0) 만 변환
    std::cout << "[Step 6] TODO: Depth 변환" << std::endl;

    std::cout << "\n📌 각 Step 의 TODO 를 채우고 결과를 확인하세요!" << std::endl;
    std::cout << "   힌트: basic.cpp 의 구현을 참고하되, 직접 타이핑하세요." << std::endl;

    return 0;
}
