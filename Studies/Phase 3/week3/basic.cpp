/**
 * Phase 3 Week 3 - 3D-2D 모션 추정 (PnP: Perspective-n-Point)
 *
 * 이미 알고 있는 3D 맵 점과 현재 이미지의 2D 관측으로 카메라 포즈를 추정.
 * VO 파이프라인의 "추적 단계"에 해당.
 *
 * Week 2 vs Week 3:
 *   Week 2 (2D-2D): 3D 맵 없음 → E 추정 → 초기 맵 생성 (초기화)
 *   Week 3 (3D-2D): 3D 맵 있음 → PnP → 카메라 포즈 추정 (추적)
 *
 * PnP의 핵심 장점:
 *   ✅ 절대 스케일 복원! (3D 점의 실제 크기를 알고 있으므로)
 *   ✅ 최소 4개 대응점으로 가능 (2D-2D는 최소 5~8개)
 *   ✅ 계산이 빠르고 안정적
 *   → 대부분의 VO/SLAM에서 초기화 이후의 포즈 추정에 PnP 사용
 */

#include "motion_3d2d.h"
#include <iostream>
#include <iomanip>

// PnP (Perspective-n-Point) — 3D↔2D 대응으로 카메라 포즈 추정
//
// 문제 정의:
//   주어진: n개의 (3D 점 Xᵢ, 2D 관측 xᵢ) 대응 쌍
//   구하기: 카메라 포즈 [R|t] (월드 → 카메라 변환)
//
// 수학적 정의:
//   xᵢ = K · [R|t] · Xᵢ  (3D → 2D 투영)
//   → R, t를 찾아서 이 투영 관계를 가장 잘 만족하도록
//
// EPnP (Efficient PnP):
//   O(n) 시간복잡도로 동작하는 PnP 알고리즘 (OpenCV 기본값).
//   원리: n개의 3D 점을 4개의 "가상 제어점(virtual control point)"으로 표현.
//     → 4개의 제어점 좌표만 추정하면 됨 → 선형 문제로 환원
//     → 큰 n에서도 빠르고 안정적
//
// ★ 최소 4개 대응점이 필요한 이유:
//   각 대응점 (Xᵢ, xᵢ)은 2개의 독립 방정식 제공 (u, v 좌표)
//   R, t의 자유도 = 6 (회전 3 + 이동 3)
//   → 3개 점: 6개 방정식 → 이론적으로는 충분하지만 해가 여러 개 (P3P)
//   → 4개 점: 8개 방정식 → 유일한 해 결정 가능
bool Motion3D2D::solvePnP(const std::vector<cv::Point3f>& points3d,
                          const std::vector<cv::Point2f>& points2d, const cv::Mat& K, cv::Mat& rvec,
                          cv::Mat& tvec)
{
    // 최소 4개 대응점 체크
    if (points3d.size() < 4 || points2d.size() < 4)
    {
        return false;
    }

    // cv::solvePnP: EPnP 알고리즘으로 [R|t] 추정
    //   points3d: 3D 점 (월드 좌표)
    //   points2d: 2D 관측 (픽셀 좌표)
    //   K: 카메라 내부 파라미터
    //   cv::Mat(): 왜곡 계수 없음 (이미 보정된 이미지라고 가정)
    //   rvec: 출력 회전 벡터 (3×1, Rodrigues 표현)
    //     → cv::Rodrigues(rvec, R)로 3×3 회전 행렬로 변환 가능
    //   tvec: 출력 이동 벡터 (3×1)
    //   false: 외부 추측값 사용 안 함 (처음부터 추정)
    //   SOLVEPNP_EPNP: EPnP 알고리즘 선택
    return cv::solvePnP(points3d, points2d, K, cv::Mat(), rvec, tvec, false, cv::SOLVEPNP_EPNP);
}

