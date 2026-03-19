/**
 * Phase 2 Week 5 - 에피폴라 기하학 직접 구현
 *
 * basic.h의 EpipolarGeometryBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make my_basic
 * 실행: ./my_basic (각 Step 구현 후 실행하여 확인)
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │                    구현 순서 가이드                            │
 * ├──────┬────────────────────────────┬────────┬─────────────────┤
 * │ Step │ 함수                        │ 난이도 │ 검증 방법        │
 * ├──────┼────────────────────────────┼────────┼─────────────────┤
 * │  1   │ computeEpipolarLine        │ 쉬움   │ ./my_basic      │
 * │  2   │ estimateEssential          │ 쉬움   │ ./my_basic      │
 * │  3   │ estimateFundamental        │ 쉬움   │ ./my_basic      │
 * │  4   │ verifyEpipolarConstraint   │ 보통   │ ./my_basic      │
 * │  5   │ recoverPose                │ 보통   │ ./my_basic      │
 * │  6   │ verifyEF_Relationship      │ 보통   │ ./my_basic      │
 * │  7   │ visualizeEpipolarLines     │ 쉬움   │ ./my_basic      │
 * │  8   │ demoPipeline               │ 어려움 │ ./my_basic      │
 * └──────┴────────────────────────────┴────────┴─────────────────┘
 *
 * 💡 각 Step 구현 후 ./my_basic을 실행하면 해당 단계의 ✅/❌를 확인할 수 있습니다.
 */
#include "basic.h"
#include <iostream>
#include <iomanip>

cv::Vec3f EpipolarGeometryBasic::computeEpipolarLine(const cv::Point2f& point,
                                                     const cv::Mat& fundamental, bool for_image2)
{
    // [Step 1] 에피폴라 선 계산 (가장 먼저 구현!)
    // 1) p = [point.x, point.y, 1.0]^T (3×1 행렬)
    // 2) for_image2이면: line = F * p (image2의 에피폴라 선)
    //    아니면:         line = F^T * p (image1의 에피폴라 선)
    // 3) 정규화: norm = sqrt(a² + b²), line /= norm
    // 4) cv::Vec3f(a, b, c) 반환 (ax + by + c = 0)
    // 참고: basic.cpp의 computeEpipolarLine()
    // 기대값: 정규화된 (a,b,c) 벡터
    return cv::Vec3f(0, 0, 0);
}

int EpipolarGeometryBasic::estimateEssential(const std::vector<cv::Point2f>& points1,
                                             const std::vector<cv::Point2f>& points2,
                                             cv::Mat& essential, int method)
{
    // [Step 2] Essential Matrix 추정
    // 1) findEssentialMat 함수 호출 (focal=1.0, pp=(0,0): 정규화 좌표 기준)
    // 2) 결과가 비어있지 않으면 essential에 저장
    // 3) inlier 수 반환 (간략화: points1.size())
    // 참고: basic.cpp의 estimateEssential()
    // 기대값: E가 3×3 행렬
    return 0;
}

int EpipolarGeometryBasic::estimateFundamental(const std::vector<cv::Point2f>& points1,
                                               const std::vector<cv::Point2f>& points2,
                                               cv::Mat& fundamental, int method)
{
    // [Step 3] Fundamental Matrix 추정
    // 1) findFundamentalMat 함수로 F 행렬 추정 (임계값 3.0, 신뢰도 0.99, inlier 마스크 사용)
    // 2) inlier 마스크에서 0이 아닌 값의 개수를 inlier 수로 반환
    // 참고: basic.cpp의 estimateFundamental()
    // 기대값: F가 3×3 행렬, inlier > 0
    return 0;
}

