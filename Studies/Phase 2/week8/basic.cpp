#include "basic.h"
#include <iostream>
#include <iomanip>
#include <cmath>

// Lucas-Kanade Sparse Optical Flow — 특징점 단위 움직임 추적
//
// 핵심 원리 (밝기 항상성 가정):
//   I(x, y, t) = I(x+u, y+v, t+1) → 같은 점은 밝기가 변하지 않는다
//   테일러 전개: Ix·u + Iy·v + It = 0
//   → 1개 방정식, 2개 미지수(u,v) → 조리개 문제(Aperture Problem)
//
// LK 해결법:
//   윈도우(예: 21×21) 내 모든 픽셀이 같은 (u,v)로 이동한다고 가정
//   → N개 방정식, 2개 미지수 → 최소제곱법으로 풀기
//   [u,v]^T = (A^T·A)^(-1) · A^T · b
//   A = [Ix_1, Iy_1; Ix_2, Iy_2; ...], b = [-It_1, -It_2, ...]
//
// 피라미드 LK (Pyramidal LK):
//   기본 LK는 작은 움직임(~수 픽셀)만 추적 가능
//   해결: 이미지 피라미드(축소)에서 큰 움직임을 먼저 추적 → 원본에서 정제
//   maxLevel=3 → Level 3(1/8 해상도)부터 Level 0(원본)까지 순차 추적
//
// SLAM에서의 역할:
//   ORB-SLAM: 특징점 매칭(디스크립터 비교)으로 추적
//   VINS-Mono: LK Optical Flow로 추적 → 디스크립터 계산 불필요, 더 빠름
double OpticalFlowBasic::lucasKanade(const cv::Mat& prev_img, const cv::Mat& curr_img,
                                     const std::vector<cv::Point2f>& prev_pts,
                                     std::vector<cv::Point2f>& curr_pts, std::vector<uchar>& status)
{
    std::vector<float> err;

    // winSize: LK 윈도우 크기. 클수록 큰 움직임 추적 가능하지만 블러 효과
    // 21×21은 일반적인 선택 (VINS-Mono도 유사한 크기 사용)
    cv::Size winSize(21, 21);

    // maxLevel: 피라미드 레벨 수
    // Level 0=원본, Level 1=1/2, Level 2=1/4, Level 3=1/8
    // 20px 움직임 → Level 3에서 2.5px로 축소되어 추적 가능
    int maxLevel = 3;

    // 종료 조건: 최대 30회 반복 또는 수렴 오차 < 0.01
    cv::TermCriteria criteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.01);

    // Pyramidal Lucas-Kanade Optical Flow
    // status[i] = 1: i번째 점 추적 성공, 0: 실패 (이미지 밖 또는 수렴 실패)
    // err[i]: 추적 오차 (윈도우 내 픽셀 차이)
    cv::calcOpticalFlowPyrLK(prev_img, curr_img, prev_pts, curr_pts, status, err, winSize, maxLevel,
                             criteria);

    // 추적 성공률 = 성공한 점 수 / 전체 점 수
    // 성공률이 낮으면 → 특징점 재검출(goodFeaturesToTrack) 필요
    int success_count = 0;
    for (uchar s : status)
    {
        if (s)
            success_count++;
    }

    return (double)success_count / prev_pts.size();
}

