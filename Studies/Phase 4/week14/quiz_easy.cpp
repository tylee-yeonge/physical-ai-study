/**
 * Phase 4 Week 14 - Kalibr 실습 기초 퀴즈
 */

#include <iostream>
#include <cmath>

void problem1_kalibr_inputs()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Kalibr 입력 데이터" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Kalibr Camera-IMU 캘리브레이션에 필요하지 않은 것은?\n" << std::endl;
    std::cout << "  (A) Camera 영상이 포함된 ROS bag" << std::endl;
    std::cout << "  (B) IMU 데이터가 포함된 ROS bag" << std::endl;
    std::cout << "  (C) 타겟 (AprilGrid) 사양 YAML" << std::endl;
    std::cout << "  (D) GPS 좌표 데이터\n" << std::endl;

    std::cout << "  힌트: Kalibr는 타겟 기반 캘리브레이션 도구" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem2_data_collection()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 데이터 수집" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Kalibr 캘리브레이션 데이터 수집에서 가장 중요한 것은?\n" << std::endl;
    std::cout << "  (A) 가능한 빠르게 움직이기" << std::endl;
    std::cout << "  (B) 모든 축에 대한 다양한 회전과 병진" << std::endl;
    std::cout << "  (C) 한 방향으로 정확하게 움직이기" << std::endl;
    std::cout << "  (D) 타겟과 최대한 먼 거리 유지\n" << std::endl;

    std::cout << "  힌트: 관측성(observability)을 확보하려면?" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem3_reprojection_error()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Reprojection Error" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Kalibr 결과에서 reprojection error가 1.2 pixel입니다." << std::endl;
    std::cout << "이 결과에 대한 올바른 판단은?\n" << std::endl;
    std::cout << "  (A) 훌륭한 결과, 바로 사용 가능" << std::endl;
    std::cout << "  (B) 보통 수준, 개선 권장" << std::endl;
    std::cout << "  (C) 불량, 재캘리브레이션 필요" << std::endl;
    std::cout << "  (D) 단위가 pixel이 아니므로 의미 없음\n" << std::endl;

    std::cout << "  힌트: 좋은 결과 < 0.5 pixel, 보통 < 1.0 pixel" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

void problem4_tagsize_error()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: tagSize 오차의 영향" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "실제 tagSize가 88mm인데 YAML에 80mm로 입력했습니다." << std::endl;
    std::cout << "이 오차가 영향을 미치는 파라미터는?\n" << std::endl;
    std::cout << "  (A) 카메라 회전 (R)" << std::endl;
    std::cout << "  (B) 카메라 초점거리 (fx, fy)" << std::endl;
    std::cout << "  (C) 병진 벡터 (t_ci)의 크기" << std::endl;
    std::cout << "  (D) 시간 오프셋 (td)\n" << std::endl;

    double true_size = 0.088;
    double wrong_size = 0.080;
    double scale_error = true_size / wrong_size;
    std::cout << "  스케일 오차: " << true_size << " / " << wrong_size << " = " << scale_error
              << std::endl;
    std::cout << "  힌트: tagSize는 3D 점의 실제 위치를 결정합니다." << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 14 Quiz - Easy (Kalibr 실습)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_kalibr_inputs();
    problem2_data_collection();
    problem3_reprojection_error();
    problem4_tagsize_error();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
