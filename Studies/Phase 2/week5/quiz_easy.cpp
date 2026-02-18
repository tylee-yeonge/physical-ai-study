/**
 * Phase 2 Week 5 - 에피폴라 기하학 기초 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <iostream>

void problem1_epipolar_constraint()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 에피폴라 제약" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Essential Matrix
    cv::Mat E = (cv::Mat_<double>(3, 3) << 0, -1, 0, 1, 0, 0, 0, 0, 1);

    // 대응점
    cv::Point2f p1(100, 150);
    cv::Point2f p2(120, 165);

    // TODO: p2^T * E * p1 계산
    // 힌트: 0에 가까워야 함

    std::cout << "💡 에피폴라 제약: p2^T * E * p1 = 0" << std::endl;
    std::cout << "   - 이 제약을 만족하는 점들만 올바른 대응" << std::endl;
    std::cout << "   - RANSAC outlier 제거에 사용" << std::endl;
}

void problem2_essential_vs_fundamental()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: E vs F" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 언제 E를 쓰고 언제 F를 쓰나요?\n" << std::endl;

    std::cout << "Essential Matrix (E):" << std::endl;
    std::cout << "   - 정규화 좌표 (캘리브레이션 사용)" << std::endl;
    std::cout << "   - x'^T * E * x = 0" << std::endl;
    std::cout << "   - E = [t]_× * R" << std::endl;
    std::cout << "   - 카메라 캘리브레이션 필요\n" << std::endl;

    std::cout << "Fundamental Matrix (F):" << std::endl;
    std::cout << "   - 픽셀 좌표 (캘리브레이션 불필요)" << std::endl;
    std::cout << "   - p'^T * F * p = 0" << std::endl;
    std::cout << "   - F = K'^-T * E * K^-1" << std::endl;
    std::cout << "   - 카메라 캘리브레이션 불필요" << std::endl;
}

void problem3_eight_point()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 8-Point Algorithm" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "질문: 왜 최소 8개 점이 필요한가요?\n" << std::endl;

    std::cout << "F는 3×3 행렬이지만:" << std::endl;
    std::cout << "   - 9개 원소 - 1 (스케일) = 8 자유도" << std::endl;
    std::cout << "   - 각 대응점 → 1개 제약식" << std::endl;
    std::cout << "   - 따라서 최소 8개 점 필요" << std::endl;
    std::cout << "\n💡 실제로는 RANSAC으로 더 많은 점 사용!" << std::endl;
}

void problem4_pose_recovery()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 포즈 복원" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "E에서 R, t를 复원하면 4가지 해가 나옵니다.\n" << std::endl;

    std::cout << "4가지 조합:" << std::endl;
    std::cout << "   1. (R1, t)" << std::endl;
    std::cout << "   2. (R1, -t)" << std::endl;
    std::cout << "   3. (R2, t)" << std::endl;
    std::cout << "   4. (R2, -t)\n" << std::endl;

    std::cout << "질문: 어떻게 올바른 해를 선택하나요?\n" << std::endl;

    std::cout << "💡 Cheirality Check:" << std::endl;
    std::cout << "   - 3D 점을 삼각측량" << std::endl;
    std::cout << "   - 두 카메라 앞에 있는지 확인 (Z > 0)" << std::endl;
    std::cout << "   - 대부분의 점이 앞에 있는 해 선택" << std::endl;
}

/**
 * @brief F 행렬에서 에피폴(epipole) 추출
 *
 * 에피폴은 F의 null space에서 구한다.
 * Fe₁ = 0 (오른쪽 null space) → e₁ = 이미지 1의 에피폴
 * Fᵀe₂ = 0 (왼쪽 null space) → e₂ = 이미지 2의 에피폴
 *
 * SVD를 이용해 null space를 구하고, 기하학적 의미를 설명한다.
 */
void problem5_epipole_from_F()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: 에피폴 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 주어진 Fundamental Matrix
    cv::Mat F = (cv::Mat_<double>(3, 3) <<
        1.0e-7, -2.0e-5,  0.004,
        2.0e-5,  3.0e-8, -0.010,
       -0.005,   0.012,   1.0);

    std::cout << "Fundamental Matrix F:" << std::endl;
    std::cout << F << std::endl;

    // TODO: SVD로 F의 오른쪽 null space 구하기
    // 힌트: cv::SVD::compute(F, w, u, vt)
    //        Vt의 마지막 행 = 가장 작은 특이값에 대응하는 벡터
    //        e1 = Vt.row(2) → 동차 좌표를 정규화 (w=1)

    // TODO: F^T의 오른쪽 null space로 e2 구하기

    std::cout << "\n기하학적 의미:" << std::endl;
    std::cout << "   - 에피폴 = 다른 카메라 중심이 이미지에 투영된 점" << std::endl;
    std::cout << "   - 모든 에피폴라 선은 에피폴을 지남" << std::endl;
    std::cout << "   - 에피폴이 이미지 밖이면 에피폴라 선이 거의 평행" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 5 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_epipolar_constraint();
    problem2_essential_vs_fundamental();
    problem3_eight_point();
    problem4_pose_recovery();
    problem5_epipole_from_F();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
