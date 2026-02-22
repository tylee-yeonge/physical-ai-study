/**
 * Phase 3 Week 2 - 2D-2D 모션 추정 (Essential Matrix)
 *
 * 두 이미지의 2D 대응점만으로 카메라의 상대 회전(R)과 이동 방향(t)을 추정.
 * VO 파이프라인의 "초기화 단계"에 해당.
 *
 * 왜 2D-2D가 필요한가?
 *   VO 시작 시에는 3D 맵이 아직 없으므로 PnP(3D-2D)를 쓸 수 없다.
 *   첫 두 프레임의 2D 대응점만으로 R, t를 추정 + 삼각측량으로 초기 3D 맵 생성.
 *   이후부터는 3D 맵이 있으므로 PnP(Week 3)로 전환.
 *
 * 전체 흐름:
 *   ① 픽셀 좌표 → 정규화 좌표 변환 (K 역변환)
 *   ② Essential Matrix E 추정 (RANSAC + 5-point algorithm)
 *   ③ E에서 R, t 복원 (SVD 분해 + Cheirality check)
 *   ④ 삼각측량으로 초기 3D 점 생성
 */

#include "motion_2d2d.h"
#include <iostream>
#include <iomanip>

// Essential Matrix 추정 — 에피폴라 기하(Epipolar Geometry)의 핵심
//
// 에피폴라 구속조건 (Epipolar Constraint):
//   p₂ᵀ · E · p₁ = 0
//   여기서 p₁, p₂는 두 이미지에서의 "정규화 좌표" (K 역변환 적용)
//
// Essential Matrix E란?
//   E = [t]× · R  (t의 반대칭 행렬 × 회전 행렬)
//   성질: rank(E) = 2, 2개의 같은 singular value
//   자유도: 5 (회전 3 + 이동 방향 2, 이동 크기는 스케일 모호)
//
// 5-Point Algorithm:
//   E의 자유도 = 5이므로 최소 5개 대응점으로 추정 가능.
//   8-Point Algorithm은 8개 사용 (과결정 → 노이즈에 더 강건).
//   OpenCV의 findEssentialMat은 5-point + RANSAC 조합.
//
// RANSAC (Random Sample Consensus):
//   목적: 대응점 중 outlier(오매칭)를 자동으로 걸러냄.
//   1. 무작위 5개 점 선택 → E 추정
//   2. 모든 점에 대해 에피폴라 제약 만족 여부 체크 → inlier 카운트
//   3. 1~2를 반복하여 inlier가 가장 많은 E 선택
//   confidence=0.999: 올바른 모델을 찾을 확률 99.9%
//   threshold=1.0: 에피폴라 선에서 1.0 픽셀 이내면 inlier
//
// ★ 정규화 좌표를 사용하는 이유:
//   E는 정규화 좌표 기준으로 정의됨. 픽셀 좌표를 사용하면
//   Fundamental Matrix F를 추정해야 함. (F = K₂⁻ᵀ E K₁⁻¹)
//   정규화 좌표를 쓰면 K가 단위행렬이 되어 E를 직접 추정 가능.
int Motion2D2D::estimateEssential(const std::vector<cv::Point2f>& points1,
                                  const std::vector<cv::Point2f>& points2, cv::Mat& E,
                                  std::vector<uchar>& inliers)
{
    // findEssentialMat: 5-point + RANSAC으로 E 추정
    //   첫 번째 인자: 이미지 1의 정규화 좌표
    //   두 번째 인자: 이미지 2의 정규화 좌표
    //   세 번째 인자: 카메라 행렬 → Identity (이미 정규화했으므로)
    //   RANSAC: outlier 제거 방법
    //   0.999: 신뢰도 (99.9% 확률로 올바른 모델 보장)
    //   1.0: inlier 판정 임계값 (정규화 좌표 단위)
    //   inliers: 각 점이 inlier(1)인지 outlier(0)인지 마스크
    E = cv::findEssentialMat(points1, points2,
                             cv::Mat::eye(3, 3, CV_64F),  // 정규화 좌표이므로 Identity
                             cv::RANSAC,
                             0.999,  // 신뢰도
                             1.0,    // 임계값
                             inliers);

    // inlier 수 카운트 — E 추정의 품질 지표
    //   inlier 비율이 높을수록 매칭 품질이 좋음
    //   50% 미만이면 매칭에 문제가 있는 것
    int inlier_count = 0;
    for (uchar status : inliers)
    {
        if (status)
            inlier_count++;
    }

    return inlier_count;
}