// PnP + RANSAC — Outlier가 섞인 데이터에서 강건하게 포즈 추정
//
// 왜 RANSAC이 필요한가?
//   실제 환경에서는 대응점에 outlier가 포함됨:
//     - 특징점 추적 실패 (배경과 물체 혼동)
//     - 동적 물체 (움직이는 사람, 차)
//     - 반복적 텍스처 (타일, 벽돌 → 오매칭)
//   outlier가 하나만 있어도 일반 PnP의 결과가 크게 틀어짐!
//
// RANSAC + PnP 동작:
//   1. 무작위 4개 대응점 선택 → PnP로 R, t 추정
//   2. 나머지 모든 점을 R, t로 투영 → 실제 관측과의 거리 계산
//   3. 거리가 임계값(8.0px) 이내면 inlier로 분류
//   4. 1~3을 반복하여 inlier가 가장 많은 R, t 선택
//
// 파라미터 의미:
//   iterationsCount=100: 최대 100번 샘플링 (보통 충분)
//   reprojectionError=8.0: 재투영 오차 8px 이내면 inlier
//     → 너무 작으면 inlier가 적어져 추정 불안정
//     → 너무 크면 outlier가 inlier에 포함되어 정확도 하락
//   confidence=0.99: 99% 확률로 올바른 모델 보장
int Motion3D2D::solvePnPRansac(const std::vector<cv::Point3f>& points3d,
                               const std::vector<cv::Point2f>& points2d, const cv::Mat& K,
                               cv::Mat& rvec, cv::Mat& tvec, std::vector<int>& inliers)
{
    if (points3d.size() < 4 || points2d.size() < 4)
    {
        return 0;
    }

    // cv::solvePnPRansac: RANSAC으로 outlier를 제거하면서 PnP 수행
    //   inliers: RANSAC이 선별한 inlier 점들의 인덱스 목록
    //     예: inliers = {0, 2, 5, 7, ...} → 0번, 2번, 5번... 점이 inlier
    cv::solvePnPRansac(points3d, points2d, K, cv::Mat(), rvec, tvec,
                       false,   // useExtrinsicGuess: 이전 추정값 사용 안 함
                       100,     // iterationsCount: RANSAC 반복 횟수
                       8.0,     // reprojectionError: inlier 판정 임계값 (픽셀)
                       0.99,    // confidence: 성공 확률
                       inliers  // output: inlier 인덱스
    );

    return inliers.size();
}

// 재투영 오차(Reprojection Error) 계산 — PnP 추정 품질 평가
//
// 재투영 오차란?
//   추정한 [R|t]로 3D 점을 2D에 투영한 위치와
//   실제 관측된 2D 위치의 차이.
//
//   공식: RMS = sqrt( (1/N) · Σ ||xᵢ_observed - xᵢ_projected||² )
//
// 이 값이 작을수록 R, t 추정이 정확한 것.
// 일반적 기준:
//   < 1.0 px: 우수 (정밀한 VO에 적합)
//   1~3 px: 보통 (노이즈가 있는 환경)
//   > 5 px: 불량 (추정에 심각한 문제)
//
// ★ 캘리브레이션의 RMS(Week 2)와 같은 개념:
//   캘리브레이션: K, dist를 추정한 후 검증
//   PnP: R, t를 추정한 후 검증
//   → 둘 다 "추정 파라미터가 관측을 얼마나 잘 설명하는가?"를 측정
double Motion3D2D::computeReprojectionError(const std::vector<cv::Point3f>& points3d,
                                            const std::vector<cv::Point2f>& points2d,
                                            const cv::Mat& K, const cv::Mat& rvec,
                                            const cv::Mat& tvec)
{
    // 추정한 rvec, tvec으로 3D → 2D 투영
    std::vector<cv::Point2f> projected;
    projectPoints(points3d, K, rvec, tvec, projected);

    // 각 점에 대해 관측값과 투영값의 거리 제곱 합산
    //   ||xᵢ_observed - xᵢ_projected||² = dx² + dy²
    double sum_error = 0.0;
    for (size_t i = 0; i < points2d.size(); i++)
    {
        double dx = projected[i].x - points2d[i].x;
        double dy = projected[i].y - points2d[i].y;
        sum_error += dx * dx + dy * dy;
    }

    // RMS (Root Mean Square): 평균 제곱 오차의 제곱근
    return std::sqrt(sum_error / points2d.size());
}

// 3D 점을 2D 픽셀 좌표로 투영 — 카메라 모델의 순방향 계산
//
// 투영 과정 (3단계):
//   ① 월드 → 카메라: P_cam = R · P_world + t
//      회전과 이동을 적용하여 카메라 좌표계로 변환
//   ② 카메라 → 정규화: x = X/Z, y = Y/Z
//      3D 점을 카메라 앞 Z=1 평면에 투영
//   ③ 정규화 → 픽셀: u = fx·x + cx, v = fy·y + cy
//      카메라 내부 파라미터 K로 픽셀 좌표 생성
//
// cv::projectPoints는 이 3단계 + 왜곡 모델까지 한번에 수행.
// 여기서는 cv::Mat() (빈 왜곡 계수)로 왜곡 없는 투영.
void Motion3D2D::projectPoints(const std::vector<cv::Point3f>& points3d, const cv::Mat& K,
                               const cv::Mat& rvec, const cv::Mat& tvec,
                               std::vector<cv::Point2f>& projected)
{
    cv::projectPoints(points3d, rvec, tvec, K, cv::Mat(), projected);
}

