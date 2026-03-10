#include "feature_detector.h"

#include <algorithm>
#include <iostream>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>

// ─────────────────────────────────────────────────────────────────────────────
// 브레젠험 원(radius=3) 위 16개 픽셀의 (col_offset, row_offset)
//
//         1  2  3
//      16         4
//     15    ●     5
//      14         6
//         13 12 11 10 9
//              8  7
//
// TODO: 1번(정상단)부터 시계 방향으로 16개 오프셋을 채우세요.
//       형식: {col_offset, row_offset}
//       힌트: 1번 픽셀은 중심에서 정상단 → {0, -3}
// ─────────────────────────────────────────────────────────────────────────────
const int FeatureDetector::kCircle_[16][2] = {
    // TODO: 16개 (col, row) 오프셋 채우기
    {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {0, 0}, {0, 0}, {0, 0}, {0, 0},
};

FeatureDetector::FeatureDetector(int threshold, int n, int nms_radius)
    : threshold_(threshold), n_(n), nms_radius_(nms_radius)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// 고속 사전 검사: 1, 5, 9, 13번 픽셀 (상하좌우)
//
// 아이디어: N=9 기준, 연속 9개가 존재하려면
//           상하좌우 4개 중 최소 3개가 같은 방향(밝거나 어둡거나)이어야 한다.
//           2개 이하면 → 나머지 12개 검사 생략 (조기 탈락)
//
// TODO: 4개 픽셀(인덱스 0, 4, 8, 12)을 읽어서
//       밝은 것 3개 이상 OR 어두운 것 3개 이상이면 true 반환
// ─────────────────────────────────────────────────────────────────────────────
bool FeatureDetector::highSpeedTest(
    int row, int col, const uchar* ptr, int step, int ip) const
{
    // TODO
    (void)row; (void)col; (void)ptr; (void)step; (void)ip;
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Segment test: 16개 픽셀을 순환하며 N개 연속 밝거나 어두운지 검사
//
// 힌트: 원형 순환 처리를 위해 인덱스를 i % 16 으로 감싸면 된다.
//       연속 카운터를 유지하다가 n_ 이상이면 즉시 true 반환.
//
// TODO: 각 픽셀을 밝음(+1) / 어두움(-1) / 중간(0) 으로 분류하고
//       n_개 연속 동일 상태가 있으면 true 반환
// ─────────────────────────────────────────────────────────────────────────────
bool FeatureDetector::segmentTest(
    int row, int col, const uchar* ptr, int step, int ip) const
{
    // TODO
    (void)row; (void)col; (void)ptr; (void)step; (void)ip;
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// 코너 점수: NMS에서 "더 강한 코너" 기준으로 사용
//
// TODO: 16개 픽셀과 중심 픽셀(ip)의 밝기 차이 절대값 합을 반환
// ─────────────────────────────────────────────────────────────────────────────
int FeatureDetector::cornerScore(
    int row, int col, const uchar* ptr, int step, int ip) const
{
    // TODO
    (void)row; (void)col; (void)ptr; (void)step; (void)ip;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Non-Maximum Suppression
//
// 아이디어: nms_radius_ 반경 내에서 자신보다 점수 높은 이웃이 있으면 제거.
//
// TODO: score_map(이미지 크기) 을 만들고,
//       각 키포인트에 대해 반경 내 최대값인지 확인 후 결과 목록에 추가
// ─────────────────────────────────────────────────────────────────────────────
std::vector<cv::KeyPoint> FeatureDetector::nonMaxSuppression(
    const std::vector<cv::KeyPoint>& keypoints, int rows, int cols) const
{
    // TODO
    (void)rows; (void)cols;
    return keypoints;  // 임시: NMS 없이 전체 반환
}

// ─────────────────────────────────────────────────────────────────────────────
// 메인 검출 함수 — 이 함수는 수정하지 않아도 됩니다.
// 위의 4개 함수를 올바르게 구현하면 자동으로 동작합니다.
// ─────────────────────────────────────────────────────────────────────────────
std::vector<cv::KeyPoint> FeatureDetector::detect(const cv::Mat& img) const
{
    CV_Assert(img.type() == CV_8UC1);

    const int margin = 3;
    const int rows = img.rows;
    const int cols = img.cols;
    const int step = static_cast<int>(img.step);
    const uchar* ptr = img.data;

    std::vector<cv::KeyPoint> candidates;

    for (int row = margin; row < rows - margin; row++)
    {
        for (int col = margin; col < cols - margin; col++)
        {
            int ip = ptr[row * step + col];

            if (!highSpeedTest(row, col, ptr, step, ip))
                continue;

            if (!segmentTest(row, col, ptr, step, ip))
                continue;

            int score = cornerScore(row, col, ptr, step, ip);

            cv::KeyPoint kp;
            kp.pt = cv::Point2f(static_cast<float>(col), static_cast<float>(row));
            kp.response = static_cast<float>(score);
            kp.size = 7.0f;
            candidates.push_back(kp);
        }
    }

    std::vector<cv::KeyPoint> result = nonMaxSuppression(candidates, rows, cols);

    std::sort(result.begin(), result.end(),
              [](const cv::KeyPoint& a, const cv::KeyPoint& b)
              { return a.response > b.response; });

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// OpenCV cv::FAST와 비교 — 구현 검증용
// ─────────────────────────────────────────────────────────────────────────────
void FeatureDetector::compareWithOpenCV(const cv::Mat& img) const
{
    auto my_kps = detect(img);

    std::vector<cv::KeyPoint> cv_kps;
    cv::FAST(img, cv_kps, threshold_, true);

    std::cout << "\n[FAST 비교]" << std::endl;
    std::cout << "  직접 구현: " << my_kps.size() << "개" << std::endl;
    std::cout << "  cv::FAST:  " << cv_kps.size() << "개" << std::endl;

    int top = std::min(5, static_cast<int>(my_kps.size()));
    std::cout << "\n  [직접 구현] 상위 " << top << "개:" << std::endl;
    for (int i = 0; i < top; i++)
        std::cout << "    (" << my_kps[i].pt.x << ", " << my_kps[i].pt.y
                  << ")  score=" << my_kps[i].response << std::endl;

    std::sort(cv_kps.begin(), cv_kps.end(),
              [](const cv::KeyPoint& a, const cv::KeyPoint& b)
              { return a.response > b.response; });
    top = std::min(5, static_cast<int>(cv_kps.size()));
    std::cout << "\n  [cv::FAST]  상위 " << top << "개:" << std::endl;
    for (int i = 0; i < top; i++)
        std::cout << "    (" << cv_kps[i].pt.x << ", " << cv_kps[i].pt.y
                  << ")  score=" << cv_kps[i].response << std::endl;
}
