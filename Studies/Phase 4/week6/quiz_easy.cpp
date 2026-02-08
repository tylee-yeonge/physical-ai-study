/**
 * Phase 4 Week 6 - Pre-integration 필요성 기초 퀴즈
 */

#include <iostream>
#include <cmath>

void problem1_frequency_mismatch() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 센서 주파수 차이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: IMU가 200Hz, 카메라가 20Hz로 동작할 때," << std::endl;
    std::cout << "두 키프레임 사이에 쌓이는 IMU 데이터는 몇 개인가요?\n" << std::endl;

    double imu_freq = 200.0;  // Hz
    double cam_freq = 20.0;   // Hz
    double cam_interval = 1.0 / cam_freq;
    double imu_interval = 1.0 / imu_freq;
    int num_imu = static_cast<int>(cam_interval / imu_interval);

    std::cout << "💡 답:" << std::endl;
    std::cout << "   카메라 간격: " << cam_interval * 1000 << " ms" << std::endl;
    std::cout << "   IMU 간격:   " << imu_interval * 1000 << " ms" << std::endl;
    std::cout << "   IMU 데이터 수: " << num_imu << "개" << std::endl;
}

void problem2_reintegration_cost() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 재적분 계산 비용" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 키프레임 20개, 키프레임 당 IMU 10개," << std::endl;
    std::cout << "최적화 30회 반복 시, 단순 적분 방식의 총 적분 횟수는?\n" << std::endl;

    int num_keyframes = 20;
    int imu_per_kf = 10;
    int opt_iters = 30;
    int total_naive = (num_keyframes - 1) * imu_per_kf * opt_iters;
    int total_preint = (num_keyframes - 1) * imu_per_kf;

    std::cout << "💡 답:" << std::endl;
    std::cout << "   단순 적분: " << total_naive << "번" << std::endl;
    std::cout << "   Pre-integration: " << total_preint << "번 (1회)" << std::endl;
    std::cout << "   절약 비율: " << opt_iters << "배!" << std::endl;
}

void problem3_pose_independent() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 포즈 독립성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: '포즈 독립적인 상대 측정값'이란 무엇인가요?" << std::endl;
    std::cout << "다음 중 올바른 설명을 고르세요.\n" << std::endl;
    std::cout << "   A) IMU 없이 포즈를 추정하는 것" << std::endl;
    std::cout << "   B) 출발 포즈가 바뀌어도 다시 계산 안 해도 되는 값" << std::endl;
    std::cout << "   C) 모든 센서를 독립적으로 처리하는 것" << std::endl;
    std::cout << "   D) 중력을 무시하고 적분하는 것\n" << std::endl;

    std::cout << "💡 답: B" << std::endl;
    std::cout << "   Pre-integrated 측정값(Δp, Δv, ΔR)은 출발 포즈(p_i, v_i, R_i)에" << std::endl;
    std::cout << "   의존하지 않으므로, 최적화로 포즈가 변경되어도 재계산 불필요" << std::endl;
}

void problem4_preintegration_analogy() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Pre-integration 비유" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 다음 중 Pre-integration과 가장 비슷한 것은?\n" << std::endl;
    std::cout << "   A) 내비게이션이 항상 절대 좌표로 안내: '위도 37.5, 경도 127'" << std::endl;
    std::cout << "   B) 내비게이션이 상대 방향으로 안내: '현재 위치에서 북쪽 100m'" << std::endl;
    std::cout << "   C) 도착 지점만 알려주는 것" << std::endl;
    std::cout << "   D) GPS를 사용하지 않는 것\n" << std::endl;

    std::cout << "💡 답: B" << std::endl;
    std::cout << "   현재 위치(출발 포즈)에서의 '상대 이동'을 알려주는 것이" << std::endl;
    std::cout << "   Pre-integration의 핵심입니다." << std::endl;
    std::cout << "   현재 위치가 바뀌어도 '북쪽 100m'라는 상대 이동은 변하지 않습니다." << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 6 Quiz - Easy" << std::endl;
    std::cout << "Pre-integration 필요성" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_frequency_mismatch();
    problem2_reintegration_cost();
    problem3_pose_independent();
    problem4_preintegration_analogy();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
