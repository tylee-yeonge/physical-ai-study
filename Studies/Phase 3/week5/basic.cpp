// Week 5: Mini VO — 완전한 Visual Odometry 파이프라인
//
// 이 파일은 Week 2(2D-2D 초기화)와 Week 3(3D-2D 추적)을 통합하여
// 완전한 VO 파이프라인을 구성하고, 합성 데이터로 전 과정을 시연한다.
//
// VO(Visual Odometry)란?
//   카메라 이미지 시퀀스만으로 카메라의 이동 궤적을 추정하는 기술.
//   GPS 없이도 "어디로 얼마나 이동했는가"를 알 수 있다.
//
// VO 파이프라인 2단계:
//   ┌─────────────────────────────────────────────────────────────┐
//   │  INITIALIZING (초기화)                                      │
//   │    입력: 첫 두 프레임의 2D-2D 대응                           │
//   │    방법: Essential Matrix → R, t 복원 → 삼각측량 → 초기 맵    │
//   │    출력: 초기 포즈(R, t) + 3D 맵 포인트                      │
//   │    조건: ||t|| = 1 (스케일 모호)                             │
//   │                                                             │
//   │  TRACKING (추적)                                            │
//   │    입력: 3D 맵 + 현재 프레임 2D 관측                         │
//   │    방법: PnP + RANSAC → 현재 포즈 추정                       │
//   │    출력: 매 프레임의 카메라 포즈                              │
//   │    장점: 3D 정보 사용 → 스케일 유지                          │
//   └─────────────────────────────────────────────────────────────┘
//
// ★ VO의 한계: 드리프트(drift) — 매 프레임 작은 오차가 누적되어 발산
//   → Week 7~9의 Bundle Adjustment로 완화, Phase 4의 Loop Closure로 해결

#include "mini_vo.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════
// 구현부
// ═══════════════════════════════════════════════════════════════

// 특징점 검출 — GFTT(Good Features To Track) 알고리즘
//
// GFTT = Shi-Tomasi 코너 검출기.
// Harris 코너의 개량판으로, 두 고유값 중 작은 값이 임계값 이상인 점을 선택.
//
// 왜 GFTT를 사용하는가?
//   1. 추적에 최적화됨 — Optical Flow와 궁합이 좋음
//   2. 균일 분포 — minDistance로 특징점이 골고루 분포
//   3. 속도 — ORB/SIFT보다 가볍고 실시간 가능
//
// qualityLevel = 0.01:
//   최대 코너 강도의 1% 이상인 점만 채택.
//   너무 낮으면 약한 코너까지 포함(노이즈), 너무 높으면 점이 부족.
//
// minDistance = 10:
//   이미 선택된 코너로부터 10픽셀 이내에는 새 코너를 배치하지 않음.
//   → 특징점이 한 곳에 몰리는 것을 방지 (텍스처가 풍부한 영역에 집중되는 문제)
std::vector<cv::Point2f> MiniVOBasic::detect_features(
    const cv::Mat& frame, int max_corners)
{
    std::vector<cv::Point2f> corners;
    cv::goodFeaturesToTrack(
        frame, corners,
        max_corners,    // 최대 코너 수 — 500개면 VO에 충분
        0.01,           // qualityLevel — 코너 품질 임계값 (최대 강도의 1%)
        10              // minDistance — 코너 간 최소 거리 (픽셀)
    );
    return corners;
}

// 특징점 추적 — Lucas-Kanade Optical Flow (피라미드 방식)
//
// Optical Flow란?
//   "이전 프레임의 점이 현재 프레임에서 어디로 이동했는가"를 추정.
//   밝기 항상성(Brightness Constancy) 가정:
//     I(x, y, t) = I(x+dx, y+dy, t+dt)
//   → 테일러 전개: Ix·u + Iy·v + It = 0  (광류 방정식)
//
// Lucas-Kanade:
//   작은 윈도우(보통 21×21) 내에서 모든 픽셀이 같은 움직임을 가진다고 가정.
//   윈도우 내 N개 픽셀에 대한 연립방정식을 최소제곱법으로 풀어 (u, v) 추정.
//
// 피라미드(PyrLK):
//   이미지를 축소(1/2, 1/4, ...)하여 큰 움직임도 추적 가능.
//   상위 레벨(저해상도)에서 대략적 움직임 추정 → 하위 레벨에서 정밀화.
//   기본 3레벨 → 최대 ~60px 움직임까지 추적.
//
// status[i] = 1이면 추적 성공, 0이면 실패 (점이 프레임 밖으로 나가거나 추적 불가)
void MiniVOBasic::track_features(
    const cv::Mat& prev, const cv::Mat& curr,
    const std::vector<cv::Point2f>& prev_pts,
    std::vector<cv::Point2f>& curr_pts,
    std::vector<uchar>& status)
{
    std::vector<float> err;
    // calcOpticalFlowPyrLK:
    //   입력: 이전/현재 이미지 + 이전 프레임 특징점
    //   출력: 현재 프레임에서의 추적된 위치 + 추적 성공 여부 + 추적 오차
    cv::calcOpticalFlowPyrLK(prev, curr, prev_pts, curr_pts, status, err);
}