// VO 추적 시뮬레이션 — 여러 프레임에 걸친 PnP 추적 과정 시연
//
// 실제 VO의 추적 루프:
//   매 프레임마다:
//     1. 3D 맵 점을 현재 이미지에 투영 → 예상 위치
//     2. 주변에서 특징점 검색 → 2D-3D 대응 생성
//     3. PnP + RANSAC → 현재 포즈 추정
//     4. inlier 비율 확인 → 너무 낮으면 추적 실패
//
// 추적 실패 조건:
//   inlier_ratio < 30%: 대부분의 매칭이 실패
//     → 갑작스러운 이동, 가림, 조명 변화 등
//     → 재초기화 필요 (2D-2D로 돌아가 새 맵 생성)
//
// ★ 이것이 VINS-Mono의 프론트엔드 동작 방식!
//   정상: PnP로 연속 추적
//   실패 시: 새 Key Frame 선택 → 2D-2D 초기화 → 맵 재구성
void Motion3D2D::simulateTracking(const std::vector<cv::Point3f>& map_points, const cv::Mat& K,
                                  int num_frames)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "VO 추적 시뮬레이션" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "초기 맵: " << map_points.size() << "개 3D 점\n" << std::endl;

    for (int frame = 0; frame < num_frames; frame++)
    {
        // 각 프레임의 Ground truth 포즈 — 일정하게 전진하는 모션
        //   rvec: 프레임마다 X, Y축으로 조금씩 회전 (전방을 바라보며 고개 돌림)
        //   tvec: 프레임마다 X방향 0.1m, Y방향 0.02m 이동 (전진 + 약간의 측면 이동)
        cv::Mat rvec_gt = (cv::Mat_<double>(3, 1) << 0.01 * frame, 0.005 * frame, 0.0);
        cv::Mat tvec_gt = (cv::Mat_<double>(3, 1) << 0.1 * frame, 0.02 * frame, 0.0);

        // 투영 → 2D 관측 생성 (실제로는 이미지에서 특징점 검출)
        std::vector<cv::Point2f> observations;
        projectPoints(map_points, K, rvec_gt, tvec_gt, observations);

        // 노이즈 추가 — 실제 특징점 검출의 불확실성
        for (auto& pt : observations)
        {
            pt.x += (rand() % 20 - 10) / 10.0;
            pt.y += (rand() % 20 - 10) / 10.0;
        }

        // PnP + RANSAC으로 포즈 추정
        cv::Mat rvec_est, tvec_est;
        std::vector<int> inliers;
        int inlier_count = solvePnPRansac(map_points, observations, K, rvec_est, tvec_est, inliers);

        // 추적 품질 지표
        double inlier_ratio = (double)inlier_count / map_points.size();
        double reproj_error =
            computeReprojectionError(map_points, observations, K, rvec_est, tvec_est);

        std::cout << "Frame " << frame << ":" << std::endl;
        std::cout << "   Inliers: " << inlier_count << "/" << map_points.size() << " ("
                  << std::fixed << std::setprecision(1) << inlier_ratio * 100 << "%)" << std::endl;
        std::cout << "   Reproj error: " << std::setprecision(2) << reproj_error << " px"
                  << std::endl;
        std::cout << "   Est t: [" << std::setprecision(3) << tvec_est.at<double>(0) << ", "
                  << tvec_est.at<double>(1) << ", " << tvec_est.at<double>(2) << "]" << std::endl;

        // inlier 비율 30% 미만 = 추적 실패
        //   원인: 대부분의 3D-2D 대응이 잘못됨
        //   대처: 재초기화 (새 Key Frame에서 2D-2D로 다시 시작)
        if (inlier_ratio < 0.3)
        {
            std::cout << "   ⚠️  추적 실패! 재초기화 필요\n" << std::endl;
            break;
        }
        std::cout << std::endl;
    }
}

