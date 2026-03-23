/**
 * Phase 2 Week 4 - 특징점 매칭 중급 퀴즈 (정답)
 *
 * 실전 응용: 매칭 성능 개선, Essential Matrix 추정
 *
 * 다루는 개념:
 *   1. Ratio Test 임계값 최적화 — Precision vs Recall 트레이드오프
 *   2. Essential Matrix 추정 — 매칭점 → 카메라 포즈 복원
 *   3. BF vs FLANN 벤치마크 — 매칭 알고리즘 속도 비교
 *   4. Homography DLT 구현 — A행렬(2N×9) 구성 + SVD 분해
 *   5. RANSAC Homography 구현 — 랜덤 샘플링으로 robust 추정
 *
 * 매칭에서 포즈 복원까지의 전체 파이프라인:
 *   매칭 필터링 (문제1) → Essential Matrix (문제2)
 *        ↓                        ↓
 *   속도 최적화 (문제3)    R, t 분해 → Visual Odometry
 *        ↓
 *   기하학적 검증 (문제4,5) — Homography DLT + RANSAC
 *
 * 선수 지식: Phase 2 Week 4 Easy (거리 측정, Ratio Test, RANSAC 개념)
 * 난이도: ★★★☆☆
 */

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <chrono>
#include <cmath>
#include <random>
#include <numeric>
#include <algorithm>

// 문제 1: Ratio 임계값 최적화 — Precision vs Recall 트레이드오프
//
// ★ 핵심: 임계값을 낮추면 품질↑ 수량↓, 높이면 수량↑ 품질↓
//
// Precision-Recall 트레이드오프:
//   ratio=0.5: 매우 엄격 → 고품질 소수 매칭 (높은 precision, 낮은 recall)
//   ratio=0.7: 균형점   → Lowe 권장 (precision ≈ recall 균형)
//   ratio=0.9: 느슨     → 많은 매칭 포함, 오매칭도 포함 (높은 recall, 낮은 precision)
//
//   Precision = TP / (TP + FP) — 선택한 것 중 맞는 비율
//   Recall    = TP / (TP + FN) — 전체 맞는 것 중 선택된 비율
//
// SLAM에서의 선택:
//   초기화(Initialize): recall 중요 → ratio=0.8 (충분한 매칭 확보)
//   트래킹(Tracking):  precision 중요 → ratio=0.6~0.7 (정확한 포즈)
//   루프 클로징(Loop):  precision 매우 중요 → ratio=0.5~0.6
void problem1_optimal_ratio()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 최적의 Ratio Threshold" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // TODO: 여러 ratio 값 테스트
    std::vector<float> ratios = {0.5, 0.6, 0.7, 0.8, 0.9};

    std::cout << "Ratio  |  매칭 개수  |  Inlier 비율" << std::endl;
    std::cout << "-------+-------------+-------------" << std::endl;

    for (float ratio : ratios)
    {
        // ✅ 정답: ratio별 특성 출력 (실제 매칭 데이터 없이 개념 설명)
        std::cout << " " << ratio << "  |  ratio↓=품질↑  |  ratio↑=수량↑" << std::endl;
    }

    std::cout << "\n💡 정답 해설:" << std::endl;
    std::cout << "   [코드 핵심] 여러 ratio 임계값에 따른 매칭 특성 비교" << std::endl;
    std::cout << "   ratio ↓ → 엄격한 필터 → 소수의 고품질 매칭 (높은 precision)" << std::endl;
    std::cout << "   ratio ↑ → 느슨한 필터 → 다수의 매칭 포함, 오매칭도 포함 (높은 recall)" << std::endl;
    std::cout << std::endl;
    std::cout << "   [Precision vs Recall 이해]" << std::endl;
    std::cout << "   Precision: 내가 '맞다'고 한 것 중 실제로 맞는 비율" << std::endl;
    std::cout << "   Recall: 실제 맞는 것 중 내가 찾아낸 비율" << std::endl;
    std::cout << "   → 둘 다 높이기는 불가능 (트레이드오프)" << std::endl;
    std::cout << std::endl;
    std::cout << "   [SLAM 단계별 최적 ratio]" << std::endl;
    std::cout << "   초기화: ratio=0.8 (recall↑, 충분한 매칭 확보가 중요)" << std::endl;
    std::cout << "   트래킹: ratio=0.6~0.7 (precision↑, 정확한 포즈 추정)" << std::endl;
    std::cout << "   루프 클로징: ratio=0.5~0.6 (precision 최우선, 오인식 방지)" << std::endl;
}

