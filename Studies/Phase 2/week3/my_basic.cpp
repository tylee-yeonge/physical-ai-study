/**
 * @file my_basic.cpp
 * @brief Week 3: 사용자 구현 뼈대 — Step 1~6
 *
 * basic.cpp 의 데모를 참고하여 각 Step 을 직접 구현하세요.
 * TODO 주석이 있는 곳을 채우면 됩니다.
 */

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <vector>

/**
 * @brief 이미지를 output/ 디렉토리에 PNG 파일로 저장
 * @param name 저장할 파일 이름 (확장자 제외)
 * @param image 저장할 이미지
 */
void save_output(const std::string& name, const cv::Mat& image)
{
    std::filesystem::create_directories("output");
    std::string path = "output/" + name + ".png";
    cv::imwrite(path, image);
    std::cout << "   저장: " << path << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 3: My Implementation" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Size image_size(640, 480);

    // ── Step 1: 카메라 파라미터 설정 ──
    // TODO: fx=500, fy=500, cx=320, cy=240 으로 K 행렬(카메라 내부 파라미터) 생성
    //       baseline=0.12m 로 T 벡터(왼쪽 카메라 기준 오른쪽 카메라의 위치) 생성
    //       R(왼쪽 카메라 기준 오른쪽 카메라의 회전) = 단위 행렬 (두 카메라가 평행)
    //       dist1, dist2(왼쪽/오른쪽 카메라의 왜곡 계수 [k1,k2,p1,p2,k3]) = 영벡터 (왜곡 없음)
    // 주의: 투영 시 [R|T] * Pw = R*Pw + T 이므로, 오른쪽 카메라가 +X 방향으로 떨어져 있을 때
    //       오른쪽 카메라 좌표의 x = p.x - baseline 이 되게 하려면 T 의 X 성분은 -baseline.
    //       (+baseline 이면 거울상 stereo 가 되어 ORB 매칭이 틀어져 F 계산이 엉망이 됨)
    cv::Mat K1;  // TODO
    cv::Mat K2;  // TODO
    cv::Mat dist1;  // TODO
    cv::Mat dist2;  // TODO
    cv::Mat R;  // TODO
    cv::Mat T;  // TODO

    std::cout << "[Step 1] 카메라 파라미터 설정 완료" << std::endl;

    // ── Step 2: 합성 스테레오 이미지 생성 ──
    // TODO: 3D 점 200개를 랜덤 생성 (x: -3~3, y: -2~2, z: 3~10)
    cv::Mat left = cv::Mat::zeros(image_size, CV_8UC1);
    cv::Mat right = cv::Mat::zeros(image_size, CV_8UC1);
    // TODO: 위에서 만든 3D 점들을 2D 이미지 평면에 투영한 뒤, cv::circle 로 그린다.
    //       1) 투영 행렬 만들기: P1 = K * [I|0] (왼쪽), P2 = K * [R|t] (오른쪽)
    //       2) 각 3D 점을 동차좌표 (x, y, z, 1)로 만들어 P * Pw 계산
    //       3) 원근 나누기: u = result(0)/w, v = result(1)/w (w는 result(2))
    //       4) w > 0 이고 이미지 범위 안이면 cv::circle 로 원 그리기

    std::cout << "[Step 2] 스테레오 쌍 생성 완료" << std::endl;

    // ── Step 3: 특징점 매칭 + Fundamental Matrix ──
    // TODO: 1) 왼쪽/오른쪽 이미지 각각에서 ORB 특징점과 디스크립터를 추출
    //          - 특징점(keypoint): 이미지에서 눈에 띄는 점(코너/에지 등)의 위치
    //          - 디스크립터(descriptor): 특징점 주변 정보를 요약한 벡터(매칭에 사용)
    //       2) 두 이미지의 디스크립터를 비교하여 매칭 쌍 획득
    //          - ORB 디스크립터는 이진 벡터이므로 해밍 거리 기반 매처 사용
    //       3) 매칭 품질 필터링 (선택이지만 권장)
    //          - 매칭을 distance 오름차순으로 정렬 (작을수록 유사도 높음)
    //          - 상위 N 개(예: 50 개)만 남기고 나머지 outlier 제거
    //          → 다음 단계의 RANSAC 부담을 줄이고 F 계산 정확도 향상
    //       4) 매칭 결과에서 왼쪽/오른쪽 픽셀 좌표 리스트를 추출
    //          (각 DMatch 의 queryIdx/trainIdx 로 원래 keypoint 위치 참조)
    //       5) 두 좌표 리스트로 Fundamental Matrix(3x3) 계산
    //          - RANSAC 으로 남은 outlier 매칭을 자동 제거
    //          - F 는 두 이미지 간의 에피폴라 기하 관계를 담음
    //       6) 계산된 F 행렬을 콘솔에 출력하여 확인
    std::cout << "[Step 3] TODO: Fundamental Matrix 계산" << std::endl;

    // ── Step 4: Stereo Rectification ──
    // TODO: 두 이미지를 수평 정렬하여 에피폴라 선이 모두 수평이 되도록 warping 한다.
    //       (rectify 후에는 disparity 계산이 1D 탐색으로 단순화됨)
    //       1) 스테레오 rectify 계산
    //          입력: 두 카메라의 K, 왜곡 계수, 이미지 크기, 두 카메라 간의 R/T
    //          출력: 각 카메라에 적용할 회전 행렬(R1, R2),
    //                rectify 후의 새 투영 행렬(P1, P2),
    //                disparity → 3D 변환 행렬(Q, Step 6 에서 활용 가능)
    //       2) 각 카메라에 대해 remap 용 좌표 맵(map1, map2) 생성
    //          (원본 픽셀이 rectify 후 어디로 이동하는지를 담은 룩업 테이블)
    //       3) 좌표 맵을 이용해 원본 이미지를 rectified 이미지로 변환
    //          (수평 보간을 적용하여 부드럽게 warping)
    std::cout << "[Step 4] TODO: Stereo Rectification" << std::endl;

    // ── Step 5: Disparity 계산 ──
    // TODO: rectified 된 왼쪽/오른쪽 이미지에서 각 픽셀의 좌우 이동량(disparity)을 계산
    //       (같은 3D 점이 왼쪽/오른쪽 이미지에서 몇 픽셀 떨어져 보이는가)
    //       1) StereoBM 매칭 객체 생성
    //          - 최대 탐색 disparity(16 의 배수)와 매칭 윈도우 크기(홀수) 지정
    //          - 윈도우가 클수록 노이즈에 강하나 세밀한 구조는 뭉개짐
    //       2) 두 rectified 이미지로 disparity 맵 계산
    //          - 원시 출력은 정수 타입이며 값이 실제 disparity 의 16 배로 저장됨
    //       3) 원시 disparity 를 실수 타입으로 변환 (16 으로 나누어 실제 값 복원)
    std::cout << "[Step 5] TODO: Disparity 계산" << std::endl;

    // ── Step 6: Depth 변환 ──
    // TODO: disparity 를 실제 거리(depth)로 변환한다.
    //       공식: Z = f * B / d
    //       - f: 초점거리(fx)        - B: baseline (카메라 간 거리)
    //       - d: disparity (픽셀)    - Z: 카메라부터의 깊이 (m)
    //       1) disparity 와 같은 크기의 실수 타입 depth 이미지 준비 (0 으로 초기화)
    //       2) 모든 픽셀을 순회하며 disparity 가 유효한 경우(예: > 1.0)에만 위 공식으로 Z 계산
    //          (disparity 가 0 또는 음수이면 계산 불가능하므로 건너뜀)
    //       3) depth 를 시각화 가능한 범위로 정규화한 뒤 파일로 저장하여 확인
    std::cout << "[Step 6] TODO: Depth 변환" << std::endl;

    std::cout << "\n📌 각 Step 의 TODO 를 채우고 결과를 확인하세요!" << std::endl;
    std::cout << "   힌트: basic.cpp 의 구현을 참고하되, 직접 타이핑하세요." << std::endl;

    return 0;
}