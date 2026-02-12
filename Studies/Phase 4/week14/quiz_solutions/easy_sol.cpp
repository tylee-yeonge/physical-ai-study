/**
 * Phase 4 Week 14 - Kalibr 실습 기초 퀴즈 풀이
 */

#include <iostream>
#include <cmath>

void problem1_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 풀이: Kalibr 입력 데이터" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (D) GPS 좌표 데이터\n" << std::endl;

    std::cout << "  Kalibr에 필요한 입력:" << std::endl;
    std::cout << "    (A) Camera 영상 ROS bag ✓" << std::endl;
    std::cout << "       → 타겟 검출을 위한 이미지" << std::endl;
    std::cout << "    (B) IMU 데이터 ROS bag ✓" << std::endl;
    std::cout << "       → 가속도, 각속도 데이터" << std::endl;
    std::cout << "    (C) 타겟 사양 YAML ✓" << std::endl;
    std::cout << "       → tagSize, tagCols, tagRows 등\n" << std::endl;
    std::cout << "    (D) GPS 좌표 ✗ → 불필요!" << std::endl;
    std::cout << "       Kalibr는 타겟 기반이므로 절대 위치 불필요" << std::endl;
    std::cout << "       타겟의 3D 좌표는 tagSize로부터 결정됨" << std::endl;
}

void problem2_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 풀이: 데이터 수집" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (B) 모든 축에 대한 다양한 회전과 병진\n" << std::endl;

    std::cout << "  각 선택지:" << std::endl;
    std::cout << "    (A) 빠른 움직임: 모션 블러 → 타겟 검출 실패" << std::endl;
    std::cout << "    (B) 다양한 운동: 관측성 확보의 핵심! ✓" << std::endl;
    std::cout << "    (C) 한 방향: 일부 축만 관측 → degenerate" << std::endl;
    std::cout << "    (D) 먼 거리: 타겟 검출 어려움, 정확도 저하\n" << std::endl;

    std::cout << "  좋은 데이터 수집 가이드:" << std::endl;
    std::cout << "    1. Roll, Pitch, Yaw 모두 30도 이상 회전" << std::endl;
    std::cout << "    2. 다양한 거리 (가까이/멀리)" << std::endl;
    std::cout << "    3. 화면 전체 영역에 타겟 노출" << std::endl;
    std::cout << "    4. 부드럽고 일정한 움직임" << std::endl;
    std::cout << "    5. 60~120초 촬영" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 풀이: Reprojection Error" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (C) 불량, 재캘리브레이션 필요\n" << std::endl;

    std::cout << "  기준:" << std::endl;
    std::cout << "    < 0.3 pixel: 우수" << std::endl;
    std::cout << "    < 0.5 pixel: 양호" << std::endl;
    std::cout << "    < 1.0 pixel: 보통 (개선 권장)" << std::endl;
    std::cout << "    > 1.0 pixel: 불량 (재캘리브레이션)\n" << std::endl;

    std::cout << "  1.2 pixel > 1.0 → 불량!\n" << std::endl;

    std::cout << "  가능한 원인과 해결:" << std::endl;
    std::cout << "    1. tagSize가 틀림 → 자로 재측정" << std::endl;
    std::cout << "    2. 모션 블러 → 더 천천히 움직이기" << std::endl;
    std::cout << "    3. 왜곡 모델 부적합 → equidistant/radtan 변경" << std::endl;
    std::cout << "    4. 데이터 부족 → 더 다양한 운동으로 재촬영" << std::endl;
}

void problem4_solution()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 풀이: tagSize 오차의 영향" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "정답: (C) 병진 벡터 (t_ci)의 크기\n" << std::endl;

    std::cout << "  설명:" << std::endl;
    std::cout << "    tagSize는 3D 점의 실제 좌표를 결정" << std::endl;
    std::cout << "    → 모든 3D 점의 좌표가 스케일만큼 변함\n" << std::endl;

    double true_size = 0.088;
    double wrong_size = 0.080;
    double scale = wrong_size / true_size;

    std::cout << "    잘못된 tagSize: " << wrong_size << "m (실제: " << true_size << "m)"
              << std::endl;
    std::cout << "    스케일 비: " << scale << "\n" << std::endl;

    std::cout << "  영향:" << std::endl;
    std::cout << "    (A) 회전 R: 스케일과 무관 → 영향 없음" << std::endl;
    std::cout << "    (B) 초점거리: 약간 영향 (보통 무시 가능)" << std::endl;
    std::cout << "    (C) 병진 t: 직접적 스케일 영향 ✓" << std::endl;
    std::cout << "        → 실제 t = 7cm인데 " << 7.0 * scale << "cm로 추정됨" << std::endl;
    std::cout << "    (D) 시간 td: 스케일과 무관\n" << std::endl;

    std::cout << "  교훈:" << std::endl;
    std::cout << "    → 인쇄 후 반드시 실제 크기를 자로 재서 확인!" << std::endl;
    std::cout << "    → mm 단위까지 정확하게 기입" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 14 Quiz Easy - 풀이" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    return 0;
}