// Farneback Dense Optical Flow — 모든 픽셀의 움직임 추정
//
// LK와의 차이:
//   LK (Sparse): 선택된 특징점만 추적 → 빠름 (~1ms)
//   Farneback (Dense): 모든 픽셀의 (u,v) 계산 → 느림 (~50ms)
//
// 동작 원리:
//   ① 각 픽셀 주변을 2차 다항식(polynomial)으로 근사
//      I(x) ≈ x^T · A · x + b^T · x + c
//   ② 두 프레임의 다항식 계수 변화로 움직임 추정
//   ③ 피라미드 + 반복으로 정확도 향상
//
// 출력: flow (CV_32FC2)
//   flow.at<Point2f>(y,x).x = 해당 픽셀의 수평 이동량 (u)
//   flow.at<Point2f>(y,x).y = 해당 픽셀의 수직 이동량 (v)
//
// 용도: 비디오 안정화, 배경 분리, 모션 세그멘테이션
//   SLAM에서는 거의 사용하지 않음 (너무 느림)
void OpticalFlowBasic::farneback(const cv::Mat& prev_img, const cv::Mat& curr_img, cv::Mat& flow)
{
    cv::calcOpticalFlowFarneback(prev_img, curr_img, flow,
                                 0.5,  // pyr_scale: 피라미드 축소 비율 (0.5 = 반으로 축소)
                                 3,    // levels: 피라미드 레벨 수
                                 15,   // winsize: 평균화 윈도우 크기 (클수록 부드러움)
                                 3,    // iterations: 각 레벨에서의 반복 횟수
                                 5,    // poly_n: 다항식 확장 이웃 크기 (5 또는 7)
                                 1.2,  // poly_sigma: 가우시안 표준편차 (poly_n=5일 때 1.1~1.2)
                                 0     // flags: 추가 옵션 없음
    );
}

// Optical Flow 평가 — 추적 성공한 점들의 평균 이동 거리 (픽셀)
//
// 계산:
//   성공한 점(status=1)에 대해서만:
//   dist_i = ||curr_pts[i] - prev_pts[i]||₂  (유클리드 거리)
//   반환값 = Σ dist_i / count
//
// 의미:
//   - 값이 크면: 카메라가 빠르게 이동 또는 물체가 크게 움직임
//   - 값이 0에 가까우면: 거의 움직임 없음 (정지 장면)
//   - 비정상적으로 큰 값: 추적 실패 가능성 → 필터링 필요
double OpticalFlowBasic::evaluateFlow(const std::vector<cv::Point2f>& prev_pts,
                                      const std::vector<cv::Point2f>& curr_pts,
                                      const std::vector<uchar>& status)
{
    double total_dist = 0.0;
    int count = 0;

    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i])  // 추적 성공한 점만 계산
        {
            double dx = curr_pts[i].x - prev_pts[i].x;
            double dy = curr_pts[i].y - prev_pts[i].y;
            total_dist += std::sqrt(dx * dx + dy * dy);
            count++;
        }
    }

    return count > 0 ? total_dist / count : 0.0;
}

// Sparse Optical Flow 시각화 — 추적 궤적 표시
//
// 시각화 요소:
//   녹색 선 (─): 이전 위치 → 현재 위치 (이동 벡터)
//   빨간 점 (●): 현재 프레임에서의 추적된 위치
//   → 선의 방향과 길이가 각 점의 움직임을 나타냄
//
// 실제 SLAM 디버깅에서 자주 사용하는 시각화 패턴
void OpticalFlowBasic::visualizeSparseFlow(const cv::Mat& img,
                                           const std::vector<cv::Point2f>& prev_pts,
                                           const std::vector<cv::Point2f>& curr_pts,
                                           const std::vector<uchar>& status, cv::Mat& output)
{
    if (img.channels() == 1)
    {
        cv::cvtColor(img, output, cv::COLOR_GRAY2BGR);
    }
    else
    {
        output = img.clone();
    }

    for (size_t i = 0; i < prev_pts.size(); i++)
    {
        if (status[i])  // 추적 성공한 점만 표시
        {
            // 이동 벡터 (녹색) — 이전 위치에서 현재 위치로의 선
            cv::line(output, prev_pts[i], curr_pts[i], cv::Scalar(0, 255, 0), 2);

            // 현재 위치 (빨강)
            cv::circle(output, curr_pts[i], 5, cv::Scalar(0, 0, 255), -1);
        }
    }
}

