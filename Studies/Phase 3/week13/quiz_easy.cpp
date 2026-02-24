/**
 * Quiz Easy - Week 13: 스케일 복구 방법
 *
 * 이 퀴즈에서 다루는 개념:
 *   1. Stereo Depth 공식 — Z = f·b/d의 직관과 물리적 의미
 *   2. Vision-IMU 상호 보완 — 각 센서의 장단점과 융합 이유
 *   3. GPS의 실내 한계 — 위성 신호 차단 문제
 *   4. VIO의 정의 — Visual-Inertial Odometry의 핵심 아이디어
 *
 * 스케일 복구의 핵심 원리:
 *
 *   단안 카메라 (Week 12): 투영에서 스케일 소거 → 절대 거리 불가
 *   해결 방법: 이미지 외부의 물리적 정보 추가
 *
 *   ┌────────────────┬────────────────────────────────┐
 *   │    방법         │         스케일 원천             │
 *   ├────────────────┼────────────────────────────────┤
 *   │ Stereo 카메라   │ baseline b (물리적 거리)        │
 *   │ IMU 융합 (VIO)  │ 가속도 → 이동 거리 (미터)      │
 *   │ GPS            │ 위성 삼변측량 (위도/경도/고도)    │
 *   │ 알려진 물체     │ 도로 표지판, 차선 폭 등          │
 *   └────────────────┴────────────────────────────────┘
 *
 * 난이도: ★☆☆ (기본 개념)
 * 선수 지식: Week 12 (스케일 모호성), Week 1 (VO 유형)
 */

#include <iostream>

// 문제 1: Stereo 카메라에서 깊이를 구하는 공식
//
// Stereo 깊이 공식:
//   Z = f · b / d
//   f: 초점 거리 (pixel), b: baseline (meter), d: disparity (pixel)
//
// disparity = x_left - x_right
//   같은 3D 점의 좌/우 카메라 이미지 x좌표 차이
//   가까운 물체 → disparity 큼 → depth 작음
//   먼 물체 → disparity 작음 → depth 큼
//
// ★ baseline b가 물리적으로 알려진 값이므로
//   단안 카메라와 달리 절대 깊이(미터)를 직접 구할 수 있음
void problem1_stereo_depth()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Stereo Depth 공식" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1. Stereo 카메라에서 깊이(depth)를 구하는 공식은?\n" << std::endl;
    std::cout << "   a) depth = f * disparity / b\n";
    std::cout << "   b) depth = f * b / disparity\n";
    std::cout << "   c) depth = b * disparity / f\n";
    std::cout << "   d) depth = f / (b * disparity)\n" << std::endl;
    std::cout << "   여기서 f = 초점 거리, b = baseline, disparity = 좌우 픽셀 차이\n" << std::endl;
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "   💡 힌트: baseline이 클수록, disparity가 클수록 가까운 물체입니다.\n"
              << std::endl;
}

// 문제 2: Vision과 IMU의 상호 보완 관계
//
// Vision (카메라):
//   + 방향(회전) 추정이 정확
//   + 장시간 드리프트가 적음 (특징점 기반)
//   - 스케일 모호성 (단안)
//   - 빠른 움직임에 약함 (모션 블러)
//
// IMU (관성 측정 장치):
//   + 가속도 적분 → 절대 스케일(미터) 제공
//   + 빠른 움직임에도 안정적 (1000Hz)
//   - 장시간 사용 시 드리프트 누적 (바이어스)
//   - 단독으로는 위치 추정 불가 (이중 적분 → 오차 제곱 증가)
//
// ★ VIO = Vision + IMU 융합
//   카메라의 방향 정확도 + IMU의 스케일 정보 → 상호 보완
void problem2_vision_imu_complementary()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Vision과 IMU의 상호 보완" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q2. Vision과 IMU의 상호 보완 관계에 대해 올바른 설명은?\n" << std::endl;
    std::cout << "   a) Vision은 스케일 정보를 제공하고, IMU는 방향 정보를 제공한다\n";
    std::cout << "   b) Vision은 방향이 정확하지만 스케일이 모호하고,\n";
    std::cout << "      IMU는 스케일 정보를 제공하지만 드리프트가 있다\n";
    std::cout << "   c) Vision과 IMU 모두 스케일 정보를 제공한다\n";
    std::cout << "   d) IMU만으로도 장시간 정확한 위치 추정이 가능하다\n" << std::endl;
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "   💡 힌트: 단안 카메라의 가장 큰 한계는 무엇이었나요? (Week 12)\n" << std::endl;
}

// 문제 3: GPS의 실내 한계
//
// GPS (Global Positioning System):
//   위성 4개 이상의 신호 수신 → 삼변측량으로 위치 추정
//   정확도: 실외 ~2-5m, DGPS/RTK ~1-2cm
//
// 실내 한계:
//   건물 벽/천장이 위성 전파를 차단 또는 반사
//   → 신호 수신 불가 또는 심한 멀티패스 오류
//   → 실내 로봇(AMR)에서는 GPS 사용 불가
//
// ★ 실내 로봇의 스케일 복구 대안:
//   IMU 융합 (VIO), Stereo 카메라, 바퀴 오도메트리
void problem3_gps_indoor()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: GPS의 실내 한계" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q3. GPS만으로 실내 로봇(AMR)의 스케일을 복구할 수 없는 이유로\n";
    std::cout << "    가장 적절한 것은?\n" << std::endl;
    std::cout << "   a) GPS 수신기가 너무 비싸기 때문\n";
    std::cout << "   b) GPS 업데이트 주파수가 너무 높기 때문\n";
    std::cout << "   c) 실내에서는 GPS 위성 신호를 수신할 수 없기 때문\n";
    std::cout << "   d) GPS는 고도 정보만 제공하기 때문\n" << std::endl;
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "   💡 힌트: GPS 신호는 위성에서 오는 전파입니다. 건물 내부에서는?\n" << std::endl;
}

// 문제 4: VIO (Visual-Inertial Odometry) 정의
//
// VIO = Visual + Inertial + Odometry
//   Visual: 카메라 이미지에서 특징점 추적
//   Inertial: IMU(가속도계 + 자이로스코프) 측정
//   Odometry: 이동 거리/방향 측정 (주행 거리계)
//
// VIO의 핵심 아이디어:
//   카메라와 IMU를 긴밀히 융합하여
//   카메라의 스케일 모호성을 IMU로 해결하고
//   IMU의 드리프트를 카메라로 보정
//
// 대표 시스템:
//   VINS-Mono, MSCKF, OKVIS, ORB-SLAM3 (IMU 모드)
//
// ★ 스마트폰, 드론, AR 기기에서 가장 널리 사용되는 방식
void problem4_vio_definition()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: VIO의 정의" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q4. VIO는 무엇의 약자이며, 핵심 아이디어는 무엇인가?\n" << std::endl;
    std::cout << "   a) Visual Image Optimization - 이미지 품질 최적화\n";
    std::cout << "   b) Visual-Inertial Odometry - 카메라와 IMU를 융합한 주행 거리 측정\n";
    std::cout << "   c) Video Input Output - 비디오 입출력 처리\n";
    std::cout << "   d) Virtual Inertial Observer - 가상 관성 관측기\n" << std::endl;
    std::cout << "   답: _____\n" << std::endl;

    std::cout << "   💡 힌트: Visual + Inertial + Odometry의 각 의미를 생각해보세요.\n"
              << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 13 Quiz - Easy" << std::endl;
    std::cout << "스케일 복구 방법" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    problem1_stereo_depth();
    problem2_vision_imu_complementary();
    problem3_gps_indoor();
    problem4_vio_definition();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
