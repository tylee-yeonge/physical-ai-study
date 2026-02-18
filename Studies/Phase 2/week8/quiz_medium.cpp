/**
 * Phase 2 Week 8 - Optical Flow 중급 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>
#include <iostream>
#include <cmath>
#include <cstdio>

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

/**
 * @brief Lucas-Kanade 단일 점 광류를 직접 구현하고 OpenCV 결과와 비교
 *
 * Sobel로 Ix, Iy를 계산하고 프레임 차이로 It을 구한 후,
 * 윈도우 내 A 행렬과 b 벡터를 구성하여 (A^T A)^-1 A^T b로
 * (u, v) 광류를 계산한다. OpenCV calcOpticalFlowPyrLK 결과와 비교한다.
 */
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

/**
 * @brief 윈도우 크기별 LK 결과를 비교하여 트레이드오프 분석
 *
 * 윈도우 크기 5, 11, 21, 41로 변화시키며 LK 결과의 정확도를 비교하고
 * 작은 윈도우 vs 큰 윈도우의 트레이드오프를 확인한다.
 */
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