// 데모 — 시뮬레이션 데이터로 3D-2D 파이프라인 전체를 시연
//
// Week 2 → Week 3 의 연결:
//   Week 2에서 삼각측량으로 생성한 초기 3D 맵이 있다고 가정.
//   이 3D 맵 + 새로운 프레임의 2D 관측 → PnP로 포즈 추정.
//   → 이후 새로 추적된 점도 삼각측량으로 3D 맵에 추가 (맵 성장)
void Motion3D2D::demo()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "3D-2D 모션 추정 (PnP) 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // PnP 개요
    //   입력: {3D 점 Xᵢ, 2D 관측 xᵢ} 의 n개 대응 쌍
    //   출력: 카메라 포즈 [R|t]
    //
    //   2D-2D(Essential Matrix)와의 핵심 차이:
    //     2D-2D: 상대 포즈만 추정 (스케일 모호)
    //     3D-2D: 절대 포즈 추정 (실제 스케일!)
    //     → 3D 점이 실제 미터 단위 좌표를 갖고 있으므로
    //       추정된 t도 실제 미터 단위!
    std::cout << "💡 [PnP 개요]" << std::endl;
    std::cout << "   입력: n개의 {3D 점 Xᵢ, 2D 관측 xᵢ} 대응" << std::endl;
    std::cout << "   출력: 카메라 포즈 [R|t] (실제 스케일!)" << std::endl;
    std::cout << "   💡 이 정의가 quiz_easy 문제 1!\n" << std::endl;

    // P3P vs EPnP 비교
    //
    //   P3P (Perspective-3-Point):
    //     최소 3개 대응점만으로 풀이 (최소 해법)
    //     해가 최대 4개 → 4번째 점으로 모호성 해소
    //     RANSAC에 최적: 샘플 크기 3 → 조합 수 적음 → 빠른 수렴
    //
    //   EPnP (Efficient Perspective-n-Point):
    //     n개 점을 모두 사용하여 한 번에 풀이
    //     O(n) 시간복잡도 (점 수에 비례)
    //     해가 유일 → 안정적, OpenCV 기본값
    //
    //   실제 사용:
    //     RANSAC 내부: P3P로 빠르게 모델 후보 생성
    //     RANSAC 후: inlier만으로 EPnP + refinement → 최종 포즈
    std::cout << "💡 [P3P vs EPnP]" << std::endl;
    std::cout << "   P3P: 최소 3점, 4해, RANSAC용" << std::endl;
    std::cout << "   EPnP: n점 O(n), 안정적, OpenCV 기본" << std::endl;
    std::cout << "   💡 이 비교가 quiz_easy 문제 2!\n" << std::endl;

    // 스케일 복원 — PnP의 핵심 장점
    //
    //   2D-2D (Essential Matrix):
    //     t는 ||t||=1로 정규화됨 → 이동 "방향"만 알 수 있음
    //     1m인지 10m인지 구분 불가
    //
    //   3D-2D (PnP):
    //     3D 점이 실제 미터 단위 좌표를 갖고 있음
    //     예: 점이 (1.0, 0.5, 3.0)m에 있다면
    //     추정된 tvec도 실제 미터 단위!
    //     → 스케일 모호성 해결!
    //
    //   ★ 단, 3D 점의 스케일이 처음부터 정확해야 함.
    //     Week 2에서 삼각측량한 3D 점은 스케일 모호 → PnP도 모호.
    //     스테레오/RGB-D/IMU로 초기 스케일을 확정해야 진정한 절대 스케일.
    std::cout << "💡 [스케일 복원]" << std::endl;
    std::cout << "   2D-2D: ||t||=1 (정규화) → 스케일 모호" << std::endl;
    std::cout << "   3D-2D: 3D 점의 절대 크기 알고 있음 → 실제 t!" << std::endl;
    std::cout << "   💡 이 개념이 quiz_easy 문제 3!\n" << std::endl;

    // 카메라 파라미터
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // 3D 맵 (Week 2의 삼각측량으로 생성했다고 가정)
    //   100개 점: 카메라 앞 3~8m에 분포
    //   이 맵이 PnP의 입력이 되는 "알려진 3D 점"
    std::vector<cv::Point3f> map_points;
    for (int i = 0; i < 100; i++)
    {
        map_points.push_back(cv::Point3f(-3.0 + (rand() % 60) / 10.0, -2.0 + (rand() % 40) / 10.0,
                                         3.0 + (rand() % 50) / 10.0));
    }

    std::cout << "3D 맵: " << map_points.size() << "개 점\n" << std::endl;

    // Ground truth 포즈
    //   rvec: 작은 회전 (X: 0.05rad ≈ 3도, Y: 0.02rad ≈ 1도)
    //   tvec: 작은 이동 (X: 0.5m, Y: 0.1m, Z: 0.05m)
    //   → 연속 프레임 간의 전형적인 이동량
    cv::Mat rvec_gt = (cv::Mat_<double>(3, 1) << 0.05, 0.02, 0.0);
    cv::Mat tvec_gt = (cv::Mat_<double>(3, 1) << 0.5, 0.1, 0.05);

    std::cout << "Ground Truth 포즈:" << std::endl;
    std::cout << "   t = " << tvec_gt.t() << "\n" << std::endl;

    // 투영 — 3D 맵을 현재 카메라에서 2D로 투영
    std::vector<cv::Point2f> observations;
    projectPoints(map_points, K, rvec_gt, tvec_gt, observations);

    // 노이즈 추가 — 특징점 검출/매칭의 불확실성
    for (auto& pt : observations)
    {
        pt.x += (rand() % 20 - 10) / 10.0;
        pt.y += (rand() % 20 - 10) / 10.0;
    }

    // PnP + RANSAC 추정
    cv::Mat rvec_est, tvec_est;
    std::vector<int> inliers;
    int inlier_count = solvePnPRansac(map_points, observations, K, rvec_est, tvec_est, inliers);

    std::cout << "PnP 추정 결과:" << std::endl;
    std::cout << "   Inliers: " << inlier_count << "/" << map_points.size() << std::endl;
    std::cout << "   Est t: " << tvec_est.t() << std::endl;

    // 이동 오차 — 추정 tvec과 정답 tvec의 L2 거리
    //   ★ 여기서 tvec은 실제 미터 단위!
    //   Week 2의 t_est는 ||t||=1이었지만,
    //   PnP의 tvec_est는 실제 크기 → 0.5m에 가까운 값
    double t_error = cv::norm(tvec_gt - tvec_est);
    std::cout << "   이동 오차: " << std::fixed << std::setprecision(4) << t_error << "\n"
              << std::endl;

    // 재투영 오차 — PnP 추정 품질 검증
    double reproj_error = computeReprojectionError(map_points, observations, K, rvec_est, tvec_est);
    std::cout << "재투영 오차: " << std::setprecision(2) << reproj_error << " px" << std::endl;
    std::cout << "   💡 이 오차 개념이 quiz_medium 문제 2!\n" << std::endl;

    // Inlier Ratio 해석 — 추적 건전성 지표
    //   inlier_ratio = (RANSAC이 선별한 inlier 수) / (전체 대응점 수)
    //
    //   > 70%: 양호 — 대부분의 대응이 정확, 안정적 추적
    //   > 50%: 보통 — 일부 오매칭 있지만 추적 유지 가능
    //   < 30%: 추적 실패 — 재초기화 필요
    //     원인: 빠른 이동, 가림(occlusion), 조명 급변, 모션 블러
    std::cout << "💡 [Inlier Ratio 해석]" << std::endl;
    double inlier_ratio = (double)inlier_count / map_points.size();
    std::cout << "   현재: " << std::setprecision(1) << inlier_ratio * 100 << "%" << std::endl;
    std::cout << "   > 70%: 양호 / > 50%: 보통 / < 30%: 추적 실패" << std::endl;
    std::cout << "   💡 이 기준이 quiz_easy 문제 4!\n" << std::endl;

    // VO 추적 시뮬레이션 — 5프레임 연속 PnP 추적
    simulateTracking(map_points, K, 5);

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

// 메인 함수 — Week 3 데모 실행
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 3: 3D-2D 모션 추정 (PnP)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Motion3D2D::demo();

    std::cout << "\n💡 핵심 내용:" << std::endl;
    std::cout << "   - PnP로 3D-2D 모션 추정" << std::endl;
    std::cout << "   - RANSAC으로 Outlier 제거" << std::endl;
    std::cout << "   - ✅ 절대 스케일 복원!" << std::endl;
    std::cout << "   - VO 추적의 핵심 방법\n" << std::endl;

    std::cout << "💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md에서 PnP 이론 읽기 → quiz_easy 문제 1~3" << std::endl;
    std::cout << "   2. my_basic.cpp Step 1~3 구현 → quiz_easy 문제 4" << std::endl;
    std::cout << "   3. my_basic.cpp Step 4~6 구현 → quiz_medium 문제 1~3" << std::endl;
    std::cout << "   4. PRACTICE.md에서 추가 실습\n" << std::endl;

    std::cout << "다음: Week 4 - 3D-3D 모션 추정 (ICP)\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
