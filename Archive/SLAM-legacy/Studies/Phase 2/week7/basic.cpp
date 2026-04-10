#include "basic.h"
#include <iostream>
#include <iomanip>
#include <cmath>

// Rodrigues 변환 — 회전 벡터(3×1) ↔ 회전 행렬(3×3) 변환
//
// 회전 벡터 rvec:
//   방향 = 회전 축 (단위 벡터)
//   크기(||rvec||) = 회전 각도 (라디안)
//   예: rvec = [0, 0, π/2] → Z축 기준 90도 회전
//
// 왜 두 가지 표현?
//   회전 벡터: 3개 파라미터 → 최적화에 적합 (오버파라미터화 없음)
//   회전 행렬: 점 변환에 직접 사용 (Pc = R·Pw + t)
//   → OpenCV는 내부적으로 rvec을 사용하고, 필요할 때 Rodrigues로 변환
void PnPBasic::rodrigues(const cv::Mat& rvec, cv::Mat& R)
{
    cv::Rodrigues(rvec, R);
}

// PnP (Perspective-n-Point) — 3D↔2D 대응으로 카메라 포즈 추정
//
// 입력: N개의 알려진 3D 점 + 이미지에서 관측된 2D 점 + 카메라 K
// 출력: 카메라 포즈 (rvec, tvec) — 6 DOF (3 회전 + 3 이동)
//
// 최소 필요 대응점 수:
//   P3P: 3개 (최소. 해가 여러 개 → 4번째로 선택)
//   DLT: 6개 (직접 선형 변환)
//   EPnP: 4개 (효율적)
//   ITERATIVE: 4개 이상 (Levenberg-Marquardt 반복 최적화)
//
// SLAM에서의 역할:
//   맵의 3D 점(랜드마크) + 현재 프레임의 2D 관측 → 현재 카메라 위치 추정
//   = Visual Odometry의 핵심 tracking 단계
bool PnPBasic::solvePnP(const std::vector<cv::Point3f>& points3d,
                        const std::vector<cv::Point2f>& points2d, const cv::Mat& K, cv::Mat& rvec,
                        cv::Mat& tvec, int method)
{
    if (points3d.size() < 4 || points3d.size() != points2d.size())
    {
        return false;
    }

    cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);

    // method: SOLVEPNP_ITERATIVE, SOLVEPNP_P3P, SOLVEPNP_EPNP 등
    bool success = cv::solvePnP(points3d, points2d, K, distCoeffs, rvec, tvec, false, method);

    return success;
}

// RANSAC + PnP — outlier에 강건한 포즈 추정
//
// 일반 PnP는 모든 점을 신뢰하므로, 하나의 outlier만 있어도 결과가 크게 왜곡됨.
// RANSAC으로 outlier를 제거한 뒤 inlier만으로 PnP를 수행.
//
// 동작:
//   ① 랜덤 4개 점 선택 → PnP로 포즈 추정
//   ② 나머지 점을 이 포즈로 재투영 → 오차 < reprojection_error면 inlier
//   ③ 100번 반복 → inlier가 가장 많은 포즈를 최종 결과로
//
// 실제 SLAM에서는 항상 RANSAC + PnP 사용 (outlier가 반드시 존재)
int PnPBasic::solvePnPRansac(const std::vector<cv::Point3f>& points3d,
                             const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                             cv::Mat& rvec, cv::Mat& tvec, std::vector<int>& inliers,
                             double reprojection_error)
{
    if (points3d.size() < 4 || points3d.size() != points2d.size())
    {
        return 0;
    }

    cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
    cv::Mat inliers_mask;

    bool success =
        cv::solvePnPRansac(points3d, points2d, K, distCoeffs, rvec, tvec, false,
                           100,                 // 최대 RANSAC 반복 횟수
                           reprojection_error,  // inlier 판정 임계값 (픽셀)
                           0.99,                // 신뢰도 (99%)
                           inliers_mask);

    if (!success)
        return 0;

    inliers.clear();
    for (int i = 0; i < inliers_mask.rows; i++)
    {
        if (inliers_mask.at<uchar>(i))
        {
            inliers.push_back(i);
        }
    }

    return inliers.size();
}