// 2D-2D 초기화 — Essential Matrix로 초기 포즈 + 3D 맵 생성
//
// VO 파이프라인의 첫 단계. 아직 3D 맵이 없으므로 PnP를 사용할 수 없고,
// 2D-2D 대응만으로 상대 포즈를 추정해야 한다.
//
// 전체 흐름:
//   ① findEssentialMat: 2D 대응 → Essential Matrix E 추정 (RANSAC)
//   ② recoverPose: E → R, t 분해 (4가지 해 중 cheirality check로 선택)
//   ③ triangulatePoints: R, t + 2D 대응 → 3D 맵 포인트 생성
//
// ★ 핵심 제약:
//   - ||t|| = 1로 정규화됨 → 실제 스케일을 알 수 없음 (스케일 모호)
//   - 충분한 시차(parallax)가 필요 → 제자리에서 회전만 하면 실패
//   - inlier가 부족하면(< 20) 초기화 실패 → 다른 프레임 쌍으로 재시도
bool MiniVOBasic::initialize_2d2d(
    const std::vector<cv::Point2f>& pts1,
    const std::vector<cv::Point2f>& pts2,
    const cv::Mat& K,
    cv::Mat& R, cv::Mat& t,
    std::vector<cv::Point3f>& map_points,
    std::vector<cv::Point2f>& map_keypoints)
{
    const int kMinInliers = 20;

    // ① Essential Matrix 추정 — 에피폴라 기하학의 핵심
    //    p₂ᵀ · E · p₁ = 0  (정규화 좌표에서의 에피폴라 제약)
    //    E = [t]× · R      (E는 R, t로 분해 가능)
    //
    //    RANSAC 파라미터:
    //      confidence = 0.999 → 99.9% 확률로 올바른 모델 보장
    //      threshold = 1.0    → 에피폴라 선까지의 거리 1px 이내면 inlier
    cv::Mat inlier_mask;
    cv::Mat E = cv::findEssentialMat(pts1, pts2, K, cv::RANSAC, 0.999, 1.0, inlier_mask);

    // ② R, t 복원 — E의 SVD 분해로 4가지 (R, t) 후보 중 하나 선택
    //    cheirality check: 삼각측량된 점이 두 카메라 앞(depth > 0)에 있는 해 선택
    //    반환값 good = cheirality를 통과한 점 개수
    int good = cv::recoverPose(E, pts1, pts2, K, R, t, inlier_mask);
    if (good < kMinInliers)
    {
        return false;
    }

    // ③ 삼각측량 — 두 카메라 시점에서의 2D 관측으로 3D 점 복원
    //
    // 투영 행렬(Projection Matrix) 구성:
    //   P₁ = K · [I | 0]      — 첫 번째 카메라 = 월드 원점
    //   P₂ = K · [R | t]      — 두 번째 카메라 = 추정된 포즈
    //   여기서 P는 3×4 행렬: 3D(4D 동차) → 2D(3D 동차) 투영
    cv::Mat P1 = cv::Mat::zeros(3, 4, CV_64F);
    K.copyTo(P1(cv::Rect(0, 0, 3, 3)));    // P1 = K · [I | 0]

    cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
    R.copyTo(RT(cv::Rect(0, 0, 3, 3)));    // RT의 왼쪽 3×3 = R
    t.copyTo(RT(cv::Rect(3, 0, 1, 3)));    // RT의 오른쪽 3×1 = t
    cv::Mat P2 = K * RT;                    // P2 = K · [R | t]

    // RANSAC inlier만 삼각측량에 사용 — outlier가 포함되면 맵 품질 저하
    std::vector<cv::Point2f> inlier_pts1, inlier_pts2;
    for (size_t i = 0; i < pts1.size(); i++)
    {
        if (inlier_mask.at<uchar>(i))
        {
            inlier_pts1.push_back(pts1[i]);
            inlier_pts2.push_back(pts2[i]);
        }
    }

    // triangulatePoints: DLT(Direct Linear Transform) 방식
    //   각 점 대응에서 선형 방정식 구성 → SVD로 풀이
    //   출력: 4×N 동차 좌표 (4행: X, Y, Z, W)
    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, inlier_pts1, inlier_pts2, points4D);

    // 3D 점 추출 — 동차 좌표 → 유클리드 좌표 변환 + 유효성 검사
    map_points.clear();
    map_keypoints.clear();
    for (int i = 0; i < points4D.cols; i++)
    {
        // 동차 좌표 (X, Y, Z, W) → 유클리드 (X/W, Y/W, Z/W)
        float w = points4D.at<float>(3, i);
        if (std::abs(w) < 1e-6)    // w ≈ 0이면 무한원점 (삼각측량 실패)
            continue;
        float x = points4D.at<float>(0, i) / w;
        float y = points4D.at<float>(1, i) / w;
        float z = points4D.at<float>(2, i) / w;

        // 깊이 유효 범위 체크:
        //   z > 0.1m — 카메라 앞에 있어야 함 (cheirality)
        //   z < 200m — 너무 먼 점은 삼각측량 정확도가 낮음
        //   ★ 시차가 작을수록 먼 점의 depth 오차가 기하급수적으로 증가
        if (z > 0.1 && z < 200.0)
        {
            map_points.push_back(cv::Point3f(x, y, z));
            // 프레임2의 2D 점을 저장 → 다음 프레임에서 추적의 출발점
            map_keypoints.push_back(inlier_pts2[i]);
        }
    }

    // 3D 맵 최소 10개 이상 생성되어야 PnP 추적이 가능
    return map_points.size() >= 10;
}