// Dense Optical Flow 시각화 — 화살표 벡터 필드
//
// flow(CV_32FC2)의 각 픽셀에 (u,v) 이동 벡터가 저장되어 있음
// step 간격으로 샘플링하여 화살표로 표시 (모든 픽셀을 그리면 혼잡)
//
// 대안 시각화: HSV 컬러맵
//   H(Hue) = 이동 방향 (각도), S(Saturation) = 이동 크기
//   → 방향별로 다른 색상, 크기별로 다른 채도
//   이 함수에서는 간단한 화살표 방식 사용
void OpticalFlowBasic::visualizeFlow(const cv::Mat& flow, cv::Mat& output, int step)
{
    output = cv::Mat::zeros(flow.size(), CV_8UC3);

    // step 간격으로 샘플링하여 화살표 표시
    // step=10 → 10픽셀마다 하나의 화살표
    for (int y = 0; y < flow.rows; y += step)
    {
        for (int x = 0; x < flow.cols; x += step)
        {
            // flow(y,x) = (u, v) — 해당 픽셀의 이동 벡터
            const cv::Point2f& fxy = flow.at<cv::Point2f>(y, x);

            cv::Point2f start(x, y);
            cv::Point2f end(x + fxy.x, y + fxy.y);

            // 화살표: 시작점 → 끝점 (이동 방향과 크기를 시각적으로 표현)
            cv::arrowedLine(output, start, end, cv::Scalar(0, 255, 0), 1, cv::LINE_AA, 0, 0.3);
        }
    }
}

// Sparse vs Dense Optical Flow 비교 설명
//
// 핵심 차이:
//   Sparse (LK): 관심 있는 점만 추적 → O(N), N=특징점 수
//   Dense (Farneback): 모든 픽셀 추적 → O(W×H), W×H=이미지 크기
//
// SLAM 프레임워크별 선택:
//   ORB-SLAM: 특징점 매칭 (디스크립터 비교, OF 사용 안 함)
//   VINS-Mono: Sparse OF (FAST + Pyramidal LK) → 빠르고 정확
//   LSD-SLAM: Semi-Dense (에지 픽셀만 추적) → 중간 전략
void OpticalFlowBasic::compareSparseVsDense()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Sparse vs Dense Optical Flow 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "1️⃣  Sparse Optical Flow (Lucas-Kanade)" << std::endl;
    std::cout << "   - 선택된 특징점만 추적" << std::endl;
    std::cout << "   - 빠름 (수백 개 점)" << std::endl;
    std::cout << "   - SLAM에 적합 (특징점 기반)\n" << std::endl;

    std::cout << "2️⃣  Dense Optical Flow (Farneback)" << std::endl;
    std::cout << "   - 모든 픽셀의 움직임" << std::endl;
    std::cout << "   - 느림 (수십만 픽셀)" << std::endl;
    std::cout << "   - 비디오 안정화, 배경 분리에 사용\n" << std::endl;

    std::cout << "💡 SLAM에서는 Sparse 사용!" << std::endl;
    std::cout << "   - ORB-SLAM: 특징점 추적" << std::endl;
    std::cout << "   - VINS: Optical Flow + 특징점\n" << std::endl;

    std::cout << "속도 비교 (640x480 이미지):" << std::endl;
    std::cout << "   Sparse (100점): ~1ms" << std::endl;
    std::cout << "   Dense (전체): ~50ms" << std::endl;
}

