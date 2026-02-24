/**
 * Phase 2 Week 8 - Optical Flow 중급 퀴즈
 *
 * 다루는 개념:
 *   - cv::calcOpticalFlowPyrLK 사용법 (Pyramidal LK)
 *   - 추적 품질 평가 (status, error, backward check)
 *   - Optical Flow vs Feature Matching 비교
 *   - LK 단일 점 직접 구현 (Sobel → AᵀA → 역행렬)
 *   - 윈도우 크기별 LK 정확도 트레이드오프 분석
 *
 * 선수 지식: week8 easy (LK 이론, Aperture Problem, Pyramid)
 *
 * LK Optical Flow 구현의 핵심 단계:
 *   1. Sobel로 Ix, Iy 계산 → It = Frame2 - Frame1
 *   2. 윈도우 내 (Ix, Iy, It) 수집 → A 행렬, b 벡터 구성
 *   3. Structure Tensor AᵀA (2×2) 구성
 *   4. [u, v]ᵀ = (AᵀA)⁻¹ · Aᵀb 로 flow 계산
 */

#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>
#include <iostream>
#include <cmath>
#include <cstdio>

// cv::calcOpticalFlowPyrLK 기본 사용 — Pyramidal Lucas-Kanade
//
// 함수 시그니처:
//   calcOpticalFlowPyrLK(prevImg, nextImg, prevPts, nextPts,
//                        status, err, winSize, maxLevel)
//   - prevImg, nextImg: 그레이스케일 입력 이미지
//   - prevPts: 추적할 점들 (vector<Point2f>)
//   - nextPts: 출력 — 추적된 점 위치
//   - status: 각 점의 추적 성공 여부 (1=성공, 0=실패)
//   - err: 각 점의 추적 오차
//   - winSize: LK 윈도우 크기 (기본 Size(21,21))
//   - maxLevel: 피라미드 레벨 수 (기본 3)
//
// 이 문제에서는 합성 이미지(원 이동)로 추적 정확도를 확인한다.
// ★ OpenCV 내부적으로 Pyramid + 반복 LK + 서브픽셀 보간 수행
void problem1_implement_lk()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: Lucas-Kanade 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 두 프레임 생성
    cv::Mat frame1 = cv::Mat::zeros(300, 400, CV_8UC1);
    cv::Mat frame2 = cv::Mat::zeros(300, 400, CV_8UC1);

    cv::circle(frame1, cv::Point(150, 150), 30, cv::Scalar(255), -1);
    cv::circle(frame2, cv::Point(180, 160), 30, cv::Scalar(255), -1);

    // 특징점
    std::vector<cv::Point2f> pts1 = {cv::Point2f(150, 150)};
    std::vector<cv::Point2f> pts2;
    std::vector<uchar> status;
    std::vector<float> err;

    // TODO: calcOpticalFlowPyrLK 사용
    cv::calcOpticalFlowPyrLK(frame1, frame2, pts1, pts2, status, err);

    if (status[0])
    {
        std::cout << "추적 성공!" << std::endl;
        std::cout << "   이동: (" << (pts2[0].x - pts1[0].x) << ", " << (pts2[0].y - pts1[0].y)
                  << ")" << std::endl;
    }

    std::cout << "\n💡 OpenCV가 Pyramidal LK 자동 수행" << std::endl;
}

// 추적 품질 평가 — 언제 추적이 실패하는지 감지하는 방법
//
// LK 추적이 실패하는 경우:
//   - 물체가 가려짐 (occlusion)
//   - 급격한 조명 변화
//   - 빠른 움직임 (피라미드 레벨 초과)
//   - 텍스처 없는 영역
//
// 실패 감지 3가지 방법:
//   1. status 확인: status[i] == 0이면 추적 실패
//   2. error 임계값: err[i] > threshold이면 품질 불량
//   3. Backward Check (역방향 확인):
//      pts1 → pts2 추적 후, pts2 → pts1' 역추적
//      ||pts1 - pts1'|| > threshold이면 신뢰 불가
//
// 실전 팁:
//   - 일정 프레임마다 goodFeaturesToTrack으로 특징점 재검출
//   - 이미지 경계 근처 점은 제거 (윈도우가 잘림)
//   - 추적 점 수가 임계값 이하로 떨어지면 전체 재초기화
//
// ★ VINS-Mono에서는 Forward-Backward Error < 1px 기준 사용
void problem2_track_quality()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: 추적 품질 평가" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "좋은 추적 점 선택:" << std::endl;
    std::cout << "   - 높은 gradient (코너)" << std::endl;
    std::cout << "   - goodFeaturesToTrack()\n" << std::endl;

    std::cout << "추적 실패 감지:" << std::endl;
    std::cout << "   1. status == 0" << std::endl;
    std::cout << "   2. error > threshold" << std::endl;
    std::cout << "   3. Backward check (역방향 확인)\n" << std::endl;

    std::cout << "💡 실전 팁:" << std::endl;
    std::cout << "   - 주기적으로 특징점 재검출" << std::endl;
    std::cout << "   - 이미지 경계 점 제거" << std::endl;
}

