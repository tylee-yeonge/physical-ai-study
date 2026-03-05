/**
 * Phase 2 Week 3 - 특징점 검출 기초 퀴즈 (정답)
 *
 * 다루는 개념:
 *   - FAST 검출기: 임계값과 특징점 개수의 관계
 *   - ORB 디스크립터: 이진 256비트 = 32바이트, 해밍 거리 매칭
 *   - NMS (Non-Maximum Suppression): 중복 제거, 균등 분포
 *   - 검출기 속도 비교: FAST vs ORB
 *   - Harris 응답 함수: R = det(M) - k·trace(M)²
 *
 * 특징점 검출은 SLAM 프론트엔드의 첫 단계이다.
 * 이미지에서 추적 가능한 점(코너, blob)을 찾아내야
 * 매칭, 포즈 추정, 삼각측량으로 이어질 수 있다.
 *
 * SLAM에서 자주 쓰이는 특징점 검출기:
 *   - FAST: 속도 최우선 (ORB-SLAM의 기반)
 *   - ORB: FAST + 회전 불변 + BRIEF 디스크립터
 *   - Harris: 이론적 기초 (Structure Tensor 기반)
 */

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <chrono>

// FAST 임계값과 특징점 개수 — 속도와 반복성의 트레이드오프
//
// FAST (Features from Accelerated Segment Test):
//   중심 픽셀 p 주위의 Bresenham 원(16픽셀) 검사
//   연속 N개(보통 12개)가 p보다 threshold 이상 밝거나 어두우면 코너
//
//   임계값 효과:
//     threshold ↑ → 더 강한 코너만 검출 → 개수 ↓, 반복성 ↑
//     threshold ↓ → 약한 코너도 검출 → 개수 ↑, 노이즈 민감 ↑
//
// ★ 실시간 SLAM에서는 threshold를 동적으로 조정하여
//   목표 특징점 수(~1000개)를 유지하는 전략 사용 (ORB-SLAM)
//
// TODO: threshold = {10, 20, 30, 40, 50}에서 특징점 개수 변화를 관찰하세요
void problem1_fast_threshold()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: FAST 임계값과 특징점 개수" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 생성
    cv::Mat image = cv::imread("test_image.jpg", cv::IMREAD_GRAYSCALE);
    if (image.empty())
    {
        // 이미지 없으면 체커보드 생성
        image = cv::Mat::zeros(600, 800, CV_8UC1);
        for (int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                if ((i + j) % 2 == 0)
                {
                    cv::rectangle(image, cv::Point(j * 50, i * 50),
                                  cv::Point((j + 1) * 50, (i + 1) * 50), cv::Scalar(255), -1);
                }
            }
        }
    }

    std::vector<int> thresholds = {10, 20, 30, 40, 50};

    std::cout << "📊 임계값에 따른 특징점 개수:\n" << std::endl;

    for (int threshold : thresholds)
    {
        std::vector<cv::KeyPoint> keypoints;

        // ✅ 정답: cv::FAST 함수로 특징점 검출
        cv::FAST(image, keypoints, threshold, true);

        std::cout << "   Threshold = " << threshold << " → 특징점: " << keypoints.size() << "개"
                  << std::endl;
    }

    std::cout << "\n💡 정답:" << std::endl;
    std::cout << "   Q1. 임계값이 증가하면 특징점 개수는 어떻게 변하나요?" << std::endl;
    std::cout << "   A1. 감소합니다." << std::endl;
    std::cout << "       임계값이 높아지면 '밝기 차이가 큰 강한 코너'만 통과합니다." << std::endl;
    std::cout << "       즉, 기준이 엄격해지므로 약한 코너는 걸러지고 개수가 줄어듭니다." << std::endl;
    std::cout << "       반대로 임계값을 낮추면 약한 코너도 검출되어 개수가 많아지지만," << std::endl;
    std::cout << "       노이즈에 의한 거짓 검출(false positive)도 함께 증가합니다." << std::endl;
    std::cout << std::endl;
    std::cout << "   Q2. 실시간 SLAM에 적합한 임계값은?" << std::endl;
    std::cout << "   A2. 고정된 하나의 값이 아니라, 동적으로 조정하는 것이 좋습니다." << std::endl;
    std::cout << "       보통 20~40 범위에서 시작하되, 프레임마다 검출된 특징점 수를 확인하여" << std::endl;
    std::cout << "       목표 개수(~1000개)에 맞게 임계값을 올리거나 내립니다." << std::endl;
    std::cout << "       ORB-SLAM에서는 이 '적응형 임계값' 전략을 실제로 사용합니다." << std::endl;
    std::cout << "       (이유: 밝은 장면에서는 특징이 많고, 어두운 장면에서는 적기 때문)" << std::endl;
}

