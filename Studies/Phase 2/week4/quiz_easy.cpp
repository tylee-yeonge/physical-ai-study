/**
 * Phase 2 Week 4 - 특징점 매칭 기초 퀴즈
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

    // TODO: 해밍 거리 계산
    int hamming_dist = 0;  // TODO

    std::cout << "ORB 디스크립터 (이진):" << std::endl;
    std::cout << "   디스크립터1: " << orb_desc1 << std::endl;
    std::cout << "   디스크립터2: " << orb_desc2 << std::endl;
    std::cout << "   해밍 거리: " << hamming_dist << " bits\n" << std::endl;

    // SIFT 디스크립터 (실수) — 128차원 float 벡터 (여기서는 4차원 예시)
    cv::Mat sift_desc1 = (cv::Mat_<float>(1, 4) << 0.5, 0.8, 0.3, 0.9);
    cv::Mat sift_desc2 = (cv::Mat_<float>(1, 4) << 0.6, 0.7, 0.4, 0.8);

    // TODO: 유클리드 거리 계산
    double euclidean_dist = 0.0;  // TODO

    std::cout << "SIFT 디스크립터 (실수):" << std::endl;
    std::cout << "   디스크립터1: " << sift_desc1 << std::endl;
    std::cout << "   디스크립터2: " << sift_desc2 << std::endl;
    std::cout << "   유클리드 거리: " << euclidean_dist << "\n" << std::endl;

    std::cout << "💡 질문:" << std::endl;
    std::cout << "   1. 왜 ORB는 해밍 거리를 사용하나요?" << std::endl;
    std::cout << "   2. 어떤 것이 더 빠른가요?" << std::endl;
    std::cout << "\n힌트: 이진 디스크립터 → XOR 연산 → 매우 빠름" << std::endl;
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

    // TODO: 비율 테스트로 수락/거절 판단
    bool accept1 = false;  // TODO
    std::cout << "   결과: " << (accept1 ? "✅ 수락" : "❌ 거절") << "\n" << std::endl;

    // 시나리오 2: 모호한 매칭
    float distance_best2 = 60.0f;
    float distance_second2 = 65.0f;
    float ratio2 = distance_best2 / distance_second2;

    std::cout << "시나리오 2 (모호한 매칭):" << std::endl;
    std::cout << "   최근접 거리: " << distance_best2 << std::endl;
    std::cout << "   차근접 거리: " << distance_second2 << std::endl;
    std::cout << "   비율: " << ratio2 << std::endl;

    // TODO: 비율 테스트로 수락/거절 판단
    bool accept2 = false;  // TODO
    std::cout << "   결과: " << (accept2 ? "✅ 수락" : "❌ 거절") << "\n" << std::endl;

    std::cout << "💡 원리:" << std::endl;
    std::cout << "   - 비율 낮음(< 0.7): 최근접이 확실히 가까움 → 좋은 매칭" << std::endl;
    std::cout << "   - 비율 높음(≥ 0.7): 차이 없음 → 모호함 → 거절" << std::endl;
    std::cout << "   - outlier 제거에 매우 효과적!" << std::endl;
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
        // TODO: 필요 반복 횟수 계산
        double N = 0;  // TODO

        std::cout << "Inlier 비율 " << (int)(w * 100) << "%:" << std::endl;
        std::cout << "   필요 반복: " << (int)N << "회\n" << std::endl;
    }

    std::cout << "💡 결론:" << std::endl;
    std::cout << "   - Inlier 많을수록 → 반복 적게 필요" << std::endl;
    std::cout << "   - Outlier 많으면 → 반복 많이 필요" << std::endl;
    std::cout << "   - Ratio Test로 미리 정제하면 효율적!" << std::endl;
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

    // TODO: Cross-check 통과? (A→B와 B→A가 서로 같은 점을 가리키는가?)
    bool is_cross_check_pass = false;  // TODO: 위 시나리오에서 양방향 일치 여부 판단

    std::cout << "결과: " << (is_cross_check_pass ? "✅ 통과" : "❌ 실패") << "\n" << std::endl;

    std::cout << "💡 Cross-Check:" << std::endl;
    std::cout << "   - A→B와 B→A가 서로 일치하는 매칭만 선택" << std::endl;
    std::cout << "   - 더 강한 필터링" << std::endl;
    std::cout << "   - OpenCV: crossCheck=true 옵션" << std::endl;
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
        // TODO: 동차좌표로 변환 후 H를 적용하세요
        double x = src_pts[i].x;
        double y = src_pts[i].y;

        double dst_x = 0.0, dst_y = 0.0;  // TODO

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

    std::cout << "\n💡 핵심:" << std::endl;
    std::cout << "   - 동차좌표: [x, y] → [x, y, 1]" << std::endl;
    std::cout << "   - 변환: s*[u, v, 1]^T = H * [x, y, 1]^T" << std::endl;
    std::cout << "   - 정규화: u = u'/w, v = v'/w (w = 3번째 원소)" << std::endl;
    std::cout << "   - 4점이면 H를 유일하게 결정 가능 (DLT)" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 4 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_distance_metrics();
    problem2_ratio_test();
    problem3_ransac_iterations();
    problem4_cross_check();
    problem5_homography_understanding();

    std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
