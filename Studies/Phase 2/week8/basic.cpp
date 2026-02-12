#include "basic.h"
#include <iostream>
#include <iomanip>
#include <cmath>

double OpticalFlowBasic::lucasKanade(const cv::Mat& prev_img, const cv::Mat& curr_img,
                                     const std::vector<cv::Point2f>& prev_pts,
                                     std::vector<cv::Point2f>& curr_pts, std::vector<uchar>& status)
{
    std::vector<float> err;
    cv::Size winSize(21, 21);
    int maxLevel = 3;  // Pyramidal

    cv::TermCriteria criteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.01);

    // Lucas-Kanade Optical Flow
    cv::calcOpticalFlowPyrLK(prev_img, curr_img, prev_pts, curr_pts, status, err, winSize, maxLevel,
                             criteria);

    // 성공률 계산
    int success_count = 0;
    for (uchar s : status)
    {
        if (s)
            success_count++;
    }

    return (double)success_count / prev_pts.size();
}

void OpticalFlowBasic::farneback(const cv::Mat& prev_img, const cv::Mat& curr_img, cv::Mat& flow)
{
    cv::calcOpticalFlowFarneback(prev_img, curr_img, flow,
                                 0.5,  // pyr_scale
                                 3,    // levels
                                 15,   // winsize
                                 3,    // iterations
                                 5,    // poly_n
                                 1.2,  // poly_sigma
                                 0     // flags
    );
}

void OpticalFlowBasic::visualizeFlow(const cv::Mat& flow, cv::Mat& output, int step)
{
    output = cv::Mat::zeros(flow.size(), CV_8UC3);

    for (int y = 0; y < flow.rows; y += step)
    {
        for (int x = 0; x < flow.cols; x += step)
        {
            const cv::Point2f& fxy = flow.at<cv::Point2f>(y, x);

            cv::Point2f start(x, y);
            cv::Point2f end(x + fxy.x, y + fxy.y);

            // 화살표 그리기
            cv::arrowedLine(output, start, end, cv::Scalar(0, 255, 0), 1, cv::LINE_AA, 0, 0.3);
        }
    }
}

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
        if (status[i])
        {
            // 궤적 그리기
            cv::line(output, prev_pts[i], curr_pts[i], cv::Scalar(0, 255, 0), 2);

            // 현재 위치
            cv::circle(output, curr_pts[i], 5, cv::Scalar(0, 0, 255), -1);
        }
    }
}

void OpticalFlowBasic::demoFeatureTracking(const std::string& video_path)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Feature Tracking 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 합성 데이터 생성
    int num_frames = 10;
    std::vector<cv::Mat> frames;

    for (int i = 0; i < num_frames; i++)
    {
        cv::Mat frame = cv::Mat::zeros(480, 640, CV_8UC1);

        // 움직이는 원
        cv::circle(frame, cv::Point(100 + i * 50, 240), 30, cv::Scalar(255), -1);

        // 고정된 패턴
        for (int j = 0; j < 5; j++)
        {
            cv::rectangle(frame, cv::Point(400 + j * 40, 100 + j * 60),
                          cv::Point(430 + j * 40, 130 + j * 60), cv::Scalar(200), -1);
        }

        frames.push_back(frame);
    }

    std::cout << "생성된 프레임: " << frames.size() << "개\n" << std::endl;

    // 첫 프레임에서 특징점 검출
    std::vector<cv::Point2f> points;
    cv::goodFeaturesToTrack(frames[0], points, 100, 0.01, 10);

    std::cout << "초기 특징점: " << points.size() << "개\n" << std::endl;

    // 프레임 간 추적
    std::vector<cv::Point2f> prev_pts = points;

    for (size_t i = 1; i < frames.size(); i++)
    {
        std::vector<cv::Point2f> curr_pts;
        std::vector<uchar> status;

        double success_rate = lucasKanade(frames[i - 1], frames[i], prev_pts, curr_pts, status);

        std::cout << "Frame " << i << ": "
                  << "추적 성공률 " << std::fixed << std::setprecision(1) << success_rate * 100
                  << "%" << std::endl;

        // 성공한 점만 유지
        std::vector<cv::Point2f> good_pts;
        for (size_t j = 0; j < status.size(); j++)
        {
            if (status[j])
            {
                good_pts.push_back(curr_pts[j]);
            }
        }

        prev_pts = good_pts;

        if (prev_pts.size() < 10)
        {
            std::cout << "   → 특징점 부족, 재검출 필요\n" << std::endl;
            break;
        }
    }

    std::cout << "\n✅ Feature Tracking 완료!" << std::endl;
}

double OpticalFlowBasic::evaluateFlow(const std::vector<cv::Point2f>& prev_pts,
                                      const std::vector<cv::Point2f>& curr_pts,
                                      const std::vector<uchar>& status)
{
    double total_dist = 0.0;
    int count = 0;

    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i])
        {
            double dx = curr_pts[i].x - prev_pts[i].x;
            double dy = curr_pts[i].y - prev_pts[i].y;
            total_dist += std::sqrt(dx * dx + dy * dy);
            count++;
        }
    }

    return count > 0 ? total_dist / count : 0.0;
}

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

void OpticalFlowBasic::demoPipeline()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Optical Flow 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Step 1: 두 프레임 생성
    std::cout << "1️⃣  테스트 프레임 생성..." << std::endl;

    cv::Mat frame1 = cv::Mat::zeros(400, 600, CV_8UC1);
    cv::Mat frame2 = cv::Mat::zeros(400, 600, CV_8UC1);

    // 움직이는 물체
    cv::circle(frame1, cv::Point(200, 200), 50, cv::Scalar(255), -1);
    cv::circle(frame2, cv::Point(250, 220), 50, cv::Scalar(255), -1);

    std::cout << "   Frame 크기: " << frame1.size() << "\n" << std::endl;

    // Step 2: Sparse Optical Flow
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

    // Step 3: Dense Optical Flow
    std::cout << "3️⃣  Dense Optical Flow (Farneback)..." << std::endl;

    cv::Mat flow;
    farneback(frame1, frame2, flow);

    // 평균 Flow 크기 계산
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

// 메인 함수
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Optical Flow 기본 데모 (Week 8)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 전체 파이프라인
    demoPipeline();

    // Feature Tracking 데모
    demoFeatureTracking();

    // 비교
    compareSparseVsDense();

    std::cout << "\n💡 Week 8 핵심 내용:" << std::endl;
    std::cout << "   - Optical Flow = 픽셀 움직임 추적" << std::endl;
    std::cout << "   - Lucas-Kanade = Sparse (빠름)" << std::endl;
    std::cout << "   - Farneback = Dense (느림)" << std::endl;
    std::cout << "   - SLAM에서는 Sparse 사용\n" << std::endl;

    std::cout << "🎉 Phase 2 (Computer Vision 기초) 완성!" << std::endl;
    std::cout << "   다음: Phase 3 (SLAM 기초 이론)\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
