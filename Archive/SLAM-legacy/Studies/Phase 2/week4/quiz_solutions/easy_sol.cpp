/**
 * Phase 2 Week 4 - 특징점 매칭 기초 퀴즈 (정답)
 *
 * 매칭 알고리즘의 기본 개념을 확인합니다.
 *
 * 다루는 개념:
 *   1. 해밍 거리 vs 유클리드 거리 — 디스크립터 유형별 거리 측정
 *   2. Lowe's Ratio Test — 모호한 매칭 제거
 *   3. RANSAC 반복 횟수 — 아웃라이어 비율에 따른 계산
 *   4. Cross-Check 매칭 — 양방향 일치 검증
 *   5. Homography 변환 — 동차좌표와 사영 변환
 *
 * 매칭 파이프라인:
 *   특징점 검출 → 디스크립터 추출 → 거리 계산 → Ratio Test → RANSAC → 최종 매칭
 *                                    ↑              ↑           ↑
 *                              문제1 (거리)    문제2 (필터)   문제3 (추정)
 *
 * 선수 지식: Phase 2 Week 3 (특징점 검출, ORB/SIFT 디스크립터)
 * 난이도: ★★☆☆☆
 */

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <cmath>

// 문제 1: 해밍 거리 vs 유클리드 거리 — 디스크립터 유형에 따른 거리 측정
//
// ★ 핵심: 디스크립터의 데이터 타입이 거리 함수를 결정한다
//
// 이진 디스크립터 (ORB, BRIEF, BRISK):
//   해밍 거리 = XOR 후 1의 개수
//   예: 10110011 XOR 10110001 = 00000010 → 해밍 거리 = 1
//   CPU의 popcount 명령어로 매우 빠름 (수 나노초)
//
// 실수 디스크립터 (SIFT, SURF):
//   유클리드(L2) 거리 = sqrt(Σ(a_i - b_i)²)
//   128/256차원 벡터의 거리 → 해밍보다 느리지만 더 정밀
//
// OpenCV에서:
//   cv::norm(desc1, desc2, NORM_HAMMING)  ← 이진용
//   cv::norm(desc1, desc2, NORM_L2)       ← 실수용
//
// SLAM 관점: ORB-SLAM은 해밍 거리의 속도 이점을 활용하여 실시간 처리
void problem1_distance_metrics()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 거리 측정 방식" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ORB 디스크립터 (이진) — 각 바이트가 8비트의 이진 패턴
    cv::Mat orb_desc1 = (cv::Mat_<uchar>(1, 4) << 0b10110011, 0b11001100, 0b00111100, 0b11110000);
    cv::Mat orb_desc2 = (cv::Mat_<uchar>(1, 4) << 0b10110001, 0b11001100, 0b00111000, 0b11110000);

    // [OpenCV] int hamming_dist = static_cast<int>(cv::norm(orb_desc1, orb_desc2, cv::NORM_HAMMING));
    // ✅ 정답: 해밍 거리 계산 — XOR 후 1인 비트 개수를 직접 센다
    int hamming_dist = 0;
    for (int i = 0; i < orb_desc1.cols; i++)
    {
        uint8_t xor_val = orb_desc1.at<uint8_t>(0, i) ^ orb_desc2.at<uint8_t>(0, i);
        while (xor_val)
        {
            hamming_dist += xor_val & 1;
            xor_val >>= 1;
        }
    }

    std::cout << "ORB 디스크립터 (이진):" << std::endl;
    std::cout << "   디스크립터1: " << orb_desc1 << std::endl;
    std::cout << "   디스크립터2: " << orb_desc2 << std::endl;
    std::cout << "   해밍 거리: " << hamming_dist << " bits\n" << std::endl;

    // SIFT 디스크립터 (실수) — 128차원 float 벡터 (여기서는 4차원 예시)
    cv::Mat sift_desc1 = (cv::Mat_<float>(1, 4) << 0.5, 0.8, 0.3, 0.9);
    cv::Mat sift_desc2 = (cv::Mat_<float>(1, 4) << 0.6, 0.7, 0.4, 0.8);

    // [OpenCV] double euclidean_dist = cv::norm(sift_desc1, sift_desc2, cv::NORM_L2);
    // ✅ 정답: 유클리드 거리 계산 — sqrt(Σ(a_i - b_i)²) 직접 계산
    double sum_sq = 0.0;
    for (int i = 0; i < sift_desc1.cols; i++)
    {
        double diff = sift_desc1.at<float>(0, i) - sift_desc2.at<float>(0, i);
        sum_sq += diff * diff;
    }
    double euclidean_dist = std::sqrt(sum_sq);

    std::cout << "SIFT 디스크립터 (실수):" << std::endl;
    std::cout << "   디스크립터1: " << sift_desc1 << std::endl;
    std::cout << "   디스크립터2: " << sift_desc2 << std::endl;
    std::cout << "   유클리드 거리: " << euclidean_dist << "\n" << std::endl;

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   Q1. 왜 ORB는 해밍 거리를 사용하나요?" << std::endl;
    std::cout << "   A1. ORB는 이진 디스크립터(0과 1로 구성)이기 때문입니다." << std::endl;
    std::cout << "       이진 데이터의 차이는 XOR 연산 → popcount로 측정하는 것이" << std::endl;
    std::cout << "       가장 자연스럽고 효율적입니다." << std::endl;
    std::cout << "       유클리드 거리는 실수 벡터(SIFT 등)에 적합합니다." << std::endl;
    std::cout << std::endl;
    std::cout << "   Q2. 어떤 것이 더 빠른가요?" << std::endl;
    std::cout << "   A2. 해밍 거리가 훨씬 빠릅니다." << std::endl;
    std::cout << "       - 해밍: XOR + popcount → CPU 1~2 사이클 (수 나노초)" << std::endl;
    std::cout << "       - 유클리드: 뺄셈 + 제곱 + 합산 + 루트 → 수십 사이클" << std::endl;
    std::cout << "       ORB-SLAM이 실시간 처리 가능한 핵심 이유 중 하나입니다." << std::endl;
    std::cout << std::endl;
    std::cout << "   [이 코드의 실행 결과]" << std::endl;
    std::cout << "   해밍 거리 = 2 bits (바이트1: bit1, 바이트3: bit2 차이)" << std::endl;
    std::cout << "   유클리드 거리 ≈ 0.2 (4차원 float 벡터 간 L2 거리)" << std::endl;
}

