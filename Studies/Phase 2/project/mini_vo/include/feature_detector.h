#ifndef MINI_VO_FEATURE_DETECTOR_H_
#define MINI_VO_FEATURE_DETECTOR_H_

#include <opencv2/core.hpp>
#include <vector>

/**
 * @brief 특징점 검출기 — FAST 직접 구현 (P2-W3)
 *
 * cv::FAST를 사용하지 않고 논문 알고리즘을 직접 구현한다.
 *
 * 핵심 구성:
 *  1. 브레젠험 원(radius=3)의 16개 픽셀 오프셋
 *  2. 고속 사전 검사: 1, 5, 9, 13번 픽셀 먼저 확인 (~80% 조기 제외)
 *  3. Segment test: N개 연속으로 밝거나 어두운 픽셀 존재 여부
 *  4. 코너 점수: 16개 픽셀 밝기 차이 합 (NMS 기준)
 *  5. NMS: 지역 반경 내 최고 점수만 생존
 */
class FeatureDetector
{
public:
    /**
     * @brief 생성자
     * @param threshold  밝기 차이 임계값 t (기본 20)
     * @param n          연속 픽셀 수 기준 (기본 9, 보통 9~12)
     * @param nms_radius NMS 반경 (기본 3)
     */
    explicit FeatureDetector(int threshold = 20, int n = 9, int nms_radius = 3);

    /**
     * @brief FAST 직접 구현으로 특징점 검출
     * @param img       입력 그레이스케일 이미지
     * @return 검출된 KeyPoint 목록 (점수 내림차순)
     */
    std::vector<cv::KeyPoint> detect(const cv::Mat& img) const;

    /**
     * @brief OpenCV cv::FAST 결과와 비교 출력
     * @param img 입력 그레이스케일 이미지
     */
    void compareWithOpenCV(const cv::Mat& img) const;

    int threshold_;   ///< 밝기 차이 임계값
    int n_;           ///< 연속 픽셀 수 기준
    int nms_radius_;  ///< NMS 반경

private:
    /**
     * @brief 브레젠험 원(radius=3) 위 16개 픽셀의 (col, row) 오프셋
     *
     * 1번 픽셀(정상단)부터 시계 방향으로 나열.
     * 픽셀 번호와 인덱스 관계: 픽셀 k → kCircle_[k-1]
     */
    static const int kCircle_[16][2];

    /**
     * @brief 고속 사전 검사 — 1, 5, 9, 13번 픽셀 검사
     *
     * 코너가 되려면 4개 중 최소 3개가 밝거나 어두워야 한다.
     * 이 조건을 통과하지 못하면 나머지 12개 검사를 생략한다.
     *
     * @param row   중심 픽셀 행
     * @param col   중심 픽셀 열
     * @param ptr   이미지 데이터 포인터
     * @param step  이미지 행 stride (bytes)
     * @param ip    중심 픽셀 밝기
     * @return 사전 검사 통과 여부
     */
    bool highSpeedTest(int row, int col, const uchar* ptr, int step, int ip) const;

    /**
     * @brief Segment test — N개 연속 픽셀 검사
     * @param row  중심 픽셀 행
     * @param col  중심 픽셀 열
     * @param ptr  이미지 데이터 포인터
     * @param step 이미지 행 stride
     * @param ip   중심 픽셀 밝기
     * @return 코너 판정 여부
     */
    bool segmentTest(int row, int col, const uchar* ptr, int step, int ip) const;

    /**
     * @brief 코너 점수 계산
     *
     * 16개 픽셀의 밝기와 중심 픽셀 밝기 차이의 절대값 합.
     * NMS에서 "더 강한 코너" 기준으로 사용한다.
     *
     * @param row  중심 픽셀 행
     * @param col  중심 픽셀 열
     * @param ptr  이미지 데이터 포인터
     * @param step 이미지 행 stride
     * @param ip   중심 픽셀 밝기
     * @return 코너 점수 (클수록 강한 코너)
     */
    int cornerScore(int row, int col, const uchar* ptr, int step, int ip) const;

    /**
     * @brief Non-Maximum Suppression
     *
     * nms_radius_ 반경 내에서 자신보다 점수 높은 이웃이 있으면 제거한다.
     *
     * @param keypoints 원본 키포인트 목록
     * @param rows      이미지 행 수 (경계 처리용)
     * @param cols      이미지 열 수 (경계 처리용)
     * @return NMS 적용 후 키포인트 목록
     */
    std::vector<cv::KeyPoint> nonMaxSuppression(
        const std::vector<cv::KeyPoint>& keypoints, int rows, int cols) const;
};

#endif  // MINI_VO_FEATURE_DETECTOR_H_