// PnP 포즈 품질 평가 — 평균 재투영 오차(Mean Reprojection Error)
//
// 동작:
//   ① 추정된 포즈(rvec, tvec)로 3D 점을 2D에 재투영
//      projected_i = K · [R|t] · P3D_i
//   ② 관측된 2D 점과 재투영된 점의 거리 계산
//      error_i = ||observed_i - projected_i||₂
//   ③ 전체 점의 평균 오차 반환
//
// 반환값의 의미:
//   < 1.0 px: 매우 정확한 포즈 추정
//   1~3 px:   일반적인 수준
//   > 5 px:   포즈가 부정확하거나 outlier 존재
//
// SLAM에서의 역할:
//   - PnP 결과의 신뢰도 판단 (오차가 크면 키프레임 추가 등 대응)
//   - BA(Bundle Adjustment)의 비용 함수로도 동일한 오차 사용
double PnPBasic::evaluatePose(const std::vector<cv::Point3f>& points3d,
                              const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                              const cv::Mat& rvec, const cv::Mat& tvec)
{
    // 추정된 포즈로 3D→2D 재투영
    // projectPoints: rvec, tvec, K를 사용해 P3D → p2d 변환
    std::vector<cv::Point2f> projected;
    cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
    cv::projectPoints(points3d, rvec, tvec, K, distCoeffs, projected);

    // 평균 유클리드 거리 = 평균 재투영 오차
    // 참고: calibrateCamera는 RMS(제곱평균제곱근)를 사용하지만,
    //       여기서는 단순 평균 거리를 사용 (L2 norm의 평균)
    double total_error = 0.0;
    for (size_t i = 0; i < points2d.size(); i++)
    {
        double dx = projected[i].x - points2d[i].x;
        double dy = projected[i].y - points2d[i].y;
        total_error += std::sqrt(dx * dx + dy * dy);
    }

    return total_error / points2d.size();
}

// PnP 결과 시각화 — 관측점 vs 재투영점 비교
//
// 시각화 요소:
//   녹색 원 (○): 실제 관측된 2D 점 (이미지에서 검출된 위치)
//   빨간 점 (●): 추정 포즈로 3D→2D 재투영한 점
//   파란 선 (─): 관측↔재투영 사이의 오차 벡터
//   → 선이 짧을수록 포즈 추정이 정확함
//
// 디버깅 활용:
//   - 특정 영역에서 오차가 크면 → 해당 영역 특징점 품질 문제
//   - 전체적으로 오차가 크면 → 포즈 추정 실패 또는 K 행렬 오류
void PnPBasic::visualizePnP(const cv::Mat& img, const std::vector<cv::Point3f>& points3d,
                            const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                            const cv::Mat& rvec, const cv::Mat& tvec, cv::Mat& output)
{
    // 그레이스케일 → BGR 변환 (컬러 시각화를 위해)
    if (img.channels() == 1)
    {
        cv::cvtColor(img, output, cv::COLOR_GRAY2BGR);
    }
    else
    {
        output = img.clone();
    }

    // 추정 포즈로 3D 점을 2D에 재투영
    std::vector<cv::Point2f> projected;
    cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
    cv::projectPoints(points3d, rvec, tvec, K, distCoeffs, projected);

    // 최대 20개 점만 표시 (과도한 시각적 혼잡 방지)
    for (size_t i = 0; i < std::min(points2d.size(), (size_t)20); i++)
    {
        // 관측점 (녹색) — 이미지에서 실제 검출된 위치
        cv::circle(output, points2d[i], 5, cv::Scalar(0, 255, 0), 2);

        // 재투영점 (빨강) — 추정 포즈로 3D→2D 변환한 위치
        cv::circle(output, projected[i], 3, cv::Scalar(0, 0, 255), -1);

        // 오차 벡터 (파랑) — 두 점 사이의 거리 = 재투영 오차
        cv::line(output, points2d[i], projected[i], cv::Scalar(255, 0, 0), 1);
    }

    // 평균 재투영 오차를 이미지 상단에 표시
    double error = evaluatePose(points3d, points2d, K, rvec, tvec);
    std::string info = "Reprojection Error: " + std::to_string(error).substr(0, 5) + " px";
    cv::putText(output, info, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8,
                cv::Scalar(0, 255, 0), 2);
}