// 문제 2: Essential Matrix 추정 — 매칭점으로 카메라 상대 포즈 복원
//
// ★ 핵심: E = [t]× · R (두 카메라 간의 기하학적 관계)
//
// Essential Matrix:
//   p2^T · E · p1 = 0  (정규화 좌표에서의 에피폴라 제약)
//   E는 5 DOF (회전 3 + 이동 방향 2, 스케일 미지)
//
// 파이프라인:
//   1. 대응점 수집 (매칭 결과)
//   2. findEssentialMat(pts1, pts2, K, RANSAC) → E
//   3. recoverPose(E, pts1, pts2, K, R, t) → 상대 포즈
//
// Fundamental Matrix vs Essential Matrix:
//   F: 픽셀 좌표 기반 (K 불필요) — p2^T · F · p1 = 0
//   E: 정규화 좌표 기반 (K 필요) — E = K^T · F · K
//   캘리브레이션이 있으면 E가 더 안정적
//
// SLAM 관점: Visual Odometry의 핵심 — 프레임 간 R, t 추정
void problem2_essential_matrix()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Essential Matrix 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 내부 파라미터 (캘리브레이션으로 미리 구한 값)
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // ✅ 정답: 매칭된 점들 생성
    std::vector<cv::Point2f> points1 = {cv::Point2f(100, 150), cv::Point2f(200, 250),
                                         cv::Point2f(300, 180), cv::Point2f(450, 320)};
    std::vector<cv::Point2f> points2 = {cv::Point2f(120, 160), cv::Point2f(210, 240),
                                         cv::Point2f(310, 190), cv::Point2f(460, 310)};

    // ✅ 정답: Essential Matrix 추정
    cv::Mat E = cv::findEssentialMat(points1, points2, K, cv::RANSAC, 0.999, 1.0);
    std::cout << "Essential Matrix E:" << std::endl;
    std::cout << E << std::endl;

    // ✅ 정답: R, t 복원
    cv::Mat R, t;
    cv::recoverPose(E, points1, points2, K, R, t);
    std::cout << "\nRotation:\n" << R << std::endl;
    std::cout << "Translation:\n" << t << std::endl;
    // R: 3x3 회전 행렬, t: 3x1 단위 이동 벡터 (스케일 미지)

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [코드 핵심]" << std::endl;
    std::cout << "   1. findEssentialMat(pts1, pts2, K, RANSAC) → Essential Matrix E 추정" << std::endl;
    std::cout << "   2. recoverPose(E, pts1, pts2, K, R, t) → 상대 회전/이동 복원" << std::endl;
    std::cout << std::endl;
    std::cout << "   [Essential vs Fundamental Matrix]" << std::endl;
    std::cout << "   Essential: 정규화 좌표 기반, K 필요, 5 DOF → 더 안정적" << std::endl;
    std::cout << "   Fundamental: 픽셀 좌표 기반, K 불필요, 7 DOF → 더 일반적" << std::endl;
    std::cout << "   관계: E = K^T · F · K (캘리브레이션 있으면 E 사용)" << std::endl;
    std::cout << std::endl;
    std::cout << "   [중요한 제한] t는 단위 벡터 (방향만, 크기 미지)" << std::endl;
    std::cout << "   실제 이동 거리를 알려면 별도의 스케일 정보 필요" << std::endl;
    std::cout << "   (스테레오 카메라 기선, 알려진 물체 크기, IMU 등)" << std::endl;
    std::cout << std::endl;
    std::cout << "   [SLAM에서의 역할]" << std::endl;
    std::cout << "   Visual Odometry: 매 프레임 E → R,t → 누적하여 경로 추정" << std::endl;
}