double EpipolarGeometryBasic::verifyEpipolarConstraint(const cv::Point2f& point1,
                                                       const cv::Point2f& point2,
                                                       const cv::Mat& essential_or_fundamental)
{
    // [Step 4] 에피폴라 제약 검증: p2^T * E * p1 = 0
    // 1) p1 = [point1.x, point1.y, 1.0]^T
    // 2) p2 = [point2.x, point2.y, 1.0]^T
    // 3) result = p2.t() * E * p1
    // 4) |result| 반환 (0에 가까울수록 정확)
    // 참고: basic.cpp의 verifyEpipolarConstraint()
    // 기대값: 정확한 대응점이면 ~0
    return -1.0;
}

bool EpipolarGeometryBasic::recoverPose(const cv::Mat& essential,
                                        const std::vector<cv::Point2f>& points1,
                                        const std::vector<cv::Point2f>& points2, const cv::Mat& K,
                                        cv::Mat& R, cv::Mat& t)
{
    // [Step 5] Essential Matrix에서 R, t 복원
    // 1) K 행렬에서 focal length 추출
    // 2) K 행렬에서 주점(principal point) 추출
    // 3) recoverPose 함수로 Essential Matrix에서 R, t 복원
    // 4) inliers > 0이면 true 반환
    // 참고: basic.cpp의 recoverPose()
    // 기대값: R(3×3), t(3×1), det(R)≈1
    return false;
}

double EpipolarGeometryBasic::verifyEF_Relationship(const cv::Mat& K, const cv::Mat& essential,
                                                    const cv::Mat& fundamental)
{
    // [Step 6] E와 F의 관계 검증
    // 1) K의 역행렬 계산
    // 2) F_from_E = K^{-T} * E * K^{-1} 공식으로 F 행렬 유도
    // 3) 두 행렬 모두 (2,2) 원소로 나누어 스케일 정규화
    // 4) 두 행렬의 차이(Frobenius norm) 반환
    // 참고: basic.cpp의 verifyEF_Relationship()
    // 기대값: 차이 < 0.1 (이상적으로 ~0)
    return -1.0;
}

void EpipolarGeometryBasic::visualizeEpipolarLines(const cv::Mat& img1, const cv::Mat& img2,
                                                   const std::vector<cv::Point2f>& points1,
                                                   const std::vector<cv::Point2f>& points2,
                                                   const cv::Mat& fundamental, cv::Mat& output)
{
    // [Step 7] 에피폴라 선 시각화
    // 1) 두 이미지를 가로로 이어 붙여 하나의 이미지로 합치기
    // 2) 그레이스케일이면 BGR로 변환
    // 3) 각 점에 원 그리기 + computeEpipolarLine()으로 선 그리기
    // 참고: basic.cpp의 visualizeEpipolarLines()
    // 기대값: output.empty() == false
}

