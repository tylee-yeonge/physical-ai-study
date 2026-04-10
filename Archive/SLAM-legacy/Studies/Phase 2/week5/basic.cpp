#include "basic.h"
#include <iostream>
#include <iomanip>

// 에피폴라 선 계산 — 한 이미지의 점이 다른 이미지에서 어떤 선 위에 있는지
//
// 에피폴라 기하학의 핵심 성질:
//   이미지1의 점 p를 관측하면, 대응점 p'는 이미지2의 "에피폴라 선" 위에 반드시 존재.
//   3D 공간의 한 점이 카메라1에서 p로 보이면, 카메라2에서는 p를 통과하는
//   광선이 이미지2에 투영된 "선" 위 어딘가에 있어야 하기 때문.
//
// 수식:
//   l' = F · p   (이미지1의 점 p → 이미지2의 에피폴라 선 l')
//   l  = Fᵀ · p' (이미지2의 점 p' → 이미지1의 에피폴라 선 l)
//
// 에피폴라 선 l = [a, b, c]: ax + by + c = 0
//   정규화(a²+b²=1)하면 점에서 선까지의 거리 = |ax+by+c|
//
// 활용: 매칭 검색 범위를 2D→1D로 축소 → 속도↑, 오매칭↓
cv::Vec3f EpipolarGeometryBasic::computeEpipolarLine(const cv::Point2f& point,
                                                     const cv::Mat& fundamental, bool for_image2)
{
    cv::Mat p = (cv::Mat_<double>(3, 1) << point.x, point.y, 1.0);
    cv::Mat line;

    if (for_image2)
    {
        line = fundamental * p;      // 이미지1의 점 → 이미지2의 에피폴라 선
    }
    else
    {
        line = fundamental.t() * p;  // 이미지2의 점 → 이미지1의 에피폴라 선
    }

    // [a, b, c]를 정규화하여 거리 계산이 바로 가능하도록
    double norm = std::sqrt(line.at<double>(0) * line.at<double>(0) +
                            line.at<double>(1) * line.at<double>(1));

    if (norm > 1e-6)
    {
        line /= norm;
    }

    return cv::Vec3f(line.at<double>(0), line.at<double>(1), line.at<double>(2));
}

// Essential Matrix 추정 — 캘리브레이션된 카메라의 기하학적 관계
//
// Essential Matrix E (3×3, rank 2, 5 DOF):
//   정규화 좌표에서의 에피폴라 제약: p2ᵀ · E · p1 = 0
//   E = [t]× · R  (R: 상대 회전, t: 상대 이동의 반대칭 행렬)
//
// Fundamental과의 차이:
//   E: 정규화 좌표(K⁻¹ 적용 후), 5 DOF → R, t 직접 분해 가능
//   F: 픽셀 좌표, 7 DOF → K를 알아야 R, t 추출 가능
//   관계: F = K'⁻ᵀ · E · K⁻¹
//
// SLAM에서는 K를 알고 있으므로 E를 주로 사용 (더 적은 DOF → 더 안정적)
int EpipolarGeometryBasic::estimateEssential(const std::vector<cv::Point2f>& points1,
                                             const std::vector<cv::Point2f>& points2,
                                             cv::Mat& essential, int method)
{
    // focal=1.0, pp=(0,0): 이미 정규화 좌표라고 가정
    // 실제 사용 시에는 K에서 추출한 focal, principal point 전달
    cv::Mat E = cv::findEssentialMat(points1, points2, 1.0, cv::Point2d(0, 0), method);

    if (!E.empty())
    {
        essential = E;
        return points1.size();
    }

    return 0;
}

// Fundamental Matrix 추정 — K를 모르는 경우의 에피폴라 제약
//
// Fundamental Matrix F (3×3, rank 2, 7 DOF):
//   픽셀 좌표에서의 에피폴라 제약: p2ᵀ · F · p1 = 0
//   최소 8개 대응점 필요 (8-point algorithm)
//   RANSAC과 결합하면 outlier에 강건한 추정 가능
//
// F의 성질:
//   rank(F) = 2 (det(F) = 0 제약)
//   에피폴: F·e = 0, Fᵀ·e' = 0 (에피폴 = 다른 카메라 중심의 투영)
int EpipolarGeometryBasic::estimateFundamental(const std::vector<cv::Point2f>& points1,
                                               const std::vector<cv::Point2f>& points2,
                                               cv::Mat& fundamental, int method)
{
    cv::Mat mask;
    // 3.0: RANSAC 재투영 오차 임계값 (픽셀)
    // 0.99: RANSAC 신뢰도 (99% 확률로 올바른 모델을 찾을 때까지 반복)
    cv::Mat F = cv::findFundamentalMat(points1, points2, method, 3.0, 0.99, mask);

    if (!F.empty())
    {
        fundamental = F;
        int inliers = cv::countNonZero(mask);
        return inliers;
    }

    return 0;
}