// Feature Tracking 데모 — 연속 프레임에서 LK로 특징점 추적
//
// SLAM에서의 Feature Tracking 흐름:
//   ① 첫 프레임: goodFeaturesToTrack으로 특징점 검출 (Shi-Tomasi 코너)
//   ② 다음 프레임: LK Optical Flow로 이전 프레임의 점들을 추적
//   ③ 추적 실패한 점 제거
//   ④ 특징점 수가 임계값 이하로 떨어지면 → 재검출
//   → 이 과정을 매 프레임 반복
//
// goodFeaturesToTrack (Shi-Tomasi):
//   Harris 코너 검출의 개선 버전
//   코너 응답 = min(λ1, λ2) > threshold → 코너로 판정
//   LK 추적에 적합한 점을 찾아줌 (텍스처가 풍부한 영역)
//
// 시뮬레이션 시나리오:
//   움직이는 원(50px/frame) + 고정 사각형 패턴 → 10개 프레임
void OpticalFlowBasic::demoFeatureTracking(const std::string& video_path)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Feature Tracking 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 합성 프레임 생성 (실제 영상 대신 시뮬레이션)
    int num_frames = 10;
    std::vector<cv::Mat> frames;

    for (int i = 0; i < num_frames; i++)
    {
        const int kFrameHeight = 480;
        const int kFrameWidth = 640;
        cv::Mat frame = cv::Mat::zeros(kFrameHeight, kFrameWidth, CV_8UC1);

        // 움직이는 원: 매 프레임 X+50px 이동 → LK가 추적해야 하는 대상
        cv::circle(frame, cv::Point(100 + i * 50, 240), 30, cv::Scalar(255), -1);

        // 고정 사각형 패턴: 정적 배경 역할 → 추적이 쉬운 대상
        for (int j = 0; j < 5; j++)
        {
            cv::rectangle(frame, cv::Point(400 + j * 40, 100 + j * 60),
                          cv::Point(430 + j * 40, 130 + j * 60), cv::Scalar(200), -1);
        }

        frames.push_back(frame);
    }

    std::cout << "생성된 프레임: " << frames.size() << "개\n" << std::endl;

    // 첫 프레임에서 특징점 검출 (Shi-Tomasi 코너)
    // maxCorners=100: 최대 100개, qualityLevel=0.01: 최고 응답의 1%까지
    // minDistance=10: 특징점 간 최소 10px 간격 (공간 분포 보장)
    std::vector<cv::Point2f> points;
    cv::goodFeaturesToTrack(frames[0], points, 100, 0.01, 10);

    std::cout << "초기 특징점: " << points.size() << "개\n" << std::endl;

    // 연속 프레임 추적 루프
    std::vector<cv::Point2f> prev_pts = points;

    for (size_t i = 1; i < frames.size(); i++)
    {
        std::vector<cv::Point2f> curr_pts;
        std::vector<uchar> status;

        // LK로 이전 프레임 → 현재 프레임 추적
        double success_rate = lucasKanade(frames[i - 1], frames[i], prev_pts, curr_pts, status);

        std::cout << "Frame " << i << ": "
                  << "추적 성공률 " << std::fixed << std::setprecision(1) << success_rate * 100
                  << "%" << std::endl;

        // 추적 실패한 점 제거 → 성공한 점만 다음 프레임으로 전달
        std::vector<cv::Point2f> good_pts;
        for (size_t j = 0; j < status.size(); j++)
        {
            if (status[j])
            {
                good_pts.push_back(curr_pts[j]);
            }
        }

        prev_pts = good_pts;

        // 특징점이 임계값(10개) 이하로 떨어지면 재검출 필요
        // 실제 SLAM: 새로운 goodFeaturesToTrack 호출 or 키프레임 생성
        if (prev_pts.size() < 10)
        {
            std::cout << "   → 특징점 부족, 재검출 필요\n" << std::endl;
            break;
        }
    }

    std::cout << "\n✅ Feature Tracking 완료!" << std::endl;
}