void EpipolarGeometryBasic::demoPipeline(const cv::Mat& img1, const cv::Mat& img2, const cv::Mat& K)
{
    // [Step 8] 전체 파이프라인 (Step 1~7 조합)
    // 1) ORB 검출 + Ratio Test 매칭
    // 2) estimateEssential() + estimateFundamental()
    // 3) verifyEF_Relationship()
    // 4) recoverPose()
    // 5) verifyEpipolarConstraint() + visualizeEpipolarLines()
    // 참고: basic.cpp의 demoPipeline()
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 에피폴라 기하학 - 단계별 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 행렬
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    // 테스트 데이터: 3D 점 → 두 카메라에서 투영
    // 카메라 2: 5도 Y축 회전 + X 이동
    double angle = 5.0 * CV_PI / 180.0;
    cv::Mat R_true = (cv::Mat_<double>(3, 3) << cos(angle), 0, sin(angle),
                      0, 1, 0,
                      -sin(angle), 0, cos(angle));
    cv::Mat t_true = (cv::Mat_<double>(3, 1) << 0.1, 0.0, 0.0);

    std::vector<cv::Point3d> pts_3d;
    cv::RNG rng(42);
    for (int i = 0; i < 20; i++)
    {
        pts_3d.push_back(cv::Point3d(
            rng.uniform(-1.0, 1.0), rng.uniform(-1.0, 1.0), rng.uniform(3.0, 7.0)));
    }

    // 정규화 좌표 (E 테스트용) + 픽셀 좌표 (F 테스트용)
    std::vector<cv::Point2f> norm_pts1, norm_pts2;
    std::vector<cv::Point2f> pix_pts1, pix_pts2;
    for (const auto& P : pts_3d)
    {
        cv::Mat p = (cv::Mat_<double>(3, 1) << P.x, P.y, P.z);
        // 카메라 1 (원점)
        norm_pts1.push_back(cv::Point2f(P.x / P.z, P.y / P.z));
        cv::Mat proj1 = K * p;
        pix_pts1.push_back(cv::Point2f(
            proj1.at<double>(0) / proj1.at<double>(2),
            proj1.at<double>(1) / proj1.at<double>(2)));

        // 카메라 2
        cv::Mat p2 = R_true * p + t_true;
        norm_pts2.push_back(cv::Point2f(
            p2.at<double>(0) / p2.at<double>(2),
            p2.at<double>(1) / p2.at<double>(2)));
        cv::Mat proj2 = K * p2;
        pix_pts2.push_back(cv::Point2f(
            proj2.at<double>(0) / proj2.at<double>(2),
            proj2.at<double>(1) / proj2.at<double>(2)));
    }

    // ── Step 1: computeEpipolarLine ─────────────
    std::cout << "Step 1: computeEpipolarLine" << std::endl;
    // F를 OpenCV로 미리 계산 (Step 1 테스트용)
    cv::Mat F_ref = cv::findFundamentalMat(pix_pts1, pix_pts2, cv::FM_8POINT);
    cv::Vec3f line = EpipolarGeometryBasic::computeEpipolarLine(pix_pts1[0], F_ref, true);
    double line_norm = std::sqrt(line[0] * line[0] + line[1] * line[1]);
    std::cout << "   에피폴라 선: [" << line[0] << ", " << line[1] << ", " << line[2] << "]"
              << (line_norm > 0.01 ? " ✅" : " ❌ 기대: 비영 벡터") << std::endl;

    // 대응점이 에피폴라 선 위에 있는지 확인: a*x + b*y + c ≈ 0
    double dist = std::abs(line[0] * pix_pts2[0].x + line[1] * pix_pts2[0].y + line[2]);
    std::cout << "   대응점-선 거리: " << std::fixed << std::setprecision(4) << dist
              << (dist < 1.0 ? " ✅ (< 1px)" : " ❌ 기대: < 1px") << std::endl;

    // ── Step 2: estimateEssential ────────────────
    std::cout << "\nStep 2: estimateEssential" << std::endl;
    cv::Mat E;
    int n_E = EpipolarGeometryBasic::estimateEssential(norm_pts1, norm_pts2, E);
    bool E_ok = !E.empty() && E.rows == 3 && E.cols == 3;
    std::cout << "   Essential Matrix: " << (E_ok ? "3×3 ✅" : "비어있음 ❌") << std::endl;
    std::cout << "   Inlier: " << n_E << (n_E > 0 ? " ✅" : " ❌ 기대: > 0") << std::endl;

    // ── Step 3: estimateFundamental ──────────────
    std::cout << "\nStep 3: estimateFundamental" << std::endl;
    cv::Mat F;
    int n_F = EpipolarGeometryBasic::estimateFundamental(pix_pts1, pix_pts2, F);
    bool F_ok = !F.empty() && F.rows == 3 && F.cols == 3;
    std::cout << "   Fundamental Matrix: " << (F_ok ? "3×3 ✅" : "비어있음 ❌") << std::endl;
    std::cout << "   Inlier: " << n_F << (n_F > 0 ? " ✅" : " ❌ 기대: > 0") << std::endl;

    // ── Step 4: verifyEpipolarConstraint ─────────
    std::cout << "\nStep 4: verifyEpipolarConstraint" << std::endl;
    if (E_ok)
    {
        double total_err = 0.0;
        for (size_t i = 0; i < norm_pts1.size(); i++)
        {
            total_err += EpipolarGeometryBasic::verifyEpipolarConstraint(
                norm_pts1[i], norm_pts2[i], E);
        }
        double avg_err = total_err / norm_pts1.size();
        std::cout << "   평균 에피폴라 오차 (E): " << std::setprecision(6) << avg_err
                  << (avg_err >= 0 && avg_err < 0.01 ? " ✅ (< 0.01)" : " ❌ 기대: ~0")
                  << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  Step 2 먼저 구현 필요" << std::endl;
    }

    // ── Step 5: recoverPose ──────────────────────
    std::cout << "\nStep 5: recoverPose" << std::endl;
    if (E_ok)
    {
        cv::Mat R, t;
        bool pose_ok = EpipolarGeometryBasic::recoverPose(E, norm_pts1, norm_pts2, K, R, t);
        std::cout << "   포즈 복원: " << (pose_ok ? "성공 ✅" : "실패 ❌") << std::endl;
        if (pose_ok)
        {
            double det_R = cv::determinant(R);
            std::cout << "   R 크기: " << R.rows << "×" << R.cols
                      << ", det(R): " << std::setprecision(3) << det_R
                      << (std::abs(det_R - 1.0) < 0.01 ? " ✅" : " ❌ 기대: ~1.0") << std::endl;
            std::cout << "   t 크기: " << t.rows << "×" << t.cols << std::endl;
        }
    }
    else
    {
        std::cout << "   ⚠️  Step 2 먼저 구현 필요" << std::endl;
    }

    // ── Step 6: verifyEF_Relationship ────────────
    std::cout << "\nStep 6: verifyEF_Relationship" << std::endl;
    if (E_ok && F_ok)
    {
        double ef_diff = EpipolarGeometryBasic::verifyEF_Relationship(K, E, F);
        std::cout << "   E-F 관계 오차: " << std::setprecision(4) << ef_diff
                  << (ef_diff >= 0 && ef_diff < 0.5 ? " ✅" : " ❌ 기대: 작은 값") << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  Step 2, 3 먼저 구현 필요" << std::endl;
    }

    // ── Step 7: visualizeEpipolarLines ───────────
    std::cout << "\nStep 7: visualizeEpipolarLines" << std::endl;
    cv::Mat img1 = cv::Mat::zeros(480, 640, CV_8UC1);
    cv::Mat img2 = cv::Mat::zeros(480, 640, CV_8UC1);
    // 간단한 패턴
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 13; j++)
        {
            if ((i + j) % 2 == 0)
            {
                cv::rectangle(img1, cv::Point(j * 50, i * 50),
                              cv::Point((j + 1) * 50, (i + 1) * 50), cv::Scalar(255), -1);
            }
        }
    }
    cv::Mat M = cv::getRotationMatrix2D(cv::Point(320, 240), 8, 1.0);
    M.at<double>(0, 2) += 30;
    cv::warpAffine(img1, img2, M, img1.size());

    if (F_ok)
    {
        cv::Mat vis;
        EpipolarGeometryBasic::visualizeEpipolarLines(img1, img2, pix_pts1, pix_pts2, F, vis);
        std::cout << "   출력 이미지: "
                  << (vis.empty() ? "비어있음 ❌" : std::to_string(vis.cols) + "×"
                                                          + std::to_string(vis.rows) + " ✅")
                  << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  Step 3 먼저 구현 필요" << std::endl;
    }

    // ── Step 8: demoPipeline ─────────────────────
    std::cout << "\nStep 8: demoPipeline" << std::endl;
    EpipolarGeometryBasic::demoPipeline(img1, img2, K);
    std::cout << "   (위 출력 확인)" << std::endl;

    // ── 요약 ────────────────────────────────────
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  모든 Step ✅ 확인 후 → quiz_easy.cpp, quiz_medium.cpp" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
