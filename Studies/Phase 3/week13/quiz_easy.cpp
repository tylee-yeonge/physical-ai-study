/**
 * Quiz Easy - Week 13: 스케일 복구 방법
 */

#include <iostream>

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
