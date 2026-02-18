// Week 5 중급 퀴즈 정답
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

int main()
{
    std::cout << "Week 5 중급 퀴즈 정답\n" << std::endl;

    std::vector<cv::Point2f> pts1 = {cv::Point2f(100, 150), cv::Point2f(200, 250)};
    std::vector<cv::Point2f> pts2 = {cv::Point2f(110, 160), cv::Point2f(210, 260)};

    cv::Mat F = cv::findFundamentalMat(pts1, pts2, cv::FM_8POINT);
    std::cout << "Fundamental Matrix:\n" << F << std::endl;

    std::cout << "\nE에서 R,t 복원은 cv::recoverPose() 사용\n" << std::endl;

    // 문제 4: 8-point 알고리즘 직접 구현
    std::cout << "문제 4: 8-point 알고리즘 핵심 단계" << std::endl;
    std::cout << "   1. Hartley 정규화: 중심=0, 평균거리=sqrt(2)" << std::endl;
    std::cout << "   2. A 행렬 (Nx9) 구성" << std::endl;
    std::cout << "   3. SVD → Vt 마지막 행 → f (9x1)" << std::endl;
    std::cout << "   4. f → F (3x3), rank-2 강제" << std::endl;
    std::cout << "   5. 역정규화: F = T2^T * F_norm * T1\n" << std::endl;

    // 정규화 변환 T 구하는 방법
    // mean = 점들의 중심
    // scale = sqrt(2) / 평균거리
    // T = [scale, 0, -scale*mean_x; 0, scale, -scale*mean_y; 0, 0, 1]

    // 문제 5: 노이즈 영향 분석
    std::cout << "문제 5: 노이즈 영향 분석 정답 예시" << std::endl;

    cv::Mat K2 = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);
    double angle = 5.0 * CV_PI / 180.0;
    cv::Mat R2 = (cv::Mat_<double>(3, 3) <<
        cos(angle), 0, sin(angle), 0, 1, 0, -sin(angle), 0, cos(angle));
    cv::Mat t2 = (cv::Mat_<double>(3, 1) << 0.1, 0.0, 0.0);

    // 3D 점 생성 및 투영
    std::vector<cv::Point2f> pts1_c, pts2_c;
    cv::RNG rng(42);
    for (int i = 0; i < 30; i++)
    {
        cv::Mat p = (cv::Mat_<double>(3, 1) <<
            rng.uniform(-1.0, 1.0), rng.uniform(-1.0, 1.0), rng.uniform(3.0, 7.0));
        cv::Mat proj1 = K2 * p;
        pts1_c.push_back(cv::Point2f(
            proj1.at<double>(0) / proj1.at<double>(2),
            proj1.at<double>(1) / proj1.at<double>(2)));
        cv::Mat p2 = R2 * p + t2;
        cv::Mat proj2 = K2 * p2;
        pts2_c.push_back(cv::Point2f(
            proj2.at<double>(0) / proj2.at<double>(2),
            proj2.at<double>(1) / proj2.at<double>(2)));
    }

    // 노이즈 수준별 오차 측정
    std::vector<double> noise_levels = {0.0, 0.5, 1.0, 2.0, 5.0};
    for (double noise : noise_levels)
    {
        std::vector<cv::Point2f> pts1_n, pts2_n;
        for (size_t i = 0; i < pts1_c.size(); i++)
        {
            pts1_n.push_back(cv::Point2f(
                pts1_c[i].x + rng.gaussian(noise),
                pts1_c[i].y + rng.gaussian(noise)));
            pts2_n.push_back(cv::Point2f(
                pts2_c[i].x + rng.gaussian(noise),
                pts2_c[i].y + rng.gaussian(noise)));
        }

        cv::Mat F_n = cv::findFundamentalMat(pts1_n, pts2_n, cv::FM_8POINT);
        if (F_n.empty()) continue;

        // 에피폴라 제약 오차 계산
        double total_error = 0.0;
        for (size_t i = 0; i < pts1_c.size(); i++)
        {
            cv::Mat p1_h = (cv::Mat_<double>(3, 1) << pts1_c[i].x, pts1_c[i].y, 1.0);
            cv::Mat p2_h = (cv::Mat_<double>(3, 1) << pts2_c[i].x, pts2_c[i].y, 1.0);
            cv::Mat err = p2_h.t() * F_n * p1_h;
            total_error += std::abs(err.at<double>(0));
        }
        double mean_error = total_error / pts1_c.size();
        std::cout << "   noise=" << noise << "px → mean|p2^T F p1|=" << mean_error << std::endl;
    }

    return 0;
}
