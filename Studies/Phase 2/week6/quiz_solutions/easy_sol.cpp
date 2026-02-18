// Week 6 기초 퀴즈 정답
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

int main()
{
    std::cout << "Week 6 기초 퀴즈 정답\n" << std::endl;

    // 문제 1
    double baseline = 0.12, focal = 600.0;
    double d1 = 60.0, d2 = 30.0;
    std::cout << "문제 1:" << std::endl;
    std::cout << "   시차 " << d1 << " → 깊이 " << (baseline * focal / d1) << " m" << std::endl;
    std::cout << "   시차 " << d2 << " → 깊이 " << (baseline * focal / d2) << " m\n" << std::endl;

    std::cout << "문제 2: DLT로 최적 3D 점 찾기" << std::endl;
    std::cout << "문제 3: 재투영 오차 = ||관측 - 투영||" << std::endl;
    std::cout << "문제 4: Baseline 선택은 응용에 따라 다름\n" << std::endl;

    // 문제 5: 회전 행렬 유효성 검증
    std::cout << "문제 5: 회전 행렬 유효성 검증" << std::endl;
    double angle = 15.0 * M_PI / 180.0;
    cv::Mat R_valid = (cv::Mat_<double>(3, 3) <<
        cos(angle), 0, sin(angle), 0, 1, 0, -sin(angle), 0, cos(angle));

    // 검증
    cv::Mat RtR = R_valid.t() * R_valid;
    double ortho_err = cv::norm(RtR - cv::Mat::eye(3, 3, CV_64F));
    double det = cv::determinant(R_valid);
    std::cout << "   R_valid: ortho_err=" << ortho_err << ", det=" << det << std::endl;

    // 노이즈 추가 행렬 복구
    cv::Mat noise = cv::Mat::zeros(3, 3, CV_64F);
    cv::RNG rng(42);
    rng.fill(noise, cv::RNG::NORMAL, 0, 0.01);
    cv::Mat R_noisy = R_valid + noise;

    cv::Mat w, u, vt;
    cv::SVD::compute(R_noisy, w, u, vt);
    cv::Mat R_fixed = u * vt;
    if (cv::determinant(R_fixed) < 0)
    {
        cv::Mat D = cv::Mat::eye(3, 3, CV_64F);
        D.at<double>(2, 2) = -1;
        R_fixed = u * D * vt;
    }
    ortho_err = cv::norm(R_fixed.t() * R_fixed - cv::Mat::eye(3, 3, CV_64F));
    det = cv::determinant(R_fixed);
    std::cout << "   R_fixed: ortho_err=" << ortho_err << ", det=" << det << "\n" << std::endl;

    // 문제 6: 스케일 모호성
    std::cout << "문제 6: 스케일 모호성" << std::endl;
    double angle2 = 10.0 * M_PI / 180.0;
    cv::Mat R2 = (cv::Mat_<double>(3, 3) <<
        cos(angle2), 0, sin(angle2), 0, 1, 0, -sin(angle2), 0, cos(angle2));

    std::vector<double> scales = {0.5, 1.0, 2.0, 5.0};
    cv::Mat E_ref;
    for (double s : scales)
    {
        cv::Mat t_s = (cv::Mat_<double>(3, 1) << s, 0, 0);
        double tx = t_s.at<double>(0);
        cv::Mat t_skew = (cv::Mat_<double>(3, 3) << 0, 0, 0, 0, 0, -tx, 0, tx, 0);
        cv::Mat E_s = t_skew * R2;
        cv::Mat E_norm = E_s / cv::norm(E_s);

        if (E_ref.empty())
            E_ref = E_norm.clone();
        else
            std::cout << "   scale=" << s << " → diff=" << cv::norm(E_norm - E_ref) << std::endl;
    }
    std::cout << "   → 정규화 후 모든 E가 동일!" << std::endl;

    return 0;
}