// 에피폴라 제약 검증 — 대응점 쌍이 기하학적으로 일관되는지 확인
//
// 완벽한 대응점이면 p2ᵀ · E · p1 = 0 이어야 함.
// 실제로는 노이즈 때문에 정확히 0이 되지 않으므로,
// 이 값이 0에 가까울수록 좋은 대응점 / 좋은 E 추정.
//
// 활용: 추정된 E/F의 품질 검증, outlier 탐지
double EpipolarGeometryBasic::verifyEpipolarConstraint(const cv::Point2f& point1,
                                                       const cv::Point2f& point2,
                                                       const cv::Mat& essential_or_fundamental)
{
    // 동차 좌표로 변환: (x, y) → (x, y, 1)
    cv::Mat p1 = (cv::Mat_<double>(3, 1) << point1.x, point1.y, 1.0);
    cv::Mat p2 = (cv::Mat_<double>(3, 1) << point2.x, point2.y, 1.0);

    // p2ᵀ · E · p1 → 스칼라 값 (0에 가까울수록 좋음)
    cv::Mat result = p2.t() * essential_or_fundamental * p1;
    double error = std::abs(result.at<double>(0));

    return error;
}

// Essential Matrix에서 카메라 포즈(R, t) 복원
//
// E를 SVD 분해하면 4가지 해가 나옴:
//   (R1, t), (R1, -t), (R2, t), (R2, -t)
//
// 이 중 물리적으로 유효한 해는 단 하나:
//   삼각측량한 3D 점이 두 카메라 모두의 앞(Z>0)에 있는 해
//   → Cheirality Check (cv::recoverPose가 자동 수행)
//
// ★ 스케일 모호성: t의 방향만 복원 가능, 크기는 알 수 없음!
//   ||t|| = 1로 정규화됨. 이것이 Monocular SLAM의 근본적 한계.
//   → 실제 스케일 복원에는 IMU, 알려진 물체 크기, 또는 스테레오 필요
bool EpipolarGeometryBasic::recoverPose(const cv::Mat& essential,
                                        const std::vector<cv::Point2f>& points1,
                                        const std::vector<cv::Point2f>& points2, const cv::Mat& K,
                                        cv::Mat& R, cv::Mat& t)
{
    double focal = K.at<double>(0, 0);
    cv::Point2d pp(K.at<double>(0, 2), K.at<double>(1, 2));

    // recoverPose: SVD 분해 → 4가지 해 → Cheirality Check → 최적 해 선택
    int inliers = cv::recoverPose(essential, points1, points2, R, t, focal, pp);

    return inliers > 0;
}

// E와 F의 관계 검증 — F = K'⁻ᵀ · E · K⁻¹
//
// 같은 카메라(K' = K)라면 이 관계가 성립.
// 스케일은 임의이므로 (2,2) 원소로 정규화 후 비교.
// diff가 0에 가까우면 E와 F가 일관되게 추정된 것.
double EpipolarGeometryBasic::verifyEF_Relationship(const cv::Mat& K, const cv::Mat& essential,
                                                    const cv::Mat& fundamental)
{
    cv::Mat K_inv = K.inv();
    cv::Mat F_from_E = K_inv.t() * essential * K_inv;

    // F는 스케일이 임의 → (2,2) 원소로 정규화하여 비교
    F_from_E /= F_from_E.at<double>(2, 2);
    cv::Mat F_normalized = fundamental / fundamental.at<double>(2, 2);

    double diff = cv::norm(F_from_E - F_normalized);

    return diff;
}