// Optical Flow vs Feature Matching — 두 추적 방법의 비교
//
// ┌──────────────┬──────────────────┬──────────────────┐
// │   항목        │  Optical Flow    │ Feature Matching │
// ├──────────────┼──────────────────┼──────────────────┤
// │ 속도          │ 빠름 ✓          │ 느림 (descriptor)│
// │ 큰 움직임     │ 제한적 (Pyramid) │ OK ✓            │
// │ 조명 변화     │ 민감 ✗          │ 강건 ✓          │
// │ 텍스처 필요   │ 적음            │ 많음             │
// │ 정확도        │ 서브픽셀 ✓      │ 정수 픽셀        │
// │ 넓은 베이스라인│ 불가 ✗         │ 가능 ✓          │
// │ 드리프트      │ 누적됨 ✗       │ 독립적 ✓        │
// └──────────────┴──────────────────┴──────────────────┘
//
// ★ 최적 조합 (Hybrid 전략):
//   - 일반 Tracking: Optical Flow (빠른 프레임 간 추적)
//   - Relocalization: Feature Matching (추적 실패 시 복구)
//   - Loop Closure: Feature Matching (과거 프레임 인식)
void problem3_flow_vs_matching()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: Flow vs Feature Matching" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Optical Flow (Lucas-Kanade):" << std::endl;
    std::cout << "   장점: 빠름, 부드러움" << std::endl;
    std::cout << "   단점: 작은 움직임만, 조명 민감\n" << std::endl;

    std::cout << "Feature Matching (ORB + BF):" << std::endl;
    std::cout << "   장점: 큰 움직임, robust" << std::endl;
    std::cout << "   단점: 느림, 텍스처 필요\n" << std::endl;

    std::cout << "💡 최선의 조합:" << std::endl;
    std::cout << "   - Tracking: Optical Flow (빠름)" << std::endl;
    std::cout << "   - Relocalization: Feature Matching (robust)" << std::endl;
}

