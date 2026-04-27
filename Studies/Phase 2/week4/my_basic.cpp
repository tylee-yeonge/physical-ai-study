/**
 * @file my_basic.cpp
 * @brief Week 4: 사용자 구현 뼈대 — 삼각측량 + PnP
 *
 * basic.cpp 를 참고하여 각 Step 을 직접 구현하세요.
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

void save_output(const std::string& name, const cv::Mat& image)
{
    std::filesystem::create_directories("output");
    std::string path = "output/" + name + ".png";
    cv::imwrite(path, image);
    std::cout << "   저장: " << path << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 4: My Implementation" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ── Step 1: 카메라 + 3D 박스 설정 ──
    // TODO: 이후 단계에서 사용할 합성 데이터를 준비한다.
    //       - 카메라 내재 K (3x3): 합리적인 초점거리(fx, fy) 와 광학 중심(cx, cy)
    //       - 3D 박스 8 코너: 자동차 정도 크기, 카메라 정면에서 약간 떨어진 위치
    //       이 박스를 "정답 (ground truth)" 으로 두고 Step 2-5 의 결과를 검증한다.
    std::cout << "[Step 1] TODO: 카메라 + 3D 박스 설정" << std::endl;

    // ── Step 2: 두 카메라에서 투영 ──
    // TODO: 박스 8 코너가 baseline 만큼 떨어진 두 카메라에서 각각 어느 픽셀에 찍히는지 계산한다.
    //       - 두 카메라의 외재 (R, t) 정의: 카메라 1 은 원점, 카메라 2 는 옆으로 약간 이동
    //       - 각 코너에 대해 월드 -> 카메라 좌표 변환 후 핀홀 투영으로 픽셀 좌표 산출
    //       - 카메라 뒤쪽 (Zc <= 0) 에 위치한 점은 보이지 않음으로 처리
    //       산출물: 카메라 1 픽셀 8 개 + 카메라 2 픽셀 8 개 (Step 3 의 입력)
    std::cout << "[Step 2] TODO: 두 카메라에서 3D 박스 투영" << std::endl;

    // ── Step 3: 삼각측량 ──
    // TODO: Step 2 의 두 카메라 픽셀 쌍과 알려진 카메라 (K, R, t) 만으로
    //       원래 3D 코너 위치를 복원한다 (Triangulation).
    //       - 각 카메라의 3x4 투영행렬을 구성해 삼각측량 알고리즘의 입력으로 사용
    //       - 결과는 동차 좌표(4D) 형태이므로 일반 3D 좌표로 정규화 필요
    //       검증: 복원된 코너와 Step 1 원본의 코너별 거리(m) 를 출력해 정확도 확인
    std::cout << "[Step 3] TODO: 삼각측량으로 3D 점 복원" << std::endl;

    // ── Step 4: PnP ──
    // TODO: 원본 3D 코너(월드 좌표) 와 카메라 2 의 2D 관측만 가지고
    //       카메라 2 의 자세 (회전 + 이동) 를 역으로 추정한다.
    //       - 입력: 3D-2D 대응 쌍, 내재 K
    //       - 출력: 카메라 2 의 회전과 이동 (OpenCV 가 요구하는 표현 형태로)
    //       검증: 추정한 이동 벡터가 Step 2 에서 사용한 실제 t2 와 얼마나 일치하는지 비교
    std::cout << "[Step 4] TODO: PnP 로 카메라 포즈 추정" << std::endl;

    // ── Step 5: 재투영 오차 ──
    // TODO: Step 4 에서 추정한 자세로 3D 코너를 다시 카메라 2 평면에 투영한 후
    //       원래 관측 픽셀과의 픽셀 단위 거리를 측정한다 (Reprojection Error).
    //       - 코너별 오차 = || 재투영 픽셀 - 원래 관측 픽셀 ||
    //       - 평균/최대 오차를 출력해 PnP 결과 품질을 정량화
    //       Perception 맥락: 3D Detection 평가/학습에서 재투영 오차는 표준 지표
    std::cout << "[Step 5] TODO: 재투영 오차 계산" << std::endl;

    std::cout << "\n📌 각 Step 의 TODO 를 채우고 결과를 확인하세요!" << std::endl;

    return 0;
}
