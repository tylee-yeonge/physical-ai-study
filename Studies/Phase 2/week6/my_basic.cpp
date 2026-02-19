/**
 * Phase 2 Week 6 - 삼각측량 직접 구현
 *
 * basic.h의 TriangulationBasic 클래스를 직접 구현해보세요.
 * 빌드: cd build && cmake .. && make my_basic
 * 실행: ./my_basic (각 Step 구현 후 실행하여 확인)
 *
 * ┌──────────────────────────────────────────────────────────────┐
 * │                    구현 순서 가이드                            │
 * ├──────┬──────────────────────────┬────────┬───────────────────┤
 * │ Step │ 함수                      │ 난이도 │ 검증 방법          │
 * ├──────┼──────────────────────────┼────────┼───────────────────┤
 * │  1   │ disparityToDepth         │ 쉬움   │ ./my_basic        │
 * │  2   │ isInFrontOfCamera        │ 쉬움   │ ./my_basic        │
 * │  3   │ reprojectionError        │ 보통   │ ./my_basic        │
 * │  4   │ triangulatePoint         │ 보통   │ ./my_basic        │
 * │  5   │ triangulatePoints        │ 보통   │ ./my_basic        │
 * │  6   │ averageReprojectionError │ 보통   │ ./my_basic        │
 * │  7   │ evaluateTriangulation    │ 보통   │ ./my_basic        │
 * │  8   │ visualize3DPoints        │ 쉬움   │ ./my_basic        │
 * │  9   │ demoPipeline             │ 어려움 │ ./my_basic        │
 * └──────┴──────────────────────────┴────────┴───────────────────┘
 *
 * 💡 각 Step 구현 후 ./my_basic을 실행하면 해당 단계의 ✅/❌를 확인할 수 있습니다.
 */
#include "basic.h"
#include <iostream>
#include <iomanip>
#include <cmath>

double TriangulationBasic::disparityToDepth(double disparity, double baseline, double focal_length)
{
    // [Step 1] 시차 → 깊이 변환 (가장 먼저 구현!)
    // depth = (baseline * focal_length) / disparity
    // disparity가 0에 가까우면 0.0 반환
    // 참고: basic.cpp의 disparityToDepth()
    // 기대값: d=60, b=0.12, f=600 → 1.2m
    return 0.0;
}

bool TriangulationBasic::isInFrontOfCamera(const cv::Point3f& point3d, const cv::Mat& R,
                                           const cv::Mat& t)
{
    // [Step 2] Cheirality Check
    // 1) pt = [X, Y, Z]^T
    // 2) pt_cam = R * pt + t (카메라 좌표계로 변환)
    // 3) pt_cam의 Z > 0이면 true
    // 참고: basic.cpp의 isInFrontOfCamera()
    // 기대값: Z=5 → true, Z=-1 → false
    return false;
}

double TriangulationBasic::reprojectionError(const cv::Point3f& point3d, const cv::Point2f& point2d,
                                             const cv::Mat& P)
{
    // [Step 3] 재투영 오차 계산
    // 1) pt3d = [X, Y, Z, 1]^T (4×1 동차 좌표)
    // 2) projected = P * pt3d (3×1)
    // 3) 정규화: u = proj[0]/proj[2], v = proj[1]/proj[2]
    // 4) 유클리드 거리: sqrt((u-u_obs)² + (v-v_obs)²)
    // 참고: basic.cpp의 reprojectionError()
    // 기대값: 정확한 투영 → ~0 픽셀
    return -1.0;
}

bool TriangulationBasic::triangulatePoint(const cv::Point2f& pt1, const cv::Point2f& pt2,
                                          const cv::Mat& P1, const cv::Mat& P2,
                                          cv::Point3f& point3d)
{
    // [Step 4] DLT 삼각측량 (단일 점)
    // 1) pts1={pt1}, pts2={pt2} 벡터로 만들기
    // 2) cv::triangulatePoints(P1, P2, pts1, pts2, points4D)
    // 3) 동차 좌표 → 3D: w = points4D.at<float>(3,0)
    //    point3d = (X/w, Y/w, Z/w)
    // 4) |w| < 1e-6이면 false
    // 참고: basic.cpp의 triangulatePoint()
    // 기대값: GT와 비교하여 오차 < 0.1m
    return false;
}

void TriangulationBasic::triangulatePoints(const std::vector<cv::Point2f>& points1,
                                           const std::vector<cv::Point2f>& points2,
                                           const cv::Mat& P1, const cv::Mat& P2,
                                           std::vector<cv::Point3f>& points3d)
{
    // [Step 5] 여러 점 삼각측량
    // 1) cv::triangulatePoints(P1, P2, points1, points2, points4D)
    // 2) 각 열에 대해 동차좌표 → 3D 변환
    // 참고: basic.cpp의 triangulatePoints()
    // 기대값: points3d.size() == points1.size()
}