// ORB 디스크립터 분석 — 이진 디스크립터의 구조와 장점
//
// ORB (Oriented FAST and Rotated BRIEF):
//   검출: FAST + 방향(orientation) 계산 (intensity centroid 방법)
//   기술: rBRIEF (Rotated BRIEF) = 회전 불변 이진 디스크립터
//
// 디스크립터 구조:
//   - 256비트 = 32바이트 (CV_8UC1, 32열)
//   - 각 비트: 특정 픽셀 쌍의 밝기 비교 결과 (0 또는 1)
//   - 총 256번의 밝기 비교 → 256비트 벡터
//
// 이진 디스크립터 vs 실수 디스크립터 (SIFT):
//   - 저장: 32 bytes vs 512 bytes (16배 차이)
//   - 매칭: 해밍 거리(XOR + popcount) vs L2 거리(곱셈+덧셈)
//   - 속도: 해밍 거리가 CPU 명령어 1개로 가능 → 훨씬 빠름
//
// ★ SLAM에서 ORB가 압도적으로 선호되는 이유:
//   빠른 검출 + 빠른 매칭 + 충분한 정확도 = 실시간 처리 가능
//
// TODO: ORB 검출 후 디스크립터의 크기와 타입을 확인하세요
void problem2_orb_descriptor()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: ORB 디스크립터 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat image = cv::Mat::zeros(400, 600, CV_8UC1);
    cv::circle(image, cv::Point(300, 200), 100, cv::Scalar(255), -1);

    // ORB 검출기 생성 — nfeatures=100: 최대 100개의 특징점만 검출
    //   nfeatures 설정 기준 (정답은 없고, 용도에 따라 결정):
    //     - 실습/테스트: 100~500 (빠르게 결과 확인)
    //     - 실시간 SLAM: 500~1000 (속도와 정확도 균형, ORB-SLAM2 기본값=1000)
    //     - 오프라인 매칭: 1000~5000 (정확도 우선)
    //   값이 클수록 매칭 정확도 ↑ but 계산량 ↑
    //   값이 작을수록 속도 ↑ but 매칭 실패 가능성 ↑
    //   create()의 디폴트는 500
    cv::Ptr<cv::ORB> orb = cv::ORB::create(100);
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;

    // ✅ 정답: ORB 검출기로 특징점과 디스크립터를 동시에 검출
    orb->detectAndCompute(image, cv::noArray(), keypoints, descriptors);

    std::cout << "검출 결과:" << std::endl;
    std::cout << "   - 특징점 개수: " << keypoints.size() << std::endl;
    std::cout << "   - 디스크립터 크기: " << descriptors.size() << std::endl;
    std::cout << "   - 디스크립터 타입: " << descriptors.type() << std::endl;

    if (!descriptors.empty())
    {
        std::cout << "\n첫 번째 디스크립터(일부):" << std::endl;
        std::cout << "   ";
        for (int i = 0; i < std::min(8, descriptors.cols); i++)
        {
            std::cout << (int)descriptors.at<uchar>(0, i) << " ";
        }
        std::cout << "..." << std::endl;
    }

    std::cout << "\n💡 정답:" << std::endl;
    std::cout << "   Q1. ORB 디스크립터는 몇 바이트인가요?" << std::endl;
    std::cout << "   A1. 32바이트 (= 256비트)입니다." << std::endl;
    std::cout << "       위 출력에서 디스크립터 크기가 [N x 32]로 나오는 것을 확인할 수 있습니다." << std::endl;
    std::cout << "       각 행이 하나의 특징점을 기술하고, 32열 = 32바이트 = 256비트입니다." << std::endl;
    std::cout << "       각 비트는 '특정 픽셀 쌍을 비교한 결과(0 or 1)'로," << std::endl;
    std::cout << "       총 256번의 비교 결과를 하나의 벡터로 압축한 것입니다." << std::endl;
    std::cout << std::endl;
    std::cout << "   Q2. 이진 디스크립터의 장점은?" << std::endl;
    std::cout << "   A2. 속도와 메모리 모두에서 유리합니다." << std::endl;
    std::cout << "       ① 메모리: ORB=32바이트 vs SIFT=512바이트 → 16배 절약" << std::endl;
    std::cout << "       ② 매칭 속도: 해밍 거리(XOR + popcount)로 비교" << std::endl;
    std::cout << "          - 해밍 거리: 두 비트 벡터를 XOR하면 다른 비트만 1이 됨" << std::endl;
    std::cout << "            → 1의 개수(popcount)를 세면 '얼마나 다른지' 알 수 있음" << std::endl;
    std::cout << "          - CPU가 한 명령어로 처리 가능 → SIFT의 L2 거리보다 훨씬 빠름" << std::endl;
    std::cout << "       SLAM에서는 매 프레임 수천 개의 매칭을 해야 하므로 이 속도 차이가 중요합니다." << std::endl;
}

