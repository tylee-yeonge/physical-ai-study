/**
 * Phase 4 Week 1 - IMU 센서 이해 기초 퀴즈 정답
 */

#include <iostream>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 4 Week 1 Quiz Solutions (Easy)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "문제 1: 정지 상태의 가속도계" << std::endl;
    std::cout << "정답: b) ~9.81 m/s²" << std::endl;
    std::cout << "설명: 가속도계는 중력도 가속도로 측정합니다." << std::endl;
    std::cout << "  a_meas = a_true + g = 0 + 9.81 = 9.81 m/s² (위 방향)" << std::endl;
    std::cout << "  아인슈타인의 등가 원리에 의해 중력과 가속도는 구별 불가." << std::endl;
    std::cout << "  이것이 정지 상태에서도 가속도계가 0이 아닌 이유입니다.\n" << std::endl;

    std::cout << "문제 2: 자이로 바이어스 영향" << std::endl;
    std::cout << "정답: c) 0.1 rad (5.7°)" << std::endl;
    std::cout << "설명: 바이어스는 시간에 비례하여 누적됩니다." << std::endl;
    std::cout << "  각도 오차 = bias × time = 0.01 × 10 = 0.1 rad" << std::endl;
    std::cout << "  0.1 rad × (180/π) = 5.73°" << std::endl;
    std::cout << "  10초 만에 약 6도의 방향 오차가 발생합니다!" << std::endl;
    std::cout << "  → 바이어스 추정이 VIO의 핵심인 이유.\n" << std::endl;

    std::cout << "문제 3: IMU vs Vision 주파수" << std::endl;
    std::cout << "정답: c) IMU 200Hz, Camera 30Hz" << std::endl;
    std::cout << "설명: IMU는 100-1000Hz, 카메라는 20-30Hz가 일반적입니다." << std::endl;
    std::cout << "  IMU가 빠른 이유:" << std::endl;
    std::cout << "  - MEMS 센서라 읽기가 매우 빠름" << std::endl;
    std::cout << "  - 빠른 움직임(진동 등) 감지 필요" << std::endl;
    std::cout << "  - 적분 정확도를 위해 고주파 필수" << std::endl;
    std::cout << "  카메라가 느린 이유:" << std::endl;
    std::cout << "  - 이미지 처리에 시간 소요 (~30ms)" << std::endl;
    std::cout << "  - 데이터 양이 큼 (수백 KB/프레임)\n" << std::endl;

    std::cout << "문제 4: 스케일 복구" << std::endl;
    std::cout << "정답: b) 절대 가속도 (실제 m/s² 단위)" << std::endl;
    std::cout << "설명: IMU는 실제 물리 단위(m/s²)로 가속도를 측정합니다." << std::endl;
    std::cout << "  모노 카메라: 스케일 모호성 → 상대적 크기만 알 수 있음" << std::endl;
    std::cout << "  IMU 가속도 적분 → 실제 속도(m/s) → 실제 거리(m)" << std::endl;
    std::cout << "  Vision 궤적 × IMU 스케일 = 실제 크기의 궤적!" << std::endl;
    std::cout << "  이것이 VIO에서 IMU가 필수적인 핵심 이유 중 하나입니다." << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