// 문제 2: Lowe's Ratio Test — 최근접/차근접 비율로 모호한 매칭 제거
//
// ★ 핵심: "확실한 매칭"은 최근접이 차근접보다 훨씬 가까움
//
// 원리:
//   knnMatch(k=2)로 최근접 2개를 찾은 뒤,
//   ratio = d(best) / d(second_best)
//   ratio < threshold(보통 0.7) → 수락, 아니면 → 거절
//
// 직관:
//   ratio 낮음 (0.3): 최근접이 확실히 가까움 → 신뢰할 수 있는 매칭
//       best ●──                  ───● second
//            25                      80
//
//   ratio 높음 (0.92): 두 후보가 비슷 → 어느 것이 진짜인지 모호
//       best ●──────             ────● second
//            60                      65
//
// Lowe (SIFT 논문, 2004)는 0.7~0.8을 권장
// SLAM에서: Ratio Test → RANSAC 전에 대부분의 오매칭을 제거
void problem2_ratio_test()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Lowe's Ratio Test" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 시나리오 1: 명확한 매칭
    float distance_best1 = 25.0f;
    float distance_second1 = 80.0f;
    float ratio1 = distance_best1 / distance_second1;

    std::cout << "시나리오 1 (명확한 매칭):" << std::endl;
    std::cout << "   최근접 거리: " << distance_best1 << std::endl;
    std::cout << "   차근접 거리: " << distance_second1 << std::endl;
    std::cout << "   비율: " << ratio1 << std::endl;

    // ✅ 정답: 비율 테스트로 수락/거절 판단
    bool accept1 = (ratio1 < 0.7f);
    std::cout << "   결과: " << (accept1 ? "✅ 수락" : "❌ 거절") << "\n" << std::endl;

    // 시나리오 2: 모호한 매칭
    float distance_best2 = 60.0f;
    float distance_second2 = 65.0f;
    float ratio2 = distance_best2 / distance_second2;

    std::cout << "시나리오 2 (모호한 매칭):" << std::endl;
    std::cout << "   최근접 거리: " << distance_best2 << std::endl;
    std::cout << "   차근접 거리: " << distance_second2 << std::endl;
    std::cout << "   비율: " << ratio2 << std::endl;

    // ✅ 정답: 비율 테스트로 수락/거절 판단
    bool accept2 = (ratio2 < 0.7f);
    std::cout << "   결과: " << (accept2 ? "✅ 수락" : "❌ 거절") << "\n" << std::endl;

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [코드 핵심] ratio = best거리 / second_best거리" << std::endl;
    std::cout << "   ratio < 0.7이면 수락, 아니면 거절" << std::endl;
    std::cout << std::endl;
    std::cout << "   [실행 결과]" << std::endl;
    std::cout << "   시나리오 1: 25/80 = 0.3125 < 0.7 → ✅ 수락 (명확한 매칭)" << std::endl;
    std::cout << "   시나리오 2: 60/65 = 0.923  ≥ 0.7 → ❌ 거절 (모호한 매칭)" << std::endl;
    std::cout << std::endl;
    std::cout << "   [왜 효과적인가?]" << std::endl;
    std::cout << "   진짜 매칭: 최근접이 확실히 가까움 → ratio 낮음" << std::endl;
    std::cout << "   가짜 매칭: 여러 후보가 비슷한 거리 → ratio 높음" << std::endl;
    std::cout << "   → ratio로 모호한 매칭을 걸러내어 RANSAC 전에 오매칭 80%+ 제거" << std::endl;
}