// NMS 효과 — 중복 특징점 제거로 균등한 분포 확보
//
// NMS (Non-Maximum Suppression):
//   같은 코너 근처에서 여러 픽셀이 코너로 검출될 수 있음
//   → 가장 강한 응답(response)만 남기고 나머지 제거
//
// NMS 없이 검출 시 문제:
//   - 하나의 코너에 수십 개의 중복 특징점
//   - 이미지 전체에 불균등한 분포
//   - 매칭 시 혼란 (비슷한 위치의 여러 점)
//   - 포즈 추정의 기하학적 제약이 약해짐
//
// NMS 적용 후:
//   - 각 코너에 하나의 대표 특징점만 남음
//   - 더 균등한 공간 분포
//   - 매칭 정확도 향상
//
// ★ SLAM에서는 NMS 외에도 Grid 기반 분포 강제를 사용:
//   이미지를 격자로 나누어 각 칸에서 최대 N개만 유지
//
// TODO: NMS 적용 전후의 특징점 개수와 감소율을 비교하세요
void problem3_nms_effect()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: NMS 효과 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat image = cv::Mat::zeros(400, 600, CV_8UC1);
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 12; j++)
        {
            if ((i + j) % 2 == 0)
            {
                cv::rectangle(image, cv::Point(j * 50, i * 50),
                              cv::Point((j + 1) * 50, (i + 1) * 50), cv::Scalar(255), -1);
            }
        }
    }

    std::vector<cv::KeyPoint> kp_without_nms, kp_with_nms;

    // ✅ 정답: NMS 없이 FAST 검출
    cv::FAST(image, kp_without_nms, 20, false);

    // ✅ 정답: NMS 적용 FAST 검출
    cv::FAST(image, kp_with_nms, 20, true);

    std::cout << "결과:" << std::endl;
    std::cout << "   NMS 없음: " << kp_without_nms.size() << "개" << std::endl;
    std::cout << "   NMS 적용: " << kp_with_nms.size() << "개" << std::endl;

    if (!kp_without_nms.empty() && !kp_with_nms.empty())
    {
        double reduction = 100.0 * (1.0 - double(kp_with_nms.size()) / kp_without_nms.size());
        std::cout << "   → 감소율: " << reduction << "%\n" << std::endl;
    }

    std::cout << "💡 정답:" << std::endl;
    std::cout << "   Q1. NMS는 왜 필요한가요?" << std::endl;
    std::cout << "   A1. 같은 코너 주변에서 여러 픽셀이 동시에 코너로 검출되기 때문입니다." << std::endl;
    std::cout << "       예: 하나의 코너에서 3~5픽셀이 모두 '코너'로 판정될 수 있음" << std::endl;
    std::cout << "       → NMS는 그 중 응답(response)이 가장 강한 하나만 남기고 나머지를 제거합니다." << std::endl;
    std::cout << "       이렇게 하면 각 코너당 딱 하나의 대표점만 남아서 깔끔해집니다." << std::endl;
    std::cout << std::endl;
    std::cout << "   Q2. SLAM에서 NMS를 안 하면 어떤 문제가 생기나요?" << std::endl;
    std::cout << "   A2. 두 가지 핵심 문제가 발생합니다:" << std::endl;
    std::cout << "       ① 매칭 혼란: 같은 코너에 여러 점이 있으면, 다음 프레임에서" << std::endl;
    std::cout << "          어떤 점과 매칭해야 할지 모호해짐 → 잘못된 매칭 증가" << std::endl;
    std::cout << "       ② 불균등 분포: 텍스처 풍부한 영역에 점이 몰리고, 텍스처 없는 영역은 비게 됨" << std::endl;
    std::cout << "          → 포즈 추정 시 특정 방향에서만 제약이 걸려 불안정해짐" << std::endl;
    std::cout << "       (위 실행 결과에서 감소율이 높을수록 NMS가 많은 중복을 제거한 것입니다)" << std::endl;
}

