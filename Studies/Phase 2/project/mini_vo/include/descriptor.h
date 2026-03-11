#ifndef MINI_VO_DESCRIPTOR_H_
#define MINI_VO_DESCRIPTOR_H_

#include <array>
#include <opencv2/core.hpp>
#include <vector>

/**
 * @brief BRIEF 디스크립터 — 직접 구현 (P2-W3)
 *
 * BRIEF(Binary Robust Independent Elementary Features):
 * 키포인트 주변 패치에서 픽셀 쌍 N개를 비교해
 * 0 또는 1로 이루어진 이진 디스크립터를 만든다.
 *
 *   쌍 (p, q) → I(p) < I(q) 이면 1, 아니면 0
 *   256쌍 → 256비트 = 32바이트
 *
 * 매칭 거리: XOR 비트 연산 → 해밍 거리 (빠른 비교)
 *
 * 구현 순서:
 *  1. generatePairs() — 가우시안 분포로 256개 픽셀쌍 오프셋 생성
 *  2. compute()       — 각 키포인트에서 쌍 비교 → 비트열 → 32바이트 Mat
 */
class Descriptor
{
public:
    /**
     * @brief 생성자
     * @param patch_size 키포인트 주변 패치 크기 (픽셀, 홀수 권장)
     * @param n_pairs    비트 수 = 픽셀 쌍 수 (8의 배수, 기본 256)
     * @param seed       픽셀쌍 생성 랜덤 시드 (고정하면 재현 가능)
     */
    explicit Descriptor(int patch_size = 31, int n_pairs = 256, int seed = 42);

    /**
     * @brief 키포인트 목록에서 BRIEF 디스크립터 계산
     *
     * 이미지 경계에 걸리는 키포인트는 자동으로 제외된다.
     * keypoints는 경계 제외 후 남은 것만 유지하도록 in-place 수정된다.
     *
     * @param img        입력 그레이스케일 이미지
     * @param keypoints  입력/출력: 키포인트 목록 (경계 키포인트 제거됨)
     * @return           디스크립터 행렬 (rows=키포인트 수, cols=32, type=CV_8UC1)
     */
    cv::Mat compute(const cv::Mat& img,
                    std::vector<cv::KeyPoint>& keypoints) const;

    /**
     * @brief OpenCV ORB 디스크립터와 비교 출력 (구현 검증용)
     * @param img       입력 그레이스케일 이미지
     * @param keypoints 키포인트 목록
     */
    void compareWithOpenCV(const cv::Mat& img,
                           std::vector<cv::KeyPoint>& keypoints) const;

    int patch_size_;  ///< 패치 크기
    int n_pairs_;     ///< 픽셀 쌍 수 (= 비트 수)

private:
    /**
     * @brief 픽셀쌍 오프셋 생성
     *
     * 각 쌍: {x1, y1, x2, y2} — 키포인트 중심 기준 상대 오프셋
     * 가우시안 분포(σ = patch_size / 5)에서 샘플링한다.
     *
     * TODO: cv::RNG(seed_)로 n_pairs_개 쌍을 생성하고
     *       패치 경계(-half ~ +half)로 클리핑하라.
     *
     * @param seed 랜덤 시드
     */
    void generatePairs(int seed);

    /** 생성된 픽셀쌍 오프셋 목록 */
    std::vector<std::array<int, 4>> pairs_;
};

#endif  // MINI_VO_DESCRIPTOR_H_
