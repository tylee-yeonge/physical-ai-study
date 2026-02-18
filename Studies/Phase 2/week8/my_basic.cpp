/**
 * Phase 2 Week 8 - Optical Flow 직접 구현
 *
 * basic.h의 OpticalFlowBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make my_basic
 * 실행: ./my_basic (각 Step 구현 후 실행하여 확인)
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │                    구현 순서 가이드                            │
 * ├──────┬──────────────────────┬────────┬───────────────────────┤
 * │ Step │ 함수                  │ 난이도 │ 검증 방법              │
 * ├──────┼──────────────────────┼────────┼───────────────────────┤
 * │  1   │ lucasKanade          │ 쉬움   │ 추적 성공률 > 0       │
 * │  2   │ farneback            │ 쉬움   │ flow 크기 = 이미지    │
 * │  3   │ evaluateFlow         │ 쉬움   │ 평균 이동 > 0         │
 * │  4   │ visualizeSparseFlow  │ 보통   │ 출력 이미지 생성      │
 * │  5   │ visualizeFlow        │ 보통   │ 출력 이미지 생성      │
 * │  6   │ compareSparseVsDense │ 보통   │ 비교 출력 확인        │
 * │  7   │ demoFeatureTracking  │ 어려움 │ 10프레임 추적         │
 * │  8   │ demoPipeline         │ 어려움 │ 전체 파이프라인       │
 * └──────┴──────────────────────┴────────┴───────────────────────┘
 *
 * 💡 각 Step 구현 후 ./my_basic을 실행하면 해당 단계의 ✅/❌를 확인할 수 있습니다.
 */
#include "basic.h"
#include <iostream>
#include <iomanip>
#include <cmath>

double OpticalFlowBasic::lucasKanade(const cv::Mat& prev_img, const cv::Mat& curr_img,
                                     const std::vector<cv::Point2f>& prev_pts,
                                     std::vector<cv::Point2f>& curr_pts, std::vector<uchar>& status)
{
    // [Step 1] Lucas-Kanade Optical Flow (Sparse)
    // 1) cv::calcOpticalFlowPyrLK(prev_img, curr_img, prev_pts, curr_pts,
    //                              status, err, winSize, maxLevel, criteria)
    //    - winSize: cv::Size(21, 21)
    //    - maxLevel: 3 (피라미드 레벨)
    //    - criteria: TermCriteria(COUNT|EPS, 30, 0.01)
    // 2) status에서 성공(1) 개수 세서 성공률 반환
    // 참고: basic.cpp의 lucasKanade()
    // 기대값: 원이 (10, 5)만큼 이동 → 성공률 > 0
    return 0.0;
}

void OpticalFlowBasic::farneback(const cv::Mat& prev_img, const cv::Mat& curr_img, cv::Mat& flow)
{
    // [Step 2] Farneback Dense Optical Flow
    // cv::calcOpticalFlowFarneback(prev_img, curr_img, flow,
    //                               0.5, 3, 15, 3, 5, 1.2, 0)
    // 참고: basic.cpp의 farneback()
    // 기대값: flow.size() == prev_img.size(), flow.channels() == 2
}

double OpticalFlowBasic::evaluateFlow(const std::vector<cv::Point2f>& prev_pts,
                                      const std::vector<cv::Point2f>& curr_pts,
                                      const std::vector<uchar>& status)
{
    // [Step 3] 추적 품질 평가 (성공한 점들의 평균 이동 거리)
    // 1) status[i] == 1인 점만 대상
    // 2) dx = curr_pts[i].x - prev_pts[i].x
    //    dy = curr_pts[i].y - prev_pts[i].y
    //    dist = sqrt(dx*dx + dy*dy)
    // 3) 합계 / 성공 개수 반환
    // 참고: basic.cpp의 evaluateFlow()
    // 기대값: 원이 (10, 5) 이동 → 평균 ~11.2
    return 0.0;
}

void OpticalFlowBasic::visualizeSparseFlow(const cv::Mat& img,
                                           const std::vector<cv::Point2f>& prev_pts,
                                           const std::vector<cv::Point2f>& curr_pts,
                                           const std::vector<uchar>& status, cv::Mat& output)
{
    // [Step 4] Sparse Flow 시각화
    // 1) 이미지를 BGR로 변환 (1채널이면 cvtColor, 아니면 clone)
    // 2) status[i] == 1인 점에 대해:
    //    - cv::line(output, prev_pts[i], curr_pts[i], 녹색, 2)  // 궤적
    //    - cv::circle(output, curr_pts[i], 5, 빨간색, -1)       // 현재 위치
    // 참고: basic.cpp의 visualizeSparseFlow()
    // 기대값: output.empty() == false
}

void OpticalFlowBasic::visualizeFlow(const cv::Mat& flow, cv::Mat& output, int step)
{
    // [Step 5] Dense Flow 화살표 시각화
    // 1) output = cv::Mat::zeros(flow.size(), CV_8UC3)
    // 2) step 간격으로 순회:
    //    fxy = flow.at<cv::Point2f>(y, x)
    //    cv::arrowedLine(output, (x,y), (x+fxy.x, y+fxy.y), 녹색, 1)
    // 참고: basic.cpp의 visualizeFlow()
    // 기대값: output.empty() == false
}

void OpticalFlowBasic::compareSparseVsDense()
{
    // [Step 6] Sparse vs Dense 비교 출력
    // Sparse: 특징점만 추적, 빠름 (~1ms), SLAM에 적합
    // Dense: 모든 픽셀, 느림 (~50ms), 비디오 안정화
    // 참고: basic.cpp의 compareSparseVsDense()
}