// 문제 3: BF vs FLANN 매칭 벤치마크 — 속도와 정확도 비교
//
// ★ 핵심: BF는 정확하지만 O(N²), FLANN은 근사이지만 O(N log N)
//
// Brute-Force (BF) 매칭:
//   모든 디스크립터 쌍을 비교 → 정확한 최근접 보장
//   시간: O(N × M × D)  (N,M: 특징점 수, D: 디스크립터 차원)
//   특징점 < 1000이면 충분히 빠름
//
// FLANN (Fast Library for Approximate Nearest Neighbors):
//   KD-tree 등의 공간 분할로 빠른 근사 탐색
//   시간: O(N log N) 수준 (인덱스 구축 비용 별도)
//   특징점 > 1000이면 BF 대비 수 배~수십 배 빠름
//
// 비교표:
//   방식    | 정확도 | 속도    | 적합한 경우
//   --------+--------+---------+---------------------------
//   BF      | 100%   | 느림    | 소수 특징점, 정밀 매칭
//   FLANN   | ~99%   | 빠름    | 대량 특징점, 실시간 SLAM
//
// SLAM 관점: ORB-SLAM은 이진 디스크립터+BF, 대규모에는 FLANN/LSH
void problem3_matching_benchmark()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 매칭 성능 벤치마크" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 대량의 특징점으로 테스트 — 2000개에서 BF와 FLANN의 속도 차이 관찰
    int num_features = 2000;

    std::cout << "특징점 개수: " << num_features << "개\n" << std::endl;

    // [OpenCV] cv::BFMatcher bf_matcher(cv::NORM_HAMMING);
    // [OpenCV] bf_matcher.match(desc1, desc2, bf_matches);
    // ✅ 정답: BF 매칭 직접 구현 — 모든 쌍의 해밍 거리를 비교하여 최근접 탐색
    cv::Mat desc1(num_features, 32, CV_8U);
    cv::Mat desc2(num_features, 32, CV_8U);
    cv::randu(desc1, 0, 255);
    cv::randu(desc2, 0, 255);

    auto start_bf = std::chrono::high_resolution_clock::now();
    std::vector<cv::DMatch> bf_matches;
    for (int i = 0; i < desc1.rows; i++)
    {
        int best_j = 0;
        int best_dist = INT_MAX;
        for (int j = 0; j < desc2.rows; j++)
        {
            // 해밍 거리: XOR 후 1인 비트 개수
            int dist = 0;
            for (int k = 0; k < desc1.cols; k++)
            {
                uint8_t xor_val = desc1.at<uint8_t>(i, k) ^ desc2.at<uint8_t>(j, k);
                while (xor_val)
                {
                    dist += xor_val & 1;
                    xor_val >>= 1;
                }
            }
            if (dist < best_dist)
            {
                best_dist = dist;
                best_j = j;
            }
        }
        bf_matches.push_back(cv::DMatch(i, best_j, (float)best_dist));
    }
    auto end_bf = std::chrono::high_resolution_clock::now();
    double bf_ms = std::chrono::duration<double, std::milli>(end_bf - start_bf).count();

    // ✅ 정답: FLANN 매칭 시간 측정
    cv::Mat desc1_f, desc2_f;
    desc1.convertTo(desc1_f, CV_32F);
    desc2.convertTo(desc2_f, CV_32F);

    auto start_flann = std::chrono::high_resolution_clock::now();
    cv::FlannBasedMatcher flann_matcher;
    std::vector<cv::DMatch> flann_matches;
    flann_matcher.match(desc1_f, desc2_f, flann_matches);
    auto end_flann = std::chrono::high_resolution_clock::now();
    double flann_ms = std::chrono::duration<double, std::milli>(end_flann - start_flann).count();

    std::cout << "매칭 알고리즘  |  시간 (ms)  |  속도비" << std::endl;
    std::cout << "---------------+-------------+---------" << std::endl;
    std::cout << "Brute-Force    |  " << bf_ms << "  |   1.0x" << std::endl;
    std::cout << "FLANN          |  " << flann_ms << "  |   " << bf_ms / flann_ms << "x" << std::endl;

    std::cout << "\n💡 정답 해설:" << std::endl;
    std::cout << "   [코드 핵심] chrono로 BF/FLANN 매칭 시간을 각각 측정하여 비교" << std::endl;
    std::cout << "   BFMatcher: 모든 쌍을 비교 → 정확하지만 O(N²)" << std::endl;
    std::cout << "   FlannBasedMatcher: KD-tree 근사 탐색 → 빠르지만 ~99% 정확" << std::endl;
    std::cout << std::endl;
    std::cout << "   [주의] FLANN은 CV_32F만 지원 → 이진 디스크립터 변환 필요" << std::endl;
    std::cout << "   이진 디스크립터(ORB)에는 LSH 기반 FLANN을 사용할 수도 있음" << std::endl;
    std::cout << std::endl;
    std::cout << "   [언제 무엇을 쓸까?]" << std::endl;
    std::cout << "   특징점 < 500: BF로도 충분히 빠름" << std::endl;
    std::cout << "   특징점 > 1000: FLANN이 수 배~수십 배 빠름" << std::endl;
    std::cout << "   ORB-SLAM: 이진 디스크립터 + BF (해밍 거리가 빠르므로 BF도 실용적)" << std::endl;
}