// 포즈 추정 방법 비교 — 2D-2D vs 3D-2D vs 3D-3D
//
// SLAM의 3가지 포즈 추정 방법을 비교 설명:
//
//   2D-2D (Essential Matrix):
//     입력: 두 이미지의 2D↔2D 대응점
//     한계: 스케일 모호성 (절대 거리를 알 수 없음)
//     용도: VO 초기화 (맵이 아직 없을 때)
//
//   3D-2D (PnP):
//     입력: 알려진 3D 점 + 이미지의 2D 관측
//     장점: 절대 스케일 복원 (3D 점이 실제 크기를 가짐)
//     용도: VO tracking (매 프레임 카메라 위치 추정)
//
//   3D-3D (ICP):
//     입력: 두 3D 점군 사이의 대응
//     장점: 깊이 정보를 직접 활용
//     용도: RGB-D SLAM, LiDAR SLAM
//
// 전형적인 Monocular VO 흐름:
//   Frame 0→1: 2D-2D (Essential) → 초기 맵 생성
//   Frame 1→N: 3D-2D (PnP) → 매 프레임 tracking
void PnPBasic::compareMethods()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "포즈 추정 방법 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "1️⃣  2D-2D (Essential Matrix)" << std::endl;
    std::cout << "   입력: 두 이미지의 2D 대응점" << std::endl;
    std::cout << "   출력: 상대 포즈 (R, t) ← 스케일 모호성!" << std::endl;
    std::cout << "   사용: VO 초기화, 처음 두 프레임\n" << std::endl;

    std::cout << "2️⃣  3D-2D (PnP)" << std::endl;
    std::cout << "   입력: 3D 점 + 2D 관측" << std::endl;
    std::cout << "   출력: 절대 포즈 (R, t) ← 스케일 복원!" << std::endl;
    std::cout << "   사용: VO tracking, 매 프레임\n" << std::endl;

    std::cout << "3️⃣  3D-3D (ICP)" << std::endl;
    std::cout << "   입력: 두 3D 점군" << std::endl;
    std::cout << "   출력: 상대 변환 (R, t)" << std::endl;
    std::cout << "   사용: RGB-D SLAM, LiDAR\n" << std::endl;

    std::cout << "💡 SLAM에서의 전형적인 흐름:" << std::endl;
    std::cout << "   Frame 0-1: 2D-2D (Essential) → 초기화" << std::endl;
    std::cout << "   Frame 1-2: 3D-2D (PnP) → Tracking" << std::endl;
    std::cout << "   Frame 2-N: 계속 PnP..." << std::endl;
}