// E에서 R, t 복원 — SVD 분해 + Cheirality Check
//
// E = U · diag(1,1,0) · Vᵀ 로 SVD 분해하면
// 4가지 가능한 (R, t) 조합이 나옴:
//   (R₁, +t), (R₁, -t), (R₂, +t), (R₂, -t)
//   여기서:
//     R₁ = U · W · Vᵀ,  R₂ = U · Wᵀ · Vᵀ
//     t = U의 3번째 열 (부호 ±)
//     W = [0 -1 0; 1 0 0; 0 0 1] (90도 회전)
//
// Cheirality Check (양의 깊이 조건):
//   4가지 해 중, 삼각측량한 3D 점이 "두 카메라 모두의 앞"에 있는 해만 물리적으로 유효.
//   즉, 두 카메라 기준으로 depth > 0인 점이 가장 많은 해를 선택.
//   → 정상적인 경우 4가지 중 딱 1가지만 이 조건을 만족.
//
// 반환: Cheirality check를 통과한 점의 개수
//   이 값이 작으면 E 추정이 잘못되었거나 퇴화(degenerate) 상황
int Motion2D2D::recoverPose(const cv::Mat& E, const std::vector<cv::Point2f>& points1,
                            const std::vector<cv::Point2f>& points2, cv::Mat& R, cv::Mat& t,
                            std::vector<uchar>& inliers)
{
    // cv::recoverPose: E를 SVD 분해 → 4가지 해 → Cheirality check → 최적 해 선택
    //   Identity 행렬: 정규화 좌표이므로 K = I
    //   R, t: 출력 (카메라 1 → 카메라 2의 상대 변환)
    //   inliers: Cheirality check 통과한 점의 마스크
    //   반환값: 양의 깊이 조건을 만족하는 점의 개수
    //
    // ⚠️ 주의: t는 항상 ||t|| = 1로 정규화됨
    //   → 이동 "방향"만 알 수 있고, 실제 거리(스케일)는 모름
    //   → 이것이 Monocular VO의 스케일 모호성!
    int good_points =
        cv::recoverPose(E, points1, points2, cv::Mat::eye(3, 3, CV_64F), R, t, inliers);

    return good_points;
}

// 픽셀 좌표 → 정규화 좌표 변환
//
// 정규화 좌표(normalized coordinate)란?
//   카메라 내부 파라미터 K의 영향을 제거한 좌표.
//   "카메라 앞 Z=1 평면에 투영된 좌표"라고도 함.
//
// 변환 공식:
//   x_norm = (u - cx) / fx
//   y_norm = (v - cy) / fy
//
// 직관적 의미:
//   픽셀 좌표 (u, v)에서:
//     cx, cy를 빼서 → 이미지 중심을 원점으로 이동
//     fx, fy로 나눠서 → 초점거리로 스케일링 (픽셀 → 물리 단위)
//
// 왜 정규화하는가?
//   Essential Matrix는 정규화 좌표 기준으로 정의됨:
//     p₂ᵀ · E · p₁ = 0  (p는 정규화 좌표)
//   픽셀 좌표를 직접 쓰면 Fundamental Matrix F를 써야 함:
//     u₂ᵀ · F · u₁ = 0  (u는 픽셀 좌표)
//     F = K₂⁻ᵀ · E · K₁⁻¹
//   K를 알고 있으면 정규화 → E를 직접 사용하는 것이 더 안정적.
void Motion2D2D::normalizePoints(const cv::Mat& K, const std::vector<cv::Point2f>& points,
                                 std::vector<cv::Point2f>& normalized)
{
    normalized.clear();

    // K에서 파라미터 추출
    double fx = K.at<double>(0, 0);
    double fy = K.at<double>(1, 1);
    double cx = K.at<double>(0, 2);
    double cy = K.at<double>(1, 2);

    // 각 픽셀 좌표를 정규화 좌표로 변환
    //   (u, v) → ((u-cx)/fx, (v-cy)/fy)
    for (const auto& pt : points)
    {
        cv::Point2f p_norm;
        p_norm.x = (pt.x - cx) / fx;
        p_norm.y = (pt.y - cy) / fy;
        normalized.push_back(p_norm);
    }
}