// Optical Flow 전체 파이프라인 — Sparse(LK) vs Dense(Farneback) 비교
//
// 4단계 파이프라인:
//   ① 두 프레임 생성 (흰 원이 (200,200)→(250,220)으로 이동)
//   ② Sparse OF: goodFeaturesToTrack → LK 추적
//   ③ Dense OF: Farneback으로 전체 flow 계산
//   ④ 시각화 결과 저장
//
// 관찰 포인트:
//   - Sparse: 원의 경계에서만 추적 (텍스처 있는 곳)
//   - Dense: 원 내부 포함 모든 픽셀의 이동 계산
void OpticalFlowBasic::demoPipeline()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Optical Flow 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Step 1: 테스트 프레임 생성
    // frame1: 원이 (200, 200)에 위치
    // frame2: 원이 (250, 220)으로 이동 → Δx=50, Δy=20
    std::cout << "1️⃣  테스트 프레임 생성..." << std::endl;

    const int kFrameHeight = 400;
    const int kFrameWidth = 600;
    cv::Mat frame1 = cv::Mat::zeros(kFrameHeight, kFrameWidth, CV_8UC1);
    cv::Mat frame2 = cv::Mat::zeros(kFrameHeight, kFrameWidth, CV_8UC1);

    cv::circle(frame1, cv::Point(200, 200), 50, cv::Scalar(255), -1);
    cv::circle(frame2, cv::Point(250, 220), 50, cv::Scalar(255), -1);

    std::cout << "   Frame 크기: " << frame1.size() << "\n" << std::endl;

    // Step 2: Sparse Optical Flow (LK)
    // goodFeaturesToTrack으로 추적할 특징점 선택 → LK로 추적
    std::cout << "2️⃣  Sparse Optical Flow (Lucas-Kanade)..." << std::endl;

    std::vector<cv::Point2f> pts1;
    cv::goodFeaturesToTrack(frame1, pts1, 50, 0.01, 10);

    std::vector<cv::Point2f> pts2;
    std::vector<uchar> status;
    double success_rate = lucasKanade(frame1, frame2, pts1, pts2, status);

    std::cout << "   초기 특징점: " << pts1.size() << "개" << std::endl;
    std::cout << "   추적 성공: " << (int)(success_rate * pts1.size()) << "개" << std::endl;

    double avg_motion = evaluateFlow(pts1, pts2, status);
    std::cout << "   평균 이동: " << std::fixed << std::setprecision(2) << avg_motion << " 픽셀\n"
              << std::endl;

    // Step 3: Dense Optical Flow (Farneback)
    // 모든 픽셀의 이동 벡터를 계산 → 평균 크기 출력
    std::cout << "3️⃣  Dense Optical Flow (Farneback)..." << std::endl;

    cv::Mat flow;
    farneback(frame1, frame2, flow);

    // 전체 픽셀의 평균 flow 크기 계산
    // 대부분 픽셀은 움직임 없음(배경) → 평균이 작음
    // 원이 있는 영역만 큰 flow 값을 가짐
    double avg_flow = 0.0;
    int count = 0;
    for (int y = 0; y < flow.rows; y++)
    {
        for (int x = 0; x < flow.cols; x++)
        {
            const cv::Point2f& f = flow.at<cv::Point2f>(y, x);
            avg_flow += std::sqrt(f.x * f.x + f.y * f.y);
            count++;
        }
    }
    avg_flow /= count;

    std::cout << "   Flow 크기: " << flow.size() << std::endl;
    std::cout << "   평균 이동: " << avg_flow << " 픽셀\n" << std::endl;

    // Step 4: 시각화 저장
    std::cout << "4️⃣  결과 시각화..." << std::endl;

    cv::Mat sparse_vis;
    visualizeSparseFlow(frame2, pts1, pts2, status, sparse_vis);
    cv::imwrite("sparse_flow.png", sparse_vis);

    cv::Mat dense_vis;
    visualizeFlow(flow, dense_vis, 10);
    cv::imwrite("dense_flow.png", dense_vis);

    std::cout << "   저장: sparse_flow.png, dense_flow.png\n" << std::endl;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 파이프라인 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