// 3D-2D 추적 — PnP + RANSAC로 현재 카메라 포즈 추정
//
// 초기화 이후의 모든 프레임에서 사용하는 추적 방법.
// 3D 맵 포인트와 현재 프레임의 2D 관측 대응으로 6DoF 포즈를 추정.
//
// PnP(Perspective-n-Point):
//   "n개의 3D↔2D 대응으로 카메라 포즈를 역산"
//   최소 4개 대응 필요 (6DoF: 3 회전 + 3 이동, 점당 2개 방정식)
//
// ★ 2D-2D(Essential)와의 핵심 차이:
//   - 2D-2D: ||t|| = 1 (스케일 모호) — 초기화용
//   - 3D-2D: 절대 스케일 유지 (3D 맵이 스케일 정보를 담고 있음) — 추적용
int MiniVOBasic::track_pnp(
    const std::vector<cv::Point3f>& pts3d,
    const std::vector<cv::Point2f>& pts2d,
    const cv::Mat& K,
    cv::Mat& R, cv::Mat& t,
    std::vector<int>& inliers)
{
    if (pts3d.size() < 4)   // PnP 최소 요구: 4개 대응
        return 0;

    cv::Mat rvec, tvec;
    // solvePnPRansac:
    //   iterationsCount = 100 — RANSAC 반복 횟수
    //   reprojectionError = 8.0 — inlier 판정 임계값 (픽셀)
    //     → 재투영 오차 8px 이내면 inlier
    //     → 값이 작을수록 엄격 (1~3px: 정밀, 5~10px: 관대)
    //   confidence = 0.99 — 성공 확률
    //   inliers — outlier 제외된 인덱스 목록
    bool ok = cv::solvePnPRansac(
        pts3d, pts2d, K, cv::noArray(),
        rvec, tvec,
        false,  // useExtrinsicGuess — 초기값 없이 새로 추정
        100,    // RANSAC 반복 횟수
        8.0,    // 재투영 오차 임계값 (px)
        0.99,   // 신뢰도
        inliers);

    if (!ok || inliers.empty())
        return 0;

    // Rodrigues: 회전 벡터(3×1) → 회전 행렬(3×3)
    //   rvec = θ·n̂ (축-각 표현) → R = exp([rvec]×)
    cv::Rodrigues(rvec, R);
    t = tvec.clone();
    return static_cast<int>(inliers.size());
}

// 새 3D 점 삼각측량 — 두 카메라 시점에서 추가 맵 포인트 생성
//
// 추적 중 맵 포인트가 점점 줄어들므로(프레임 밖으로 나가거나 추적 실패),
// 주기적으로 새 3D 점을 생성하여 맵을 보충해야 한다.
//
// 삼각측량 정확도에 영향을 주는 요인:
//   1. 시차(baseline): 클수록 정확 — 근접 촬영보다 이동 후 촬영이 유리
//   2. 깊이(depth): 가까울수록 정확 — depth ∝ (1/parallax)
//   3. 노이즈: 2D 검출 오차가 3D 오차로 전파
//
// ★ depth 오차 ∝ depth² / (baseline × fx)
//   → 먼 점일수록 오차가 급격히 증가 (역수 관계가 아닌 제곱!)
std::vector<cv::Point3f> MiniVOBasic::triangulate_new_points(
    const std::vector<cv::Point2f>& pts1,
    const std::vector<cv::Point2f>& pts2,
    const cv::Mat& K,
    const cv::Mat& R1, const cv::Mat& t1,
    const cv::Mat& R2, const cv::Mat& t2)
{
    // 투영 행렬 P = K · [R | t]
    //   P₁: 프레임1의 카메라 포즈
    //   P₂: 프레임2의 카메라 포즈
    //   두 P로부터 삼각측량하여 3D 점 복원
    cv::Mat RT1 = cv::Mat::zeros(3, 4, CV_64F);
    R1.copyTo(RT1(cv::Rect(0, 0, 3, 3)));
    t1.copyTo(RT1(cv::Rect(3, 0, 1, 3)));
    cv::Mat P1 = K * RT1;

    cv::Mat RT2 = cv::Mat::zeros(3, 4, CV_64F);
    R2.copyTo(RT2(cv::Rect(0, 0, 3, 3)));
    t2.copyTo(RT2(cv::Rect(3, 0, 1, 3)));
    cv::Mat P2 = K * RT2;

    cv::Mat points4D;
    cv::triangulatePoints(P1, P2, pts1, pts2, points4D);

    // 3D 점 추출 + 유효성 검사 (동차→유클리드 + depth 체크)
    std::vector<cv::Point3f> result;
    for (int i = 0; i < points4D.cols; i++)
    {
        float w = points4D.at<float>(3, i);
        if (std::abs(w) < 1e-6)
            continue;
        float x = points4D.at<float>(0, i) / w;
        float y = points4D.at<float>(1, i) / w;
        float z = points4D.at<float>(2, i) / w;
        if (z > 0.1 && z < 200.0)
        {
            result.push_back(cv::Point3f(x, y, z));
        }
    }
    return result;
}

