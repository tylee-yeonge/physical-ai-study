/**
 * Quiz Solutions - Medium
 */

#include <iostream>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 9 Quiz Solutions (Medium)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Q1. Schur Complement가 계산량을 줄이는 원리\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "정답:\n\n";

    std::cout << "1. Hpp의 특수 구조 활용\n";
    std::cout << "   - Hpp는 블록 대각 행렬\n";
    std::cout << "   - 각 점이 독립적이므로 3x3 블록만 대각선에 존재\n";
    std::cout << "   - Hpp 역행렬 = 각 블록의 역행렬 (O(M))\n\n";

    std::cout << "2. 문제 크기 감소\n";
    std::cout << "   원래 시스템:\n";
    std::cout << "   [Hcc  Hcp][Δc]   [bc]\n";
    std::cout << "   [Hpc  Hpp][Δp] = [bp]\n\n";

    std::cout << "   Schur Complement 적용:\n";
    std::cout << "   S = Hcc - Hcp * Hpp^(-1) * Hpc\n";
    std::cout << "   S * Δc = bc - Hcp * Hpp^(-1) * bp\n\n";

    std::cout << "   크기 변화:\n";
    std::cout << "   - 원래: (6N + 3M) x (6N + 3M)\n";
    std::cout << "   - Schur 후: 6N x 6N\n";
    std::cout << "   - 예: 30,600 → 600 (점 10,000개 기준)\n\n";

    std::cout << "3. 계산량\n";
    std::cout << "   - 일반 해법: O((N+M)^3) ≈ O(M^3)\n";
    std::cout << "   - Schur: O(N^3) + O(M)\n";
    std::cout << "   - M >> N이므로 훨씬 빠름!\n\n";

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Q2. Local BA의 장단점 (시스템 관점)\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "정답:\n\n";

    std::cout << "장점:\n";
    std::cout << "  1. 실시간 처리 가능\n";
    std::cout << "     - 변수 수가 제한됨 (최근 N개 키프레임)\n";
    std::cout << "     - 일정한 계산 시간 보장\n";
    std::cout << "     - 30fps 처리 가능\n\n";

    std::cout << "  2. 메모리 효율\n";
    std::cout << "     - 오래된 키프레임 제거\n";
    std::cout << "     - 무한한 맵에서도 메모리 제한\n\n";

    std::cout << "  3. 최신 정보 반영\n";
    std::cout << "     - 최근 관측에 집중\n";
    std::cout << "     - 빠른 수렴\n\n";

    std::cout << "단점:\n";
    std::cout << "  1. 드리프트 누적\n";
    std::cout << "     - 과거 오차가 수정되지 않음\n";
    std::cout << "     - 긴 경로에서 오차 증가\n\n";

    std::cout << "  2. 전역 일관성 부족\n";
    std::cout << "     - Loop closure 없이는 맵이 틀어짐\n";
    std::cout << "     - 시작점으로 돌아와도 맞지 않음\n\n";

    std::cout << "  3. 정보 손실\n";
    std::cout << "     - Marginalization으로 정보 압축\n";
    std::cout << "     - 선형화 오차 누적 가능\n\n";

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Q3. VINS가 Local BA를 사용하는 이유\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "정답:\n\n";

    std::cout << "1. 실시간 VIO 요구사항\n";
    std::cout << "   - 드론, 로봇은 실시간 제어 필요\n";
    std::cout << "   - 30Hz 이상 처리 필수\n";
    std::cout << "   - Global BA는 너무 느림\n\n";

    std::cout << "2. 메모리 제약\n";
    std::cout << "   - 임베디드 시스템 (드론, 모바일)\n";
    std::cout << "   - 무한 맵 저장 불가\n";
    std::cout << "   - Sliding window로 제한\n\n";

    std::cout << "3. IMU와의 긴밀한 결합\n";
    std::cout << "   - IMU pre-integration은 최근 데이터만 사용\n";
    std::cout << "   - 과거 IMU 데이터 재처리 비효율\n";
    std::cout << "   - Sliding window에 자연스럽게 맞음\n\n";

    std::cout << "4. Marginalization 활용\n";
    std::cout << "   - 오래된 정보를 prior로 요약\n";
    std::cout << "   - 정보 손실 최소화\n";
    std::cout << "   - 계산 효율 + 정확도 균형\n\n";

    std::cout << "5. Loop Closure는 별도 처리\n";
    std::cout << "   - 전역 일관성은 pose graph로\n";
    std::cout << "   - Local BA와 역할 분리\n";
    std::cout << "   - 효율적인 시스템 구조\n";

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
