/**
 * Phase 4 Week 6 - Pre-integration 필요성 기초 퀴즈 정답
 */

#include <iostream>

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 6 Easy Quiz - 정답 해설" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "문제 1: 센서 주파수 차이" << std::endl;
    std::cout << "  정답: 10개" << std::endl;
    std::cout << "  해설: 카메라 간격 50ms / IMU 간격 5ms = 10" << std::endl;
    std::cout << "        이 10개의 IMU 데이터를 하나의 상대 측정값으로" << std::endl;
    std::cout << "        합치는 것이 Pre-integration의 역할입니다.\n" << std::endl;

    std::cout << "문제 2: 재적분 계산 비용" << std::endl;
    std::cout << "  정답: 단순 적분 5,700번 vs Pre-integration 190번" << std::endl;
    std::cout << "  해설: 단순 적분은 최적화 반복(30) x 키프레임 쌍(19) x IMU(10)" << std::endl;
    std::cout << "        Pre-integration은 키프레임 쌍(19) x IMU(10) = 190번 한 번만" << std::endl;
    std::cout << "        30배 차이! 이것이 Pre-integration의 핵심 이점입니다.\n" << std::endl;

    std::cout << "문제 3: 포즈 독립성" << std::endl;
    std::cout << "  정답: B" << std::endl;
    std::cout << "  해설: Pre-integrated 측정값(Δp, Δv, ΔR)은" << std::endl;
    std::cout << "        출발 포즈(p_i, v_i, R_i)에 의존하지 않습니다." << std::endl;
    std::cout << "        오직 IMU 측정값과 바이어스에만 의존하므로," << std::endl;
    std::cout << "        최적화로 포즈가 업데이트되어도 재계산이 필요 없습니다.\n" << std::endl;

    std::cout << "문제 4: Pre-integration 비유" << std::endl;
    std::cout << "  정답: B" << std::endl;
    std::cout << "  해설: Pre-integration은 '현재 위치에서의 상대 이동'입니다." << std::endl;
    std::cout << "        절대 좌표(위도/경도)가 아닌 상대 방향(북쪽 100m)으로" << std::endl;
    std::cout << "        표현하면, 출발 위치가 변해도 이동 지시는 동일합니다." << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "핵심 정리:" << std::endl;
    std::cout << "  Pre-integration = 포즈 독립적 상대 측정값 미리 계산" << std::endl;
    std::cout << "  → 최적화 시 재적분 불필요 → 실시간 VIO 가능!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