// Visual Odometry 시뮬레이션 — PnP를 이용한 연속 프레임 tracking
//
// 시뮬레이션 시나리오:
//   ① 월드 좌표계에 50개 3D 랜드마크(점)를 랜덤 배치
//   ② 카메라가 X 방향으로 직선 이동 (frame당 0.1m)
//   ③ 매 프레임마다:
//      - GT 포즈로 랜드마크를 2D에 투영 + 노이즈 추가
//      - RANSAC+PnP로 포즈 추정
//      - GT와 추정 포즈 비교 → 오차 출력
//
// 실제 VO와의 차이:
//   이 시뮬레이션: 3D 랜드마크를 미리 알고 있음 (Ground Truth)
//   실제 VO:       이전 프레임들의 삼각측량으로 3D 점을 생성한 뒤 PnP
//                  → 오차가 누적될 수 있음 (drift)
//
// 관찰 포인트:
//   - 노이즈가 있어도 PnP가 얼마나 정확한지 확인
//   - inlier 비율이 높을수록 추정이 안정적
void PnPBasic::demoVisualOdometry(const cv::Mat& K)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Visual Odometry 시뮬레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "시나리오: 카메라가 이동하면서 3D 랜드마크 추적\n" << std::endl;

    // 3D 랜드마크 (월드 좌표계)
    // X: -5~5, Y: -3~3, Z: 5~15 범위에 랜덤 배치
    // Z > 0 이므로 모든 점이 카메라 전방에 위치
    std::vector<cv::Point3f> landmarks;
    for (int i = 0; i < 50; i++)
    {
        landmarks.push_back(cv::Point3f(-5.0 + (rand() % 100) / 10.0,  // X: -5 ~ 5
                                        -3.0 + (rand() % 60) / 10.0,   // Y: -3 ~ 3
                                        5.0 + (rand() % 100) / 10.0    // Z: 5 ~ 15
                                        ));
    }

    std::cout << "랜드마크 개수: " << landmarks.size() << "개\n" << std::endl;

    // 카메라 궤적 저장용
    std::vector<cv::Mat> gt_poses;   // Ground truth 위치
    std::vector<cv::Mat> est_poses;  // PnP 추정 위치

    // 10개 프레임에 대해 VO 시뮬레이션
    for (int frame = 0; frame < 10; frame++)
    {
        // Ground truth 포즈: 회전 없이 X 방향으로 직선 이동
        // frame 0: (0,0,0), frame 1: (0.1,0,0), ... frame 9: (0.9,0,0)
        cv::Mat rvec_gt = (cv::Mat_<double>(3, 1) << 0.0, 0.0, 0.0);
        cv::Mat tvec_gt = (cv::Mat_<double>(3, 1) << frame * 0.1, 0.0, 0.0);

        gt_poses.push_back(tvec_gt.clone());

        // GT 포즈로 3D 랜드마크를 2D에 투영 → 관측 생성
        std::vector<cv::Point2f> observations;
        cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
        cv::projectPoints(landmarks, rvec_gt, tvec_gt, K, distCoeffs, observations);

        // 관측에 ±1 픽셀 가우시안 유사 노이즈 추가
        // 실제 특징점 검출기의 위치 불확실성을 모사
        for (auto& obs : observations)
        {
            obs.x += (rand() % 20 - 10) / 10.0;
            obs.y += (rand() % 20 - 10) / 10.0;
        }

        // RANSAC + PnP로 포즈 추정
        // 알려진 3D 랜드마크 + 노이즈가 있는 2D 관측 → 카메라 위치 복원
        cv::Mat rvec_est, tvec_est;
        std::vector<int> inliers;
        int inlier_count =
            solvePnPRansac(landmarks, observations, K, rvec_est, tvec_est, inliers, 5.0);

        est_poses.push_back(tvec_est.clone());

        // GT와 추정 포즈 사이의 유클리드 거리 = 위치 오차 (미터 단위)
        double error = cv::norm(tvec_gt - tvec_est);

        std::cout << "Frame " << frame << ": "
                  << "GT=(" << tvec_gt.at<double>(0) << "), "
                  << "Est=(" << tvec_est.at<double>(0) << "), "
                  << "Error=" << std::fixed << std::setprecision(4) << error << " m, "
                  << "Inliers=" << inlier_count << "/" << landmarks.size() << std::endl;
    }

    std::cout << "\n✅ Visual Odometry 완료!" << std::endl;
}

