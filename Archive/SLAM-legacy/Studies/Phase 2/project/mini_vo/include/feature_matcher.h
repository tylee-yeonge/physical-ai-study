#ifndef MINI_VO_FEATURE_MATCHER_H_
#define MINI_VO_FEATURE_MATCHER_H_

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <vector>

/**
 * @brief Brute-Force 특징점 매칭기 (P2-W4)
 *
 * 구현 흐름:
 *  1. bfMatch()      — 디스크립터 간 해밍 거리로 최근접 2개 이웃 탐색
 *  2. ratioTest()    — Lowe's Ratio Test로 모호한 매칭 제거
 *  3. crossCheck()   — 양방향 매칭이 일치하는 쌍만 선택
 *  4. match()        — 위 세 단계를 조합해 최종 매칭 반환
 *
 * 해밍 거리: XOR → popcount (이진 디스크립터에 최적)
 * Ratio Test: d1 / d2 < ratio_thresh → 최근접이 충분히 가까운 경우만 통과
 * Cross-check: i→j 매칭과 j→i 매칭이 서로 동일한 쌍만 유지
 */
class FeatureMatcher
{
public:
    /**
     * @brief 생성자
     * @param ratio_thresh Lowe's Ratio Test 임계값 (기본 0.75)
     */
    explicit FeatureMatcher(float ratio_thresh = 0.75f);

    /**
     * @brief 두 디스크립터 집합을 매칭해 인라이어 쌍 반환
     *
     * 내부적으로 bfMatch → ratioTest → crossCheck 순서로 수행한다.
     *
     * @param desc1 첫 번째 프레임 디스크립터 (rows=N, cols=32, CV_8UC1)
     * @param desc2 두 번째 프레임 디스크립터 (rows=M, cols=32, CV_8UC1)
     * @return      필터링된 DMatch 목록 (queryIdx: desc1, trainIdx: desc2)
     */
    std::vector<cv::DMatch> match(
        const cv::Mat& desc1,
        const cv::Mat& desc2) const;

private:
    /**
     * @brief desc1의 각 행에 대해 desc2에서 가장 가까운 2개 이웃 탐색
     *
     * 결과는 knn_matches[i] = {best, second_best} 형태로 반환된다.
     * 거리 지표: 해밍 거리 (이진 디스크립터용 XOR + popcount)
     *
     * TODO: desc1의 각 행 i에 대해
     *       - desc2의 모든 행 j와 hamming(desc1[i], desc2[j])를 계산
     *       - 거리 기준 상위 2개를 knn_matches[i]에 저장
     *
     * @param desc1      쿼리 디스크립터
     * @param desc2      탐색 대상 디스크립터
     * @return           knn_matches[i]: i번째 쿼리의 {1-NN, 2-NN} DMatch 쌍
     */
    std::vector<std::vector<cv::DMatch>> bfMatch(
        const cv::Mat& desc1,
        const cv::Mat& desc2) const;

    /**
     * @brief Lowe's Ratio Test — 모호한 매칭 제거
     *
     * 조건: knn[0].distance < ratio_thresh_ * knn[1].distance
     * 위 조건을 만족하는 매칭(knn[0])만 통과시킨다.
     *
     * TODO: knn_matches를 순회하며 위 조건을 만족하는 것만 골라 반환
     *
     * @param knn_matches bfMatch() 결과
     * @return            Ratio Test 통과 매칭 목록
     */
    std::vector<cv::DMatch> ratioTest(
        const std::vector<std::vector<cv::DMatch>>& knn_matches) const;

    /**
     * @brief Cross-check — 양방향 매칭이 일치하는 쌍만 선택
     *
     * forward:  desc1[i] → desc2[j] (최근접)
     * backward: desc2[j] → desc1[k] (최근접)
     * i == k 인 경우에만 진짜 매칭으로 채택한다.
     *
     * TODO:
     *  1. desc2 → desc1 방향으로 bfMatch() 수행해 backward 맵 생성
     *     backward_map[j] = desc1에서 desc2[j]의 최근접 인덱스
     *  2. forward의 각 매칭 (i→j)에 대해 backward_map[j] == i 확인
     *  3. 조건 만족 시 매칭 유지
     *
     * @param forward_matches ratioTest() 통과 매칭 목록
     * @param desc1           쿼리 디스크립터 (backward 탐색에 사용)
     * @param desc2           탐색 대상 디스크립터 (backward 탐색에 사용)
     * @return                Cross-check 통과 매칭 목록
     */
    std::vector<cv::DMatch> crossCheck(
        const std::vector<cv::DMatch>& forward_matches,
        const cv::Mat& desc1,
        const cv::Mat& desc2) const;

    /**
     * @brief 두 이진 디스크립터(32바이트) 간 해밍 거리 계산
     *
     * 해밍 거리 = XOR 결과에서 1인 비트 수 (popcount)
     *
     * TODO: row_a와 row_b를 XOR한 뒤 각 바이트의 popcount 합산
     *       힌트: __builtin_popcount() 또는 직접 비트 카운팅
     *
     * @param row_a 첫 번째 디스크립터 행 (1 x 32, CV_8UC1)
     * @param row_b 두 번째 디스크립터 행 (1 x 32, CV_8UC1)
     * @return      해밍 거리 (0 ~ 256)
     */
    int hammingDistance(const cv::Mat& row_a, const cv::Mat& row_b) const;

    float ratio_thresh_;  ///< Lowe's Ratio Test 임계값
};

#endif  // MINI_VO_FEATURE_MATCHER_H_
