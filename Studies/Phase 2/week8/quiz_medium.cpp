/**
 * Phase 2 Week 8 - Optical Flow 중급 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>
#include <iostream>

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

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 8 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
                 " << std::endl;

        problem1_implement_lk() problem2_track_quality() problem3_flow_vs_matching()

            std::cout
              << "
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout
              << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