// 검출기 속도 비교 — FAST vs ORB 실행 시간 측정
//
// FAST:
//   - 검출만 수행 (키포인트 위치)
//   - 디스크립터 없음
//   - 매우 빠름 (~1ms)
//
// ORB:
//   - 검출 (FAST 기반) + 방향 계산 + 디스크립터(rBRIEF) 계산
//   - 디스크립터 포함이므로 FAST보다 느림 (~5-15ms)
//   - 하지만 디스크립터가 있어야 매칭 가능
//
// 실시간 SLAM 시간 예산 (30 FPS = 33ms/프레임):
//   - 특징점 검출: ~5ms
//   - 매칭: ~10ms
//   - 포즈 추정: ~5ms
//   - 맵 관리: ~5ms
//   - 여유: ~8ms
//
// ★ 속도가 중요한 이유: 검출이 느리면 전체 파이프라인이 실시간 불가
//
// TODO: FAST와 ORB의 검출 시간을 chrono로 측정하세요
void problem4_speed_comparison()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 검출기 속도 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC3);
    for (int i = 0; i < 50; i++)
    {
        cv::Point center(rand() % 800, rand() % 600);
        int radius = 10 + rand() % 30;
        cv::circle(image, center, radius, cv::Scalar(rand() % 255, rand() % 255, rand() % 255), -1);
    }

    // ✅ 정답: FAST 시간 측정
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    std::vector<cv::KeyPoint> fast_kp;
    auto t1 = std::chrono::high_resolution_clock::now();
    cv::FAST(gray, fast_kp, 20, true);
    auto t2 = std::chrono::high_resolution_clock::now();
    double fast_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "   FAST: " << fast_kp.size() << "개 검출, " << fast_ms << " ms" << std::endl;

    // ✅ 정답: ORB 시간 측정
    cv::Ptr<cv::ORB> orb = cv::ORB::create();
    std::vector<cv::KeyPoint> orb_kp;
    cv::Mat orb_desc;
    auto t3 = std::chrono::high_resolution_clock::now();
    orb->detectAndCompute(gray, cv::noArray(), orb_kp, orb_desc);
    auto t4 = std::chrono::high_resolution_clock::now();
    double orb_ms = std::chrono::duration<double, std::milli>(t4 - t3).count();
    std::cout << "   ORB: " << orb_kp.size() << "개 검출, " << orb_ms << " ms" << std::endl;

    std::cout << "💡 정답:" << std::endl;
    std::cout << "   Q1. 어떤 검출기가 더 빠른가요?" << std::endl;
    std::cout << "   A1. FAST가 훨씬 빠릅니다 (보통 5~10배 이상)." << std::endl;
    std::cout << "       FAST는 '키포인트 위치 검출'만 수행하므로 ~1ms 이내에 끝납니다." << std::endl;
    std::cout << "       ORB는 FAST 검출 + 방향 계산 + rBRIEF 디스크립터 계산까지 해서 ~5-15ms 걸립니다." << std::endl;
    std::cout << "       (위 결과에서 실제 시간 차이를 직접 확인해보세요)" << std::endl;
    std::cout << std::endl;
    std::cout << "   Q2. 실시간 SLAM (30 FPS)에 적합한 것은?" << std::endl;
    std::cout << "   A2. 둘 다 사용합니다 — 역할이 다르기 때문입니다." << std::endl;
    std::cout << "       - FAST: 빠르게 '어디에' 특징점이 있는지 검출" << std::endl;
    std::cout << "       - ORB: 검출된 점에 디스크립터를 붙여서 '어떤' 특징점인지 기술" << std::endl;
    std::cout << "       ORB-SLAM은 내부적으로 FAST로 검출 후 ORB 디스크립터를 계산합니다." << std::endl;
    std::cout << "       30FPS = 33ms/프레임이므로, 검출+매칭+포즈추정을 모두 이 안에 끝내야 합니다." << std::endl;
    std::cout << "       FAST 단독(~1ms) + 디스크립터(~5ms) = 충분히 실시간 가능합니다." << std::endl;
}