double TriangulationBasic::averageReprojectionError(const std::vector<cv::Point3f>& points3d,
                                                    const std::vector<cv::Point2f>& points2d,
                                                    const cv::Mat& P)
{
    // [Step 6] 평균 재투영 오차
    // 각 점에 대해 reprojectionError() 호출 후 평균
    // 참고: basic.cpp의 averageReprojectionError()
    // 기대값: 노이즈 없으면 ~0
    return -1.0;
}

void TriangulationBasic::evaluateTriangulation(const std::vector<cv::Point3f>& points3d,
                                               const std::vector<cv::Point2f>& points1,
                                               const std::vector<cv::Point2f>& points2,
                                               const cv::Mat& P1, const cv::Mat& P2)
{
    // [Step 7] 삼각측량 품질 평가
    // 1) averageReprojectionError(points3d, points1, P1) — 카메라1 오차
    // 2) averageReprojectionError(points3d, points2, P2) — 카메라2 오차
    // 3) visualize3DPoints(points3d) — 3D 점 통계
    // 참고: basic.cpp의 evaluateTriangulation()
}

void TriangulationBasic::visualize3DPoints(const std::vector<cv::Point3f>& points3d)
{
    // [Step 8] 3D 점군 통계 출력
    // 점 개수, 깊이(Z) 범위, 평균 깊이 출력
    // 참고: basic.cpp의 visualize3DPoints()
}