// 에피폴라 선 시각화 — 대응점과 에피폴라 선을 이미지 위에 표시
//
// 시각화 구조:
//   [이미지1 | 이미지2]를 가로로 이어 붙인 뒤:
//   - 녹색 점: 이미지1의 특징점 + 이미지2의 대응점
//   - 파란 선: 이미지1의 점으로부터 계산된 이미지2의 에피폴라 선
//
// 올바른 추정이면:
//   이미지2의 대응점(녹색 점)이 에피폴라 선(파란 선) 위에 정확히 놓임
//   → 점이 선에서 멀면 F/E 추정이 부정확하거나 잘못된 매칭
//
// 에피폴라 선 방정식: ax + by + c = 0
//   y = -(ax + c) / b  로 변환하여 x=0, x=width 양 끝점 계산
void EpipolarGeometryBasic::visualizeEpipolarLines(const cv::Mat& img1, const cv::Mat& img2,
                                                   const std::vector<cv::Point2f>& points1,
                                                   const std::vector<cv::Point2f>& points2,
                                                   const cv::Mat& fundamental, cv::Mat& output)
{
    // 두 이미지를 좌우로 이어 붙이기
    cv::Mat combined;
    cv::hconcat(img1, img2, combined);

    if (combined.channels() == 1)
    {
        cv::cvtColor(combined, output, cv::COLOR_GRAY2BGR);
    }
    else
    {
        output = combined.clone();
    }

    int img_width = img1.cols;

    // 최대 10개 점에 대해 에피폴라 선 표시
    for (size_t i = 0; i < std::min(points1.size(), (size_t)10); i++)
    {
        // 이미지1의 특징점 (녹색)
        cv::circle(output, points1[i], 5, cv::Scalar(0, 255, 0), -1);

        // 이미지2의 대응점 (녹색, img_width만큼 오른쪽 이동)
        cv::Point2f pt2_shifted(points2[i].x + img_width, points2[i].y);
        cv::circle(output, pt2_shifted, 5, cv::Scalar(0, 255, 0), -1);

        // 이미지1의 점 → 이미지2의 에피폴라 선 계산 (l' = F·p)
        cv::Vec3f line = computeEpipolarLine(points1[i], fundamental, true);

        // ax + by + c = 0 → y = -(ax + c) / b
        float a = line[0], b = line[1], c = line[2];

        if (std::abs(b) > 1e-6)
        {
            int x1 = 0, x2 = img2.cols;
            int y1 = -(a * x1 + c) / b;
            int y2 = -(a * x2 + c) / b;

            // 에피폴라 선 (파랑) — 이미지2 영역에 그리기
            cv::line(output, cv::Point(x1 + img_width, y1), cv::Point(x2 + img_width, y2),
                     cv::Scalar(255, 0, 0), 1);
        }
    }

    cv::putText(output, "Epipolar Lines", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0,
                cv::Scalar(0, 255, 0), 2);
}

