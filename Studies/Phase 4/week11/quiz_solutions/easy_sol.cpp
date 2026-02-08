/**
 * Quiz Solutions - Easy
 * Week 11: VIO 초기화 문제
 */

#include <iostream>

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 11 Quiz Solutions (Easy)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Q1
    std::cout << "Q1. VIO 초기화 시 추정해야 할 미지수가 아닌 것은?\n";
    std::cout << "정답: c) 카메라 초점 거리 (focal length)\n";
    std::cout << "설명:\n";
    std::cout << "  카메라 내부 파라미터(초점 거리, 주점 등)는\n";
    std::cout << "  VIO 시작 전에 캘리브레이션으로 이미 알고 있습니다.\n";
    std::cout << "  \n";
    std::cout << "  VIO 초기화에서 추정하는 것:\n";
    std::cout << "    ✅ 스케일 s: Vision 궤적의 실제 크기\n";
    std::cout << "    ✅ 중력 방향 g: 월드 좌표계의 중력 벡터\n";
    std::cout << "    ✅ 자이로 바이어스 b_g: 각속도 측정 편향\n";
    std::cout << "    ✅ 초기 속도 v: 각 키프레임의 속도\n";
    std::cout << "    ❌ 초점 거리: 사전 캘리브레이션으로 알려짐\n\n";

    // Q2
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Q2. Vision-only SfM에서 스케일을 알 수 없는 이유?\n";
    std::cout << "정답: b) 단안 카메라는 깊이 정보가 없어 크기와 거리를 구분할 수 없으므로\n";
    std::cout << "설명:\n";
    std::cout << "  단안 카메라의 근본적 한계:\n";
    std::cout << "  - 하나의 이미지에서 물체의 크기와 거리를 동시에 알 수 없음\n";
    std::cout << "  - 작은 물체가 가까이 있는 것 vs 큰 물체가 멀리 있는 것\n";
    std::cout << "    → 동일한 이미지를 생성할 수 있음!\n";
    std::cout << "  \n";
    std::cout << "  SfM에서의 결과:\n";
    std::cout << "  - Essential Matrix에서 R, t를 복원할 때\n";
    std::cout << "  - t의 방향만 알 수 있고, 크기(||t||)는 알 수 없음\n";
    std::cout << "  - 관례적으로 ||t|| = 1로 정규화\n";
    std::cout << "  - 결과: 형태는 맞지만 실제 크기(미터)를 모름\n\n";

    // Q3
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Q3. 중력 벡터의 자유도(DoF)는?\n";
    std::cout << "정답: b) 2 (방향만 추정, 크기 9.81 m/s²는 알고 있음)\n";
    std::cout << "설명:\n";
    std::cout << "  중력 벡터 g = [gx, gy, gz]^T:\n";
    std::cout << "  - 원래 3개 성분 (3 DoF)\n";
    std::cout << "  - 하지만 크기 ||g|| = 9.81 m/s²는 알려진 상수\n";
    std::cout << "  - 따라서 gx² + gy² + gz² = 9.81² (구속 1개)\n";
    std::cout << "  - 자유도 = 3 - 1 = 2\n";
    std::cout << "  \n";
    std::cout << "  기하학적 해석:\n";
    std::cout << "  - 반지름 9.81의 구(sphere) 위의 한 점\n";
    std::cout << "  - 위도(latitude)와 경도(longitude)로 표현 가능\n";
    std::cout << "  - 2개의 각도로 완전히 결정됨\n\n";

    // Q4
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Q4. VIO 초기화가 실패하기 쉬운 상황은?\n";
    std::cout << "정답: c) 카메라를 정지 상태로 두고 있을 때\n";
    std::cout << "설명:\n";
    std::cout << "  정지 상태에서 초기화가 실패하는 이유:\n";
    std::cout << "  \n";
    std::cout << "  1. Vision 측면:\n";
    std::cout << "     - 시차(parallax) = 0\n";
    std::cout << "     - 삼각측량 불가능 → 3D 점 복원 실패\n";
    std::cout << "     - SfM 자체가 성립하지 않음\n";
    std::cout << "  \n";
    std::cout << "  2. IMU 측면:\n";
    std::cout << "     - 가속도 = 중력만 (운동 가속도 = 0)\n";
    std::cout << "     - 스케일 정보를 추출할 수 없음\n";
    std::cout << "     - 바이어스와 중력 방향 분리 어려움\n";
    std::cout << "  \n";
    std::cout << "  좋은 초기화를 위한 움직임:\n";
    std::cout << "     ✅ 다양한 방향으로 회전 + 이동\n";
    std::cout << "     ✅ 2-3초 이상 충분한 움직임\n";
    std::cout << "     ❌ 정지, 순수 직선, 순수 회전\n";

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 4 Week 11 - Easy Quiz 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
