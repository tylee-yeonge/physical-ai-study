/**
 * Phase 2 Week 5 - 에피폴라 기하학 기초 퀴즈 (정답)
 *
 * 다루는 개념:
 *   - 에피폴라 제약 (Epipolar Constraint): p2^T · E · p1 = 0
 *   - Essential Matrix (E) vs Fundamental Matrix (F)
 *   - 8-Point Algorithm의 원리
 *   - E에서 R, t 복원 (포즈 분해)
 *   - 에피폴 (Epipole) 추출
 *
 * 에피폴라 기하학은 두 카메라 뷰 사이의 기하학적 관계를 다룬다.
 * SLAM에서 카메라 포즈 추정의 수학적 기초가 되는 핵심 이론이다.
 *
 *        카메라1          카메라2
 *          O1 ─────────── O2
 *         /|\             /|\
 *        / | \           / | \
 *       /  |  \         /  |  \
 *      e1  |   \       /   |  e2     ← 에피폴 (상대 카메라 중심의 투영)
 *     ─────┼────      ────┼─────
 *   이미지1 |  p1     p2  | 이미지2
 *           |             |
 *           └─────X───────┘          ← 3D 점
 *
 *   에피폴라 선: p1을 지나는 에피폴라 선 위에 p2가 반드시 존재
 *   → 2D 탐색을 1D 탐색으로 줄여줌 (매칭 효율화)
 */

#include <opencv2/opencv.hpp>
#include <iostream>

// 에피폴라 제약 검증 — 두 대응점이 올바른 매칭인지 확인
//
// 에피폴라 제약: p2^T · E · p1 = 0
//   - E: 3×3 Essential Matrix (두 카메라 간 기하 관계 인코딩)
//   - p1, p2: 정규화 좌표 (K^-1 · [u, v, 1]^T)
//
// 이 값이 0에 가까울수록 올바른 대응점.
// SLAM에서 RANSAC으로 outlier를 제거할 때 이 제약을 사용한다.
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

    // ✅ 정답: p2^T * E * p1 계산
    cv::Mat p1_h = (cv::Mat_<double>(3, 1) << p1.x, p1.y, 1.0);
    cv::Mat p2_h = (cv::Mat_<double>(3, 1) << p2.x, p2.y, 1.0);
    cv::Mat result = p2_h.t() * E * p1_h;
    double epipolar_value = result.at<double>(0, 0);
    std::cout << "   p2^T * E * p1 = " << epipolar_value << std::endl;
    std::cout << "   (0에 가까울수록 올바른 매칭)\n" << std::endl;

    std::cout << "💡 에피폴라 제약: p2^T * E * p1 = 0" << std::endl;
    std::cout << "   - 이 제약을 만족하는 점들만 올바른 대응" << std::endl;
    std::cout << "   - RANSAC outlier 제거에 사용" << std::endl;
}

// Essential Matrix vs Fundamental Matrix — 언제 무엇을 쓰는가
//
// Essential Matrix (E):
//   - 정규화 좌표에서 동작: x'^T · E · x = 0
//   - E = [t]_× · R  (카메라 간 R, t로 구성)
//   - 5 자유도 (3 회전 + 2 이동방향, 스케일 제외)
//   - ★ 카메라 캘리브레이션(K)이 필요
//
// Fundamental Matrix (F):
//   - 픽셀 좌표에서 동작: p'^T · F · p = 0
//   - F = K'^-T · E · K^-1  (K로 E를 감싼 형태)
//   - 7 자유도 (9원소 - 스케일 - rank2 제약)
//   - ★ 카메라 캘리브레이션 불필요
//
// SLAM에서는 보통 캘리브레이션된 카메라를 사용하므로 E를 선호한다.
// E에서 직접 R, t를 분해할 수 있어 포즈 추정에 유리하다.
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

