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
#include <iomanip>
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

    // 여러 ratio 값 테스트
    std::vector<float> ratios = {0.5, 0.6, 0.7, 0.8, 0.9};

    std::cout << "Ratio  |  매칭 개수  |  Inlier 비율" << std::endl;
    std::cout << "-------+-------------+-------------" << std::endl;

    // 이미지 로드
    cv::Mat img1 = cv::imread("../images/box.png", cv::IMREAD_GRAYSCALE);
    cv::Mat img2 = cv::imread("../images/box_in_scene.png", cv::IMREAD_GRAYSCALE);
    if (img1.empty() || img2.empty())
    {
        std::cerr << "이미지를 로드할 수 없습니다!" << std::endl;
        return;
    }

    // TODO 1: ORB 특징점 검출 + 디스크립터 추출
    // - ORB 검출기를 생성하고 (최대 500개 특징점)
    // - 각 이미지에서 키포인트와 디스크립터를 한 번에 추출
    //   kp: 특징점 위치/스케일/방향 정보
    //   desc: 32바이트(256비트) 이진 디스크립터 벡터
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat desc1, desc2;

    std::cout << "이미지: box.png (" << kp1.size() << " 특징점) vs "
              << "box_in_scene.png (" << kp2.size() << " 특징점)\n" << std::endl;

    // TODO 2: KNN 매칭 수행 (k=2)
    // - 해밍 거리 기반의 전수 비교 매처 생성
    //   해밍 거리: 이진 디스크립터(ORB)의 거리 = XOR 후 1인 비트 수(popcount)
    //   (SIFT/SURF 같은 실수 디스크립터는 L2 거리 사용)
    // - 각 디스크립터마다 가장 가까운 2개 후보를 탐색 (k=2)
    //   왜 2개? → ratio test에서 1등(best)과 2등(second_best)의 거리를 비교하기 위해
    std::vector<std::vector<cv::DMatch>> knn_matches;

    for (float ratio : ratios)
    {
        // TODO 3: ratio test 수행
        // - 각 매칭 쌍에서 1등과 2등의 거리를 비교:
        //   1등 거리 < ratio × 2등 거리 → 통과
        // - 의미: 1등이 2등보다 "충분히" 가까워야 신뢰할 수 있는 매칭
        //   (1등과 2등이 비슷하면 → 어느 쪽이 진짜인지 불확실 → 버림)
        // - 후보가 2개 미만인 경우는 건너뛰기
        std::vector<cv::DMatch> good_matches;


        // pass_rate = 통과한 매칭 수 / 전체 KNN 매칭 수 × 100
        double pass_rate = knn_matches.empty() ? 0.0
            : 100.0 * good_matches.size() / knn_matches.size();
        std::cout << " " << ratio << "  |     " << std::setw(3) << good_matches.size()
                  << "     |    " << std::fixed << std::setprecision(1) << pass_rate << "%" << std::endl;
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

    // 이미지 로드 + 특징점 매칭 (문제 1과 동일한 파이프라인)
    cv::Mat img1 = cv::imread("../images/box.png", cv::IMREAD_GRAYSCALE);
    cv::Mat img2 = cv::imread("../images/box_in_scene.png", cv::IMREAD_GRAYSCALE);
    if (img1.empty() || img2.empty())
    {
        std::cerr << "이미지를 로드할 수 없습니다!" << std::endl;
        return;
    }

    const int kMaxFeatures = 500;  // ORB 최대 특징점 수
    auto orb = cv::ORB::create(kMaxFeatures);
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat desc1, desc2;
    orb->detectAndCompute(img1, cv::noArray(), kp1, desc1);
    orb->detectAndCompute(img2, cv::noArray(), kp2, desc2);

    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<std::vector<cv::DMatch>> knn_matches;
    matcher.knnMatch(desc1, desc2, knn_matches, 2);

    // Ratio test로 좋은 매칭만 선별
    std::vector<cv::DMatch> good_matches;
    for (const auto& m : knn_matches)
    {
        if (m.size() >= 2 && m[0].distance < 0.7f * m[1].distance)
            good_matches.push_back(m[0]);
    }

    // 매칭된 키포인트 좌표 추출
    std::vector<cv::Point2f> points1, points2;
    for (const auto& m : good_matches)
    {
        points1.push_back(kp1[m.queryIdx].pt);
        points2.push_back(kp2[m.trainIdx].pt);
    }

    std::cout << "매칭점: " << points1.size() << "쌍\n" << std::endl;

    // 카메라 내부 파라미터 (가상 — 실제로는 캘리브레이션 결과 사용)
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // TODO 1: Essential Matrix 추정 (8-Point Algorithm 직접 구현)
    // [OpenCV 원라이너] cv::Mat E = cv::findEssentialMat(points1, points2, K, cv::RANSAC);
    //
    // --- 단계 1-1: 픽셀 좌표 → 정규화 좌표 변환 ---
    //   p_norm = K^{-1} * p_pixel
    //   픽셀 좌표 (u,v)는 카메라마다 해상도/렌즈가 달라서 기하학적 비교 불가.
    //   K^{-1}을 곱하면 카메라 의존성이 제거된 "정규화 좌표"가 된다:
    //     x_norm = (u - cx) / fx,  y_norm = (v - cy) / fy
    //   → "카메라 앞 1m 평면"의 좌표. 에피폴라 제약 p2^T·E·p1 = 0은 여기서만 성립.

    // --- 단계 1-2: A행렬 구성 (N×9) ---
    //   에피폴라 제약 p2^T·E·p1 = 0을 전개하면:
    //     E를 9×1 벡터 e로 펼쳤을 때, 각 대응점마다 1개 방정식이 나온다:
    //     [x2*x1, x2*y1, x2, y2*x1, y2*y1, y2, x1, y1, 1] · e = 0
    //   N개 대응점 → N×9 행렬 A → Ae = 0 풀기
    int N = 0;
    
    // --- 단계 1-3: SVD로 해 구하기 ---
    //   Ae = 0 → "A에 곱했을 때 가장 0에 가까운 벡터"
    //   SVD(A) = U·S·V^T 에서 V^T의 마지막 행이 해
    //   이 9×1 벡터를 3×3으로 reshape → E의 초기 추정값

    // --- 단계 1-4: rank-2 제약 적용 ---
    //   E = [t]×·R 이므로 수학적으로 rank 2여야 한다.
    //   노이즈 때문에 1-3의 결과는 rank 3일 수 있으므로:
    //   E_raw를 다시 SVD → 특이값을 (σ_avg, σ_avg, 0)으로 강제
    //   (σ_avg = 상위 2개 특이값의 평균, 3번째를 0으로 → rank 2 보장)

    std::cout << "Essential Matrix E:" << std::endl;

    // TODO 2: E에서 R, t 복원 (SVD 분해 + Cheirality 검증)
    // [OpenCV 원라이너] int inliers = cv::recoverPose(E, points1, points2, K, R, t);
    
    cv::Mat best_R, best_t;
    int best_count = 0;
    // --- 단계 2-1: R, t 후보 생성 ---
    //   E의 SVD: E = U·diag(σ,σ,0)·V^T (TODO 1에서 이미 분해됨)
    //   W 행렬 (Z축 90도 회전): W = [0 -1 0; 1 0 0; 0 0 1]
    //   R 후보 2개: R1 = U·W·V^T,   R2 = U·W^T·V^T
    //   t 후보 2개: t1 = +U의 3번째 열, t2 = -U의 3번째 열
    //   → 총 4가지 (R,t) 조합. 물리적으로 유효한 것은 1개뿐.
    //   주의: det(R) < 0이면 거울 반사 → R = -R로 보정
    
    // --- 단계 2-2: Cheirality 검증 ---
    //   "삼각측량한 3D 점이 두 카메라 모두의 앞에 있는가?" (Z > 0)
    //   각 (R,t) 조합에 대해:
    //     1) 정규화 좌표 p1, p2로부터 삼각측량 → 3D 점 X
    //        - 카메라1: X = λ·p1 (원점에서 p1 방향 광선)
    //        - 카메라2: X = R^T·(μ·p2 - t) (O2에서 p2 방향 광선)
    //        - 두 광선의 교차 → 4×3 과결정 시스템 A·X = b → SVD로 최소제곱 해
    //     2) z1 = X의 Z좌표 (카메라1 기준 깊이)
    //        X_cam2 = R·X + t → z2 = X_cam2의 Z좌표 (카메라2 기준 깊이)
    //     3) z1 > 0 && z2 > 0이면 유효한 점
    //   → Z > 0인 점이 가장 많은 (R,t) 조합이 정답

    // TODO가 구현되면 아래 블록이 실행된다.
    if (!best_t.empty())
    {
        // t를 단위 벡터로 정규화
        // Essential Matrix에서 복원한 t는 방향만 의미 있고, 절대 크기(스케일)는 알 수 없다.
        // (단안 카메라의 근본적 한계 — 스테레오나 IMU 없이는 실제 거리를 모른다)
        best_t = best_t / cv::norm(best_t);

        std::cout << "\nRotation:\n" << best_R << std::endl;
        std::cout << "Translation:\n" << best_t << std::endl;
        std::cout << "Cheirality 통과: " << best_count << " / " << N << "개" << std::endl;
    }
    else
    {
        std::cout << "\n⚠️ TODO 미구현: E, R, t가 아직 계산되지 않았습니다." << std::endl;
    }

    // OpenCV 결과와 비교
    cv::Mat E_cv = cv::findEssentialMat(points1, points2, K, cv::RANSAC, 0.999, 1.0);
    cv::Mat R_cv, t_cv;
    cv::recoverPose(E_cv, points1, points2, K, R_cv, t_cv);
    std::cout << "\n📊 OpenCV 비교:" << std::endl;
    std::cout << "E (OpenCV):\n" << E_cv << std::endl;
    std::cout << "R (OpenCV):\n" << R_cv << std::endl;
    std::cout << "t (OpenCV):\n" << t_cv << std::endl;
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

    // ── 이미지에서 ORB 디스크립터 추출 ──
    //
    //   랜덤 데이터가 아닌 실제 이미지에서 특징점을 검출하고 디스크립터를 계산한다.
    //   실제 디스크립터는 랜덤과 달리 구조적 패턴이 있어 매칭 결과가 더 현실적이다.
    int num_features = 2000;

    // graf1.png, graf3.png: OpenCV 공식 샘플 — 그래피티 벽을 다른 시점에서 촬영한 이미지 쌍
    // (problem1,2의 box 이미지와 다른 장면을 사용하여 다양한 상황에서의 매칭 성능을 확인)
    cv::Mat img1 = cv::imread("../images/graf1.png", cv::IMREAD_GRAYSCALE);
    cv::Mat img2 = cv::imread("../images/graf3.png", cv::IMREAD_GRAYSCALE);
    if (img1.empty() || img2.empty())
    {
        std::cerr << "이미지 로드 실패!" << std::endl;
        return;
    }

    auto orb = cv::ORB::create(num_features);
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat desc1, desc2;
    orb->detectAndCompute(img1, cv::noArray(), kp1, desc1);
    orb->detectAndCompute(img2, cv::noArray(), kp2, desc2);

    std::cout << "특징점 개수: img1=" << desc1.rows << "개, img2=" << desc2.rows << "개\n" << std::endl;

    auto start_bf = std::chrono::high_resolution_clock::now();
    // TODO: BF 매칭 구현
    //
    // --- BF(Brute-Force) 매칭의 원리 ---
    //   모든 디스크립터 쌍(N×M)의 거리를 하나씩 비교 → 최근접 보장
    //   이진 디스크립터의 해밍 거리: XOR 후 1인 비트 수 (popcount)
    //
    // 구현 방법:
    //   1. 이중 for문: desc1의 각 행 i에 대해 desc2의 모든 행 j와 해밍 거리 비교
    //      해밍 거리 = 바이트별 XOR → 비트 카운트 합산
    //   2. 가장 거리가 작은 j를 best match로 기록
    //
    // 또는 간단하게: 해밍 거리 기반 전수 비교 매처로도 측정 가능
    auto end_bf = std::chrono::high_resolution_clock::now();
    double bf_ms = std::chrono::duration<double, std::milli>(end_bf - start_bf).count();

    // FLANN 매칭 시간 측정
    //
    // FLANN = Fast Library for Approximate Nearest Neighbors
    //   KD-tree 등 공간 분할 자료구조로 근사 최근접 탐색 → O(N log N) 수준
    //   정확도 ~99%이지만 속도가 수 배~수십 배 빠름
    //   직접 구현하려면 KD-tree 구축 + 근사 탐색 알고리즘이 필요해 ~200줄 이상이므로
    //   여기서는 OpenCV 함수를 사용한다.
    //
    //   주의: FLANN은 실수형(CV_32F)만 지원 → 이진 디스크립터를 float로 변환 필요
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
    //
    // --- DLT(Direct Linear Transform)의 핵심 아이디어 ---
    //   Homography: dst = H * src (동차좌표)
    //   → H의 9개 원소를 미지수로, 대응점마다 2개 방정식을 세워서 Ah = 0 형태로 풀기
    //
    // --- Step 1: A 행렬 구성 (2N × 9) ---
    int N = static_cast<int>(src_pts.size());
    const int kHomographyParams = 3 * 3;  // H 행렬(3×3)의 원소 수
    cv::Mat A = cv::Mat::zeros(2 * N, kHomographyParams, CV_64F);

    // 각 대응점 (x,y) ↔ (u,v)에서 투영 방정식을 전개하면 2개 행이 나온다:
    //
    //   u = (h1·x + h2·y + h3) / (h7·x + h8·y + h9)  ← 분모 제거 후 정리
    //   v = (h4·x + h5·y + h6) / (h7·x + h8·y + h9)
    //
    //   행 2i:   [-x, -y, -1,  0,  0,  0, u*x, u*y, u]  ← u 방정식
    //   행 2i+1: [ 0,  0,  0, -x, -y, -1, v*x, v*y, v]  ← v 방정식
    //
    // A 행렬의 해당 위치에 값을 채우기
    for (int i = 0; i < N; i++)
    {
        double x = src_pts[i].x, y = src_pts[i].y;
        double u = dst_pts[i].x, v = dst_pts[i].y;
        // TODO: 각 대응점에 대해 위 행 배치 규칙으로 A 행렬 채우기
    }

    // --- Step 2: SVD 분해 후 H 추출 ---
    cv::Mat H_dlt = cv::Mat::eye(3, 3, CV_64F);
    // A를 SVD로 분해
    // Ah = 0의 비자명 해 = ||h||=1 조건에서 ||Ah||를 최소화하는 벡터
    //   = V^T의 마지막 행 (가장 작은 특이값에 대응하는 오른쪽 특이벡터)
    // 이 1×9 벡터를 3×3 행렬로 재배열
    // h33으로 나누어 정규화 (h33 = 1 관례)

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

    // --- RANSAC 5단계 구현 가이드 ---
    //
    // 준비: 0~total-1 인덱스 배열 생성, 셔플용
    //
    // 반복 (max_iters회):
    //
    //   Step 1: 랜덤 4개 대응점 선택
    //     인덱스 배열을 랜덤 셔플한 뒤 앞 4개 사용
    //     → 해당 인덱스의 src, dst 점들을 담기
    //
    //   Step 2: DLT로 H 추정
    //     4개 점으로 문제4의 DLT 알고리즘 수행 (함수로 분리하면 재사용 편리)
    //     H가 유효하지 않으면 continue
    //
    //   Step 3-4: 모든 점에 H 적용 → inlier 카운트
    //     각 점에 대해: p' = H * [x, y, 1]^T → 동차좌표 정규화 (w로 나누기)
    //     재투영 오차: e = sqrt((proj_x - dst_x)² + (proj_y - dst_y)²)
    //     e < threshold이면 inlier
    //
    //   Step 5: best_inlier_count보다 크면 best_H 갱신

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