// 궤적 오차 계산 — ATE RMSE (Absolute Trajectory Error)
//
// ATE = 추정 궤적과 정답 궤적 간의 절대 위치 오차.
// SLAM 성능 평가의 표준 지표 (evo 라이브러리에서 자주 사용).
//
// 공식: ATE_RMSE = sqrt( (1/N) · Σ ||t_est[i] - t_gt[i]||² )
//
// 해석:
//   ATE < 0.1m: 매우 정확 (실내 환경)
//   ATE < 1.0m: 양호 (실외 환경)
//   ATE > 5.0m: 심각한 드리프트
//
// 💡 ATE는 정렬(alignment) 후 계산해야 함
//   실제 평가에서는 Sim(3) 정렬 (스케일 + 회전 + 이동)을 먼저 수행.
//   이 데모에서는 합성 데이터이므로 이미 정렬된 상태.
double MiniVOBasic::compute_trajectory_error(
    const std::vector<cv::Mat>& est_t,
    const std::vector<cv::Mat>& gt_t)
{
    int n = std::min(est_t.size(), gt_t.size());
    if (n == 0)
        return -1.0;

    double sum_sq = 0.0;
    for (int i = 0; i < n; i++)
    {
        // ||est - gt||² = L2 거리의 제곱
        sum_sq += cv::norm(est_t[i] - gt_t[i], cv::NORM_L2SQR);
    }
    return std::sqrt(sum_sq / n);  // RMSE
}

