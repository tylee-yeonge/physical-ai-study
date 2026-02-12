/**
 * Phase 2 Week 7 - PnP 중급 퀴즈
 */

#include <opencv2/opencv.hpp>
#include <iostream>

void problem1_implement_pnp()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: PnP 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 3D 점들 (월드 좌표계)
    std::vector<cv::Point3f> pts3d = {cv::Point3f(0, 0, 5), cv::Point3f(1, 0, 5),
                                      cv::Point3f(0, 1, 5), cv::Point3f(1, 1, 5)};

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);

    // Ground truth 포즈
    cv::Mat rvec_gt = (cv::Mat_<double>(3, 1) << 0, 0, 0);    // 회전 없음
    cv::Mat tvec_gt = (cv::Mat_<double>(3, 1) << 0.5, 0, 0);  // X 이동

    // 2D 관측 생성
    std::vector<cv::Point2f> pts2d;
    cv::projectPoints(pts3d, rvec_gt, tvec_gt, K, cv::Mat(), pts2d);

    // TODO: solvePnP로 포즈 추정
    cv::Mat rvec, tvec;
    cv::solvePnP(pts3d, pts2d, K, cv::Mat(), rvec, tvec);

    std::cout << "Ground Truth t: " << tvec_gt.t() << std::endl;
    std::cout << "Estimated t:    " << tvec.t() << std::endl;
    std::cout << "Error: " << cv::norm(tvec_gt - tvec) << " m\n" << std::endl;

    std::cout << "💡 정확히 복원됨!" << std::endl;
}

void problem2_ransac_iterations()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: RANSAC 반복 횟수" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "PnP는 최소 4개 점 필요 (s=4)\n" << std::endl;

    // N = log(1-p) / log(1-w^s)
    double p = 0.99;
    int s = 4;

    std::vector<double> inlier_ratios = {0.5, 0.7, 0.9};

    std::cout << "Inlier 비율  |  필요 반복" << std::endl;
    std::cout << "-------------+-----------" << std::endl;

    for (double w : inlier_ratios)
    {
        int N = (int)(log(1 - p) / log(1 - pow(w, s)));
        std::cout << "   " << (int)(w * 100) << "%       |    " << N << "회" << std::endl;
    }

    std::cout << "\n💡 Inlier 많을수록 반복 적게 필요" << std::endl;
}

void problem3_pose_optimization()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 포즈 최적화" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Linear PnP의 한계:" << std::endl;
    std::cout << "   - 재투영 오차를 직접 최소화 안 함" << std::endl;
    std::cout << "   - 노이즈에 민감\n" << std::endl;

    std::cout << "비선형 최적화 (Refinement):" << std::endl;
    std::cout << "   - Minimize: Σ ||p_obs - π(R, t, X)||²" << std::endl;
    std::cout << "   - Levenberg-Marquardt" << std::endl;
    std::cout << "   - OpenCV: cv::solvePnPRefineLM()\n" << std::endl;

    std::cout << "💡 SLAM에서:" << std::endl;
    std::cout << "   - PnP → 초기값" << std::endl;
    std::cout << "   - BA (Bundle Adjustment) → 최적화" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 7 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
                 " << std::endl;

        problem1_implement_pnp() problem2_ransac_iterations() problem3_pose_optimization()

            std::cout
              << "
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout
              << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
