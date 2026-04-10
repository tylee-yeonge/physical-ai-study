/**
 * @file my_basic.cpp
 * @brief Week 4: 사용자 구현 뼈대 — 삼각측량 + PnP
 *
 * basic.cpp 를 참고하여 각 Step 을 직접 구현하세요.
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 4: My Implementation" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ── Step 1: 카메라 + 3D 박스 설정 ──
    // TODO: K 행렬 생성 (fx=fy=500, cx=320, cy=240)
    //       자동차 크기 3D 박스 8 코너 생성 (중심 (1,0,8), 크기 1.8×1.5×4.5)
    std::cout << "[Step 1] TODO: 카메라 + 3D 박스 설정" << std::endl;

    // ── Step 2: 두 카메라에서 투영 ──
    // TODO: 카메라 1 (R=I, t=0) 에서 8 코너 투영
    //       카메라 2 (R=I, t=[0.5,0,0]) 에서 8 코너 투영
    //       Pc = R * Pw + t → u = fx*Xc/Zc + cx, v = fy*Yc/Zc + cy
    std::cout << "[Step 2] TODO: 두 카메라에서 3D 박스 투영" << std::endl;

    // ── Step 3: 삼각측량 ──
    // TODO: P1 = K * [R1|t1], P2 = K * [R2|t2]
    //       cv::triangulatePoints(P1, P2, pts1, pts2, points_4d)
    //       동차 좌표 → 3D 좌표 변환 (x/w, y/w, z/w)
    //       원본 vs 복원 비교
    std::cout << "[Step 3] TODO: 삼각측량으로 3D 점 복원" << std::endl;

    // ── Step 4: PnP ──
    // TODO: cv::solvePnP(objectPoints, imagePoints, K, noArray(), rvec, tvec)
    //       추정된 tvec 과 실제 t2 비교
    std::cout << "[Step 4] TODO: PnP 로 카메라 포즈 추정" << std::endl;

    // ── Step 5: 재투영 오차 ──
    // TODO: cv::projectPoints(objPts, rvec, tvec, K, noArray(), projected)
    //       각 점의 ||projected - observed||₂ 계산
    //       평균/최대 오차 출력
    std::cout << "[Step 5] TODO: 재투영 오차 계산" << std::endl;

    std::cout << "\n📌 각 Step 의 TODO 를 채우고 결과를 확인하세요!" << std::endl;

    return 0;
}