// 8-Point Algorithm — F 또는 E를 추정하는 기본 알고리즘
//
// F는 3×3 행렬 = 9개 원소, 하지만:
//   - 스케일 자유도 1개 제거 → 8개 미지수
//   - 각 대응점 (p1, p2)는 1개 방정식 제공: p2^T · F · p1 = 0
//   - 따라서 최소 8개 대응점 필요
//
// 알고리즘:
//   1. 각 대응점에서 방정식 생성 → A 행렬 (N×9) 구성
//   2. SVD로 A 분해 → V의 마지막 열 = f 벡터
//   3. f를 3×3으로 reshape → F̃
//   4. Rank-2 강제: SVD(F̃)에서 가장 작은 특이값을 0으로 → F
//
// ★ Rank-2 강제가 중요: F는 반드시 rank 2여야 에피폴라 선이 한 점(에피폴)에서 만남
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

// E에서 R, t 복원 — SVD 분해로 카메라 포즈 추출
//
// E를 SVD 분해하면: E = U · diag(1, 1, 0) · V^T
//
//   W = [0 -1 0; 1 0 0; 0 0 1]  (90도 회전 행렬)
//
//   4가지 해:
//     R1 = U · W · V^T,    t1 = +U의 3번째 열
//     R1 = U · W · V^T,    t2 = -U의 3번째 열
//     R2 = U · W^T · V^T,  t1 = +U의 3번째 열
//     R2 = U · W^T · V^T,  t2 = -U의 3번째 열
//
// ★ Cheirality Check로 올바른 해 1개 선택:
//   - 삼각측량으로 3D 점 복원
//   - 두 카메라 모두에서 Z > 0인 해 = 올바른 해
//   - (카메라 앞에 점이 있어야 물리적으로 유효)
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
 *   F · e₁ = 0  (오른쪽 null space) → e₁ = 이미지 1의 에피폴
 *   F^T · e₂ = 0 (왼쪽 null space) → e₂ = 이미지 2의 에피폴
 *
 * SVD를 이용해 null space를 구하고, 기하학적 의미를 설명한다.
 *
 * 에피폴의 기하학적 의미:
 *   - 에피폴 = 상대 카메라 중심이 이 이미지에 투영된 점
 *   - 모든 에피폴라 선은 에피폴을 지남 (방사형 패턴)
 *   - 에피폴이 이미지 밖이면 에피폴라 선이 거의 평행
 *     (= 전진 운동, 자동차 주행 시)
 *
 * SVD로 null space 구하기:
 *   F = U · S · V^T 분해 후,
 *   V의 마지막 열 = 가장 작은 특이값(≈0)에 대응 = null space
 *   동차 좌표를 마지막 원소로 나누어 정규화
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

    // ✅ 정답: SVD로 F의 오른쪽 null space 구하기
    cv::Mat w, u, vt;
    cv::SVD::compute(F, w, u, vt);
    cv::Mat e1 = vt.row(2).t();
    e1 = e1 / e1.at<double>(2);
    std::cout << "\n에피폴 1 (이미지 1): " << e1.t() << std::endl;

    // ✅ 정답: F^T의 오른쪽 null space로 e2 구하기
    cv::SVD::compute(F.t(), w, u, vt);
    cv::Mat e2 = vt.row(2).t();
    e2 = e2 / e2.at<double>(2);
    std::cout << "에피폴 2 (이미지 2): " << e2.t() << std::endl;

    std::cout << "\n기하학적 의미:" << std::endl;
    std::cout << "   - 에피폴 = 다른 카메라 중심이 이미지에 투영된 점" << std::endl;
    std::cout << "   - 모든 에피폴라 선은 에피폴을 지남" << std::endl;
    std::cout << "   - 에피폴이 이미지 밖이면 에피폴라 선이 거의 평행" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 5 Quiz - Easy (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_epipolar_constraint();
    problem2_essential_vs_fundamental();
    problem3_eight_point();
    problem4_pose_recovery();
    problem5_epipole_from_F();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
