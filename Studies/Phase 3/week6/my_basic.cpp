/**
 * Phase 3 Week 6 - Keyframe 관리 직접 구현
 *
 * keyframe_manager.h의 클래스들을 직접 구현해보세요.
 * (테스트 파일 없음 - 직접 main에서 테스트하세요)
 */
#include "keyframe_manager.h"
#include <iostream>
#include <algorithm>

// Keyframe
Keyframe::Keyframe(int id, const cv::Mat& img) : id(id), image(img.clone()), num_tracked(0)
{
    R = Eigen::Matrix3d::Identity();
    t = Eigen::Vector3d::Zero();
}

void Keyframe::addCovisibility(Keyframe* other, int num_shared)
{
    // TODO: covisible_keyframes에 추가
}

std::vector<Keyframe*> Keyframe::getCovisibleKeyframes(int min_shared)
{
    // TODO: min_shared 이상 공유하는 keyframe 반환
    return {};
}

// KeyframeSelector
KeyframeSelector::KeyframeSelector(int min_frames, int max_frames, double min_tracked_ratio,
                                   double min_parallax)
    : min_frames_(min_frames),
      max_frames_(max_frames),
      min_tracked_ratio_(min_tracked_ratio),
      min_parallax_(min_parallax)
{
}

bool KeyframeSelector::needNewKeyframe(int num_frames_since_last, int num_tracked,
                                       int total_map_points, double avg_parallax)
{
    // TODO: 새 keyframe 필요 여부 판단
    return false;
}

// KeyframeCuller
KeyframeCuller::KeyframeCuller(double redundancy_threshold)
    : redundancy_threshold_(redundancy_threshold)
{
}

void KeyframeCuller::cullRedundantKeyframes(std::vector<Keyframe*>& keyframes,
                                            const std::vector<MapPoint>& map_points)
{
    // TODO: 중복 keyframe 제거
}

bool KeyframeCuller::isRedundant(const Keyframe* kf, const std::vector<MapPoint>& map_points)
{
    // TODO: keyframe 중복 판단
    return false;
}

// CovisibilityGraph
void CovisibilityGraph::updateCovisibility(Keyframe* kf1, Keyframe* kf2,
                                           const std::vector<MapPoint>& map_points)
{
    // TODO: covisibility 업데이트
}

std::vector<Keyframe*> CovisibilityGraph::getLocalKeyframes(Keyframe* curr_kf, int max_keyframes)
{
    // TODO: local keyframes 반환
    return {};
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] Keyframe 관리 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Keyframe 생성
    cv::Mat img = cv::Mat::zeros(480, 640, CV_8UC1);
    Keyframe kf1(0, img), kf2(1, img), kf3(2, img);

    // Covisibility 추가
    kf1.addCovisibility(&kf2, 50);
    kf1.addCovisibility(&kf3, 30);

    auto covisible = kf1.getCovisibleKeyframes(20);
    std::cout << "KF0 covisible (>=20): " << covisible.size() << "개" << std::endl;

    // KeyframeSelector 테스트
    KeyframeSelector selector(5, 30, 0.7, 1.0);
    bool need = selector.needNewKeyframe(10, 50, 100, 2.0);
    std::cout << "새 키프레임 필요: " << (need ? "예" : "아니오") << std::endl;

    std::cout << "\n직접 실행하여 테스트하세요" << std::endl;
    return 0;
}
#endif
