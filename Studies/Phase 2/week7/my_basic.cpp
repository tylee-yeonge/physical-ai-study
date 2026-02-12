/**
 * Phase 2 Week 7 - PnP 직접 구현
 *
 * basic.h의 PnPBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make test_my_basic
 * 테스트: ./test_my_basic
 */
#include "basic.h"
#include <iostream>

bool PnPBasic::solvePnP(const std::vector<cv::Point3f>& points3d,
                        const std::vector<cv::Point2f>& points2d, const cv::Mat& K, cv::Mat& rvec,
                        cv::Mat& tvec, int method)
{
    // TODO: cv::solvePnP 사용
    return false;
}

int PnPBasic::solvePnPRansac(const std::vector<cv::Point3f>& points3d,
                             const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                             cv::Mat& rvec, cv::Mat& tvec, std::vector<int>& inliers,
                             double reprojection_error)
{
    // TODO: cv::solvePnPRansac 사용
    return 0;
}

void PnPBasic::rodrigues(const cv::Mat& rvec, cv::Mat& R)
{
    // TODO: cv::Rodrigues 사용 (회전 벡터 → 회전 행렬)
}

double PnPBasic::evaluatePose(const std::vector<cv::Point3f>& points3d,
                              const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                              const cv::Mat& rvec, const cv::Mat& tvec)
{
    // TODO: cv::projectPoints → 관측값과 비교 → 평균 오차
    return -1.0;
}

void PnPBasic::visualizePnP(const cv::Mat& img, const std::vector<cv::Point3f>& points3d,
                            const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                            const cv::Mat& rvec, const cv::Mat& tvec, cv::Mat& output)
{
    // TODO: 투영점과 관측점 시각화
}

void PnPBasic::demoVisualOdometry(const cv::Mat& K)
{
    // TODO: VO 시뮬레이션
}

void PnPBasic::compareMethods()
{
    // TODO: PnP vs Essential vs ICP 비교
}

void PnPBasic::demoPipeline(const cv::Mat& K)
{
    // TODO: 전체 파이프라인
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] PnP 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 3D-2D 대응점
    std::vector<cv::Point3f> pts3d = {{0, 0, 5}, {1, 0, 5}, {0, 1, 5}, {1, 1, 5}, {0.5f, 0.5f, 3}};
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    // 투영하여 2D 점 생성
    std::vector<cv::Point2f> pts2d;
    for (const auto& p : pts3d)
    {
        float u = 500.0f * p.x / p.z + 320.0f;
        float v = 500.0f * p.y / p.z + 240.0f;
        pts2d.push_back({u, v});
    }

    // PnP 풀기
    cv::Mat rvec, tvec;
    bool ok = PnPBasic::solvePnP(pts3d, pts2d, K, rvec, tvec);
    std::cout << "PnP 결과: " << (ok ? "성공" : "실패") << std::endl;

    if (ok)
    {
        double err = PnPBasic::evaluatePose(pts3d, pts2d, K, rvec, tvec);
        std::cout << "재투영 오차: " << err << " px" << std::endl;
    }

    std::cout << "\n구현 완료 후 테스트: ./test_my_basic" << std::endl;
    return 0;
}
#endif