// PnP 전체 파이프라인 데모 — RANSAC 유무에 따른 비교
//
// 6단계 파이프라인:
//   ① 3D 랜드마크 생성 (월드 좌표계)
//   ② Ground Truth 포즈 설정 (약간의 회전 + 이동)
//   ③ GT 포즈로 2D 관측 생성 + 노이즈 + 10% outlier 추가
//   ④ 일반 PnP (RANSAC 없음) → outlier에 취약
//   ⑤ RANSAC + PnP → outlier 제거 후 정확한 추정
//   ⑥ GT와 비교하여 두 방법의 정확도 차이 확인
//
// 핵심 관찰:
//   outlier가 있을 때 일반 PnP는 크게 왜곡되지만,
//   RANSAC+PnP는 inlier만 선별하여 정확한 결과를 냄
void PnPBasic::demoPipeline(const cv::Mat& K)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "PnP 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Step 1: 3D 랜드마크 생성
    // X: -2~2, Y: -1.5~1.5, Z: 2~8 범위에 30개 점 랜덤 배치
    std::cout << "1️⃣  3D 랜드마크 생성..." << std::endl;

    std::vector<cv::Point3f> points3d;
    for (int i = 0; i < 30; i++)
    {
        points3d.push_back(cv::Point3f(-2.0 + (rand() % 40) / 10.0, -1.5 + (rand() % 30) / 10.0,
                                       2.0 + (rand() % 60) / 10.0));
    }

    std::cout << "   랜드마크: " << points3d.size() << "개\n" << std::endl;

    // Step 2: Ground Truth 카메라 포즈 설정
    // rvec: X축 0.1rad, Y축 0.05rad 회전 (약 5.7°, 2.9°)
    // tvec: (0.5, 0.2, 0.1) 미터 이동
    std::cout << "2️⃣  Ground Truth 포즈 설정..." << std::endl;

    cv::Mat rvec_gt = (cv::Mat_<double>(3, 1) << 0.1, 0.05, 0.0);
    cv::Mat tvec_gt = (cv::Mat_<double>(3, 1) << 0.5, 0.2, 0.1);

    cv::Mat R_gt;
    rodrigues(rvec_gt, R_gt);

    std::cout << "   회전 벡터:\n" << rvec_gt << std::endl;
    std::cout << "   이동 벡터:\n" << tvec_gt << "\n" << std::endl;

    // Step 3: 2D 관측 생성
    std::cout << "3️⃣  2D 관측 생성 (노이즈 추가)..." << std::endl;

    std::vector<cv::Point2f> points2d;
    cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
    cv::projectPoints(points3d, rvec_gt, tvec_gt, K, distCoeffs, points2d);

    // 노이즈 + Outlier 추가
    // ±1 픽셀 노이즈: 특징점 검출기의 위치 불확실성 모사
    // 10% outlier (+50px): 잘못된 특징점 매칭을 모사
    //   → 이것이 RANSAC이 필요한 이유
    for (size_t i = 0; i < points2d.size(); i++)
    {
        points2d[i].x += (rand() % 20 - 10) / 10.0;
        points2d[i].y += (rand() % 20 - 10) / 10.0;

        // 10% 확률로 큰 오차 추가 (outlier)
        if (rand() % 100 < 10)
        {
            points2d[i].x += 50;
        }
    }

    std::cout << "   관측점: " << points2d.size() << "개\n" << std::endl;

    // Step 4: 일반 PnP (RANSAC 없음)
    // 모든 점(inlier+outlier)을 신뢰 → outlier가 결과를 왜곡
    // SOLVEPNP_ITERATIVE: Levenberg-Marquardt 반복 최적화
    std::cout << "4️⃣  PnP 추정 (RANSAC 없음)..." << std::endl;

    cv::Mat rvec_simple, tvec_simple;
    solvePnP(points3d, points2d, K, rvec_simple, tvec_simple, cv::SOLVEPNP_ITERATIVE);

    double error_simple = evaluatePose(points3d, points2d, K, rvec_simple, tvec_simple);
    std::cout << "   재투영 오차: " << std::fixed << std::setprecision(3) << error_simple
              << " 픽셀\n"
              << std::endl;

    // Step 5: RANSAC + PnP
    // outlier를 제거한 뒤 inlier만으로 PnP → 정확한 추정
    std::cout << "5️⃣  PnP 추정 (RANSAC)..." << std::endl;

    cv::Mat rvec_ransac, tvec_ransac;
    std::vector<int> inliers;
    int inlier_count =
        solvePnPRansac(points3d, points2d, K, rvec_ransac, tvec_ransac, inliers, 5.0);

    double error_ransac = evaluatePose(points3d, points2d, K, rvec_ransac, tvec_ransac);

    std::cout << "   Inliers: " << inlier_count << " / " << points3d.size() << std::endl;
    std::cout << "   재투영 오차: " << error_ransac << " 픽셀\n" << std::endl;

    // Step 6: GT와 비교
    // cv::norm(tvec_gt - tvec_est) = 이동 벡터의 유클리드 거리 (미터)
    // RANSAC이 outlier를 제거하므로 오차가 작을 것으로 기대
    std::cout << "6️⃣  Ground Truth와 비교..." << std::endl;

    double t_error_simple = cv::norm(tvec_gt - tvec_simple);
    double t_error_ransac = cv::norm(tvec_gt - tvec_ransac);

    std::cout << "   Simple PnP 이동 오차: " << t_error_simple << " m" << std::endl;
    std::cout << "   RANSAC PnP 이동 오차: " << t_error_ransac << " m" << std::endl;

    std::cout << "\n💡 관찰:" << std::endl;
    std::cout << "   - RANSAC이 outlier를 제거하여 더 정확" << std::endl;
    std::cout << "   - 실제 SLAM에서는 항상 RANSAC 사용" << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 파이프라인 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