// Harris 응답 함수 — 코너/에지/평면 판별의 수학적 기초
//
// Structure Tensor M (= 이미지 그래디언트의 2차 모멘트 행렬):
//   M = Σ_window [ Ix²    IxIy ]
//                [ IxIy   Iy²  ]
//
// Harris 응답 함수:
//   R = det(M) - k · trace(M)²
//   = λ₁·λ₂ - k·(λ₁ + λ₂)²
//
// 판별 기준:
//   R >> 0: 코너 — λ₁, λ₂ 모두 큼 → 모든 방향으로 밝기 변화
//   R << 0: 에지 — 하나만 큼 → det 작고 trace 큼 → R 음수
//   |R| ≈ 0: 평면 — 둘 다 작음 → det ≈ 0, trace ≈ 0
//
// k 파라미터 (보통 0.04~0.06):
//   k ↑ → 코너 판별 기준 엄격 (R 감소) → 더 적은 코너 검출
//   k ↓ → 코너 판별 기준 느슨 → 더 많은 코너 검출
//
// ★ Harris와 Shi-Tomasi의 차이:
//   Harris: R = det(M) - k·trace(M)²
//   Shi-Tomasi: R = min(λ₁, λ₂) → 이론적으로 더 안정적
//   goodFeaturesToTrack()는 Shi-Tomasi 기본 사용
//
// TODO: 3개 케이스(코너, 에지, 평면)에 대해 det, trace, R을 계산하세요
void problem5_harris_response()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: Harris 응답 함수 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Structure Tensor M의 의미:" << std::endl;
    std::cout << "       [ Ix^2   IxIy ]" << std::endl;
    std::cout << "   M = [ IxIy   Iy^2 ]" << std::endl;
    std::cout << "\n   R = det(M) - k * trace(M)^2" << std::endl;
    std::cout << "     = lambda1*lambda2 - k*(lambda1+lambda2)^2\n" << std::endl;

    // 케이스 1: 코너 (두 고유값 모두 큼)
    // M = [500, 10; 10, 480]
    double m11_a = 500.0, m12_a = 10.0, m22_a = 480.0;

    // 케이스 2: 에지 (한 고유값만 큼)
    // M = [800, 5; 5, 10]
    double m11_b = 800.0, m12_b = 5.0, m22_b = 10.0;

    // 케이스 3: 평면 (두 고유값 모두 작음)
    // M = [3, 1; 1, 2]
    double m11_c = 3.0, m12_c = 1.0, m22_c = 2.0;

    double k = 0.04;

    // ✅ 정답: 각 케이스에 대해 det, trace, R 계산
    double det_a = m11_a * m22_a - m12_a * m12_a;
    double trace_a = m11_a + m22_a;
    double R_a = det_a - k * trace_a * trace_a;

    double det_b = m11_b * m22_b - m12_b * m12_b;
    double trace_b = m11_b + m22_b;
    double R_b = det_b - k * trace_b * trace_b;

    double det_c = m11_c * m22_c - m12_c * m12_c;
    double trace_c = m11_c + m22_c;
    double R_c = det_c - k * trace_c * trace_c;

    std::cout << "k = " << k << "\n" << std::endl;
    std::cout << "케이스 1 (코너): M = [500, 10; 10, 480]" << std::endl;
    std::cout << "   det = " << det_a << ", trace = " << trace_a << ", R = " << R_a << std::endl;
    std::cout << "   판별: " << (R_a > 0 ? "코너" : (R_a < 0 ? "에지" : "평면")) << "\n" << std::endl;

    std::cout << "케이스 2 (에지): M = [800, 5; 5, 10]" << std::endl;
    std::cout << "   det = " << det_b << ", trace = " << trace_b << ", R = " << R_b << std::endl;
    std::cout << "   판별: " << (R_b > 0 ? "코너" : (R_b < 0 ? "에지" : "평면")) << "\n" << std::endl;

    std::cout << "케이스 3 (평면): M = [3, 1; 1, 2]" << std::endl;
    std::cout << "   det = " << det_c << ", trace = " << trace_c << ", R = " << R_c << std::endl;
    std::cout << "   판별: " << (R_c > 0 ? "코너" : (R_c < 0 ? "에지" : "평면")) << "\n" << std::endl;

    // 추가: k 값 변화에 따른 영향
    std::cout << "--- k 값 변화에 따른 케이스 1 (코너)의 R 변화 ---\n" << std::endl;
    std::vector<double> k_values = {0.02, 0.04, 0.06, 0.10};
    for (double kv : k_values)
    {
        // ✅ 정답: 케이스 1에 대해 각 k로 R 계산
        double R_k = det_a - kv * trace_a * trace_a;
        std::cout << "   k = " << kv << " → R = " << R_k
                  << (R_k > 0 ? " (코너)" : " (코너 아님)") << std::endl;
    }

    std::cout << "\n💡 결론:" << std::endl;
    std::cout << "   - R >> 0: 코너 (두 고유값 모두 큼)" << std::endl;
    std::cout << "   - R << 0: 에지 (한 고유값만 큼)" << std::endl;
    std::cout << "   - |R| ≈ 0: 평면 (두 고유값 모두 작음)" << std::endl;
    std::cout << "   - k가 커지면 코너 판별 기준이 엄격해짐 (R 감소)" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 3 Quiz - Easy (정답)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_fast_threshold();
    problem2_orb_descriptor();
    problem3_nms_effect();
    problem4_speed_comparison();
    problem5_harris_response();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