// 에피폴라 기하학 전체 파이프라인 데모
//
// 6단계 파이프라인:
//   ① ORB 특징점 검출 + BF 매칭 + Ratio Test
//   ② Essential Matrix 추정 (RANSAC)
//   ③ Fundamental Matrix 추정 (비교용)
//   ④ E에서 R, t 복원 (recoverPose)
//   ⑤ 에피폴라 제약 검증 (p2ᵀ·E·p1 ≈ 0)
//   ⑥ 에피폴라 선 시각화 저장
//
// SLAM에서 이 파이프라인의 위치:
//   초기화 단계: 첫 두 프레임에서 E 추정 → R, t 복원 → 초기 맵 생성
//   이후에는 PnP(week7)로 전환
void EpipolarGeometryBasic::demoPipeline(const cv::Mat& img1, const cv::Mat& img2, const cv::Mat& K)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "에피폴라 기하학 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Step 1: ORB 특징점 검출 + BF 매칭 + Lowe's Ratio Test
    std::cout << "1️⃣  특징점 검출 및 매칭..." << std::endl;

    const int kMaxFeatures = 500;  // ORB 최대 특징점 수
    cv::Ptr<cv::ORB> orb = cv::ORB::create(kMaxFeatures);
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat desc1, desc2;

    orb->detectAndCompute(img1, cv::noArray(), kp1, desc1);
    orb->detectAndCompute(img2, cv::noArray(), kp2, desc2);

    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<std::vector<cv::DMatch>> knn_matches;
    matcher.knnMatch(desc1, desc2, knn_matches, 2);

    // Lowe's Ratio Test (ratio=0.7)
    // 최선 매칭 거리 < 0.7 × 차선 매칭 거리 → 신뢰할 수 있는 매칭
    std::vector<cv::DMatch> good_matches;
    for (const auto& m : knn_matches)
    {
        if (m.size() >= 2 && m[0].distance < 0.7 * m[1].distance)
        {
            good_matches.push_back(m[0]);
        }
    }

    std::cout << "   매칭 개수: " << good_matches.size() << "개\n" << std::endl;

    if (good_matches.size() < 8)
    {
        std::cout << "❌ 매칭이 부족합니다 (최소 8개 필요)" << std::endl;
        return;
    }

    // DMatch에서 좌표 추출 (E/F 추정에 필요)
    std::vector<cv::Point2f> points1, points2;
    for (const auto& match : good_matches)
    {
        points1.push_back(kp1[match.queryIdx].pt);
        points2.push_back(kp2[match.trainIdx].pt);
    }

    // Step 2: Essential Matrix 추정 (RANSAC)
    // focal, pp를 직접 전달하여 내부적으로 정규화 좌표 변환
    // RANSAC 신뢰도 0.999, 임계값 1.0 픽셀
    std::cout << "2️⃣  Essential Matrix 추정..." << std::endl;

    double focal = K.at<double>(0, 0);
    cv::Point2d pp(K.at<double>(0, 2), K.at<double>(1, 2));

    cv::Mat E, mask_E;
    E = cv::findEssentialMat(points1, points2, focal, pp, cv::RANSAC, 0.999, 1.0, mask_E);

    int inliers_E = cv::countNonZero(mask_E);
    std::cout << "   Inliers: " << inliers_E << " / " << points1.size() << std::endl;
    std::cout << "   Essential Matrix:\n" << E << "\n" << std::endl;

    // Step 3: Fundamental Matrix 추정 (E와 비교용)
    // FM_RANSAC: RANSAC 기반 8-point algorithm
    std::cout << "3️⃣  Fundamental Matrix 추정..." << std::endl;

    cv::Mat F, mask_F;
    F = cv::findFundamentalMat(points1, points2, cv::FM_RANSAC, 3.0, 0.99, mask_F);

    int inliers_F = cv::countNonZero(mask_F);
    std::cout << "   Inliers: " << inliers_F << " / " << points1.size() << std::endl;
    std::cout << "   Fundamental Matrix:\n" << F << "\n" << std::endl;

    // E와 F 관계 검증: F = K⁻ᵀ · E · K⁻¹ 이 성립하는지 확인
    double ef_diff = verifyEF_Relationship(K, E, F);
    std::cout << "   E-F 관계 오차: " << ef_diff << "\n" << std::endl;

    // Step 4: E에서 카메라 포즈(R, t) 복원
    // SVD 분해 → 4가지 해 → Cheirality Check → 최적 해 선택
    std::cout << "4️⃣  카메라 포즈 복원..." << std::endl;

    cv::Mat R, t;
    int pose_inliers = cv::recoverPose(E, points1, points2, R, t, focal, pp, mask_E);

    std::cout << "   회전 행렬 R:\n" << R << std::endl;
    std::cout << "   이동 벡터 t:\n" << t << std::endl;
    std::cout << "   Pose inliers: " << pose_inliers << "\n" << std::endl;

    // Step 5: 에피폴라 제약 검증
    // 각 대응점 쌍에 대해 p2ᵀ·E·p1 ≈ 0 인지 확인
    // 값이 0에 가까울수록 E 추정이 정확함
    std::cout << "5️⃣  에피폴라 제약 검증..." << std::endl;

    double total_error = 0.0;
    int count = 0;
    for (size_t i = 0; i < std::min(points1.size(), (size_t)10); i++)
    {
        double error = verifyEpipolarConstraint(points1[i], points2[i], E);
        total_error += error;
        count++;
    }

    std::cout << "   평균 에피폴라 제약 오차: " << total_error / count << "\n" << std::endl;

    // Step 6: 시각화
    std::cout << "6️⃣  에피폴라 선 시각화..." << std::endl;

    cv::Mat output;
    visualizeEpipolarLines(img1, img2, points1, points2, F, output);
    cv::imwrite("epipolar_lines_demo.png", output);

    std::cout << "   💾 저장: epipolar_lines_demo.png\n" << std::endl;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 파이프라인 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