// 합성 이미지 시퀀스 생성 — 전진 궤적 + 3D 점 투영
//
// 실제 카메라 없이 VO를 테스트하기 위한 시뮬레이터.
//
// 시뮬레이션 전략:
//   ① 500개 3D 월드 점을 카메라 전방에 랜덤 배치
//   ② 카메라가 +Z 방향으로 전진 (KITTI 자율주행 스타일)
//   ③ 각 프레임에서 3D→2D 투영하여 가상 이미지 생성
//   ④ 가우시안 노이즈로 실제 센서 노이즈 모사
//
// 궤적 모델:
//   직진 + 서서히 좌회전 (yaw 증가)
//   프레임당 0.5m 전진, 0.3° 회전 → 100 프레임에서 약 50m 이동
//
// ★ 합성 데이터의 장점:
//   정답(GT)을 정확히 알고 있어 알고리즘 성능을 정량적으로 평가 가능
void MiniVOBasic::generate_synthetic_sequence(
    const cv::Mat& K, int num_frames,
    std::vector<cv::Mat>& images,
    std::vector<cv::Mat>& gt_R,
    std::vector<cv::Mat>& gt_t)
{
    double fx = K.at<double>(0, 0);
    double fy = K.at<double>(1, 1);
    double cx = K.at<double>(0, 2);
    double cy = K.at<double>(1, 2);
    int width = static_cast<int>(cx * 2);   // cx가 이미지 중심이므로 width = 2*cx
    int height = static_cast<int>(cy * 2);

    // 3D 월드 점: 카메라 전방에 분포
    // 카메라가 +Z 방향으로 전진하므로 점들은 Z > 0 영역에 배치
    //   X: [-10, 10]  — 좌우 범위
    //   Y: [-5, 5]    — 상하 범위 (좁음 — 도로 환경 모사)
    //   Z: [3, 53]    — 전방 거리 (가까운 점~먼 점)
    std::vector<cv::Point3f> world_pts;
    srand(42);  // 재현성을 위한 고정 시드
    for (int i = 0; i < 500; i++)
    {
        float x = (rand() % 200 - 100) / 10.0f;   // [-10, 10]
        float y = (rand() % 100 - 50) / 10.0f;     // [-5, 5]
        float z = 3.0f + (rand() % 500) / 10.0f;   // [3, 53] — 전방 넓은 범위
        world_pts.push_back(cv::Point3f(x, y, z));
    }

    // 전진 궤적 생성 (KITTI 스타일: +Z 방향 이동 + 약간의 회전)
    for (int i = 0; i < num_frames; i++)
    {
        double progress = i * 0.5;  // 프레임당 0.5m 전진
        double yaw = i * 0.005;     // 프레임당 약 0.3° 회전 (서서히 곡선)

        // 카메라 위치 — 원형 곡선 궤적
        //   x = progress · sin(yaw) → 옆으로 이동 (회전에 의해)
        //   z = progress · cos(yaw) → 앞으로 이동
        cv::Mat t = (cv::Mat_<double>(3, 1) <<
            progress * std::sin(yaw), 0.0, progress * std::cos(yaw));

        // Y축 회전 행렬 (yaw)
        //   [cos θ   0  sin θ]
        //   [  0      1    0  ]
        //   [-sin θ  0  cos θ]
        cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
        R.at<double>(0, 0) = std::cos(yaw);
        R.at<double>(0, 2) = std::sin(yaw);
        R.at<double>(2, 0) = -std::sin(yaw);
        R.at<double>(2, 2) = std::cos(yaw);

        gt_R.push_back(R.clone());
        gt_t.push_back(t.clone());

        // 가상 이미지 생성 — 3D 점을 카메라 좌표계로 변환 후 2D 투영
        //   Pc = R · (Pw - t)  — 월드 점을 카메라 좌표로 변환
        //   u = fx · Pc.x / Pc.z + cx  — 정규화 좌표 → 픽셀 좌표
        //   v = fy · Pc.y / Pc.z + cy
        cv::Mat image = cv::Mat::zeros(height, width, CV_8UC1);
        for (const auto& pt : world_pts)
        {
            cv::Mat Pw = (cv::Mat_<double>(3, 1) << pt.x, pt.y, pt.z);
            cv::Mat Pc = R * (Pw - t);

            double Zc = Pc.at<double>(2);
            if (Zc > 0.5)  // 카메라 앞에 있는 점만 투영
            {
                int u = static_cast<int>(fx * Pc.at<double>(0) / Zc + cx);
                int v = static_cast<int>(fy * Pc.at<double>(1) / Zc + cy);
                if (u >= 4 && u < width - 4 && v >= 4 && v < height - 4)
                {
                    // 반경 4의 원으로 렌더링 — Optical Flow가 추적할 수 있는 텍스처 생성
                    cv::circle(image, cv::Point(u, v), 4,
                               cv::Scalar(160 + rand() % 95), -1);
                }
            }
        }

        // 가우시안 노이즈 추가 — 실제 카메라 센서 노이즈 모사
        //   σ = 2 — 매우 약한 노이즈 (센서 노이즈 수준)
        cv::Mat noise(image.size(), CV_8UC1);
        cv::randn(noise, 0, 2);
        image += noise;
        images.push_back(image);
    }
}

// ═══════════════════════════════════════════════════════════════
// 교육 블록
// ═══════════════════════════════════════════════════════════════

// ── 블록 1: VO 파이프라인 구조 ──
//
// VO의 두 핵심 단계를 합성 데이터로 시연:
//   INITIALIZING (2D-2D): Essential Matrix → 초기 맵
//   TRACKING (3D-2D): PnP → 프레임별 포즈 추정
//
// 💡 quiz_easy Q1: VO 파이프라인의 2단계는?
// 💡 quiz_easy Q2: 초기화에서 왜 Essential Matrix를 사용하는가?
static void demoVOPipeline()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 1] VO 파이프라인 = 초기화 + 추적" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // VO 상태 전이 다이어그램
    //   INITIALIZING: 3D 맵이 없는 상태 → 2D-2D로 초기 맵 생성
    //   TRACKING: 3D 맵이 있는 상태 → PnP로 포즈 추정
    //   LOST: 추적 실패 → 재초기화 필요 (inlier 비율이 너무 낮을 때)
    std::cout << "VO 시스템 상태 전이:" << std::endl;
    std::cout << "  INITIALIZING ──[2D-2D 성공]──→ TRACKING" << std::endl;
    std::cout << "  TRACKING ──[inlier < 30%]──→ LOST" << std::endl;
    std::cout << "  LOST ──[재초기화]──→ INITIALIZING\n" << std::endl;

    // 초기화 vs 추적의 핵심 차이
    //   초기화(Week 2): 2D-2D만 사용 → 스케일 모호 (||t||=1)
    //   추적(Week 3):   3D-2D 사용 → 스케일 유지 (맵이 절대 스케일을 담고 있음)
    std::cout << "초기화 (Week 2): Essential Matrix" << std::endl;
    std::cout << "  입력: 2D-2D 대응 (첫 두 프레임)" << std::endl;
    std::cout << "  출력: R, t (||t||=1) + 초기 3D 맵" << std::endl;
    std::cout << "  조건: 시차 > 1.5 px, inlier > 50%\n" << std::endl;

    std::cout << "추적 (Week 3): PnP + RANSAC" << std::endl;
    std::cout << "  입력: 3D 맵 + 현재 2D 관측" << std::endl;
    std::cout << "  출력: 현재 포즈 (R, t)" << std::endl;
    std::cout << "  장점: 스케일 유지 (3D 정보 사용)\n" << std::endl;

    // ── 수치 예시: 합성 데이터로 초기화 실행 ──
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);
    std::vector<cv::Mat> images, gt_R, gt_t;
    MiniVOBasic::generate_synthetic_sequence(K, 5, images, gt_R, gt_t);

    // 프레임 0, 1로 초기화: 특징점 검출 → 추적 → 2D-2D 초기화
    auto kp0 = MiniVOBasic::detect_features(images[0]);
    std::vector<cv::Point2f> kp1;
    std::vector<uchar> status;
    MiniVOBasic::track_features(images[0], images[1], kp0, kp1, status);

    // 추적 성공한 매칭만 추출 (status[i]==1인 점만)
    std::vector<cv::Point2f> matched0, matched1;
    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i])
        {
            matched0.push_back(kp0[i]);
            matched1.push_back(kp1[i]);
        }
    }

    cv::Mat R, t;
    std::vector<cv::Point3f> map_pts;
    std::vector<cv::Point2f> map_kp;
    bool ok = MiniVOBasic::initialize_2d2d(matched0, matched1, K, R, t, map_pts, map_kp);

    // 결과 해석:
    //   특징점 → 매칭 → 맵: 각 단계에서 점이 줄어드는 것이 정상
    //   ||t|| = 1.0: 스케일이 정규화됨 (실제 이동 거리를 모름)
    std::cout << "초기화 결과:" << std::endl;
    std::cout << "  특징점: " << kp0.size() << " → 매칭: " << matched0.size()
              << " → 맵: " << map_pts.size() << "개" << std::endl;
    std::cout << "  ||t|| = " << cv::norm(t) << " (정규화됨 = 스케일 모호)"
              << std::endl;
    std::cout << "  초기화 " << (ok ? "성공" : "실패") << std::endl;
}