// 삼각측량(Triangulation) — 두 시점의 2D 관측으로 3D 점 복원
//
// 원리:
//   같은 3D 점 X를 두 카메라에서 각각 x₁, x₂로 관측했을 때,
//   x₁을 지나는 광선(ray)과 x₂를 지나는 광선의 교차점이 X.
//
//   이상적으로: x₁ × (P₁ · X) = 0,  x₂ × (P₂ · X) = 0
//   노이즈 때문에 두 광선은 정확히 만나지 않으므로,
//   최소자승법(DLT)으로 가장 가까운 점을 추정.
//
// 투영 행렬:
//   P₁ = K · [I | 0]      (카메라 1: 원점, 정면)
//   P₂ = K · [R | t]      (카메라 2: R, t만큼 이동/회전)
//   P₁ · X = x₁ 이고 P₂ · X = x₂ 가 되어야 하는 것이 조건.
//
// ★ 삼각측량의 정밀도:
//   baseline(두 카메라 간 거리)이 클수록 정밀
//   baseline이 너무 작으면 → 두 광선이 거의 평행 → 교차점 부정확
//   baseline이 너무 크면 → 매칭 실패 가능성 증가
//   → 적절한 baseline이 중요 (일반적으로 depth의 10~20% 정도)
void Motion2D2D::triangulate(const cv::Mat& K, const cv::Mat& R, const cv::Mat& t,
                             const std::vector<cv::Point2f>& points1,
                             const std::vector<cv::Point2f>& points2,
                             std::vector<cv::Point3f>& points3d)
{
    // 카메라 1의 투영 행렬: P₁ = K · [I | 0]
    //   카메라 1을 월드 좌표계의 원점으로 설정
    //   → 3×4 행렬의 왼쪽 3×3 = K, 오른쪽 3×1 = 0
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));

    // 카메라 2의 투영 행렬: P₂ = K · [R | t]
    //   카메라 1 대비 R만큼 회전, t만큼 이동한 위치
    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R.copyTo(RT(cv::Rect(0, 0, 3, 3)));  // 왼쪽 3×3에 R 복사
    t.copyTo(RT(cv::Rect(3, 0, 1, 3)));  // 오른쪽 열에 t 복사
    cv::Mat P2 = K * RT;

    // triangulatePoints: DLT(Direct Linear Transform)로 3D 점 복원
    //   입력: P1, P2 (투영 행렬), points1, points2 (2D 대응점)
    //   출력: points4D (4×N 행렬, 동차 좌표 [X, Y, Z, W])
    //
    //   내부 동작:
    //     각 대응점 쌍에 대해 4개의 선형 방정식을 세움:
    //       x₁ × (P₁ · X) = 0  →  2개 독립 방정식
    //       x₂ × (P₂ · X) = 0  →  2개 독립 방정식
    //     → 4×4 행렬의 SVD로 X 추정 (가장 작은 singular value의 벡터)
    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, points1, points2, points4D);

    // 동차 좌표 → 유클리드 3D 좌표 변환
    //   동차 좌표: [X, Y, Z, W] → 유클리드: (X/W, Y/W, Z/W)
    //   W ≈ 0이면 무한원점(점이 매우 멀리 있음) → 제외
    points3d.clear();
    for (int i = 0; i < points4D.cols; i++)
    {
        float w = points4D.at<float>(3, i);
        if (std::abs(w) > 1e-6)
        {
            cv::Point3f pt;
            pt.x = points4D.at<float>(0, i) / w;
            pt.y = points4D.at<float>(1, i) / w;
            pt.z = points4D.at<float>(2, i) / w;

            // 깊이(depth) 양수 체크 — 카메라 앞에 있는 점만 유효
            //   카메라 좌표계에서 Z > 0 = 카메라 앞
            //   Z < 0 = 카메라 뒤 → 물리적으로 불가능한 점
            if (pt.z > 0)
            {
                points3d.push_back(pt);
            }
        }
    }
}