void TriangulationBasic::demoPipeline(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t)
{
    // [Step 9] 전체 파이프라인 (Step 1~8 조합)
    // 1) 투영 행렬 P1=[K|0], P2=K*[R|t] 생성
    // 2) 3D 점 생성 → 투영 → 삼각측량 복원
    // 3) evaluateTriangulation()으로 품질 평가
    // 4) disparityToDepth() 예제
    // 참고: basic.cpp의 demoPipeline()
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] 삼각측량 - 단계별 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600, 0, 400, 0, 600, 300, 0, 0, 1);
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t = (cv::Mat_<double>(3, 1) << 0.12, 0, 0);  // 12cm baseline

    // 투영 행렬
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));
    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R.copyTo(RT(cv::Rect(0, 0, 3, 3)));
    t.copyTo(RT(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT;

    // GT 3D 점
    cv::Point3f gt_pt(0.5, 0.3, 3.0);
    // 투영
    cv::Mat pt4 = (cv::Mat_<double>(4, 1) << gt_pt.x, gt_pt.y, gt_pt.z, 1.0);
    cv::Mat proj1 = P1 * pt4;
    cv::Mat proj2 = P2 * pt4;
    cv::Point2f obs1(proj1.at<double>(0) / proj1.at<double>(2),
                     proj1.at<double>(1) / proj1.at<double>(2));
    cv::Point2f obs2(proj2.at<double>(0) / proj2.at<double>(2),
                     proj2.at<double>(1) / proj2.at<double>(2));

    // ── Step 1: disparityToDepth ─────────────────
    std::cout << "Step 1: disparityToDepth" << std::endl;
    double depth = TriangulationBasic::disparityToDepth(60.0, 0.12, 600.0);
    std::cout << "   d=60, b=0.12, f=600 → depth=" << std::fixed << std::setprecision(2) << depth
              << (std::abs(depth - 1.2) < 0.01 ? " ✅ (기대: 1.2m)" : " ❌ 기대: 1.2m")
              << std::endl;
    double depth_zero = TriangulationBasic::disparityToDepth(0.0, 0.12, 600.0);
    std::cout << "   d=0 → depth=" << depth_zero
              << (depth_zero == 0.0 ? " ✅ (0으로 처리)" : " ❌ 기대: 0") << std::endl;

    // ── Step 2: isInFrontOfCamera ────────────────
    std::cout << "\nStep 2: isInFrontOfCamera" << std::endl;
    cv::Mat R_eye = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t_zero = cv::Mat::zeros(3, 1, CV_64F);
    bool front = TriangulationBasic::isInFrontOfCamera(cv::Point3f(0, 0, 5), R_eye, t_zero);
    bool behind = TriangulationBasic::isInFrontOfCamera(cv::Point3f(0, 0, -1), R_eye, t_zero);
    std::cout << "   Z=5: " << (front ? "앞 ✅" : "뒤 ❌") << std::endl;
    std::cout << "   Z=-1: " << (!behind ? "뒤 ✅" : "앞 ❌") << std::endl;

    // ── Step 3: reprojectionError ────────────────
    std::cout << "\nStep 3: reprojectionError" << std::endl;
    double reproj = TriangulationBasic::reprojectionError(gt_pt, obs1, P1);
    std::cout << "   정확한 투영 → 오차: " << std::setprecision(4) << reproj
              << (reproj >= 0 && reproj < 0.01 ? " px ✅ (~0)" : " px ❌ 기대: ~0") << std::endl;

    // ── Step 4: triangulatePoint ─────────────────
    std::cout << "\nStep 4: triangulatePoint" << std::endl;
    cv::Point3f recovered;
    bool tri_ok = TriangulationBasic::triangulatePoint(obs1, obs2, P1, P2, recovered);
    if (tri_ok)
    {
        double err_3d = cv::norm(cv::Point3f(recovered.x - gt_pt.x,
                                             recovered.y - gt_pt.y,
                                             recovered.z - gt_pt.z));
        std::cout << "   복원: (" << std::setprecision(2) << recovered.x << ", " << recovered.y
                  << ", " << recovered.z << ")" << std::endl;
        std::cout << "   GT와 차이: " << std::setprecision(4) << err_3d
                  << (err_3d < 0.1 ? "m ✅" : "m ❌ 기대: < 0.1m") << std::endl;
    }
    else
    {
        std::cout << "   삼각측량 실패 ❌" << std::endl;
    }

    // ── Step 5: triangulatePoints ────────────────
    std::cout << "\nStep 5: triangulatePoints" << std::endl;
    // 여러 GT 점 생성
    std::vector<cv::Point3f> gt_pts = {
        {0, 0, 3}, {1, 0, 4}, {-1, 0.5, 2.5}, {0.5, -0.5, 5}, {-0.5, 1, 3.5}};
    std::vector<cv::Point2f> multi_obs1, multi_obs2;
    for (const auto& gp : gt_pts)
    {
        cv::Mat p4 = (cv::Mat_<double>(4, 1) << gp.x, gp.y, gp.z, 1.0);
        cv::Mat pr1 = P1 * p4;
        cv::Mat pr2 = P2 * p4;
        multi_obs1.push_back(cv::Point2f(pr1.at<double>(0) / pr1.at<double>(2),
                                         pr1.at<double>(1) / pr1.at<double>(2)));
        multi_obs2.push_back(cv::Point2f(pr2.at<double>(0) / pr2.at<double>(2),
                                         pr2.at<double>(1) / pr2.at<double>(2)));
    }
    std::vector<cv::Point3f> multi_recovered;
    TriangulationBasic::triangulatePoints(multi_obs1, multi_obs2, P1, P2, multi_recovered);
    std::cout << "   입력: " << gt_pts.size() << "개, 복원: " << multi_recovered.size() << "개"
              << (multi_recovered.size() == gt_pts.size() ? " ✅" : " ❌") << std::endl;

    // ── Step 6: averageReprojectionError ─────────
    std::cout << "\nStep 6: averageReprojectionError" << std::endl;
    if (!multi_recovered.empty())
    {
        double avg_err = TriangulationBasic::averageReprojectionError(
            multi_recovered, multi_obs1, P1);
        std::cout << "   평균 재투영 오차: " << std::setprecision(4) << avg_err
                  << (avg_err >= 0 && avg_err < 0.1 ? " px ✅" : " px ❌ 기대: ~0")
                  << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  Step 5 먼저 구현 필요" << std::endl;
    }

    // ── Step 7: evaluateTriangulation ────────────
    std::cout << "\nStep 7: evaluateTriangulation" << std::endl;
    if (!multi_recovered.empty())
    {
        TriangulationBasic::evaluateTriangulation(
            multi_recovered, multi_obs1, multi_obs2, P1, P2);
        std::cout << "   (위 출력 확인)" << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  Step 5 먼저 구현 필요" << std::endl;
    }

    // ── Step 8: visualize3DPoints ────────────────
    std::cout << "\nStep 8: visualize3DPoints" << std::endl;
    if (!multi_recovered.empty())
    {
        TriangulationBasic::visualize3DPoints(multi_recovered);
        std::cout << "   (위 출력 확인)" << std::endl;
    }
    else
    {
        std::cout << "   ⚠️  Step 5 먼저 구현 필요" << std::endl;
    }

    // ── Step 9: demoPipeline ─────────────────────
    std::cout << "\nStep 9: demoPipeline" << std::endl;
    TriangulationBasic::demoPipeline(K, R, t);
    std::cout << "   (위 출력 확인)" << std::endl;

    // ── 요약 ────────────────────────────────────
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  모든 Step ✅ 확인 후 → quiz_easy.cpp, quiz_medium.cpp" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
