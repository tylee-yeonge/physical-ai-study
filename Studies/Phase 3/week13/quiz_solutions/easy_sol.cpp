/**
 * Quiz Solutions - Easy
 * Week 13: 스케일 복구 방법
 */

#include <iostream>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 13 Quiz Solutions (Easy)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Q1
    std::cout << "Q1. Stereo 카메라에서 깊이(depth)를 구하는 공식은?\n";
    std::cout << "정답: b) depth = f * b / disparity\n";
    std::cout << "설명:\n";
    std::cout << "  - f: 초점 거리 (pixel)\n";
    std::cout << "  - b: baseline, 두 카메라 사이 거리 (m)\n";
    std::cout << "  - disparity: 좌우 이미지에서 같은 점의 x좌표 차이 (pixel)\n";
    std::cout << "  \n";
    std::cout << "  직관적 이해:\n";
    std::cout << "  - disparity가 크면 → 물체가 가까움 → depth 작음\n";
    std::cout << "  - disparity가 작으면 → 물체가 멀리 있음 → depth 큼\n";
    std::cout << "  - baseline이 크면 → 같은 거리에서 disparity 증가 → 정밀도 향상\n";
    std::cout << "  \n";
    std::cout << "  예시: f=500, b=0.12m, disparity=10pixel\n";
    std::cout << "        depth = 500 * 0.12 / 10 = 6.0m\n";
    std::cout << "  \n";
    std::cout << "  핵심: baseline이 알려져 있으므로 depth의 단위가 미터!\n";
    std::cout << "  → 스케일 모호성이 없음!\n\n";

    // Q2
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Q2. Vision과 IMU의 상호 보완 관계\n";
    std::cout << "정답: b) Vision은 방향이 정확하지만 스케일이 모호하고,\n";
    std::cout << "        IMU는 스케일 정보를 제공하지만 드리프트가 있다\n";
    std::cout << "설명:\n";
    std::cout << "  Vision (단안 카메라):\n";
    std::cout << "    ✅ 방향(회전) 추정 정확\n";
    std::cout << "    ✅ 장시간 드리프트 적음\n";
    std::cout << "    ❌ 스케일 모호 (||t||=1로 정규화)\n";
    std::cout << "    ❌ 빠른 움직임 시 모션 블러\n";
    std::cout << "  \n";
    std::cout << "  IMU:\n";
    std::cout << "    ✅ 가속도 단위가 m/s² → 절대 스케일\n";
    std::cout << "    ✅ 고속 샘플링 (200Hz+) → 빠른 움직임 추적\n";
    std::cout << "    ❌ 이중 적분의 드리프트 (오차 ∝ t²)\n";
    std::cout << "    ❌ 바이어스 변화\n";
    std::cout << "  \n";
    std::cout << "  → 서로의 약점을 보완!\n\n";

    // Q3
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Q3. GPS가 실내에서 사용할 수 없는 이유\n";
    std::cout << "정답: c) 실내에서는 GPS 위성 신호를 수신할 수 없기 때문\n";
    std::cout << "설명:\n";
    std::cout << "  1. GPS는 위성에서 보내는 전파를 수신하여 위치를 계산\n";
    std::cout << "  2. 건물 내부에서는 전파가 차단되어 신호 수신 불가\n";
    std::cout << "  3. 수신되더라도 멀티패스(반사)로 정밀도 매우 낮음\n";
    std::cout << "  4. AMR은 주로 창고, 공장 등 실내에서 운용\n";
    std::cout << "  \n";
    std::cout << "  따라서 실내 AMR에는 GPS 대신 VIO가 적합!\n";
    std::cout << "  - 추가 인프라 없이 작동\n";
    std::cout << "  - IMU는 이미 로봇에 탑재\n\n";

    // Q4
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Q4. VIO의 정의\n";
    std::cout << "정답: b) Visual-Inertial Odometry - 카메라와 IMU를 융합한 주행 거리 측정\n";
    std::cout << "설명:\n";
    std::cout << "  Visual:   카메라 (시각 센서)\n";
    std::cout << "  Inertial: IMU (관성 센서 - 가속도계 + 자이로스코프)\n";
    std::cout << "  Odometry: 주행 거리 측정 (위치/자세 추정)\n";
    std::cout << "  \n";
    std::cout << "  핵심 아이디어:\n";
    std::cout << "  - 카메라로 환경을 관측하여 방향/구조 추정\n";
    std::cout << "  - IMU로 가속도/각속도를 측정하여 스케일/동적 보정\n";
    std::cout << "  - 두 센서를 최적 융합 (EKF 또는 비선형 최적화)\n";
    std::cout << "  \n";
    std::cout << "  대표 시스템:\n";
    std::cout << "  - VINS-Mono: 홍콩과기대, Ceres 기반 최적화\n";
    std::cout << "  - OKVIS: ETH Zurich, 키프레임 기반\n";
    std::cout << "  - MSCKF: UPenn, EKF 기반 (효율적)\n";
    std::cout << "  - ORB-SLAM3: IMU 모드 지원\n";

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 3 Week 13 - Easy Quiz 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