// 메인 함수 — Optical Flow 학습 데모 전체 실행
//
// 실행 흐름:
//   ① 밝기 항상성, 피라미드 LK, Sparse vs Dense 개념 교육
//   ② demoPipeline: Sparse(LK) vs Dense(Farneback) 비교
//   ③ demoFeatureTracking: 연속 프레임 특징점 추적 시뮬레이션
//   ④ compareSparseVsDense: 두 방법의 특성과 용도 비교
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Optical Flow 기본 데모 (Week 8)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ── 교육 블록 A: 밝기 항상성 가정 ──────────────
    // 💡 quiz_easy 문제 1 (OF 가정), 문제 5 (LK 방정식)에서 이 수식 사용!
    std::cout << "📖 [개념] 밝기 항상성 (Brightness Constancy)" << std::endl;
    std::cout << "   핵심 가정: I(x, y, t) = I(x+dx, y+dy, t+dt)" << std::endl;
    std::cout << "   \"같은 점은 시간이 지나도 밝기가 변하지 않는다\"" << std::endl;
    std::cout << std::endl;
    std::cout << "   테일러 전개:" << std::endl;
    std::cout << "     Ix*u + Iy*v + It = 0" << std::endl;
    std::cout << "     (Ix, Iy: 공간 그래디언트, It: 시간 그래디언트)" << std::endl;
    std::cout << "     (u, v: 광류 — 구하려는 값!)" << std::endl;
    std::cout << std::endl;
    std::cout << "   ⚠️ 1개 방정식, 2개 미지수 → 조리개 문제 (Aperture Problem)!" << std::endl;
    std::cout << "   → LK 해결: 윈도우 내 모든 픽셀이 같은 (u,v)라고 가정" << std::endl;
    std::cout << "   → [u,v]^T = (A^T A)^{-1} A^T b  (최소제곱 해)\n" << std::endl;

    // ── 교육 블록 B: 피라미드 LK ──────────────────
    // 💡 quiz_easy 문제 3 (Pyramidal OF)에서 이 개념 출제!
    std::cout << "📖 [개념] 피라미드 Lucas-Kanade" << std::endl;
    std::cout << "   LK는 작은 움직임만 추적 가능 → 피라미드로 확장" << std::endl;
    std::cout << "   Level 2 (1/4): 움직임 5px → 추적!" << std::endl;
    std::cout << "   Level 1 (1/2): 결과 전파 + 정제" << std::endl;
    std::cout << "   Level 0 (원본): 최종 결과" << std::endl;
    std::cout << std::endl;
    std::cout << "   calcOpticalFlowPyrLK 파라미터:" << std::endl;
    std::cout << "     winSize = (21, 21)  ← 추적 윈도우" << std::endl;
    std::cout << "     maxLevel = 3        ← 피라미드 레벨 수\n" << std::endl;

    // ── 교육 블록 C: Sparse vs Dense ─────────────
    // 💡 quiz_easy 문제 4 (SLAM 활용), quiz_medium 문제 3 (Flow vs Matching)!
    std::cout << "📖 [개념] Sparse vs Dense Optical Flow" << std::endl;
    std::cout << "   Sparse (LK): 특정 점만 추적 → 빠름 (~1ms)" << std::endl;
    std::cout << "   Dense (Farneback): 모든 픽셀 → 느림 (~50ms)" << std::endl;
    std::cout << "   SLAM에서는 Sparse 사용! (VINS = FAST + Pyramid LK)\n" << std::endl;

    // 전체 파이프라인
    OpticalFlowBasic::demoPipeline();

    // Feature Tracking 데모
    OpticalFlowBasic::demoFeatureTracking();

    // 비교
    OpticalFlowBasic::compareSparseVsDense();

    std::cout << "\n💡 Week 8 핵심 내용:" << std::endl;
    std::cout << "   - Optical Flow = 픽셀 움직임 추적" << std::endl;
    std::cout << "   - Lucas-Kanade = Sparse (빠름)" << std::endl;
    std::cout << "   - Farneback = Dense (느림)" << std::endl;
    std::cout << "   - SLAM에서는 Sparse 사용\n" << std::endl;

    // ── 다음 단계 ─────────────────────────────────
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  📋 다음 단계" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  1. README.md 이론 학습 (위 출력과 대조)" << std::endl;
    std::cout << "  2. quiz_easy.cpp 초급 퀴즈" << std::endl;
    std::cout << "  3. my_basic.cpp 직접 구현 (8단계)" << std::endl;
    std::cout << "  4. quiz_medium.cpp 중급 퀴즈" << std::endl;
    std::cout << "  5. PRACTICE.md 실시간 추적 실습" << std::endl;
    std::cout << std::endl;
    std::cout << "🎉 Phase 2 (Computer Vision 기초) 완성!" << std::endl;
    std::cout << "   다음: Phase 3 (SLAM 기초 이론)\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