// 문제 3: RANSAC 반복 횟수 — 아웃라이어 비율에 따른 최소 반복 계산
//
// ★ 핵심 공식:
//   N = log(1 - p) / log(1 - w^s)
//
//   p: 성공 확률 (보통 0.99 = "99% 확신")
//   w: inlier 비율 (전체 중 올바른 매칭의 비율)
//   s: 모델 추정에 필요한 최소 점 개수
//
// 직관:
//   한 번의 샘플링에서 s개 점이 모두 inlier일 확률 = w^s
//   s개 중 하나라도 outlier일 확률 = 1 - w^s
//   N번 시도 후에도 한 번도 성공하지 못할 확률 = (1 - w^s)^N
//   이것이 (1 - p) 이하가 되길 원함 → N 유도
//
// 예시 (s=4, p=0.99):
//   w=0.5 → N=72,  w=0.7 → N=16,  w=0.9 → N=5
//   → inlier 비율이 높을수록 적은 반복으로 충분!
//
// SLAM 관점: Ratio Test로 inlier 비율을 높이면 RANSAC이 빨라짐
void problem3_ransac_iterations()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: RANSAC 반복 횟수" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // RANSAC 공식의 각 파라미터
    // p: 성공 확률 (99% → 거의 확실히 올바른 모델을 찾음)
    // w: inlier 비율 (매칭 품질에 따라 달라짐)
    // s: 최소 점 개수 (호모그래피: 4, Essential: 5, Fundamental: 8)

    double p = 0.99;  // 99% 성공 확률
    int s = 4;        // 호모그래피는 4개 점 필요

    std::vector<double> inlier_ratios = {0.5, 0.7, 0.9};

    std::cout << "호모그래피 추정 (최소 4개 점 필요):\n" << std::endl;

    for (double w : inlier_ratios)
    {
        // ✅ 정답: 필요 반복 횟수 계산
        double N = std::ceil(std::log(1 - p) / std::log(1 - std::pow(w, s)));

        std::cout << "Inlier 비율 " << (int)(w * 100) << "%:" << std::endl;
        std::cout << "   필요 반복: " << (int)N << "회\n" << std::endl;
    }

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [실행 결과] N = ceil(log(1-0.99) / log(1-w^4))" << std::endl;
    std::cout << "   Inlier 50%: N = 72회 (절반이 outlier → 많은 시도 필요)" << std::endl;
    std::cout << "   Inlier 70%: N = 16회 (양호한 매칭 품질)" << std::endl;
    std::cout << "   Inlier 90%: N =  5회 (고품질 매칭 → 소수 반복으로 충분)" << std::endl;
    std::cout << std::endl;
    std::cout << "   [핵심 포인트]" << std::endl;
    std::cout << "   공식의 직관: 4개를 뽑았을 때 전부 inlier일 확률 = w^4" << std::endl;
    std::cout << "   w=0.5이면 0.5^4 = 6.25% → 한 번에 성공할 확률이 매우 낮음" << std::endl;
    std::cout << "   w=0.9이면 0.9^4 = 65.6% → 몇 번이면 성공" << std::endl;
    std::cout << "   → Ratio Test로 먼저 outlier를 걸러 w를 높이면 RANSAC이 빨라짐!" << std::endl;
}

