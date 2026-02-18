// Phase 2 Week 4 - 중급 퀴즈 정답 (핵심)
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <cmath>
#include <random>

// 문제 2: Essential Matrix 예제
void essentialMatrixExample()
{
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);

    std::vector<cv::Point2f> pts1 = {cv::Point2f(100, 150), cv::Point2f(200, 250),
                                     cv::Point2f(300, 180), cv::Point2f(450, 320)};

    std::vector<cv::Point2f> pts2 = {cv::Point2f(120, 160), cv::Point2f(210, 240),
                                     cv::Point2f(310, 190), cv::Point2f(460, 310)};

    // Essential Matrix 추정
    cv::Mat E = cv::findEssentialMat(pts1, pts2, K, cv::RANSAC, 0.999, 1.0);

    std::cout << "Essential Matrix:\n" << E << std::endl;

    // R, t 복원
    cv::Mat R, t;
    cv::recoverPose(E, pts1, pts2, K, R, t);

    std::cout << "\nRotation:\n" << R << std::endl;
    std::cout << "\nTranslation:\n" << t << std::endl;
}

int main()
{
    std::cout << "Week 4 중급 퀴즈 핵심 정답\n" << std::endl;

    std::cout << "문제 1: Ratio 0.7은 좋은 균형점" << std::endl;
    std::cout << "   - 낮으면: precise but few" << std::endl;
    std::cout << "   - 높으면: many but noisy\n" << std::endl;

    std::cout << "문제 2: Essential Matrix" << std::endl;
    essentialMatrixExample();

    std::cout << "\n문제 3: FLANN이 BF보다 10-100x 빠름" << std::endl;
    std::cout << "   - 특징점 많을수록 차이 커짐\n" << std::endl;

    // 문제 4: Homography DLT
    std::cout << "문제 4: DLT Homography 구현" << std::endl;
    {
        double theta = 10.0 * CV_PI / 180.0;
        cv::Mat H_true = (cv::Mat_<double>(3, 3) <<
            1.05 * cos(theta), -1.05 * sin(theta), 30.0,
            1.05 * sin(theta),  1.05 * cos(theta), 20.0,
            0.0,                0.0,                1.0);

        std::vector<cv::Point2d> src = {
            {100, 100}, {250, 100}, {250, 250}, {100, 250}, {150, 150}, {200, 180}
        };
        std::vector<cv::Point2d> dst;
        cv::perspectiveTransform(src, dst, H_true);

        // A 행렬 구성
        int N = (int)src.size();
        cv::Mat A = cv::Mat::zeros(2 * N, 9, CV_64F);
        for (int i = 0; i < N; i++)
        {
            double x = src[i].x, y = src[i].y;
            double u = dst[i].x, v = dst[i].y;
            A.at<double>(2 * i, 0) = -x;
            A.at<double>(2 * i, 1) = -y;
            A.at<double>(2 * i, 2) = -1;
            A.at<double>(2 * i, 6) = u * x;
            A.at<double>(2 * i, 7) = u * y;
            A.at<double>(2 * i, 8) = u;
            A.at<double>(2 * i + 1, 3) = -x;
            A.at<double>(2 * i + 1, 4) = -y;
            A.at<double>(2 * i + 1, 5) = -1;
            A.at<double>(2 * i + 1, 6) = v * x;
            A.at<double>(2 * i + 1, 7) = v * y;
            A.at<double>(2 * i + 1, 8) = v;
        }

        // SVD
        cv::Mat w, u_mat, vt;
        cv::SVD::compute(A, w, u_mat, vt);
        cv::Mat h = vt.row(vt.rows - 1);
        cv::Mat H_dlt = h.reshape(1, 3);
        H_dlt /= H_dlt.at<double>(2, 2);

        std::cout << "   DLT 결과:\n" << H_dlt << std::endl;
        std::cout << "   Ground Truth:\n" << H_true << "\n" << std::endl;
    }

    // 문제 5: RANSAC Homography
    std::cout << "문제 5: RANSAC Homography" << std::endl;
    {
        double theta = 10.0 * CV_PI / 180.0;
        cv::Mat H_true = (cv::Mat_<double>(3, 3) <<
            1.05 * cos(theta), -1.05 * sin(theta), 30.0,
            1.05 * sin(theta),  1.05 * cos(theta), 20.0,
            0.0,                0.0,                1.0);

        std::mt19937 rng(42);
        std::uniform_real_distribution<double> dist(50.0, 250.0);
        std::normal_distribution<double> noise(0.0, 1.0);

        std::vector<cv::Point2d> src, dst_all;
        int n_in = 40, n_out = 15;

        for (int i = 0; i < n_in; i++)
        {
            cv::Point2d pt(dist(rng), dist(rng));
            src.push_back(pt);
            cv::Mat p = (cv::Mat_<double>(3, 1) << pt.x, pt.y, 1.0);
            cv::Mat p2 = H_true * p;
            dst_all.push_back(cv::Point2d(
                p2.at<double>(0) / p2.at<double>(2) + noise(rng),
                p2.at<double>(1) / p2.at<double>(2) + noise(rng)));
        }
        for (int i = 0; i < n_out; i++)
        {
            src.push_back(cv::Point2d(dist(rng), dist(rng)));
            dst_all.push_back(cv::Point2d(dist(rng), dist(rng)));
        }

        cv::Mat mask;
        cv::Mat H_cv = cv::findHomography(src, dst_all, cv::RANSAC, 5.0, mask);
        int inliers = cv::countNonZero(mask);

        std::cout << "   OpenCV RANSAC: " << inliers << "/" << (n_in + n_out) << " inliers" << std::endl;
        std::cout << "   실제 inlier: " << n_in << std::endl;
    }

    return 0;
}