// LK 단일 점 직접 구현 — Sobel 그래디언트부터 AᵀA 역행렬까지
//
// 구현 단계:
//   1. 그래디언트 계산:
//      Ix = Sobel_x(I₁+I₂)/2  (두 프레임 평균으로 안정화)
//      Iy = Sobel_y(I₁+I₂)/2
//      It = I₂ - I₁  (시간 변화)
//
//   2. 윈도우 내 데이터 수집:
//      추적점 주변 21×21 윈도우에서 (Ix, Iy, It) 값 수집
//
//   3. Structure Tensor 구성:
//      AᵀA = [ ΣIx²    ΣIxIy ]    Aᵀb = [ -ΣIxIt ]
//            [ ΣIxIy   ΣIy²  ]          [ -ΣIyIt ]
//
//   4. Flow 계산:
//      [u, v]ᵀ = (AᵀA)⁻¹ · Aᵀb
//
//   5. 고유값 확인:
//      AᵀA의 고유값으로 추적 가능성 판별
//      λ₁, λ₂ 모두 크면 코너 = 추적 안정
//
// ★ OpenCV LK와 차이가 나는 이유:
//   OpenCV는 피라미드 + 반복 최적화 + 서브픽셀 보간 적용
//   직접 구현은 단일 레벨, 1회 계산
void problem4_lk_single_point()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Lucas-Kanade 단일 점 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 두 프레임 생성: 원이 오른쪽+아래로 이동
    cv::Mat frame1 = cv::Mat::zeros(200, 200, CV_8UC1);
    cv::Mat frame2 = cv::Mat::zeros(200, 200, CV_8UC1);

    cv::circle(frame1, cv::Point(80, 80), 20, cv::Scalar(200), -1);
    cv::circle(frame2, cv::Point(90, 85), 20, cv::Scalar(200), -1);

    // 실제 이동량
    double true_u = 10.0, true_v = 5.0;
    std::cout << "실제 이동: u=" << true_u << ", v=" << true_v << "\n" << std::endl;

    // 그래디언트 계산 (Sobel)
    cv::Mat Ix, Iy, It;
    cv::Mat frame1_f, frame2_f;
    frame1.convertTo(frame1_f, CV_64F);
    frame2.convertTo(frame2_f, CV_64F);

    cv::Mat Ix1, Ix2, Iy1, Iy2;
    cv::Sobel(frame1_f, Ix1, CV_64F, 1, 0, 3);
    cv::Sobel(frame2_f, Ix2, CV_64F, 1, 0, 3);
    cv::Sobel(frame1_f, Iy1, CV_64F, 0, 1, 3);
    cv::Sobel(frame2_f, Iy2, CV_64F, 0, 1, 3);

    // 두 프레임 평균
    Ix = (Ix1 + Ix2) / 2.0;
    Iy = (Iy1 + Iy2) / 2.0;
    It = frame2_f - frame1_f;

    // 추적할 점 (원 중심)
    int px = 80, py = 80;
    int window_size = 21;
    int half_w = window_size / 2;

    // 윈도우 내 A, b 구성
    std::vector<double> Ix_win, Iy_win, It_win;

    for (int y = py - half_w; y <= py + half_w; y++)
    {
        for (int x = px - half_w; x <= px + half_w; x++)
        {
            if (y >= 0 && y < Ix.rows && x >= 0 && x < Ix.cols)
            {
                Ix_win.push_back(Ix.at<double>(y, x));
                Iy_win.push_back(Iy.at<double>(y, x));
                It_win.push_back(It.at<double>(y, x));
            }
        }
    }

    int n = (int)Ix_win.size();

    // A^T A (2x2) 와 A^T b (2x1) 직접 계산
    double sum_IxIx = 0, sum_IxIy = 0, sum_IyIy = 0;
    double sum_IxIt = 0, sum_IyIt = 0;

    for (int i = 0; i < n; i++)
    {
        sum_IxIx += Ix_win[i] * Ix_win[i];
        sum_IxIy += Ix_win[i] * Iy_win[i];
        sum_IyIy += Iy_win[i] * Iy_win[i];
        sum_IxIt += Ix_win[i] * It_win[i];
        sum_IyIt += Iy_win[i] * It_win[i];
    }

    cv::Mat AtA = (cv::Mat_<double>(2, 2) << sum_IxIx, sum_IxIy, sum_IxIy, sum_IyIy);
    cv::Mat Atb = (cv::Mat_<double>(2, 1) << -sum_IxIt, -sum_IyIt);

    // (A^T A)^-1 * A^T b
    cv::Mat flow_vec = AtA.inv() * Atb;

    double u_est = flow_vec.at<double>(0);
    double v_est = flow_vec.at<double>(1);

    std::cout << "직접 구현 LK 결과:" << std::endl;
    std::cout << "   u=" << u_est << ", v=" << v_est << std::endl;

    // 고유값 확인 (추적 가능성)
    cv::Mat eigvals;
    cv::eigen(AtA, eigvals);
    std::cout << "   고유값: λ1=" << eigvals.at<double>(0)
              << ", λ2=" << eigvals.at<double>(1) << "\n" << std::endl;

    // OpenCV calcOpticalFlowPyrLK 비교
    std::vector<cv::Point2f> pts1 = {cv::Point2f((float)px, (float)py)};
    std::vector<cv::Point2f> pts2;
    std::vector<uchar> status;
    std::vector<float> err;

    cv::calcOpticalFlowPyrLK(frame1, frame2, pts1, pts2, status, err,
                             cv::Size(window_size, window_size), 0);

    if (status[0])
    {
        double u_cv = pts2[0].x - pts1[0].x;
        double v_cv = pts2[0].y - pts1[0].y;
        std::cout << "OpenCV LK 결과:" << std::endl;
        std::cout << "   u=" << u_cv << ", v=" << v_cv << "\n" << std::endl;
    }

    std::cout << "💡 직접 구현과 OpenCV 결과가 유사하면 정상!" << std::endl;
    std::cout << "   차이가 있다면: OpenCV는 피라미드 + 반복 최적화 사용" << std::endl;
}

