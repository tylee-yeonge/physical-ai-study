/**
 * Phase 2 Week 4 - 특징점 매칭 중급 퀴즈
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

    // TODO: 랜덤 디스크립터로 KNN 매칭 후, ratio test 적용
    // 1. 랜덤 디스크립터 생성 (num_features x 32, CV_8U)
    // 2. BFMatcher(NORM_HAMMING)로 knnMatch(k=2) 수행
    // 3. 각 ratio에 대해 ratio test 적용:
    //    best.distance < ratio * second_best.distance 이면 통과
    // 4. 통과한 매칭 수와 통과율(pass_rate) 출력

    for (float ratio : ratios)
    {    
        // TODO: ratio test 수행
        // good_matches: match_pair[0].distance < ratio * match_pair[1].distance 인 것만 선별

        // TODO: 통과율 계산 후 결과 출력
        // pass_rate = 통과한 매칭 수 / 전체 KNN 매칭 수 × 100
        // 출력 형식: " {ratio}  |     {good_matches 수}     |    {pass_rate}%"
    }

    std::cout << "\n💡 관찰:" << std::endl;
    std::cout << "   - Ratio ↓: 매칭 개수 ↓, 품질 ↑ (precision)" << std::endl;
    std::cout << "   - Ratio ↑: 매칭 개수 ↑, 품질 ↓ (recall)" << std::endl;
    std::cout << "   - 0.7은 좋은 균형점 (Lowe 논문)" << std::endl;
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

    // TODO: 매칭된 점들 생성 (실제로는 특징점 검출+매칭 결과)
    std::vector<cv::Point2f> points1, points2;
    // 시뮬레이션 데이터 생성...

    // TODO: Essential Matrix 추정
    // 힌트: OpenCV에서 대응점과 K 행렬로 E를 추정하는 함수를 찾아보세요

    std::cout << "Essential Matrix E:" << std::endl;

    // TODO: E에서 R, t 복원
    // 힌트: E를 분해하여 회전과 이동을 복원하는 함수를 찾아보세요

    std::cout << "💡 SLAM에서의 의미:" << std::endl;
    std::cout << "   - E를 분해 → R (회전), t (이동)" << std::endl;
    std::cout << "   - 두 프레임 간 상대 포즈!" << std::endl;
    std::cout << "   - Visual Odometry의 핵심" << std::endl;
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

    // TODO: BF 매칭 시간 측정
    // 힌트: 이진 디스크립터에 적합한 매처를 생성하고 시간을 측정하세요

    // TODO: FLANN 매칭 시간 측정
    // 힌트: 근사 최근접 탐색 기반 매처를 생성하고 비교하세요
    //       FLANN은 입력 데이터 타입에 제약이 있습니다

    std::cout << "매칭 알고리즘  |  시간 (ms)  |  속도비" << std::endl;
    std::cout << "---------------+-------------+---------" << std::endl;
    std::cout << "Brute-Force    |     ???     |   1.0x" << std::endl;
    std::cout << "FLANN          |     ???     |   ???x" << std::endl;

    std::cout << "\n💡 결론:" << std::endl;
    std::cout << "   - BF: 정확하지만 느림 O(N²)" << std::endl;
    std::cout << "   - FLANN: 빠른 근사 O(N log N)" << std::endl;
    std::cout << "   - 실시간 SLAM: FLANN 또는 NN 기반" << std::endl;
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
    // 아래는 동일한 동작의 직접 구현 — H*p 행렬 곱 + 동차좌표 정규화
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

    // TODO: DLT 구현
    // 1. A 행렬 구성 (2*N x 9)
    int N = (int)src_pts.size();
    cv::Mat A = cv::Mat::zeros(2 * N, 9, CV_64F);

    // 각 대응점 (x,y)↔(u,v)에 대해 2행 추가
    //   행 2i:   [-x, -y, -1,  0,  0,  0, u*x, u*y, u]  ← u 방정식
    //   행 2i+1: [ 0,  0,  0, -x, -y, -1, v*x, v*y, v]  ← v 방정식
    for (int i = 0; i < N; i++)
    {
        double x = src_pts[i].x, y = src_pts[i].y;
        double u = dst_pts[i].x, v = dst_pts[i].y;
        // TODO: 각 대응점에 대해 위 행 배치 규칙으로 A 행렬 채우기
    }

    // TODO: SVD 분해 후 H 추출
    cv::Mat H_dlt = cv::Mat::eye(3, 3, CV_64F);
    // 힌트: Ah=0의 비자명 해는 SVD에서 가장 작은 특이값에 대응하는 벡터입니다
    //       이 벡터를 3x3으로 재배열하고 정규화하면 H가 됩니다

    // OpenCV findHomography로 비교
    cv::Mat H_cv = cv::findHomography(src_pts, dst_pts);

    std::cout << "\n📊 DLT 결과:" << std::endl;
    std::cout << H_dlt << "\n" << std::endl;

    std::cout << "📊 OpenCV findHomography 결과:" << std::endl;
    std::cout << H_cv << "\n" << std::endl;

    std::cout << "📊 Ground Truth:" << std::endl;
    std::cout << H_true << "\n" << std::endl;

    std::cout << "💡 핵심:" << std::endl;
    std::cout << "   - DLT는 최소 4점, 더 많으면 최소제곱 해" << std::endl;
    std::cout << "   - SVD의 마지막 V 열이 Ah=0의 최소 노름 해" << std::endl;
    std::cout << "   - 노이즈가 있으면 RANSAC과 함께 사용" << std::endl;
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

    // TODO: RANSAC 구현
    int max_iters = 500;      // 충분한 반복 횟수 (실제로는 공식으로 계산)
    double threshold = 5.0;   // 재투영 오차 임계값 (픽셀)
    int best_inlier_count = 0;
    cv::Mat best_H;

    // 힌트: 위 주석의 RANSAC 알고리즘 5단계를 구현하세요
    //       각 반복에서 최소 점 수로 모델을 추정하고,
    //       전체 데이터에 대한 합의(consensus)를 평가합니다

    std::cout << "📊 RANSAC 결과:" << std::endl;
    std::cout << "   검출된 inlier: " << best_inlier_count << " / " << total << std::endl;

    // 성능 평가 — True Positive(올바른 inlier 검출), False Positive(outlier 오검출)
    if (!best_H.empty())
    {
        // [OpenCV] cv::perspectiveTransform(src_pts, projected, best_H);
        // [OpenCV] double err = cv::norm(projected[i] - dst_pts[i]);
        // 아래는 동일한 동작의 직접 구현
        int tp = 0, fp = 0;
        for (int i = 0; i < total; i++)
        {
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

    std::cout << "💡 핵심:" << std::endl;
    std::cout << "   - 아웃라이어가 있어도 robust하게 H 추정" << std::endl;
    std::cout << "   - 반복 횟수: log(1-p)/log(1-w^4)" << std::endl;
    std::cout << "   - threshold: 보통 1~5 픽셀" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 4 Quiz - Medium" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_optimal_ratio();
    problem2_essential_matrix();
    problem3_matching_benchmark();
    problem4_homography_dlt();
    problem5_ransac_homography();

    std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
