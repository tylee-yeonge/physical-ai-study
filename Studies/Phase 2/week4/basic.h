#ifndef PHASE2_WEEK4_BASIC_H_
#define PHASE2_WEEK4_BASIC_H_

/**
 * @file basic.h
 * @brief Week 4: 삼각측량 + PnP 헬퍼
 *
 * Perception 맥락:
 *   - 삼각측량 = Multi-view 3D Detection 의 기초 연산
 *   - PnP = 3D 박스 ↔ 2D 이미지 관계 검증
 *   - 재투영 오차 = 3D Detection 평가 지표
 */

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

/**
 * @brief 합성 3D 박스 (8개 코너) 생성
 * @param center 박스 중심 (미터)
 * @param size 박스 크기 (width, height, depth) (미터)
 * @return 8개 코너 좌표
 */
std::vector<cv::Point3d> generate_3d_box(
    const cv::Point3d& center,
    const cv::Vec3d& size);

/**
 * @brief 3D 점들을 카메라에 투영
 * @param points_3d 3D 점 목록
 * @param K 카메라 내부 행렬
 * @param R 회전 행렬
 * @param t 이동 벡터
 * @return 2D 투영 좌표
 */
std::vector<cv::Point2d> project_points(
    const std::vector<cv::Point3d>& points_3d,
    const cv::Mat& K,
    const cv::Mat& R,
    const cv::Mat& t);

/**
 * @brief 삼각측량으로 3D 점 복원
 * @param pts1 첫 번째 카메라의 2D 점
 * @param pts2 두 번째 카메라의 2D 점
 * @param P1 첫 번째 카메라 투영 행렬 (3×4)
 * @param P2 두 번째 카메라 투영 행렬 (3×4)
 * @return 복원된 3D 점
 */
std::vector<cv::Point3d> triangulate(
    const std::vector<cv::Point2d>& pts1,
    const std::vector<cv::Point2d>& pts2,
    const cv::Mat& P1,
    const cv::Mat& P2);

/**
 * @brief 재투영 오차 계산
 * @param points_3d 3D 점
 * @param points_2d 관측된 2D 점
 * @param K 카메라 내부 행렬
 * @param rvec 회전 벡터
 * @param tvec 이동 벡터
 * @return 각 점의 재투영 오차 (px)
 */
std::vector<double> compute_reprojection_errors(
    const std::vector<cv::Point3d>& points_3d,
    const std::vector<cv::Point2d>& points_2d,
    const cv::Mat& K,
    const cv::Mat& rvec,
    const cv::Mat& tvec);

/**
 * @brief 결과를 output/ 디렉토리에 저장
 */
void save_output(const std::string& name, const cv::Mat& image);

#endif  // PHASE2_WEEK4_BASIC_H_