// 2D-2D 모션 추정 전체 파이프라인
//
// 호출 순서:
//   ① normalizePoints: 픽셀 → 정규화 좌표 (K 역변환)
//   ② estimateEssential: E 추정 (5-point + RANSAC)
//   ③ recoverPose: E에서 R, t 복원 (SVD + Cheirality)
//   ④ triangulate: 2D 대응점 → 3D 맵 생성
//
// 반환: 충분한 3D 점(≥10개)이 복원되면 true
//   3D 점이 너무 적으면 초기화 실패 → 다른 프레임 쌍으로 재시도
bool Motion2D2D::pipeline(const cv::Mat& K, const std::vector<cv::Point2f>& points1,
                          const std::vector<cv::Point2f>& points2, cv::Mat& R, cv::Mat& t,
                          std::vector<cv::Point3f>& points3d)
{
    // Step 1: 정규화 — 픽셀 좌표에서 K의 영향 제거
    std::vector<cv::Point2f> pts1_norm, pts2_norm;
    normalizePoints(K, points1, pts1_norm);
    normalizePoints(K, points2, pts2_norm);

    // Step 2: Essential Matrix 추정 — RANSAC으로 outlier 제거하며 E 계산
    cv::Mat E;
    std::vector<uchar> inliers_e;
    int inlier_count = estimateEssential(pts1_norm, pts2_norm, E, inliers_e);

    // inlier가 8개 미만이면 E 추정 불가
    //   5-point algorithm은 최소 5개 필요하지만,
    //   안정적인 추정을 위해 8개 이상을 요구
    if (inlier_count < 8)
    {
        std::cout << "❌ Inlier 부족: " << inlier_count << std::endl;
        return false;
    }

    // Step 3: R, t 복원 — E를 SVD 분해 → 4가지 해 → Cheirality check
    std::vector<uchar> inliers_r;
    int good_points = recoverPose(E, pts1_norm, pts2_norm, R, t, inliers_r);

    // Step 4: 삼각측량 — 원본 픽셀 좌표로 3D 점 생성
    //   ★ 주의: 삼각측량에는 원본 픽셀 좌표를 사용!
    //   투영 행렬 P = K · [R|t]에 K가 이미 포함되어 있으므로
    //   정규화하지 않은 원본 좌표가 필요.
    triangulate(K, R, t, points1, points2, points3d);

    return points3d.size() >= 10;
}