// DLT 헬퍼 함수 — 대응점으로 Homography를 직접 추정
// 문제4에서 구현 원리를 설명하고, 문제5의 RANSAC에서도 재사용
cv::Mat compute_homography_dlt(const std::vector<cv::Point2d>& src,
                               const std::vector<cv::Point2d>& dst)
{
    int n = (int)src.size();
    if (n < 4) return cv::Mat();

    cv::Mat A = cv::Mat::zeros(2 * n, 9, CV_64F);
    for (int i = 0; i < n; i++)
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

    cv::Mat S, U_svd, Vt;
    cv::SVD::compute(A, S, U_svd, Vt);
    cv::Mat h = Vt.row(Vt.rows - 1);
    cv::Mat H = h.reshape(1, 3);
    if (std::abs(H.at<double>(2, 2)) < 1e-10) return cv::Mat();
    H /= H.at<double>(2, 2);
    return H.clone();
}

// 문제 4: Homography DLT 직접 구현 — A행렬 구성 + SVD 분해
//
// ★ 핵심: 대응점으로 연립방정식 Ah=0을 만들고, SVD로 최소 노름 해를 구함
//
// DLT(Direct Linear Transform) 알고리즘:
//   각 대응점 (x,y) ↔ (u,v)에서 2개 방정식을 유도:
//
//   [-x  -y  -1   0   0   0  ux  uy  u] [h1]   [0]
//   [ 0   0   0  -x  -y  -1  vx  vy  v] [h2] = [0]
//                                        [..]
//                                        [h9]
//
//   N개 대응점 → 2N×9 행렬 A, 미지수 벡터 h (9×1)
//   Ah = 0 의 비자명 해 → SVD(A)의 V^T 마지막 행
//
// SVD 분해:
//   A = U · S · V^T
//   V^T의 마지막 행 (가장 작은 특이값에 대응) → h 벡터
//   h를 3×3으로 reshape → Homography H
//   H를 h33으로 나누어 정규화 (h33 = 1)
//
// 최소 점 수: 4 (2×4=8 방정식, 미지수 8개 — h33=1 정규화)
// N > 4: 과결정 시스템 → 최소제곱 해 (노이즈에 강건)
void problem4_homography_dlt()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Homography DLT 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "DLT 알고리즘:" << std::endl;
    std::cout << "   1. 각 대응점 (x,y)↔(u,v)에서 2개 방정식 생성" << std::endl;
    std::cout << "   2. A 행렬 (2N x 9) 구성" << std::endl;
    std::cout << "   3. SVD 분해: A = U * S * V^T" << std::endl;
    std::cout << "   4. V의 마지막 열을 3x3으로 reshape → H\n" << std::endl;

    // Ground truth Homography 생성
    double theta = 10.0 * CV_PI / 180.0;
    double scale = 1.05;
    cv::Mat H_true = (cv::Mat_<double>(3, 3) <<
        scale * cos(theta), -scale * sin(theta), 30.0,
        scale * sin(theta),  scale * cos(theta), 20.0,
        0.0,                 0.0,                1.0);

    // 소스 포인트 (사각형 + 추가 점)
    std::vector<cv::Point2d> src_pts = {
        {100, 100}, {250, 100}, {250, 250}, {100, 250},
        {150, 150}, {200, 180}
    };

    // [OpenCV] cv::perspectiveTransform(src_pts, dst_pts, H_true);
    // H_true로 변환하여 목적 포인트 생성 — 직접 행렬 곱으로 변환
    std::vector<cv::Point2d> dst_pts;
    for (const auto& pt : src_pts)
    {
        cv::Mat p = (cv::Mat_<double>(3, 1) << pt.x, pt.y, 1.0);
        cv::Mat p2 = H_true * p;
        dst_pts.push_back(cv::Point2d(
            p2.at<double>(0) / p2.at<double>(2),
            p2.at<double>(1) / p2.at<double>(2)));
    }

    std::cout << "대응점 (6쌍):" << std::endl;
    for (size_t i = 0; i < src_pts.size(); i++)
    {
        std::cout << "   (" << src_pts[i].x << ", " << src_pts[i].y << ") → ("
                  << dst_pts[i].x << ", " << dst_pts[i].y << ")" << std::endl;
    }

    // [OpenCV] cv::Mat H_dlt = cv::findHomography(src_pts, dst_pts, 0);
    // ✅ 정답: DLT 헬퍼 함수로 H 추정
    // A 행렬 구성 (2N×9) → SVD 분해 → Vt 마지막 행 → 3×3 reshape
    // 상세 구현은 compute_homography_dlt() 참조
    cv::Mat H_dlt = compute_homography_dlt(src_pts, dst_pts);

    // OpenCV findHomography로 비교
    cv::Mat H_cv = cv::findHomography(src_pts, dst_pts);

    std::cout << "\n📊 DLT 결과:" << std::endl;
    std::cout << H_dlt << "\n" << std::endl;

    std::cout << "📊 OpenCV findHomography 결과:" << std::endl;
    std::cout << H_cv << "\n" << std::endl;

    std::cout << "📊 Ground Truth:" << std::endl;
    std::cout << H_true << "\n" << std::endl;

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [코드 핵심] DLT 구현 4단계:" << std::endl;
    std::cout << "   1. 각 대응점에서 2행 생성 → A 행렬 (2N×9)" << std::endl;
    std::cout << "      행 2i:   [-x, -y, -1,  0,  0,  0, ux, uy, u]" << std::endl;
    std::cout << "      행 2i+1: [ 0,  0,  0, -x, -y, -1, vx, vy, v]" << std::endl;
    std::cout << "   2. SVD::compute(A, S, U, Vt) 분해" << std::endl;
    std::cout << "   3. Vt 마지막 행(가장 작은 특이값) → h 벡터 (1×9)" << std::endl;
    std::cout << "   4. h를 3×3 reshape + h33으로 나누어 정규화" << std::endl;
    std::cout << std::endl;
    std::cout << "   [왜 SVD 마지막 행인가?]" << std::endl;
    std::cout << "   Ah=0에서 ||h||=1 조건 하의 최소 ||Ah|| 해" << std::endl;
    std::cout << "   = 가장 작은 특이값에 대응하는 오른쪽 특이벡터" << std::endl;
    std::cout << "   점이 4개 초과면 과결정 → 최소제곱 해 (노이즈에 강건)" << std::endl;
}

