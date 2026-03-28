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
#include <iomanip>
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

    // ✅ 정답 TODO 1: ORB 특징점 검출 + 디스크립터 추출
    const int kMaxFeatures = 500;  // ORB 최대 특징점 수
    auto orb = cv::ORB::create(kMaxFeatures);
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat desc1, desc2;
    orb->detectAndCompute(img1, cv::noArray(), kp1, desc1);
    orb->detectAndCompute(img2, cv::noArray(), kp2, desc2);

    std::cout << "이미지: box.png (" << kp1.size() << " 특징점) vs "
              << "box_in_scene.png (" << kp2.size() << " 특징점)\n" << std::endl;

    // ✅ 정답 TODO 2: KNN 매칭 수행 (k=2)
    // 이진 디스크립터(ORB)에는 NORM_HAMMING 사용
    // k=2로 best와 second_best를 함께 얻어 ratio test에 활용
    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<std::vector<cv::DMatch>> knn_matches;
    matcher.knnMatch(desc1, desc2, knn_matches, 2);

    for (float ratio : ratios)
    {
        // ✅ 정답 TODO 3: ratio test 수행
        // best.distance < ratio * second_best.distance 이면 통과
        std::vector<cv::DMatch> good_matches;
        for (const auto& match_pair : knn_matches)
        {
            if (match_pair.size() >= 2 &&
                match_pair[0].distance < ratio * match_pair[1].distance)
            {
                good_matches.push_back(match_pair[0]);
            }
        }

        double pass_rate = 100.0 * good_matches.size() / knn_matches.size();
        std::cout << " " << ratio << "  |     " << std::setw(3) << good_matches.size()
                  << "     |    " << std::fixed << std::setprecision(1) << pass_rate << "%" << std::endl;
    }

    std::cout << "\n💡 정답 해설:" << std::endl;
    std::cout << "   [TODO 1: ORB 생성 + detectAndCompute]" << std::endl;
    std::cout << "   - ORB::create(500): 최대 500개 특징점 검출" << std::endl;
    std::cout << "   - detectAndCompute(img, noArray(), kp, desc)" << std::endl;
    std::cout << "   - kp: 특징점 위치/스케일/방향, desc: 32바이트 이진 벡터" << std::endl;
    std::cout << std::endl;
    std::cout << "   [TODO 2: BFMatcher + knnMatch(k=2)]" << std::endl;
    std::cout << "   - NORM_HAMMING: 이진 디스크립터(ORB)의 거리 = XOR 후 popcount" << std::endl;
    std::cout << "   - k=2인 이유: ratio test에 best와 second_best 둘 다 필요" << std::endl;
    std::cout << std::endl;
    std::cout << "   [TODO 3: ratio test]" << std::endl;
    std::cout << "   - best.distance < ratio * second_best.distance → 통과" << std::endl;
    std::cout << "   - 의미: best가 second_best보다 '충분히' 가까워야 신뢰" << std::endl;
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

    // ✅ 정답: 실제 이미지에서 매칭점 추출 후 Essential Matrix 추정
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

    // ✅ 정답 TODO 1: Essential Matrix 추정 (8-Point Algorithm 직접 구현)
    // [OpenCV] cv::Mat E = cv::findEssentialMat(points1, points2, K, cv::RANSAC, 0.999, 1.0);

    // 1-1. 픽셀 좌표 → 정규화 좌표 변환: p_norm = K^{-1} * p_pixel
    //
    //   픽셀 좌표 (u, v)는 카메라마다 해상도/렌즈가 달라서 기하학적 비교 불가.
    //   K^{-1}을 곱해서 카메라 의존성을 제거한 "정규화 좌표"로 변환한다.
    //
    //     x_norm = (u - cx) / fx   ← 주점(cx)을 빼서 중심 이동, 초점거리(fx)로 나눠서 스케일 제거
    //     y_norm = (v - cy) / fy
    //
    //   결과: "카메라 앞 1m 평면"에서의 좌표 (카메라 무관, 순수 기하학)
    //   Essential Matrix의 에피폴라 제약 p2^T · E · p1 = 0 은 정규화 좌표에서만 성립
    cv::Mat K_inv = K.inv();
    // norm_pts: 픽셀 좌표를 K^{-1}로 변환한 정규화 좌표를 담는 벡터
    std::vector<cv::Point2d> norm_pts1, norm_pts2;
    for (size_t i = 0; i < points1.size(); i++)
    {
        // 2D 픽셀 좌표 (u, v)를 3×1 동차좌표 [u, v, 1]^T로 변환
        // → 3×3 행렬(K^{-1})과 곱하려면 벡터도 3×1이어야 하므로 1.0 추가
        cv::Mat p1 = (cv::Mat_<double>(3, 1) << points1[i].x, points1[i].y, 1.0);
        cv::Mat p2 = (cv::Mat_<double>(3, 1) << points2[i].x, points2[i].y, 1.0);
        cv::Mat n1 = K_inv * p1;
        cv::Mat n2 = K_inv * p2;
        norm_pts1.push_back(cv::Point2d(n1.at<double>(0), n1.at<double>(1)));
        norm_pts2.push_back(cv::Point2d(n2.at<double>(0), n2.at<double>(1)));
    }

    // 1-2. 8-Point Algorithm: A행렬 구성
    //
    //   에피폴라 제약: p2^T · E · p1 = 0
    //     "같은 3D 점의 투영 p1, p2는 반드시 에피폴라 선 위에 있다"
    //     → 카메라1에서 본 점의 광선을 카메라2에 투영하면 직선(에피폴라 선)이 되고,
    //       대응점 p2는 그 직선 위에만 존재할 수 있다는 기하학적 제약.
    //     → 이 제약을 수식으로 쓰면 E를 구할 수 있고, E에서 R, t를 복원한다.
    //
    //   E를 벡터로 펼치기:
    //     E = [e1 e2 e3]  → e = [e1, e2, e3, e4, e5, e6, e7, e8, e9]^T
    //         [e4 e5 e6]
    //         [e7 e8 e9]
    //
    //   p1 = [x1, y1, 1]^T, p2 = [x2, y2, 1]^T를 대입해서 전개하면:
    //     x2·x1·e1 + x2·y1·e2 + x2·e3 + y2·x1·e4 + y2·y1·e5 + y2·e6 + x1·e7 + y1·e8 + e9 = 0
    //   → 행렬 형태: [x2·x1, x2·y1, x2, y2·x1, y2·y1, y2, x1, y1, 1] · e = 0
    //
    //   N개 대응점을 쌓으면 N×9 행렬 A가 된다:
    //     A = [ x2₁·x1₁  x2₁·y1₁  x2₁  y2₁·x1₁  y2₁·y1₁  y2₁  x1₁  y1₁  1 ]
    //         [ x2₂·x1₂  x2₂·y1₂  x2₂  y2₂·x1₂  y2₂·y1₂  y2₂  x1₂  y1₂  1 ]
    //         [  ...                                                              ]
    //   → Ae = 0의 비자명 해 = SVD의 V^T 마지막 행
    //
    //   왜 "8-Point"인가?
    //     E는 9개 미지수이지만 스케일이 자유(상수배해도 같은 E)이므로
    //     실질 자유도는 8 → 최소 8쌍의 대응점이 필요
    int N = static_cast<int>(norm_pts1.size());
    const int kEssentialParams = 3 * 3;  // E 행렬(3×3)의 원소 수
    cv::Mat A(N, kEssentialParams, CV_64F);
    for (int i = 0; i < N; i++)
    {
        double x1 = norm_pts1[i].x, y1 = norm_pts1[i].y;
        double x2 = norm_pts2[i].x, y2 = norm_pts2[i].y;
        double* row = A.ptr<double>(i);
        row[0] = x2 * x1;  row[1] = x2 * y1;  row[2] = x2;
        row[3] = y2 * x1;  row[4] = y2 * y1;  row[5] = y2;
        row[6] = x1;       row[7] = y1;       row[8] = 1.0;
    }

    // 1-3. SVD로 해 구하기 — A·e = 0 연립방정식의 해를 구하는 단계
    //
    //   목표: "A에 곱했을 때 결과가 가장 0에 가까운 벡터 e"를 찾는 것
    //   왜 역행렬로 못 푸는가?
    //     Ax = b (b≠0) 이면: x = A⁻¹·b 로 해를 바로 구할 수 있다.
    //     Ax = 0 이면: x = A⁻¹·0 = 0 → 항상 자명해(x=0)만 나온다.
    //     E 행렬이 전부 0이면 쓸 수 없으므로, 자명해가 아닌 의미 있는 해가 필요하다.
    //     → 역행렬 대신 SVD를 써서 "0이 아니면서 Ax를 최소화하는 x"를 찾는다.
    //
    //   SVD의 성질을 이용한다:
    //     SVD(A) = U · S · V^T 로 분해한다.
    //
    //     [SVD란?] 어떤 행렬이든 "회전 → 스케일 → 회전" 3단계로 분해할 수 있다는 정리.
    //       A·x를 계산하면 입력 벡터 x에 V^T → S → U 순서로 변환이 적용된다:
    //         ① V^T : 입력을 작업하기 좋은 축 방향으로 회전 (좌표축 정렬)
    //         ② S   : 각 축 방향으로 늘리기/줄이기 (대각 원소 = 특이값)
    //                  특이값 > 1 → 그 방향으로 늘림
    //                  특이값 < 1 → 그 방향으로 줄임
    //                  특이값 = 0 → 그 방향 정보가 완전히 사라짐 (차원 축소)
    //         ③ U   : 최종 출력 방향으로 다시 회전
    //       특이값이 크면 그 방향으로 많이 늘린다 = 중요한 방향.
    //
    //     V^T의 마지막 행 = 가장 작은 특이값에 대응하는 벡터
    //     → 이것이 "A에 곱했을 때 결과가 가장 0에 가까운" 벡터, 즉 우리가 원하는 e
    //
    //   이 9×1 벡터를 3×3으로 reshape하면 E 행렬의 초기 추정값(E_raw)이 된다.
    cv::Mat w_a;   // 특이값 벡터 (singular values)
    cv::Mat u_a;   // 좌측 특이벡터 행렬 (U)
    cv::Mat vt_a;  // 우측 특이벡터 행렬의 전치 (V^T)
    cv::SVD::compute(A, w_a, u_a, vt_a);
    cv::Mat E_raw = vt_a.row(vt_a.rows - 1).reshape(0, 3);

    // 1-4. rank-2 제약 적용 — E_raw를 수학적으로 올바른 Essential Matrix로 교정하는 단계
    //
    //   위에서 구한 E_raw는 노이즈 때문에 "거의 맞지만 조건을 만족하지 않는" 행렬이다.
    //   Essential Matrix가 만족해야 하는 조건: 특이값이 (σ, σ, 0) 형태 (같은 값 2개 + 0)
    //     → "두 방향으로는 동일하게 늘리고, 한 방향은 완전히 눌러 3D→2D로 차원을 줄인다"는 의미
    //     → 3D 공간의 점들이 에피폴라 평면(2D)에 놓인다는 기하학적 의미와 일치
    //   하지만 E_raw의 특이값은 보통 (1.05, 0.97, 0.03) 같은 형태로 조건에서 벗어나 있다.
    //
    //   교정 과정:
    //     1) E_raw를 SVD 분해 → 특이값 (1.05, 0.97, 0.03)
    //     2) 세 번째 특이값을 0으로 강제 → (1.05, 0.97, 0) — rank 2 보장
    //     3) 나머지 두 특이값을 평균으로 통일 → (1.01, 1.01, 0) — E의 성질 만족
    //     4) 교정된 특이값으로 다시 조립 → E = U · diag(1.01, 1.01, 0) · V^T
    cv::Mat w_e;   // 특이값 벡터 (σ, σ, 0) — E의 rank-2 성질을 나타냄
    cv::Mat u_e;   // 좌측 특이벡터 (U) — 병진(translation) 방향 정보 (t = ±U의 3번째 열)
    cv::Mat vt_e;  // 우측 특이벡터 전치 (V^T) — 회전(rotation) 복원에 사용 (R = U·W·V^T)
    cv::SVD::compute(E_raw, w_e, u_e, vt_e);
    w_e.at<double>(2) = 0.0;
    double avg_sv = (w_e.at<double>(0) + w_e.at<double>(1)) / 2.0;
    w_e.at<double>(0) = avg_sv;
    w_e.at<double>(1) = avg_sv;
    cv::Mat E = u_e * cv::Mat::diag(w_e) * vt_e;

    std::cout << "Essential Matrix E (8-Point):" << std::endl;
    std::cout << E << std::endl;

    // ✅ 정답 TODO 2: E에서 R, t 복원 (SVD 분해 + Cheirality 검증)
    // [OpenCV] cv::Mat R, t;
    // [OpenCV] int inliers = cv::recoverPose(E, points1, points2, K, R, t);

    // 2-1. E에서 R, t 후보 생성
    //
    //   E = U · diag(σ, σ, 0) · V^T 로 분해되어 있다 (1-4에서 이미 수행).
    //
    //   [배경] E = [t]× · R 이므로, E의 SVD에서 R과 t를 역으로 분리해야 한다.
    //   skew-symmetric 행렬 [t]×의 SVD 구조에는 Z축 90도 회전이 자연스럽게 등장한다:
    //
    //     [t]× = U · diag(σ,σ,0) · Z · U^T
    //     Z = [0 1 0; -1 0 0; 0 0 0]  (반대칭 행렬)
    //
    //   W(Z축 +90도 회전)와 Z의 관계:
    //     W  = [0 -1 0; 1 0 0; 0 0 1]
    //     W^T= [0  1 0;-1 0 0; 0 0 1]
    //     Z는 W^T에서 (3,3)만 0으로 바뀐 것이다.
    //     diag(σ,σ,0) · Z = diag(σ,σ,0) · W^T  (3번째 행이 0이므로 (3,3) 차이 무관)
    //
    //   이 관계를 이용하면 E에서 R과 t를 분리할 수 있다:
    //     [t]× 부분 = U · diag(σ,σ,0) · W · U^T
    //     R 부분    = U · W^T · V^T
    //
    //   [부호 모호성] W/W^T 선택과 t 부호에서 2×2 = 4가지 후보 발생:
    //     R 후보: R1 = U · W · V^T,  R2 = U · W^T · V^T  (2가지)
    //     t 후보: t1 = +U의 3번째 열,  t2 = -U의 3번째 열  (2가지)
    //     → 총 4가지 (R,t) 조합
    //
    //   수학적으로 E를 만족하는 (R,t) 조합은 4개이지만,
    //   물리적으로 의미 있는 것은 1개뿐이다.
    //   나머지 3개는 3D 점이 카메라 뒤에 위치하는 비현실적 경우이다.
    //   → cheirality 검증: 삼각측량한 점이 두 카메라 모두의 앞에 있는 조합 1개를 선택.
    cv::Mat W = (cv::Mat_<double>(3, 3) << 0, -1, 0, 1, 0, 0, 0, 0, 1);

    cv::Mat R1 = u_e * W * vt_e;
    cv::Mat R2 = u_e * W.t() * vt_e;
    cv::Mat t1 = u_e.col(2).clone();
    cv::Mat t2 = -t1;

    // 회전 행렬은 det(R) = +1 이어야 한다 (정회전).
    // det = -1이면 거울 반사(reflection)이므로 부호를 반전시킨다.
    if (cv::determinant(R1) < 0) R1 = -R1;
    if (cv::determinant(R2) < 0) R2 = -R2;

    // 2-2. Cheirality 검증
    //
    //   4가지 (R,t) 조합 중 "물리적으로 맞는 해"를 고르는 과정.
    //   기준: 삼각측량으로 복원한 3D 점이 두 카메라 모두의 앞에 있어야 한다 (Z > 0).
    //
    //   틀린 해의 예: t 방향이 반대이면 3D 점이 카메라 뒤에 놓임 → Z < 0
    //   → 4개 중 Z > 0인 점이 가장 많은 조합 = 정답
    std::vector<std::pair<cv::Mat, cv::Mat>> candidates = {
        {R1, t1}, {R1, t2}, {R2, t1}, {R2, t2}
    };

    cv::Mat best_R, best_t;
    int best_count = 0;

    for (const auto& [Rc, tc] : candidates)
    {
        int positive_depth_count = 0;
        for (int i = 0; i < N; i++)
        {
            cv::Mat p1 = (cv::Mat_<double>(3, 1) << norm_pts1[i].x, norm_pts1[i].y, 1.0);
            cv::Mat p2 = (cv::Mat_<double>(3, 1) << norm_pts2[i].x, norm_pts2[i].y, 1.0);

            // 삼각측량: 두 카메라의 광선이 만나는 3D 점 X를 구한다.
            //
            //   카메라1: 원점(O1)에서 p1 방향으로 광선 → X = λ · p1
            //   카메라2: O2에서 p2 방향으로 광선      → X = Rc^T · (μ · p2 - tc)
            //
            //   투영 방정식을 전개하면 선형 시스템 A·X = b 형태로 정리:
            //     카메라1: x1 = X/Z, y1 = Y/Z  →  X - x1·Z = 0,  Y - y1·Z = 0
            //     카메라2: x2 = (R행1·X + tx) / (R행3·X + tz)  →  전개 후 정리
            cv::Mat A_tri(4, 3, CV_64F);
            cv::Mat b_tri(4, 1, CV_64F);

            // 카메라1 방정식 (2행)
            A_tri.at<double>(0, 0) = 1;  A_tri.at<double>(0, 1) = 0;  A_tri.at<double>(0, 2) = -p1.at<double>(0);
            A_tri.at<double>(1, 0) = 0;  A_tri.at<double>(1, 1) = 1;  A_tri.at<double>(1, 2) = -p1.at<double>(1);

            // 카메라2 방정식 (2행): 교차곱 전개로 분모 제거
            cv::Mat row2 = Rc.row(0) - p2.at<double>(0) * Rc.row(2);
            cv::Mat row3 = Rc.row(1) - p2.at<double>(1) * Rc.row(2);
            row2.copyTo(A_tri.row(2));
            row3.copyTo(A_tri.row(3));

            b_tri.at<double>(0) = 0;
            b_tri.at<double>(1) = 0;
            b_tri.at<double>(2) = -(tc.at<double>(0) - p2.at<double>(0) * tc.at<double>(2));
            b_tri.at<double>(3) = -(tc.at<double>(1) - p2.at<double>(1) * tc.at<double>(2));

            // 최소제곱 해 (과결정 시스템 → SVD로 풀기)
            cv::Mat X;
            cv::solve(A_tri, b_tri, X, cv::DECOMP_SVD);

            // 깊이 부호 확인
            double z1 = X.at<double>(2);                     // 카메라1 기준 깊이
            cv::Mat X_cam2 = Rc * X + tc;                    // 카메라2 좌표계로 변환
            double z2 = X_cam2.at<double>(2);                // 카메라2 기준 깊이

            // 두 카메라 모두에서 물체가 앞에 있어야 유효
            if (z1 > 0 && z2 > 0)
                positive_depth_count++;
        }

        if (positive_depth_count > best_count)
        {
            best_count = positive_depth_count;
            best_R = Rc.clone();
            best_t = tc.clone();
        }
    }

    // t를 단위 벡터로 정규화
    // Essential Matrix에서 복원한 t는 방향만 의미 있고, 절대 크기(스케일)는 알 수 없다.
    // (단안 카메라의 근본적 한계 — 스테레오나 IMU 없이는 실제 거리를 모른다)
    best_t = best_t / cv::norm(best_t);

    std::cout << "\nRotation:\n" << best_R << std::endl;
    std::cout << "Translation:\n" << best_t << std::endl;
    std::cout << "Cheirality 통과: " << best_count << " / " << N << "개" << std::endl;

    // OpenCV 결과와 비교
    cv::Mat E_cv = cv::findEssentialMat(points1, points2, K, cv::RANSAC, 0.999, 1.0);
    cv::Mat R_cv, t_cv;
    cv::recoverPose(E_cv, points1, points2, K, R_cv, t_cv);
    std::cout << "\n📊 OpenCV 비교:" << std::endl;
    std::cout << "E (OpenCV):\n" << E_cv << std::endl;
    std::cout << "R (OpenCV):\n" << R_cv << std::endl;
    std::cout << "t (OpenCV):\n" << t_cv << std::endl;

    std::cout << "\n💡 정답 해설:" << std::endl;
    std::cout << "   [TODO 1: 8-Point Algorithm]" << std::endl;
    std::cout << "   1. 픽셀 좌표 → 정규화 좌표 (K^{-1} 적용)" << std::endl;
    std::cout << "   2. 에피폴라 제약 p2^T E p1 = 0 → A행렬(N×9) 구성" << std::endl;
    std::cout << "   3. SVD(A)의 마지막 V행 → 3×3 reshape" << std::endl;
    std::cout << "   4. rank-2 제약: 특이값을 (σ, σ, 0)으로 강제" << std::endl;
    std::cout << std::endl;
    std::cout << "   [TODO 2: E → R, t 분해]" << std::endl;
    std::cout << "   1. E = U diag(1,1,0) V^T 에서 W 행렬로 R 후보 2개 생성" << std::endl;
    std::cout << "   2. t 후보 = ±U의 3번째 열 → 총 4가지 (R,t) 조합" << std::endl;
    std::cout << "   3. 각 조합으로 삼각측량 → Z>0인 점이 가장 많은 해 선택" << std::endl;
    std::cout << "   4. t는 단위 벡터 (스케일 미지)" << std::endl;
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
    int n = static_cast<int>(src.size());
    if (n < 4) return cv::Mat();

    const int kHomographyParams = 3 * 3;  // H 행렬(3×3)의 원소 수
    cv::Mat A = cv::Mat::zeros(2 * n, kHomographyParams, CV_64F);
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