// 데모 — 시뮬레이션 데이터로 2D-2D 파이프라인 전체를 시연
//
// 시뮬레이션 전략 (Week 2와 동일):
//   ① 가상의 3D 점 생성
//   ② 두 카메라(정답 R, t)로 3D → 2D 투영
//   ③ 노이즈 추가하여 실제 검출 오차 모사
//   ④ pipeline()으로 R, t 추정
//   ⑤ 정답과 비교
void Motion2D2D::demo()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "2D-2D 모션 추정 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Essential Matrix E vs Fundamental Matrix F
    //
    //   E: 정규화 좌표 기준 → p₂ᵀ E p₁ = 0
    //     - K를 알고 있어야 사용 가능
    //     - 자유도 5 (회전 3 + 이동 방향 2)
    //     - 정의: E = [t]× · R
    //
    //   F: 픽셀 좌표 기준 → u₂ᵀ F u₁ = 0
    //     - K를 몰라도 사용 가능 (비캘리브레이션 카메라)
    //     - 자유도 7 (E의 5 + K의 fx, fy 비율 등)
    //     - 관계: F = K₂⁻ᵀ · E · K₁⁻¹
    //
    //   K를 알면 E가 더 정확 (제약 조건이 강함 → 노이즈에 강건)
    //   K를 모르면 F를 사용 → K를 알게 된 후 E로 변환
    std::cout << "💡 [Essential vs Fundamental]" << std::endl;
    std::cout << "   Essential (E): 정규화 좌표, K 필요, p'^T E p = 0" << std::endl;
    std::cout << "   Fundamental (F): 픽셀 좌표, K 불필요, F = K₂⁻ᵀ E K₁⁻¹" << std::endl;
    std::cout << "   💡 이 비교가 quiz_easy 문제 1!\n" << std::endl;

    // 5-Point Algorithm
    //   E의 자유도 = 5:
    //     회전 R: 자유도 3 (roll, pitch, yaw)
    //     이동 t: 자유도 2 (방향만, 크기는 스케일 모호)
    //     → t의 크기를 ||t||=1로 고정하면 방향의 자유도 = 2
    //
    //   왜 5개가 최소인가?
    //     에피폴라 구속: p₂ᵀ E p₁ = 0 → 대응점 1개 = 방정식 1개
    //     E의 미지수(자유도) = 5 → 최소 5개 방정식 = 5개 대응점
    //
    //   8-Point Algorithm과의 비교:
    //     8-Point: E를 9개 원소의 벡터로 취급 (rank-2 제약 없음)
    //     5-Point: E의 특수 구조(rank-2, 같은 singular value)를 활용
    //     → 5-Point이 더 정확하지만 계산이 복잡
    std::cout << "💡 [5-Point Algorithm]" << std::endl;
    std::cout << "   E의 자유도 = 5 (회전 3 + 이동 방향 2)" << std::endl;
    std::cout << "   → 최소 5개 대응점 필요 (8-Point는 과결정)" << std::endl;
    std::cout << "   💡 이 개념이 quiz_easy 문제 2!\n" << std::endl;

    // 카메라 파라미터 — Phase 2에서 캘리브레이션으로 구한 값
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    std::cout << "카메라 내부 파라미터 K:\n" << K << "\n" << std::endl;

    // Ground truth 포즈 — 정답 R, t (시뮬레이션이므로 미리 설정)
    //   R_gt: 약 1도의 작은 회전 (Y축 기준)
    //     연속 프레임 간 회전은 보통 1~5도 이내
    //     너무 큰 회전: 특징점 추적 실패, 매칭 오류 증가
    //   t_gt: X축으로 1m 이동 (옆으로 이동하는 모션)
    //     ⚠️ 실제로는 ||t||=1로 정규화되어 복원됨
    cv::Mat R_gt = (cv::Mat_<double>(3, 3) << 0.9998, -0.0175, 0.0000, 0.0175, 0.9998, 0.0000,
                    0.0000, 0.0000, 1.0000);  // 약 1도 회전

    cv::Mat t_gt = (cv::Mat_<double>(3, 1) << 1.0, 0.0, 0.0);

    std::cout << "Ground Truth 포즈:" << std::endl;
    std::cout << "   t = " << t_gt.t() << "\n" << std::endl;

    // 가상 3D 점 생성 — 카메라 앞 3~7m에 랜덤 배치
    //   50개 점: 실제 VO에서 200~500개 특징점 중 매칭 성공한 점
    //   X: -2 ~ +2m, Y: -1.5 ~ +1.5m, Z: 3 ~ 7m
    //   Z(깊이)가 양수인 이유: 카메라 앞에 있어야 투영 가능
    std::vector<cv::Point3f> pts3d_world;
    for (int i = 0; i < 50; i++)
    {
        pts3d_world.push_back(cv::Point3f(-2.0 + (rand() % 40) / 10.0, -1.5 + (rand() % 30) / 10.0,
                                          3.0 + (rand() % 40) / 10.0));
    }

    // 3D → 2D 투영 — 두 카메라의 투영 행렬로 2D 관측 생성
    //
    //   P₁ = K · [I | 0]: 카메라 1 (원점, 정면)
    //   P₂ = K · [R | t]: 카메라 2 (R_gt, t_gt만큼 이동/회전)
    //
    //   투영: p = P · X_homogeneous / Z
    //     X_homogeneous = [X, Y, Z, 1]ᵀ (동차 좌표)
    //     proj = P · X_homogeneous = [u·Z, v·Z, Z]ᵀ
    //     (u, v) = (proj[0]/proj[2], proj[1]/proj[2])
    std::vector<cv::Point2f> points1, points2;

    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));

    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R_gt.copyTo(RT(cv::Rect(0, 0, 3, 3)));
    t_gt.copyTo(RT(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT;

    for (const auto& pt3d : pts3d_world)
    {
        // 동차 좌표로 변환: [X, Y, Z] → [X, Y, Z, 1]
        cv::Mat pt = (cv::Mat_<double>(4, 1) << pt3d.x, pt3d.y, pt3d.z, 1.0);

        // 카메라 1에서 투영
        cv::Mat proj1 = P1 * pt;
        points1.push_back(cv::Point2f(proj1.at<double>(0) / proj1.at<double>(2),
                                      proj1.at<double>(1) / proj1.at<double>(2)));

        // 카메라 2에서 투영 (R, t 적용)
        cv::Mat proj2 = P2 * pt;
        points2.push_back(cv::Point2f(proj2.at<double>(0) / proj2.at<double>(2),
                                      proj2.at<double>(1) / proj2.at<double>(2)));
    }

    // 노이즈 추가 — 실제 특징점 검출의 측정 오차 모사
    //   ±1.0 픽셀 범위의 균일 노이즈
    //   실제 cornerSubPix는 σ ≈ 0.1~0.5 픽셀이므로 이보다 큰 노이즈
    //   → RANSAC의 outlier 제거 능력을 시험하는 효과
    for (auto& pt : points2)
    {
        pt.x += (rand() % 20 - 10) / 10.0;
        pt.y += (rand() % 20 - 10) / 10.0;
    }

    std::cout << "생성된 대응점: " << points1.size() << "개\n" << std::endl;

    // 파이프라인 실행 — 정규화 → E 추정 → R,t 복원 → 삼각측량
    cv::Mat R_est, t_est;
    std::vector<cv::Point3f> points3d_est;

    bool success = pipeline(K, points1, points2, R_est, t_est, points3d_est);

    // Cheirality Check 설명
    //   E를 SVD 분해하면 4가지 (R, t) 해가 나옴.
    //   각 해로 삼각측량 → 3D 점이 두 카메라 모두의 앞(Z > 0)인 해를 선택.
    //
    //   직관: 물체는 카메라 "앞"에 있어야 보인다.
    //     해 1: 두 카메라 앞 ✅ (유일한 정답)
    //     해 2: 카메라 1 앞, 카메라 2 뒤 ❌
    //     해 3: 카메라 1 뒤, 카메라 2 앞 ❌
    //     해 4: 두 카메라 뒤 ❌
    std::cout << "\n💡 [Cheirality Check]" << std::endl;
    std::cout << "   E 분해 → 4가지 (R,t) 해" << std::endl;
    std::cout << "   3D 점이 두 카메라 앞(depth > 0)인 해 선택" << std::endl;
    std::cout << "   💡 이 과정이 quiz_easy 문제 3!\n" << std::endl;

    if (success)
    {
        std::cout << "✅ 추정 성공!" << std::endl;
        std::cout << "   추정 t = " << t_est.t() << std::endl;
        std::cout << "   복원된 3D 점: " << points3d_est.size() << "개" << std::endl;

        // 이동 벡터 오차 — 추정값과 정답의 L2 거리
        double t_error = cv::norm(t_gt - t_est);
        std::cout << "   이동 오차: " << std::fixed << std::setprecision(4) << t_error << "\n"
                  << std::endl;

        // ★ 스케일 모호성 확인
        //   recoverPose의 t는 항상 ||t||=1로 정규화됨.
        //   정답 t_gt = [1, 0, 0]도 ||t_gt||=1이므로 여기서는 우연히 일치.
        //   만약 t_gt = [2, 0, 0]이었다면:
        //     ||t_gt|| = 2이지만 ||t_est|| = 1
        //     → 방향은 맞지만 크기가 2배 차이 → 이것이 스케일 모호성!
        std::cout << "⚠️  주의: 스케일 모호성!" << std::endl;
        std::cout << "   - GT: ||t|| = " << cv::norm(t_gt) << std::endl;
        std::cout << "   - Est: ||t|| = " << cv::norm(t_est) << std::endl;
        std::cout << "   - 정규화되어 둘 다 1.0" << std::endl;
        std::cout << "   💡 이 현상이 quiz_easy 문제 4!\n" << std::endl;
    }

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

// 메인 함수 — Week 2 데모 실행
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 2: 2D-2D 모션 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    Motion2D2D::demo();

    std::cout << "\n💡 핵심 내용:" << std::endl;
    std::cout << "   - Essential Matrix로 2D-2D 모션 추정" << std::endl;
    std::cout << "   - R, t 복원 (Cheirality check)" << std::endl;
    std::cout << "   - 삼각측량으로 초기 3D 맵" << std::endl;
    std::cout << "   - ⚠️ 스케일 모호성 존재!\n" << std::endl;

    std::cout << "💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md에서 E/F 이론 읽기 → quiz_easy 문제 1~2" << std::endl;
    std::cout << "   2. my_basic.cpp Step 1~3 구현 → quiz_easy 문제 3~4" << std::endl;
    std::cout << "   3. my_basic.cpp Step 4~5 구현 → quiz_medium 문제 1~3" << std::endl;
    std::cout << "   4. PRACTICE.md에서 추가 실습\n" << std::endl;

    std::cout << "다음: Week 3 - 3D-2D 모션 추정 (PnP)\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