// 메인 함수 — PnP 학습 데모 전체 실행
//
// 실행 흐름:
//   ① 카메라 내부 파라미터 K 설정
//   ② PnP 개념, Rodrigues, PnP vs Essential 교육 출력
//   ③ demoPipeline: 일반 PnP vs RANSAC+PnP 비교
//   ④ demoVisualOdometry: 연속 프레임 tracking 시뮬레이션
//   ⑤ compareMethods: 2D-2D vs 3D-2D vs 3D-3D 비교
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  PnP 기본 데모 (Week 7)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 내부 파라미터 K
    // fx=fy=600 (초점거리), cx=400, cy=300 (주점=이미지 중심)
    // 800×600 해상도를 가정한 가상 카메라
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    std::cout << "카메라 파라미터 K:\n" << K << "\n" << std::endl;

    // 💡 [교육] PnP란?
    std::cout << "💡 [교육] PnP란? (quiz 문제 1에서 사용!)" << std::endl;
    std::cout << "   입력: N개의 3D 점(맵) + 2D 관측(이미지) + K" << std::endl;
    std::cout << "   출력: 카메라 포즈 (R, t) ← 6 DoF" << std::endl;
    std::cout << "   최소 3점 필요 (P3P), 실제로는 RANSAC + 수십 개 사용\n" << std::endl;

    // 💡 [교육] Rodrigues 변환
    std::cout << "💡 [교육] Rodrigues 변환:" << std::endl;
    std::cout << "   rvec (3×1 회전 벡터) ↔ R (3×3 회전 행렬)" << std::endl;
    std::cout << "   |rvec| = 회전 각도 (라디안)" << std::endl;
    std::cout << "   rvec/|rvec| = 회전 축 (단위 벡터)" << std::endl;
    std::cout << "   OpenCV: cv::Rodrigues(rvec, R)\n" << std::endl;

    // 💡 [교육] PnP vs Essential
    std::cout << "💡 [교육] PnP vs Essential (quiz 문제 2에서 사용!):" << std::endl;
    std::cout << "   2D-2D (Essential): 스케일 모호성 O → VO 초기화용" << std::endl;
    std::cout << "   3D-2D (PnP): 절대 스케일 복원! → VO tracking용" << std::endl;
    std::cout << "   💡 SLAM: Frame 0-1은 E, Frame 1~N은 PnP\n" << std::endl;

    // 전체 파이프라인
    PnPBasic::demoPipeline(K);

    // Visual Odometry 시뮬레이션
    PnPBasic::demoVisualOdometry(K);

    // 방법 비교
    PnPBasic::compareMethods();

    std::cout << "\n💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md 이론 읽기 (PnP, DLT, RANSAC)" << std::endl;
    std::cout << "   2. quiz_easy.cpp — PnP 정의, VO 흐름, DLT 개념" << std::endl;
    std::cout << "   3. my_basic.cpp — Step 1~8 순서대로 직접 구현" << std::endl;
    std::cout << "   4. quiz_medium.cpp — PnP 구현, RANSAC 횟수, DLT 구현" << std::endl;
    std::cout << "   5. PRACTICE.md — PnP/삼각측량 실습\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