// ── 블록 2: 추적과 드리프트 ──
//
// 초기화 후 PnP로 매 프레임을 추적하면서 드리프트가 누적되는 과정을 관찰.
//
// 드리프트(Drift)란?
//   매 프레임 추정에서 발생하는 작은 오차가 누적되어
//   시간이 지날수록 추정 궤적이 실제 궤적에서 점점 벗어나는 현상.
//   공식: error(t) ≈ error_per_frame × √t  (랜덤 워크 모델)
//
// 💡 quiz_easy Q3: 드리프트의 원인 3가지
// 💡 quiz_medium Q1: inlier ratio로 추적 건강 상태 판단
static void demoDriftAccumulation()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 2] 추적 + 드리프트 누적" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 드리프트 발생 메커니즘:
    //   1. 매 프레임 특징점 추적 오차 (서브픽셀 오차)
    //   2. PnP 추정 오차 (노이즈, outlier 영향)
    //   3. 삼각측량 오차 (특히 먼 점에서 큼)
    //   → 이 오차들이 매 프레임 누적 → 궤적이 점점 틀어짐
    std::cout << "드리프트 원인:" << std::endl;
    std::cout << "  매 프레임 작은 오차 → 누적 → 발산" << std::endl;
    std::cout << "  예: 프레임당 0.1% 오차 × 1000 프레임 = ~100% 오차\n" << std::endl;

    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);
    const int kNumFrames = 30;
    std::vector<cv::Mat> images, gt_R, gt_t;
    MiniVOBasic::generate_synthetic_sequence(K, kNumFrames, images, gt_R, gt_t);

    // ── 초기화 ──
    auto kp0 = MiniVOBasic::detect_features(images[0]);
    std::vector<cv::Point2f> kp1;
    std::vector<uchar> status;
    MiniVOBasic::track_features(images[0], images[1], kp0, kp1, status);

    std::vector<cv::Point2f> matched0, matched1;
    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i])
        {
            matched0.push_back(kp0[i]);
            matched1.push_back(kp1[i]);
        }
    }

    cv::Mat R, t;
    std::vector<cv::Point3f> map_pts;
    std::vector<cv::Point2f> map_kp;
    bool init_ok = MiniVOBasic::initialize_2d2d(matched0, matched1, K, R, t, map_pts, map_kp);
    if (!init_ok || map_pts.empty())
    {
        std::cout << "  초기화 실패 — 합성 데이터 재생성 필요" << std::endl;
        return;
    }

    // ── 추적 루프 ──
    // est_t_list: 추정된 궤적 (GT와 비교할 이동 벡터 시퀀스)
    std::vector<cv::Mat> est_t_list;
    est_t_list.push_back(cv::Mat::zeros(3, 1, CV_64F));  // 프레임 0: 원점
    est_t_list.push_back(t.clone());                       // 프레임 1: 초기화 결과

    // map_kp: map_pts에 대응하는 2D 키포인트 (인덱스 일치)
    // → Optical Flow 추적의 출발점으로 사용
    std::vector<cv::Point2f> prev_2d = map_kp;
    std::vector<cv::Point3f> curr_map = map_pts;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "프레임별 추적:" << std::endl;

    for (int f = 2; f < kNumFrames; f++)
    {
        // ① Optical Flow로 이전 프레임의 2D 점을 현재 프레임에서 추적
        std::vector<cv::Point2f> curr_kp;
        std::vector<uchar> flow_status;
        MiniVOBasic::track_features(images[f - 1], images[f], prev_2d, curr_kp, flow_status);

        // ② 유효한 대응만 추출 (추적 성공 + 인덱스 범위 내)
        std::vector<cv::Point3f> valid_3d;
        std::vector<cv::Point2f> valid_2d;
        for (size_t i = 0; i < flow_status.size(); i++)
        {
            if (flow_status[i] && i < curr_map.size())
            {
                valid_3d.push_back(curr_map[i]);
                valid_2d.push_back(curr_kp[i]);
            }
        }

        // ③ PnP로 포즈 추정 (3D↔2D 대응 → R, t)
        cv::Mat R_new, t_new;
        std::vector<int> inliers;
        int n_inlier = MiniVOBasic::track_pnp(valid_3d, valid_2d, K, R_new, t_new, inliers);

        if (n_inlier > 0)
        {
            est_t_list.push_back(t_new.clone());

            // 드리프트 측정: 추정 위치와 GT 위치의 유클리드 거리
            double drift = cv::norm(t_new - gt_t[f]);
            // inlier ratio = 추적 건강 상태 지표
            //   > 70%: 양호, 30~70%: 주의, < 30%: 위험 (LOST 직전)
            double ratio = (double)n_inlier / valid_3d.size() * 100;

            if (f % 5 == 0 || f == kNumFrames - 1)
            {
                std::cout << "  Frame " << std::setw(2) << f
                          << ": inlier=" << n_inlier
                          << "/" << valid_3d.size()
                          << " (" << std::setw(4) << ratio << "%)"
                          << "  drift=" << drift << "m" << std::endl;
            }

            // ④ 다음 프레임용 업데이트 — inlier만 유지
            //    outlier를 제거하면 다음 프레임 추적 품질 향상
            curr_map.clear();
            prev_2d.clear();
            for (int idx : inliers)
            {
                curr_map.push_back(valid_3d[idx]);
                prev_2d.push_back(valid_2d[idx]);
            }
        }
        else
        {
            std::cout << "  Frame " << f << ": 추적 실패 (LOST)" << std::endl;
            break;
        }
    }

    // ── ATE (Absolute Trajectory Error) 계산 ──
    //   추정 궤적과 GT 궤적의 RMSE
    //   드리프트 비율 = ATE / 총 이동 거리 × 100 (%)
    //   VO 일반적 기준: 1~5% 드리프트
    std::vector<cv::Mat> gt_subset(gt_t.begin(), gt_t.begin() + est_t_list.size());
    double ate = MiniVOBasic::compute_trajectory_error(est_t_list, gt_subset);
    double total_dist = cv::norm(gt_t.back());
    std::cout << "\n  ATE RMSE: " << ate << " m" << std::endl;
    if (total_dist > 0)
    {
        std::cout << "  드리프트 비율: " << (ate / total_dist * 100) << "%" << std::endl;
    }
}

