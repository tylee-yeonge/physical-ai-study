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

    // ── 용어 정리 ──
    // 내재 (Intrinsic): 카메라 자체의 광학 특성. 초점거리 (fx, fy), 광학 중심 (cx, cy) 등.
    //                   같은 카메라면 위치/방향이 바뀌어도 변하지 않음. K 행렬 (3x3) 로 표현.
    //                   "이 카메라는 어떤 렌즈/센서를 가졌나" 를 기술.
    // 외재 (Extrinsic): 카메라가 월드의 어디에, 어떤 방향으로 놓여 있는지. 회전 R (3x3) + 이동 t (3x1).
    //                   카메라를 움직이면 외재가 바뀜. 같은 3D 점도 외재에 따라 다른 픽셀에 찍힘.
    //                   "이 카메라는 지금 어디에 있나" 를 기술.

    // ── Step 1: 카메라 + 3D 박스 설정 ──
    // TODO: 이후 단계에서 사용할 합성 데이터를 준비한다.
    //       - 카메라 내재 K (3x3): fx = fy = 500 px, cx = 320, cy = 240 (640x480 가정)
    //       - 3D 박스: 중심 (1, 0, 8) m, 크기 1.8 x 1.5 x 4.5 m (W x H x D, 자동차 한 대 크기)
    //       - 위 중심/크기로부터 8 개 코너 좌표 생성
    //       이 박스를 "정답 (ground truth)" 으로 두고 Step 2-5 의 결과를 검증한다.
    std::cout << "[Step 1] TODO: 카메라 + 3D 박스 설정" << std::endl;

    // ── Step 2: 두 카메라에서 투영 ──
    // TODO: 박스 8 코너가 baseline 만큼 떨어진 두 카메라에서 각각 어느 픽셀에 찍히는지 계산한다.
    //       - 카메라 1 외재: R = I (3x3 단위행렬), t = [0, 0, 0]   (원점)
    //       - 카메라 2 외재: R = I,                  t = [0.5, 0, 0]  (X 방향 0.5 m baseline)
    //       - 각 코너에 대해 월드 -> 카메라 좌표 변환 후 핀홀 투영으로 픽셀 좌표 산출
    //       - 카메라 뒤쪽 (Zc <= 0) 에 위치한 점은 보이지 않음으로 처리
    //       산출물: 카메라 1 픽셀 8 개 + 카메라 2 픽셀 8 개 (Step 3 의 입력)
    std::cout << "[Step 2] TODO: 두 카메라에서 3D 박스 투영" << std::endl;

    // ── Step 3: 삼각측량 (Triangulation) ──
    // TODO: Step 2 의 두 카메라 픽셀 쌍과 알려진 카메라 (K, R, t) 만으로
    //       원래 3D 코너 위치를 복원한다.
    //
    // [직관] 픽셀 하나 = 카메라 중심에서 뻗는 광선 (ray) 하나.
    //        픽셀 하나로는 깊이를 알 수 없지만 (광선 위 어디든 같은 픽셀로 찍힘),
    //        다른 위치의 카메라가 같은 점을 보면 광선이 하나 더 생긴다.
    //        두 광선이 만나는 지점이 곧 3D 점.
    //
    //              X (3D point)
    //              *
    //             / \
    //            /   \
    //          C1     C2     C1, C2: camera centers
    //          |       |
    //        pix1    pix2
    //
    //        baseline (C1-C2 거리) 이 0 이면 두 광선이 같아져 교차점이 정의되지 않음.
    //        베이스라인이 클수록 깊이 추정의 수치 안정성이 좋다.
    //
    // [수학 유도]
    //   핀홀 모델을 동차 좌표로 쓰면
    //       s * x = P * X_h ,   P = K * [R | t]   (3x4 투영행렬)
    //       x   : 픽셀 동차 좌표 [u, v, 1]^T
    //       X_h : 3D 점 동차 좌표 [X, Y, Z, 1]^T
    //       s   : 미지의 스케일 (= 카메라 좌표계의 깊이 Zc)
    //
    //   "s * x = P * X_h" 는 두 벡터가 동일하다는 게 아니라
    //   "P*X_h 가 x 의 어떤 배수다" = "두 벡터가 평행하다" 라는 뜻.
    //   평행한 두 벡터의 외적은 0 이므로 양변에 x 를 외적하면
    //       x x (s * x) = x x (P * X_h)
    //       s * (x x x) = x x (P * X_h)        <- 외적 성질: a x a = 0
    //               0   = x x (P * X_h)        <- 미지수 s 가 깔끔히 소거됨
    //
    // [외적 공식]
    //                [ a2*b3 - a3*b2 ]
    //       a x b =  [ a3*b1 - a1*b3 ]
    //                [ a1*b2 - a2*b1 ]
    //
    //   x = [u, v, 1]^T, P*X_h = [p1^T*X_h, p2^T*X_h, p3^T*X_h]^T 에 대입하면
    //   세 성분 = 0 이 나오지만, 외적 결과는 rank 2 이므로 선형 독립인 식은 2 개.
    //   카메라 한 대당 다음 두 식이 X_h 에 대한 선형 제약이 된다.
    //       u * (p3^T * X_h) - (p1^T * X_h) = 0
    //       v * (p3^T * X_h) - (p2^T * X_h) = 0
    //
    //   카메라 1 + 카메라 2 -> 식 4 개 -> 4x4 선형 시스템 A * X_h = 0.
    //   A 의 SVD 의 최소 특이값에 해당하는 우특이벡터가 X_h 의 비자명 해.
    //   OpenCV cv::triangulatePoints 가 이 DLT 과정을 대신 수행한다.
    //
    // [구현 절차]
    //   1) 외재 합치기:   cv::hconcat(R, t, Rt)              // 3x4
    //   2) 투영행렬:      P = K * Rt                          // 3x4
    //                     P.convertTo(P, CV_64F)              // dtype 통일
    //   3) 삼각측량:      cv::triangulatePoints(P1, P2, pts1f, pts2f, points_4d)
    //                     - pts 는 Point2f (float) 여야 함
    //                     - points_4d 는 4xN, dtype 은 float
    //   4) 동차 -> 3D:    X = points_4d[0:3, i] / points_4d[3, i]
    //
    // [검증] 복원된 코너와 Step 1 원본의 코너별 거리 (m) 출력.
    //        합성 데이터 (노이즈 없음) 이므로 평균 오차는 1e-6 m 수준 (사실상 0).
    //
    // [자주 발생하는 실수]
    //   - P 의 dtype 이 float 으로 남음           -> CV_64F 로 명시 변환
    //   - points_4d 를 at<double> 로 읽음         -> at<float> 가 맞음
    //   - 카메라 뒤 (-1, -1) 픽셀을 그대로 입력    -> 보이는 점만 골라 전달
    std::cout << "[Step 3] TODO: 삼각측량으로 3D 점 복원" << std::endl;

    // ── Step 4: PnP ──
    // TODO: 원본 3D 코너(월드 좌표) 와 카메라 2 의 2D 관측만 가지고
    //       카메라 2 의 자세 (회전 + 이동) 를 역으로 추정한다.
    //       - 입력: 3D-2D 대응 쌍 (8 점), 내재 K
    //       - 출력: 카메라 2 의 회전과 이동 (OpenCV 가 요구하는 표현 형태로)
    //       검증: 추정한 이동 벡터를 Step 2 의 실제 t2 = [0.5, 0, 0] 과 비교
    //             합성 데이터 기준 오차는 1e-6 m 수준 이어야 함
    std::cout << "[Step 4] TODO: PnP 로 카메라 포즈 추정" << std::endl;

    // ── Step 5: 재투영 오차 ──
    // TODO: Step 4 에서 추정한 자세로 3D 코너를 다시 카메라 2 평면에 투영한 후
    //       원래 관측 픽셀과의 픽셀 단위 거리를 측정한다 (Reprojection Error).
    //       - 코너별 오차 = || 재투영 픽셀 - 원래 관측 픽셀 ||
    //       - 평균/최대 오차를 출력해 PnP 결과 품질을 정량화
    //       검증 기준: 합성 데이터 -> 평균 < 0.01 px (사실상 0)
    //                 실제 데이터 -> 평균 < 1 px 우수 / 1-3 px 양호 / > 5 px 불량
    //       Perception 맥락: 3D Detection 평가/학습에서 재투영 오차는 표준 지표
    std::cout << "[Step 5] TODO: 재투영 오차 계산" << std::endl;

    std::cout << "\n📌 각 Step 의 TODO 를 채우고 결과를 확인하세요!" << std::endl;

    return 0;
}
