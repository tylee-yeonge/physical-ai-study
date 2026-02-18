/**
 * Phase 2 Week 4 - 특징점 매칭 기초 퀴즈
 *
 * 매칭 알고리즘의 기본 개념을 확인합니다.
 */

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <cmath>

/**
 * 문제 1: 해밍 거리 vs 유클리드 거리
 *
 * ORB와 SIFT 디스크립터에 각각 어떤 거리 측정을 사용하는지 이해하세요.
 */
void problem1_distance_metrics()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 거리 측정 방식" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ORB 디스크립터 (이진)
    cv::Mat orb_desc1 = (cv::Mat_<uchar>(1, 4) << 0b10110011, 0b11001100, 0b00111100, 0b11110000);
    cv::Mat orb_desc2 = (cv::Mat_<uchar>(1, 4) << 0b10110001, 0b11001100, 0b00111000, 0b11110000);

    // TODO: 해밍 거리 계산
    // int hamming_dist = cv::norm(orb_desc1, orb_desc2, cv::NORM_HAMMING);
    int hamming_dist = 0;  // TODO

    std::cout << "ORB 디스크립터 (이진):" << std::endl;
    std::cout << "   디스크립터1: " << orb_desc1 << std::endl;
    std::cout << "   디스크립터2: " << orb_desc2 << std::endl;
    std::cout << "   해밍 거리: " << hamming_dist << " bits\n" << std::endl;

    // SIFT 디스크립터 (실수)
    cv::Mat sift_desc1 = (cv::Mat_<float>(1, 4) << 0.5, 0.8, 0.3, 0.9);
    cv::Mat sift_desc2 = (cv::Mat_<float>(1, 4) << 0.6, 0.7, 0.4, 0.8);

    // TODO: 유클리드 거리 계산
    // double euclidean_dist = cv::norm(sift_desc1, sift_desc2, cv::NORM_L2);
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

/**
 * 문제 2: Lowe's Ratio Test 원리
 *
 * 최근접/차근접 거리 비율로 좋은 매칭을 선별하는 원리를 이해하세요.
 */
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

    // TODO: 비율이 0.7 이하인가?
    bool accept1 = false;  // TODO: ratio1 < 0.7
    std::cout << "   결과: " << (accept1 ? "✅ 수락" : "❌ 거절") << "\n" << std::endl;

    // 시나리오 2: 모호한 매칭
    float distance_best2 = 60.0f;
    float distance_second2 = 65.0f;
    float ratio2 = distance_best2 / distance_second2;

    std::cout << "시나리오 2 (모호한 매칭):" << std::endl;
    std::cout << "   최근접 거리: " << distance_best2 << std::endl;
    std::cout << "   차근접 거리: " << distance_second2 << std::endl;
    std::cout << "   비율: " << ratio2 << std::endl;

    // TODO: 비율이 0.7 이하인가?
    bool accept2 = false;  // TODO
    std::cout << "   결과: " << (accept2 ? "✅ 수락" : "❌ 거절") << "\n" << std::endl;

    std::cout << "💡 원리:" << std::endl;
    std::cout << "   - 비율 낮음(< 0.7): 최근접이 확실히 가까움 → 좋은 매칭" << std::endl;
    std::cout << "   - 비율 높음(≥ 0.7): 차이 없음 → 모호함 → 거절" << std::endl;
    std::cout << "   - outlier 제거에 매우 효과적!" << std::endl;
}

/**
 * 문제 3: RANSAC 반복 횟수
 *
 * Outlier 비율에 따른 필요 반복 횟수를 이해하세요.
 */
void problem3_ransac_iterations()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: RANSAC 반복 횟수" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // RANSAC 공식: N = log(1-p) / log(1-w^s)
    // p: 성공 확률 (보통 0.99)
    // w: inlier 비율
    // s: 모델 추정에 필요한 최소 점 개수 (호모그래피: 4개)

    double p = 0.99;  // 99% 성공 확률
    int s = 4;        // 호모그래피는 4개 점 필요

    std::vector<double> inlier_ratios = {0.5, 0.7, 0.9};

    std::cout << "호모그래피 추정 (최소 4개 점 필요):\n" << std::endl;

    for (double w : inlier_ratios)
    {
        // TODO: 필요 반복 횟수 계산
        // N = log(1-p) / log(1-w^s)
        double N = 0;  // TODO

        std::cout << "Inlier 비율 " << (int)(w * 100) << "%:" << std::endl;
        std::cout << "   필요 반복: " << (int)N << "회\n" << std::endl;
    }

    std::cout << "💡 결론:" << std::endl;
    std::cout << "   - Inlier 많을수록 → 반복 적게 필요" << std::endl;
    std::cout << "   - Outlier 많으면 → 반복 많이 필요" << std::endl;
    std::cout << "   - Ratio Test로 미리 정제하면 효율적!" << std::endl;
}

/**
 * 문제 4: Cross-Check 매칭
 *
 * 양방향 매칭으로 신뢰도를 높이는 방법을 이해하세요.
 */
void problem4_cross_check()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: Cross-Check 매칭" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "시나리오:" << std::endl;
    std::cout << "   A → B 매칭: A의 특징점 0 → B의 특징점 3" << std::endl;
    std::cout << "   B → A 매칭: B의 특징점 3 → A의 특징점 0\n" << std::endl;

    // TODO: Cross-check 통과?
    bool is_cross_check_pass = false;  // TODO: 양방향이 일치하는가?

    std::cout << "결과: " << (is_cross_check_pass ? "✅ 통과" : "❌ 실패") << "\n" << std::endl;

    std::cout << "💡 Cross-Check:" << std::endl;
    std::cout << "   - A→B와 B→A가 서로 일치하는 매칭만 선택" << std::endl;
    std::cout << "   - 더 강한 필터링" << std::endl;
    std::cout << "   - OpenCV: crossCheck=true 옵션" << std::endl;
}

/**
 * @brief Homography 변환 이해 (DOF 8, 최소 4점)
 *
 * 주어진 3x3 Homography 행렬 H로 점을 변환하고 결과를 검증하세요.
 * 동차좌표 변환 후 정규화 과정을 이해합니다.
 */
void problem5_homography_understanding()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: Homography 변환 이해" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Homography = 평면 → 평면 변환 (사영 변환)" << std::endl;
    std::cout << "   - 8 DOF (자유도): h33=1로 정규화하면 8개 미지수" << std::endl;
    std::cout << "   - 최소 4점 필요 (각 점이 2개 방정식 제공)\n" << std::endl;

    // Homography 행렬 (회전 15도 + 이동 + 약간의 스케일)
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
        // 1. [x, y, 1]^T 동차좌표 벡터 생성
        // 2. H * [x, y, 1]^T 행렬 곱셈
        // 3. 결과를 w(3번째 원소)로 나누어 정규화
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