// ── 블록 3: 삼각측량과 맵 관리 ──
//
// 시차(baseline)에 따른 삼각측량 정확도를 수치로 확인.
//
// ★ 핵심 직관:
//   시차 = 두 카메라 사이의 거리 (baseline)
//   시차가 클수록 → 두 시선의 교차각이 커져 → depth 추정이 정확
//   시차가 작으면 → 시선이 거의 평행 → depth가 불확정
//
// 삼각측량 depth 오차 공식:
//   σ_Z ≈ Z² · σ_pixel / (baseline · fx)
//   → depth(Z)가 2배 → 오차 4배! (제곱 관계)
//   → baseline이 2배 → 오차 1/2 (선형 감소)
//
// 💡 quiz_easy Q4: 맵 관리 전략
// 💡 quiz_medium Q2: 삼각측량 품질 조건
static void demoTriangulationAndMap()
{
    std::cout << "\n══════════════════════════════════════════" << std::endl;
    std::cout << " [블록 3] 삼각측량 + 맵 관리" << std::endl;
    std::cout << "══════════════════════════════════════════\n" << std::endl;

    // 삼각측량 품질 조건 3가지:
    //   1. 충분한 시차 — parallax > 1.5px (너무 작으면 depth 불확정)
    //   2. Depth > 0 — 카메라 앞에 있어야 함 (cheirality check)
    //   3. 재투영 오차 < 3px — 삼각측량 결과가 관측과 일치
    std::cout << "삼각측량 품질 조건:" << std::endl;
    std::cout << "  1. 충분한 시차 (> 1.5 px)" << std::endl;
    std::cout << "  2. Depth > 0 (카메라 앞)" << std::endl;
    std::cout << "  3. 재투영 오차 < 3 px\n" << std::endl;

    // 맵 관리 전략:
    //   추가: Keyframe에서만 새 점 생성 (매 프레임 생성하면 중복 폭증)
    //   제거: 오래되거나 관측 부족한 점 제거 (메모리/속도 유지)
    //   목표: 맵 크기를 적절하게 유지 (500~1000점 수준)
    std::cout << "맵 관리 전략:" << std::endl;
    std::cout << "  추가: Keyframe에서만 새 점 생성" << std::endl;
    std::cout << "  제거: 나이 > 30 프레임, 관측 부족" << std::endl;
    std::cout << "  목표: 적절한 맵 크기 유지 (500-1000점)\n" << std::endl;

    // ── 수치 시연: 시차에 따른 삼각측량 정확도 ──
    // GT depth = 5m인 점을 baseline을 바꿔가며 삼각측량하여 depth 오차 관찰
    cv::Mat K = (cv::Mat_<double>(3, 3) << 500, 0, 320, 0, 500, 240, 0, 0, 1);

    std::cout << "시차 vs 삼각측량 정확도 (GT depth=5m):" << std::endl;

    cv::Mat R1 = cv::Mat::eye(3, 3, CV_64F);           // 카메라1: 원점
    cv::Mat t1 = cv::Mat::zeros(3, 1, CV_64F);

    // baseline = 카메라 간 X축 이동 거리
    //   0.01m = 거의 제자리 → 시차 극소 → 오차 매우 큼
    //   1.0m = 충분한 이동 → 시차 크 → 오차 작음
    double baselines[] = {0.01, 0.05, 0.1, 0.5, 1.0};
    for (double b : baselines)
    {
        cv::Mat R2 = cv::Mat::eye(3, 3, CV_64F);
        cv::Mat t2 = (cv::Mat_<double>(3, 1) << b, 0, 0);  // X축으로 b만큼 이동

        // GT 3D 점: (0, 0, 5) — 이미지 중심에서 5m 전방
        double gt_z = 5.0;
        cv::Point2f p1(320, 240);  // 카메라1에서의 관측 = 이미지 중심
        // 카메라2에서의 관측: parallax = fx × baseline / depth
        cv::Point2f p2(static_cast<float>(320 + 500 * b / gt_z), 240);

        double parallax = std::abs(p2.x - p1.x);  // 시차 (px)

        std::vector<cv::Point2f> pts1 = {p1};
        std::vector<cv::Point2f> pts2 = {p2};
        auto result = MiniVOBasic::triangulate_new_points(pts1, pts2, K, R1, t1, R2, t2);

        if (!result.empty())
        {
            double err = std::abs(result[0].z - gt_z);
            // parallax ↑ → depth_err ↓ 관계를 관찰
            std::cout << "  baseline=" << std::setw(4) << b
                      << "m  parallax=" << std::setw(5) << parallax
                      << "px  depth_err=" << err << "m" << std::endl;
        }
        else
        {
            std::cout << "  baseline=" << b << "m  삼각측량 실패" << std::endl;
        }
    }
}