// 문제 4: Cross-Check 매칭 — 양방향 일치로 신뢰도 향상
//
// ★ 핵심: A→B 매칭과 B→A 매칭이 서로 일치하는 것만 유지
//
// 원리:
//   1. A→B 매칭: A의 특징점 i → B의 특징점 j (A 기준 최근접)
//   2. B→A 매칭: B의 특징점 j → A의 특징점 k (B 기준 최근접)
//   3. i == k 이면 → 양방향 일치 → 신뢰도 높음 ✅
//      i != k 이면 → 일방적 매칭 → 제거 ❌
//
//   A              B
//   ● i ─────→ ● j    (A→B: i→j)
//   ● i ←───── ● j    (B→A: j→i)  → 양방향 일치!
//
//   ● i ─────→ ● j    (A→B: i→j)
//   ● k ←───── ● j    (B→A: j→k, k≠i)  → 불일치, 제거
//
// OpenCV: BFMatcher(normType, crossCheck=true)
// 단점: 2배 느림 (양방향 매칭 필요), Ratio Test와 동시 사용 불가
void problem4_cross_check()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Cross-Check 매칭" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "시나리오:" << std::endl;
    std::cout << "   A → B 매칭: A의 특징점 0 → B의 특징점 3" << std::endl;
    std::cout << "   B → A 매칭: B의 특징점 3 → A의 특징점 0\n" << std::endl;

    // ✅ 정답: Cross-check 통과 (A→B와 B→A가 서로 같은 점을 가리킴)
    bool is_cross_check_pass = true;

    std::cout << "결과: " << (is_cross_check_pass ? "✅ 통과" : "❌ 실패") << "\n" << std::endl;

    std::cout << "💡 정답 해설:" << std::endl;
    std::cout << "   [정답] Cross-Check ✅ 통과!" << std::endl;
    std::cout << "   A→B: 특징점 0 → 특징점 3" << std::endl;
    std::cout << "   B→A: 특징점 3 → 특징점 0 (원래 점으로 되돌아옴)" << std::endl;
    std::cout << "   → 양방향 일치 = 신뢰할 수 있는 매칭" << std::endl;
    std::cout << std::endl;
    std::cout << "   [통과하지 못하는 예시]" << std::endl;
    std::cout << "   A→B: 특징점 0 → 특징점 3" << std::endl;
    std::cout << "   B→A: 특징점 3 → 특징점 2 (다른 점! 0이 아님)" << std::endl;
    std::cout << "   → 불일치 = 일방적 매칭이므로 제거" << std::endl;
    std::cout << std::endl;
    std::cout << "   [Ratio Test와의 차이]" << std::endl;
    std::cout << "   Ratio Test: 거리 비율로 모호함을 판단 (knnMatch 필요)" << std::endl;
    std::cout << "   Cross-Check: 양방향 일치 여부로 판단 (match 2회 필요)" << std::endl;
    std::cout << "   → 동시 사용 불가! OpenCV에서는 둘 중 하나 선택" << std::endl;
}

