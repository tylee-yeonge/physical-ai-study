#include "descriptor.h"

#include <iostream>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>

Descriptor::Descriptor(int patch_size, int n_pairs, int seed)
    : patch_size_(patch_size), n_pairs_(n_pairs)
{
    generatePairs(seed);
}

// ─────────────────────────────────────────────────────────────────────────────
// 픽셀쌍 오프셋 생성
//
// BRIEF 논문에서는 가우시안 분포로 샘플링하는 방법을 권장한다.
// σ = patch_size / 5 로 두면 패치 중심 근처에 집중되어 노이즈에 강하다.
//
// 각 쌍: {x1, y1, x2, y2} — 키포인트 중심(0, 0) 기준 상대 오프셋
// 범위: [-half, +half] 로 클리핑 (half = patch_size / 2)
//
// TODO:
//  1. cv::RNG rng(seed) 로 난수 생성기를 만든다.
//  2. n_pairs_번 반복하며 가우시안 분포(평균 0, 표준편차 sigma)에서
//     x1, y1, x2, y2를 각각 샘플링한다.
//     힌트: rng.gaussian(sigma) 사용
//  3. 각 값을 [-half, +half] 범위로 클리핑한다.
//     힌트: std::clamp(val, -half, half)
//  4. pairs_에 {x1, y1, x2, y2} 를 push_back한다.
// ─────────────────────────────────────────────────────────────────────────────
void Descriptor::generatePairs(int seed)
{
    pairs_.clear();
    pairs_.reserve(n_pairs_);

    // TODO
    (void)seed;
}

// ─────────────────────────────────────────────────────────────────────────────
// BRIEF 디스크립터 계산
//
// 각 키포인트에 대해:
//  1. 패치 추출 전 가우시안 블러 (sigma=2) — 노이즈 억제
//  2. n_pairs_ 개 쌍을 비교해서 비트열 생성
//     I(p1) < I(p2) → 1, 아니면 → 0
//  3. 비트 8개를 1바이트로 패킹 → 32바이트 디스크립터
//
// 경계 처리:
//  - 키포인트가 이미지 경계에서 half 픽셀 이내면 제외한다.
//  - 제외 후 keypoints와 반환 Mat의 행 수가 일치해야 한다.
//
// TODO:
//  1. half = patch_size_ / 2 계산
//  2. 경계 키포인트 필터링
//     힌트: 새 vector로 유효한 키포인트만 모으고 마지막에 keypoints = 교체
//  3. 유효한 키포인트마다:
//     a. 주변 패치 추출 (cv::getRectSubPix 또는 이미지 영역 직접 접근)
//     b. cv::GaussianBlur(patch, blurred, cv::Size(5,5), 2.0) 적용
//     c. pairs_의 각 쌍에 대해 픽셀 비교 → 비트 1개 생성
//     d. 비트 8개를 1바이트로 누적, 32바이트를 디스크립터 1행으로 저장
// ─────────────────────────────────────────────────────────────────────────────
cv::Mat Descriptor::compute(const cv::Mat& img,
                            std::vector<cv::KeyPoint>& keypoints) const
{
    // TODO
    (void)img;
    (void)keypoints;

    // 임시: 빈 행렬 반환 (구현 전)
    return cv::Mat();
}

// ─────────────────────────────────────────────────────────────────────────────
// OpenCV ORB 디스크립터와 비교 (검증용)
//
// 직접 구현한 BRIEF와 cv::ORB 디스크립터의 해밍 거리 분포를 비교한다.
// 두 구현이 같은 이미지에서 같은 키포인트를 쓸 때,
// 자기 자신과의 해밍 거리는 0이어야 한다. (동일성 검증)
// ─────────────────────────────────────────────────────────────────────────────
void Descriptor::compareWithOpenCV(const cv::Mat& img,
                                   std::vector<cv::KeyPoint>& keypoints) const
{
    // 직접 구현
    auto kps_copy = keypoints;
    cv::Mat my_desc = compute(img, kps_copy);

    // OpenCV ORB 디스크립터
    auto orb = cv::ORB::create();
    std::vector<cv::KeyPoint> cv_kps = keypoints;
    cv::Mat cv_desc;
    orb->compute(img, cv_kps, cv_desc);

    std::cout << "\n[BRIEF 디스크립터 비교]" << std::endl;

    if (my_desc.empty())
    {
        std::cout << "  직접 구현: 미완성 (TODO)" << std::endl;
    }
    else
    {
        std::cout << "  직접 구현: " << my_desc.rows << "개 키포인트"
                  << " (" << my_desc.cols << "바이트/디스크립터)" << std::endl;
    }

    std::cout << "  cv::ORB:   " << cv_desc.rows << "개 키포인트"
              << " (" << cv_desc.cols << "바이트/디스크립터)" << std::endl;

    // 직접 구현이 완성된 경우 자기 자신과의 해밍 거리 확인
    if (!my_desc.empty() && my_desc.rows > 0)
    {
        // 같은 디스크립터끼리 매칭 → 해밍 거리가 모두 0이어야 함
        cv::BFMatcher matcher(cv::NORM_HAMMING);
        std::vector<cv::DMatch> matches;
        matcher.match(my_desc, my_desc, matches);

        int nonzero = 0;
        for (const auto& m : matches)
            if (m.distance > 0)
                nonzero++;

        std::cout << "  자기매칭 비영 해밍거리: " << nonzero
                  << "개 (0이어야 정상)" << std::endl;
    }
}