// ═══════════════════════════════════════════════════════════════
// 전체 데모 + main
// ═══════════════════════════════════════════════════════════════

// 전체 Week 5 데모 실행
//   블록 1: VO 파이프라인 구조 (초기화 + 추적)
//   블록 2: 추적과 드리프트 누적 (ATE 측정)
//   블록 3: 삼각측량과 맵 관리 (baseline vs depth 오차)
void MiniVOBasic::demo()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Week 5: Mini VO — 완전한 VO 파이프라인" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    std::cout << "\nWeek 2 (초기화) + Week 3 (추적) = 완전한 VO\n" << std::endl;

    demoVOPipeline();
    demoDriftAccumulation();
    demoTriangulationAndMap();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  정리" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // VO의 본질적 한계 3가지:
    //   1. 드리프트: 오차 누적 → 궤적이 점점 틀어짐 (BA로 완화)
    //   2. 스케일 모호: 단안 카메라에서 ||t||=1 (스테레오/IMU로 해결)
    //   3. Loop Closure 없음: 같은 장소 재방문을 감지하지 못함 (SLAM으로 해결)
    std::cout << "VO 한계:" << std::endl;
    std::cout << "  1. 드리프트 불가피 (오차 누적)" << std::endl;
    std::cout << "  2. 스케일 모호 (||t||=1)" << std::endl;
    std::cout << "  3. Loop Closure 없음\n" << std::endl;

    std::cout << "해결 방향:" << std::endl;
    std::cout << "  Week 6: Keyframe 관리" << std::endl;
    std::cout << "  Week 7-9: BA로 드리프트 감소" << std::endl;
    std::cout << "  Week 12-13: 스케일 복구" << std::endl;

    std::cout << "\n다음 단계:" << std::endl;
    std::cout << "  1. ./my_basic 으로 직접 구현 연습" << std::endl;
    std::cout << "  2. quiz_easy / quiz_medium 풀기" << std::endl;
}

int main()
{
    MiniVOBasic::demo();
    return 0;
}