// 문제 5: RANSAC Homography 직접 구현 — 아웃라이어에 강건한 추정
//
// ★ 핵심: 랜덤 최소 샘플 → 모델 추정 → 합의 검증을 반복
//
// RANSAC (RANdom SAmple Consensus) 알고리즘:
//   for iter = 1 to max_iters:
//     1. 랜덤으로 4개 대응점 선택 (Homography 최소 점 수)
//     2. DLT로 H 추정 (문제4의 알고리즘)
//     3. 모든 점에 H 적용: p' = H * p, 재투영 오차 계산
//     4. 오차 < threshold인 점 = inlier
//     5. inlier 수가 최대이면 best_H 갱신
//   최종: best_H의 inlier로 다시 DLT → refined H
//
// 재투영 오차:
//   e = || dst_i - H * src_i ||₂  (유클리드 거리)
//   threshold: 보통 1~5 픽셀
//
// 이 문제의 데이터:
//   inlier 40개 (H_true + 노이즈) + outlier 15개 (완전 랜덤)
//   → outlier 비율 ≈ 27% → 약 16회 반복이면 99% 성공
//
// SLAM 관점: findHomography(pts, pts, RANSAC)가 내부적으로 이 알고리즘 수행
void problem5_ransac_homography()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: RANSAC Homography 구현" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "RANSAC 알고리즘:" << std::endl;
    std::cout << "   1. 랜덤 4점 선택" << std::endl;
    std::cout << "   2. DLT로 H 추정" << std::endl;
    std::cout << "   3. 모든 점에 H 적용, 재투영 오차 계산" << std::endl;
    std::cout << "   4. 오차 < threshold인 점 = inlier" << std::endl;
    std::cout << "   5. 반복하여 최다 inlier H 선택\n" << std::endl;

    // Ground truth H
    double theta = 10.0 * CV_PI / 180.0;
    cv::Mat H_true = (cv::Mat_<double>(3, 3) <<
        1.05 * cos(theta), -1.05 * sin(theta), 30.0,
        1.05 * sin(theta),  1.05 * cos(theta), 20.0,
        0.0,                0.0,                1.0);

    // Inlier 생성 (40개)
    int n_inliers = 40;
    int n_outliers = 15;
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(50.0, 250.0);
    std::normal_distribution<double> noise(0.0, 1.0);

    std::vector<cv::Point2d> src_pts, dst_pts;
    std::vector<bool> gt_mask;

    // Inlier 생성: H_true로 정확하게 변환 + 가우시안 노이즈 (σ=1 픽셀)
    for (int i = 0; i < n_inliers; i++)
    {
        cv::Point2d pt(dist(rng), dist(rng));
        src_pts.push_back(pt);

        cv::Mat p = (cv::Mat_<double>(3, 1) << pt.x, pt.y, 1.0);
        cv::Mat p2 = H_true * p;
        dst_pts.push_back(cv::Point2d(
            p2.at<double>(0) / p2.at<double>(2) + noise(rng),
            p2.at<double>(1) / p2.at<double>(2) + noise(rng)));
        gt_mask.push_back(true);
    }

    // Outlier 생성: src, dst 모두 랜덤 → H_true와 무관한 점
    for (int i = 0; i < n_outliers; i++)
    {
        src_pts.push_back(cv::Point2d(dist(rng), dist(rng)));
        dst_pts.push_back(cv::Point2d(dist(rng), dist(rng)));
        gt_mask.push_back(false);
    }

    int total = n_inliers + n_outliers;
    std::cout << "테스트 데이터:" << std::endl;
    std::cout << "   총 대응점: " << total << std::endl;
    std::cout << "   실제 inlier: " << n_inliers << std::endl;
    std::cout << "   실제 outlier: " << n_outliers << "\n" << std::endl;

    // ✅ 정답: RANSAC 구현
    int max_iters = 500;
    double threshold = 5.0;
    int best_inlier_count = 0;
    cv::Mat best_H;

    std::vector<int> indices(total);
    std::iota(indices.begin(), indices.end(), 0);

    for (int iter = 0; iter < max_iters; iter++)
    {
        // Step 1: 랜덤 4개 인덱스 선택
        std::shuffle(indices.begin(), indices.end(), rng);
        std::vector<cv::Point2d> src_4, dst_4;
        for (int k = 0; k < 4; k++)
        {
            src_4.push_back(src_pts[indices[k]]);
            dst_4.push_back(dst_pts[indices[k]]);
        }

        // [OpenCV] cv::Mat H_est = cv::findHomography(src_4, dst_4, 0);
        // Step 2: 4개 점으로 DLT 직접 구현하여 H 추정
        cv::Mat H_est = compute_homography_dlt(src_4, dst_4);
        if (H_est.empty()) continue;

        // [OpenCV] cv::perspectiveTransform(src_pts, projected, H_est);
        // [OpenCV] if (cv::norm(projected[i] - dst_pts[i]) < threshold) inlier_count++;
        // Step 3-4: 직접 H*p 변환 + 재투영 오차로 inlier 카운트
        int inlier_count = 0;
        for (int i = 0; i < total; i++)
        {
            cv::Mat p = (cv::Mat_<double>(3, 1) << src_pts[i].x, src_pts[i].y, 1.0);
            cv::Mat p2 = H_est * p;
            double w = p2.at<double>(2);
            if (std::abs(w) < 1e-10) continue;
            double proj_x = p2.at<double>(0) / w;
            double proj_y = p2.at<double>(1) / w;
            double dx = proj_x - dst_pts[i].x;
            double dy = proj_y - dst_pts[i].y;
            if (std::sqrt(dx * dx + dy * dy) < threshold)
                inlier_count++;
        }

        // Step 5: best 갱신
        if (inlier_count > best_inlier_count)
        {
            best_inlier_count = inlier_count;
            best_H = H_est.clone();
        }
    }

    std::cout << "📊 RANSAC 결과:" << std::endl;
    std::cout << "   검출된 inlier: " << best_inlier_count << " / " << total << std::endl;

    // 성능 평가 — True Positive(올바른 inlier 검출), False Positive(outlier 오검출)
    if (!best_H.empty())
    {
        int tp = 0, fp = 0;
        for (int i = 0; i < total; i++)
        {
            // [OpenCV] cv::perspectiveTransform + cv::norm
            // 직접 H*p 변환 + 유클리드 거리 계산
            cv::Mat p = (cv::Mat_<double>(3, 1) << src_pts[i].x, src_pts[i].y, 1.0);
            cv::Mat p2 = best_H * p;
            double w = p2.at<double>(2);
            double proj_x = p2.at<double>(0) / w;
            double proj_y = p2.at<double>(1) / w;
            double dx = proj_x - dst_pts[i].x;
            double dy = proj_y - dst_pts[i].y;
            double err = std::sqrt(dx * dx + dy * dy);
            bool is_inlier = err < threshold;
            if (is_inlier && gt_mask[i]) tp++;
            if (is_inlier && !gt_mask[i]) fp++;
        }
        std::cout << "   True Positive: " << tp << std::endl;
        std::cout << "   False Positive: " << fp << std::endl;
    }
    else
    {
        std::cout << "   (TODO를 구현하세요)" << std::endl;
    }

    // OpenCV 비교
    cv::Mat mask;
    cv::Mat H_cv = cv::findHomography(src_pts, dst_pts, cv::RANSAC, threshold, mask);
    int cv_inliers = cv::countNonZero(mask);

    std::cout << "\n📊 OpenCV RANSAC 결과:" << std::endl;
    std::cout << "   inlier: " << cv_inliers << " / " << total << "\n" << std::endl;

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [코드 핵심] RANSAC 루프 5단계 (모두 직접 구현):" << std::endl;
    std::cout << "   1. indices를 shuffle → 앞 4개 선택 (랜덤 최소 샘플)" << std::endl;
    std::cout << "   2. compute_homography_dlt(4pts) → DLT로 H 직접 추정" << std::endl;
    std::cout << "   3. H*p 행렬 곱으로 전체 점 변환" << std::endl;
    std::cout << "   4. sqrt(dx²+dy²) < threshold → inlier 카운트" << std::endl;
    std::cout << "   5. 최다 inlier 모델을 best_H로 갱신" << std::endl;
    std::cout << std::endl;
    std::cout << "   [예상 결과]" << std::endl;
    std::cout << "   40개 inlier + 15개 outlier (27%) → ~16회면 99% 성공" << std::endl;
    std::cout << "   500회 반복이므로 충분 → inlier ~40개 검출, FP ~0개" << std::endl;
    std::cout << std::endl;
    std::cout << "   [threshold 선택 가이드]" << std::endl;
    std::cout << "   1~3 픽셀: 엄격 (정밀한 매칭만)" << std::endl;
    std::cout << "   3~5 픽셀: 일반적 (노이즈 허용)" << std::endl;
    std::cout << "   5+ 픽셀: 느슨 (많은 inlier, 정확도↓)" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 4 Quiz - Medium (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_optimal_ratio();
    problem2_essential_matrix();
    problem3_matching_benchmark();
    problem4_homography_dlt();
    problem5_ransac_homography();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