// 메인 함수 — 에피폴라 기하학 학습 데모 전체 실행
//
// 실행 흐름:
//   ① 카메라 K 설정 + 체커보드 패턴 이미지 생성
//   ② 카메라 이동 시뮬레이션 (회전 10° + 이동 (50,30))
//   ③ 에피폴라 제약, E vs F, 포즈 복원 교육 출력
//   ④ demoPipeline: 6단계 전체 파이프라인 실행
//
// 시뮬레이션 전략:
//   실제 3D 장면 대신 2D 아핀 변환으로 카메라 이동을 모사
//   체커보드 패턴: ORB 특징점이 잘 검출되는 텍스처 제공
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  에피폴라 기하학 기본 데모 (Week 5)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 카메라 내부 파라미터 K
    // fx=fy=600 (초점거리), cx=400, cy=300 (주점)
    cv::Mat K = (cv::Mat_<double>(3, 3) << 600.0, 0.0, 400.0, 0.0, 600.0, 300.0, 0.0, 0.0, 1.0);

    // 테스트 이미지 생성: 50×50 체커보드 패턴
    // 특징점 검출에 적합한 풍부한 에지/코너 텍스처
    const int kImageHeight = 600;
    const int kImageWidth = 800;
    cv::Mat img1 = cv::Mat::zeros(kImageHeight, kImageWidth, CV_8UC1);
    cv::Mat img2 = cv::Mat::zeros(kImageHeight, kImageWidth, CV_8UC1);

    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            if ((i + j) % 2 == 0)
            {
                cv::rectangle(img1, cv::Point(j * 50, i * 50),
                              cv::Point((j + 1) * 50, (i + 1) * 50), cv::Scalar(255), -1);
            }
        }
    }

    // 이미지 2: 아핀 변환으로 카메라 이동 시뮬레이션
    // 10° 회전 + (50, 30) 픽셀 이동
    // 참고: 실제 카메라 이동과 2D 아핀 변환은 다르지만, 데모 목적으로 사용
    cv::Mat M = cv::getRotationMatrix2D(cv::Point(400, 300), 10, 1.0);
    M.at<double>(0, 2) += 50;
    M.at<double>(1, 2) += 30;
    cv::warpAffine(img1, img2, M, img2.size());

    std::cout << "📸 테스트 이미지 생성 완료" << std::endl;
    std::cout << "   카메라 이동: 회전 10° + 이동 (50, 30)\n" << std::endl;

    // 💡 [교육] 에피폴라 제약이란?
    std::cout << "💡 [교육] 에피폴라 제약이란? (quiz 문제 1에서 사용!)" << std::endl;
    std::cout << "   두 이미지에서 같은 3D 점을 보면:" << std::endl;
    std::cout << "   p2^T * E * p1 = 0 (정규화 좌표)" << std::endl;
    std::cout << "   p2^T * F * p1 = 0 (픽셀 좌표)" << std::endl;
    std::cout << "   → 0에 가까우면 올바른 대응점!\n" << std::endl;

    // 💡 [교육] E vs F 선택 가이드
    std::cout << "💡 [교육] E vs F 선택 가이드 (quiz 문제 2에서 사용!):" << std::endl;
    std::cout << "   캘리브레이션 됨 (K 있음) → Essential Matrix (E)" << std::endl;
    std::cout << "     - 정규화 좌표 사용, 5 DOF" << std::endl;
    std::cout << "     - R, t 직접 분해 가능 → SLAM에서 주로 사용" << std::endl;
    std::cout << "   캘리브레이션 안 됨 → Fundamental Matrix (F)" << std::endl;
    std::cout << "     - 픽셀 좌표 사용, 7 DOF" << std::endl;
    std::cout << "     - 관계: F = K'^-T * E * K^-1\n" << std::endl;

    // 💡 [교육] 포즈 복원: 4가지 해 → Cheirality Check
    std::cout << "💡 [교육] 포즈 복원 (quiz 문제 4에서 사용!):" << std::endl;
    std::cout << "   E를 SVD 분해 → (R1,t), (R1,-t), (R2,t), (R2,-t)" << std::endl;
    std::cout << "   → 삼각측량 후 Z > 0인 점이 가장 많은 해 선택 (Cheirality Check)" << std::endl;
    std::cout << "   💡 스케일 모호성: t의 방향만 알 수 있고, 크기는 모름!" << std::endl;
    std::cout << "   → Monocular SLAM의 근본적 한계\n" << std::endl;

    // 전체 파이프라인 실행
    EpipolarGeometryBasic::demoPipeline(img1, img2, K);

    std::cout << "\n💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md 이론 읽기 (에피폴라 제약, E vs F, 8-point)" << std::endl;
    std::cout << "   2. quiz_easy.cpp — 에피폴라 제약, E vs F, 포즈 복원" << std::endl;
    std::cout << "   3. my_basic.cpp — Step 1~8 순서대로 직접 구현" << std::endl;
    std::cout << "   4. quiz_medium.cpp — 제약 검증 구현, 8-Point 직접 구현" << std::endl;
    std::cout << "   5. PRACTICE.md — 에피폴라 기하학 실습\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
