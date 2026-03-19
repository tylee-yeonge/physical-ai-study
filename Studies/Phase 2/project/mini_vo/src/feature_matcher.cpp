#include "feature_matcher.h"

FeatureMatcher::FeatureMatcher(float ratio_thresh)
    : ratio_thresh_(ratio_thresh)
{
}

int FeatureMatcher::hammingDistance(const cv::Mat& row_a, const cv::Mat& row_b) const
{
    // TODO: 두 디스크립터 행을 바이트 단위로 XOR하고,
    //       각 바이트에서 1인 비트 수(popcount)를 모두 합산해 반환한다.
    //       row_a.cols == 32이므로 총 256비트를 비교하게 된다.
    return 0;
}

std::vector<std::vector<cv::DMatch>> FeatureMatcher::bfMatch(
    const cv::Mat& desc1,
    const cv::Mat& desc2) const
{
    // TODO: desc1의 각 디스크립터 i에 대해 desc2의 모든 디스크립터 j와
    //       해밍 거리를 계산한다.
    //       거리가 가장 작은 1-NN과 2-NN을 각각 DMatch로 만들어
    //       knn_matches[i]에 저장한다.
    return {};
}

std::vector<cv::DMatch> FeatureMatcher::ratioTest(
    const std::vector<std::vector<cv::DMatch>>& knn_matches) const
{
    // TODO: 각 knn 쌍에서 1-NN 거리가 2-NN 거리의 ratio_thresh_ 배 미만인
    //       경우에만 통과시킨다. 이 조건을 만족하는 1-NN 매칭만 결과에 추가한다.
    return {};
}

std::vector<cv::DMatch> FeatureMatcher::crossCheck(
    const std::vector<cv::DMatch>& forward_matches,
    const cv::Mat& desc1,
    const cv::Mat& desc2) const
{
    // TODO: desc2→desc1 방향으로 역방향 매칭을 수행한다.
    //       forward의 각 매칭 (i→j)에 대해, desc2[j]의 최근접이 다시 desc1[i]인
    //       경우에만 진짜 매칭으로 채택한다.
    return {};
}

std::vector<cv::DMatch> FeatureMatcher::match(
    const cv::Mat& desc1,
    const cv::Mat& desc2) const
{
    // TODO: bfMatch → ratioTest → crossCheck 순서로 호출해 최종 매칭을 반환한다.
    return {};
}