// 윈도우 크기별 LK 정확도 분석 — Spatial Coherence 범위의 트레이드오프
//
// LK는 윈도우 내 모든 픽셀이 같은 flow를 가진다고 가정한다.
// 윈도우 크기가 이 가정의 유효 범위를 결정한다.
//
// 작은 윈도우 (5×5):
//   + 정밀한 추적 (서로 다른 움직임 구분 가능)
//   + 작은 물체/근접 경계면 추적 가능
//   - 제약식 부족 → 노이즈에 민감
//   - AᵀA의 조건수 나빠짐
//   - 큰 이동에 대응 불가
//
// 큰 윈도우 (41×41):
//   + 많은 제약식 → 노이즈에 강건
//   + 큰 이동도 처리 가능 (윈도우 내에 들어옴)
//   - 서로 다른 움직임이 혼합됨 (물체 경계에서 부정확)
//   - Spatial Coherence 가정이 깨지기 쉬움
//   - 계산량 증가
//
// ★ 실전 권장: 15~31 (VINS-Mono 기본값: 21×21)
//   피라미드를 사용하면 작은 윈도우로도 큰 이동 대응 가능
void problem5_window_size_analysis()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: 윈도우 크기별 LK 정확도" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 프레임 생성
    cv::Mat frame1 = cv::Mat::zeros(300, 300, CV_8UC1);
    cv::Mat frame2 = cv::Mat::zeros(300, 300, CV_8UC1);

    // 여러 도형 배치
    cv::rectangle(frame1, cv::Rect(50, 50, 60, 40), cv::Scalar(200), -1);
    cv::circle(frame1, cv::Point(200, 200), 25, cv::Scalar(180), -1);

    // 이동 (dx=8, dy=5)
    cv::rectangle(frame2, cv::Rect(58, 55, 60, 40), cv::Scalar(200), -1);
    cv::circle(frame2, cv::Point(208, 205), 25, cv::Scalar(180), -1);

    double true_u = 8.0, true_v = 5.0;

    // 추적할 점 (사각형 코너)
    std::vector<cv::Point2f> pts1 = {cv::Point2f(50, 50)};

    std::cout << "실제 이동: u=" << true_u << ", v=" << true_v << "\n" << std::endl;
    std::cout << "윈도우 크기  |  추정 (u, v)     |  오차" << std::endl;
    std::cout << "-------------+------------------+--------" << std::endl;

    std::vector<int> window_sizes = {5, 11, 21, 41};

    for (int ws : window_sizes)
    {
        std::vector<cv::Point2f> pts2;
        std::vector<uchar> status;
        std::vector<float> err;

        cv::calcOpticalFlowPyrLK(frame1, frame2, pts1, pts2, status, err,
                                 cv::Size(ws, ws), 0);

        if (status[0])
        {
            double u_est = pts2[0].x - pts1[0].x;
            double v_est = pts2[0].y - pts1[0].y;
            double error = sqrt(pow(u_est - true_u, 2) + pow(v_est - true_v, 2));

            printf("    %2d x %-2d  |  (%6.2f, %5.2f)  |  %5.2f\n",
                   ws, ws, u_est, v_est, error);
        }
        else
        {
            printf("    %2d x %-2d  |  추적 실패        |   -\n", ws, ws);
        }
    }

    std::cout << "\n트레이드오프:" << std::endl;
    std::cout << "   작은 윈도우 (5x5):" << std::endl;
    std::cout << "     + 정밀, 작은 물체 추적 가능" << std::endl;
    std::cout << "     - 노이즈에 민감, 큰 이동에 실패\n" << std::endl;

    std::cout << "   큰 윈도우 (41x41):" << std::endl;
    std::cout << "     + 안정적, 큰 이동 처리 가능" << std::endl;
    std::cout << "     - 뭉뚱그려짐, 여러 움직임 혼합\n" << std::endl;

    std::cout << "💡 실전 권장: 15~31 (VINS 기본값: 21x21)" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 8 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_implement_lk();
    problem2_track_quality();
    problem3_flow_vs_matching();
    problem4_lk_single_point();
    problem5_window_size_analysis();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