// 문제 5: Homography 변환 이해 — 평면 간 사영 변환
//
// ★ 핵심: Homography H는 한 평면의 점을 다른 평면으로 매핑하는 3x3 행렬
//
// 수식:
//   s * [u]   [h11 h12 h13] [x]
//       [v] = [h21 h22 h23] [y]
//       [1]   [h31 h32 h33] [1]
//
//   정규화: u = (h11·x + h12·y + h13) / (h31·x + h32·y + h33)
//           v = (h21·x + h22·y + h23) / (h31·x + h32·y + h33)
//
// 자유도(DOF): 8 (h33=1로 정규화하면 8개 미지수)
// 최소 점 수: 4 (각 점이 u, v 2개 방정식 제공 → 4점 = 8방정식)
//
// 변환 단계:
//   1. 동차좌표 생성: [x, y] → [x, y, 1]^T
//   2. 행렬 곱: p' = H * [x, y, 1]^T → [x', y', w]^T
//   3. 정규화: u = x'/w, v = y'/w (동차좌표 → 유클리드좌표)
//
// SLAM 관점: 평면 장면에서 두 뷰 간 변환, 이미지 스티칭, AR
void problem5_homography_understanding()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: Homography 변환 이해" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Homography = 평면 → 평면 변환 (사영 변환)" << std::endl;
    std::cout << "   - 8 DOF (자유도): h33=1로 정규화하면 8개 미지수" << std::endl;
    std::cout << "   - 최소 4점 필요 (각 점이 2개 방정식 제공)\n" << std::endl;

    // Homography 행렬 구성 (회전 15° + 이동 + 스케일 1.1)
    // h31=h32=0이므로 아핀 변환 (사영 성분 없음)
    double theta = 15.0 * CV_PI / 180.0;
    double scale = 1.1;
    double tx = 50.0, ty = 30.0;

    cv::Mat H = (cv::Mat_<double>(3, 3) <<
        scale * cos(theta), -scale * sin(theta), tx,
        scale * sin(theta),  scale * cos(theta), ty,
        0.0,                 0.0,                1.0);

    std::cout << "Homography H:" << std::endl;
    std::cout << H << "\n" << std::endl;

    // 변환할 점들 (사각형의 4개 꼭짓점)
    std::vector<cv::Point2d> src_pts = {
        {100.0, 100.0}, {200.0, 100.0}, {200.0, 200.0}, {100.0, 200.0}
    };

    std::cout << "📊 점 변환 결과:" << std::endl;
    for (size_t i = 0; i < src_pts.size(); i++)
    {
        // ✅ 정답: 동차좌표로 변환 후 H 적용
        double x = src_pts[i].x;
        double y = src_pts[i].y;

        cv::Mat p = (cv::Mat_<double>(3, 1) << x, y, 1.0);
        cv::Mat p_prime = H * p;
        double dst_x = p_prime.at<double>(0) / p_prime.at<double>(2);
        double dst_y = p_prime.at<double>(1) / p_prime.at<double>(2);

        std::cout << "   (" << x << ", " << y << ") → ("
                  << dst_x << ", " << dst_y << ")" << std::endl;
    }

    // OpenCV perspectiveTransform으로 검증
    std::vector<cv::Point2d> dst_pts;
    cv::perspectiveTransform(src_pts, dst_pts, H);

    std::cout << "\n📊 OpenCV perspectiveTransform 검증:" << std::endl;
    for (size_t i = 0; i < dst_pts.size(); i++)
    {
        std::cout << "   (" << src_pts[i].x << ", " << src_pts[i].y << ") → ("
                  << dst_pts[i].x << ", " << dst_pts[i].y << ")" << std::endl;
    }

    std::cout << "\n💡 정답 해설:" << std::endl;
    std::cout << "   [코드 핵심] Homography 변환의 3단계:" << std::endl;
    std::cout << "   1. 동차좌표 생성: (x, y) → [x, y, 1]^T" << std::endl;
    std::cout << "   2. 행렬 곱: [x', y', w]^T = H * [x, y, 1]^T" << std::endl;
    std::cout << "   3. 정규화: u = x'/w, v = y'/w (w로 나누어 유클리드 좌표 복원)" << std::endl;
    std::cout << std::endl;
    std::cout << "   [왜 w로 나누나?]" << std::endl;
    std::cout << "   Homography는 사영 변환 → 결과가 동차좌표로 나옴" << std::endl;
    std::cout << "   h31=h32=0이면 w=1 (아핀 변환), 0이 아니면 w≠1 (원근 변환)" << std::endl;
    std::cout << "   이 예제는 h31=h32=0이므로 아핀이지만, 정규화 코드는 항상 필요" << std::endl;
    std::cout << std::endl;
    std::cout << "   [SLAM 응용]" << std::endl;
    std::cout << "   평면 물체 인식, 이미지 스티칭(파노라마), AR 마커 추적에 사용" << std::endl;
    std::cout << "   최소 4점이면 H를 유일하게 결정 (DLT 알고리즘)" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 4 Quiz - Easy (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_distance_metrics();
    problem2_ratio_test();
    problem3_ransac_iterations();
    problem4_cross_check();
    problem5_homography_understanding();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