void OpticalFlowBasic::demoFeatureTracking(const std::string& video_path)
{
    // [Step 7] Feature Tracking 데모 (10프레임 추적)
    // 1) 합성 프레임 10개 생성 (움직이는 원 + 고정 패턴)
    // 2) 첫 프레임에서 cv::goodFeaturesToTrack()
    // 3) 매 프레임 lucasKanade()로 추적
    // 4) 성공한 점만 유지, 부족하면 재검출
    // 참고: basic.cpp의 demoFeatureTracking()
}

void OpticalFlowBasic::demoPipeline()
{
    // [Step 8] 전체 파이프라인 (Step 1~7 조합)
    // 1) 테스트 프레임 2개 생성
    // 2) Sparse: goodFeaturesToTrack + lucasKanade
    // 3) Dense: farneback
    // 4) 시각화: visualizeSparseFlow + visualizeFlow
    // 참고: basic.cpp의 demoPipeline()
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] Optical Flow - 단계별 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 생성: 원이 (10, 5) 이동
    cv::Mat prev = cv::Mat::zeros(240, 320, CV_8UC1);
    cv::Mat curr = cv::Mat::zeros(240, 320, CV_8UC1);
    cv::circle(prev, cv::Point(100, 100), 20, cv::Scalar(255), -1);
    cv::circle(curr, cv::Point(110, 105), 20, cv::Scalar(255), -1);

    // 특징점: 원 중심
    std::vector<cv::Point2f> prev_pts;
    cv::goodFeaturesToTrack(prev, prev_pts, 50, 0.01, 10);

    // ── Step 1: lucasKanade ───────────────────────
    std::cout << "Step 1: lucasKanade" << std::endl;
    std::vector<cv::Point2f> curr_pts;
    std::vector<uchar> status;
    double success_rate = 0.0;

    if (prev_pts.empty())
    {
        std::cout << "   ⚠️  특징점 검출 실패" << std::endl;
    }
    else
    {
        success_rate = OpticalFlowBasic::lucasKanade(prev, curr, prev_pts, curr_pts, status);
        std::cout << "   추적 성공률: " << std::fixed << std::setprecision(1)
                  << success_rate * 100 << "%"
                  << (success_rate > 0 ? " ✅" : " ❌ 기대: > 0%") << std::endl;
    }

    // ── Step 2: farneback ─────────────────────────
    std::cout << "\nStep 2: farneback" << std::endl;
    cv::Mat flow;
    OpticalFlowBasic::farneback(prev, curr, flow);
    if (flow.empty())
    {
        std::cout << "   flow: 비어있음 ❌ 기대: " << prev.cols << "x" << prev.rows << std::endl;
    }
    else
    {
        std::cout << "   flow 크기: " << flow.cols << "x" << flow.rows
                  << (flow.size() == prev.size() ? " ✅" : " ❌")
                  << ", 채널: " << flow.channels()
                  << (flow.channels() == 2 ? " ✅" : " ❌ 기대: 2") << std::endl;
    }

    // ── Step 3: evaluateFlow ──────────────────────
    std::cout << "\nStep 3: evaluateFlow" << std::endl;
    if (success_rate > 0)
    {
        double avg_dist = OpticalFlowBasic::evaluateFlow(prev_pts, curr_pts, status);
        std::cout << "   평균 이동 거리: " << std::setprecision(2) << avg_dist << " px"
                  << (avg_dist > 0 ? " ✅" : " ❌ 기대: > 0") << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  Step 1 먼저 구현 필요" << std::endl;
    }

    // ── Step 4: visualizeSparseFlow ───────────────
    std::cout << "\nStep 4: visualizeSparseFlow" << std::endl;
    if (success_rate > 0)
    {
        cv::Mat sparse_vis;
        OpticalFlowBasic::visualizeSparseFlow(curr, prev_pts, curr_pts, status, sparse_vis);
        std::cout << "   출력 이미지: "
                  << (sparse_vis.empty() ? "비어있음 ❌"
                                         : std::to_string(sparse_vis.cols) + "x"
                                               + std::to_string(sparse_vis.rows) + " ✅")
                  << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  Step 1 먼저 구현 필요" << std::endl;
    }

    // ── Step 5: visualizeFlow ─────────────────────
    std::cout << "\nStep 5: visualizeFlow" << std::endl;
    if (!flow.empty())
    {
        cv::Mat dense_vis;
        OpticalFlowBasic::visualizeFlow(flow, dense_vis, 10);
        std::cout << "   출력 이미지: "
                  << (dense_vis.empty() ? "비어있음 ❌"
                                        : std::to_string(dense_vis.cols) + "x"
                                              + std::to_string(dense_vis.rows) + " ✅")
                  << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  Step 2 먼저 구현 필요" << std::endl;
    }

    // ── Step 6: compareSparseVsDense ──────────────
    std::cout << "\nStep 6: compareSparseVsDense" << std::endl;
    OpticalFlowBasic::compareSparseVsDense();
    std::cout << "   (위 출력 확인)" << std::endl;

    // ── Step 7: demoFeatureTracking ───────────────
    std::cout << "\nStep 7: demoFeatureTracking" << std::endl;
    OpticalFlowBasic::demoFeatureTracking();
    std::cout << "   (위 출력 확인)" << std::endl;

    // ── Step 8: demoPipeline ──────────────────────
    std::cout << "\nStep 8: demoPipeline" << std::endl;
    OpticalFlowBasic::demoPipeline();
    std::cout << "   (위 출력 확인)" << std::endl;

    // ── 요약 ────────────────────────────────────
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  모든 Step ✅ 확인 후 → quiz_easy.cpp, quiz_medium.cpp" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
