/**
 * Phase 2 Week 4 - 특징점 매칭 중급 퀴즈
 *
 * 실전 응용: 매칭 성능 개선, Essential Matrix 추정
 */

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <chrono>
#include <cmath>
#include <random>

/**
 * 문제 1: 최적의 Ratio 임계값 찾기
 *
 * 다양한 ratio threshold를 시도하여
 * precision-recall 트레이드오프를 관찰하세요.
 */
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
        // TODO: ratio test 수행
        // int matches_count = ...
        // double inlier_ratio = ...

        std::cout << " " << ratio << "  |     ???     |    ???" << std::endl;
    }

    std::cout << "\n💡 관찰:" << std::endl;
    std::cout << "   - Ratio ↓: 매칭 개수 ↓, 품질 ↑ (precision)" << std::endl;
    std::cout << "   - Ratio ↑: 매칭 개수 ↑, 품질 ↓ (recall)" << std::endl;
    std::cout << "   - 0.7은 좋은 균형점 (Lowe 논문)" << std::endl;
}

/**
 * 문제 2: Essential Matrix 추정
 *
 * 매칭된 점들로부터 Essential Matrix를 구하고
 * 카메라 포즈를 복원하세요. (SLAM의 핵심!)
 */
void problem2_essential_matrix()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Essential Matrix 추정" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 행렬 (캘리브레이션 결과)
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // TODO: 매칭된 점들 (실제로는 특징점 매칭 결과)
    std::vector<cv::Point2f> points1, points2;
    // 시뮬레이션 데이터 생성...

    // TODO: Essential Matrix 추정
    /*
    cv::Mat E = cv::findEssentialMat(
        points1, points2, K,
        cv::RANSAC, 0.999, 1.0
    );
    */

    std::cout << "Essential Matrix E:" << std::endl;
    // std::cout << E << "\n" << std::endl;

    // TODO: R, t 복원
    /*
    cv::Mat R, t;
    cv::recoverPose(E, points1, points2, K, R, t);
    */

    std::cout << "💡 SLAM에서의 의미:" << std::endl;
    std::cout << "   - E를 분해 → R (회전), t (이동)" << std::endl;
    std::cout << "   - 두 프레임 간 상대 포즈!" << std::endl;
    std::cout << "   - Visual Odometry의 핵심" << std::endl;
}

/**
 * 문제 3: 매칭 성능 벤치마크
 *
 * BF vs FLANN의 속도와 정확도를 비교하세요.
 */
void problem3_matching_benchmark()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 매칭 성능 벤치마크" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // TODO: 대량의 특징점으로 테스트
    int num_features = 2000;

    std::cout << "특징점 개수: " << num_features << "개\n" << std::endl;

    // TODO: BF 매칭 시간 측정
    // auto start_bf = ...
    // auto end_bf = ...

    // TODO: FLANN 매칭 시간 측정
    // auto start_flann = ...
    // auto end_flann = ...

    std::cout << "매칭 알고리즘  |  시간 (ms)  |  속도비" << std::endl;
    std::cout << "---------------+-------------+---------" << std::endl;
    std::cout << "Brute-Force    |     ???     |   1.0x" << std::endl;
    std::cout << "FLANN          |     ???     |   ???x" << std::endl;

    std::cout << "\n💡 결론:" << std::endl;
    std::cout << "   - BF: 정확하지만 느림 O(N²)" << std::endl;
    std::cout << "   - FLANN: 빠른 근사 O(N log N)" << std::endl;
    std::cout << "   - 실시간 SLAM: FLANN 또는 NN 기반" << std::endl;
}

/**
 * @brief Homography DLT(Direct Linear Transform) 직접 구현
 *
 * 4개 이상의 대응점으로 A 행렬(2N x 9)을 구성하고,
 * SVD로 분해하여 마지막 V 열에서 H를 추출하세요.
 * OpenCV findHomography() 결과와 비교합니다.
 */
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

    // H_true로 변환하여 목적 포인트 생성
    std::vector<cv::Point2d> dst_pts;
    cv::perspectiveTransform(src_pts, dst_pts, H_true);

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

    // 힌트: 각 대응점 (x,y)↔(u,v)에 대해 2행 추가
    //   행 2i:   [-x, -y, -1,  0,  0,  0, u*x, u*y, u]
    //   행 2i+1: [ 0,  0,  0, -x, -y, -1, v*x, v*y, v]
    for (int i = 0; i < N; i++)
    {
        double x = src_pts[i].x, y = src_pts[i].y;
        double u = dst_pts[i].x, v = dst_pts[i].y;
        (void)x; (void)y; (void)u; (void)v;  // TODO: 아래 주석 해제

        // TODO: A 행렬 채우기
        // A.at<double>(2*i, 0) = -x;
        // A.at<double>(2*i, 1) = -y;
        // A.at<double>(2*i, 2) = -1;
        // A.at<double>(2*i, 6) = u * x;
        // A.at<double>(2*i, 7) = u * y;
        // A.at<double>(2*i, 8) = u;
        // A.at<double>(2*i+1, 3) = -x;
        // A.at<double>(2*i+1, 4) = -y;
        // A.at<double>(2*i+1, 5) = -1;
        // A.at<double>(2*i+1, 6) = v * x;
        // A.at<double>(2*i+1, 7) = v * y;
        // A.at<double>(2*i+1, 8) = v;
    }

    // TODO: SVD 분해 후 H 추출
    cv::Mat H_dlt = cv::Mat::eye(3, 3, CV_64F);
    // cv::Mat w, u_mat, vt;
    // cv::SVD::compute(A, w, u_mat, vt);
    // cv::Mat h = vt.row(vt.rows - 1);  // 마지막 행 = V의 마지막 열
    // H_dlt = h.reshape(1, 3);
    // H_dlt /= H_dlt.at<double>(2, 2);  // h33=1로 정규화

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

/**
 * @brief RANSAC Homography 직접 구현
 *
 * 아웃라이어가 섞인 매칭에서 랜덤 4점 선택 → DLT → 인라이어 카운트를
 * 반복하여 최적의 Homography를 찾으세요.
 */
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

    // Inlier: H_true로 정확하게 변환 + 약간 노이즈
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

    // Outlier: 랜덤
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
    int max_iters = 500;
    double threshold = 5.0;
    int best_inlier_count = 0;
    cv::Mat best_H;

    // 힌트:
    //   for (int iter = 0; iter < max_iters; iter++) {
    //     1. 랜덤 4개 인덱스 선택 (std::shuffle 또는 random_choice)
    //     2. 4개 점으로 DLT (cv::findHomography 또는 직접 구현)
    //     3. 모든 점에 H 적용하여 재투영 오차 계산
    //        cv::perspectiveTransform(src_pts, projected, H)
    //        오차 = norm(projected[i] - dst_pts[i])
    //     4. 오차 < threshold인 점 개수 세기
    //     5. 최다 inlier이면 best_H 갱신
    //   }

    std::cout << "📊 RANSAC 결과:" << std::endl;
    std::cout << "   검출된 inlier: " << best_inlier_count << " / " << total << std::endl;

    // 성능 평가
    if (!best_H.empty())
    {
        std::vector<cv::Point2d> projected;
        cv::perspectiveTransform(src_pts, projected, best_H);
        int tp = 0, fp = 0;
        for (int i = 0; i < total; i++)
        {
            double err = cv::norm(projected[i] - dst_pts[i]);
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
