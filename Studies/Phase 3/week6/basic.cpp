#include "keyframe_manager.h"
#include <iostream>
#include <algorithm>

// ====================
// Keyframe
// ====================

Keyframe::Keyframe(int id, const cv::Mat& img) : id(id), image(img.clone()), num_tracked(0)
{
    R = Eigen::Matrix3d::Identity();
    t = Eigen::Vector3d::Zero();
}

void Keyframe::addCovisibility(Keyframe* other, int num_shared)
{
    covisible_keyframes[other] = num_shared;
}

std::vector<Keyframe*> Keyframe::getCovisibleKeyframes(int min_shared)
{
    std::vector<Keyframe*> result;
    for (auto& [kf, count] : covisible_keyframes)
    {
        if (count >= min_shared)
        {
            result.push_back(kf);
        }
    }
    return result;
}

// ====================
// KeyframeSelector
// ====================

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
    // 기본 조건
    if (num_frames_since_last < min_frames_)
    {
        return false;
    }

    // 조건 A: 최대 간격
    if (num_frames_since_last >= max_frames_)
    {
        std::cout << "  Keyframe: 최대 간격 도달 (" << num_frames_since_last << " frames)"
                  << std::endl;
        return true;
    }

    // 조건 B: 추적 품질
    if (total_map_points > 0)
    {
        double tracked_ratio = (double)num_tracked / total_map_points;
        if (tracked_ratio < min_tracked_ratio_)
        {
            std::cout << "  Keyframe: 추적 품질 저하 (" << tracked_ratio * 100 << "%)" << std::endl;
            return true;
        }
    }

    // 조건 C: 시차
    if (avg_parallax > min_parallax_)
    {
        std::cout << "  Keyframe: 시차 충분 (" << avg_parallax << " px)" << std::endl;
        return true;
    }

    return false;
}

// ====================
// KeyframeCuller
// ====================

KeyframeCuller::KeyframeCuller(double redundancy_threshold)
    : redundancy_threshold_(redundancy_threshold)
{
}

void KeyframeCuller::cullRedundantKeyframes(std::vector<Keyframe*>& keyframes,
                                            const std::vector<MapPoint>& map_points)
{
    int num_culled = 0;

    for (int i = keyframes.size() - 1; i > 0; i--)
    {  // 첫 KF 제외
        Keyframe* kf = keyframes[i];

        if (isRedundant(kf, map_points))
        {
            // Covisibility 업데이트
            for (auto& [other, _] : kf->covisible_keyframes)
            {
                other->covisible_keyframes.erase(kf);
            }

            delete kf;
            keyframes.erase(keyframes.begin() + i);
            num_culled++;
        }
    }

    if (num_culled > 0)
    {
        std::cout << "  Culled " << num_culled << " redundant keyframes" << std::endl;
    }
}

bool KeyframeCuller::isRedundant(const Keyframe* kf, const std::vector<MapPoint>& map_points)
{
    int redundant_obs = 0;
    int total_obs = 0;

    for (int mp_idx : kf->map_point_indices)
    {
        if (mp_idx < 0 || mp_idx >= map_points.size())
            continue;

        const MapPoint& mp = map_points[mp_idx];
        total_obs++;

        // 3개 이상 다른 Keyframe에서 관측
        if (mp.num_observations >= 3)
        {
            redundant_obs++;
        }
    }

    if (total_obs == 0)
        return false;

    double redundancy = (double)redundant_obs / total_obs;
    return redundancy > redundancy_threshold_;
}

// ====================
// CovisibilityGraph
// ====================

void CovisibilityGraph::updateCovisibility(Keyframe* kf1, Keyframe* kf2,
                                           const std::vector<MapPoint>& map_points)
{
    // 공유하는 맵 포인트 개수 세기
    int num_shared = 0;

    for (int idx1 : kf1->map_point_indices)
    {
        for (int idx2 : kf2->map_point_indices)
        {
            if (idx1 == idx2 && idx1 >= 0)
            {
                num_shared++;
            }
        }
    }

    if (num_shared >= 15)
    {  // 임계값
        kf1->addCovisibility(kf2, num_shared);
        kf2->addCovisibility(kf1, num_shared);
    }
}

std::vector<Keyframe*> CovisibilityGraph::getLocalKeyframes(Keyframe* curr_kf, int max_keyframes)
{
    // Covisibility가 높은 순서로 정렬
    std::vector<std::pair<Keyframe*, int>> covis_vec;
    for (auto& [kf, count] : curr_kf->covisible_keyframes)
    {
        covis_vec.push_back({kf, count});
    }

    std::sort(covis_vec.begin(), covis_vec.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // 상위 max_keyframes개만
    std::vector<Keyframe*> result;
    for (int i = 0; i < std::min((int)covis_vec.size(), max_keyframes); i++)
    {
        result.push_back(covis_vec[i].first);
    }

    return result;
}

// ====================
// Demo
// ====================

void demoKeyframeManagement()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Keyframe Management 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    KeyframeSelector selector;
    KeyframeCuller culler;

    std::vector<Keyframe*> keyframes;
    int last_kf_frame = 0;

    for (int frame = 0; frame < 200; frame++)
    {
        int frames_since_last = frame - last_kf_frame;

        // 시뮬레이션 데이터
        int num_tracked = 80 + rand() % 40;  // 80-120
        int total_map_points = 150;
        double avg_parallax = frames_since_last * 0.5;  // 간단한 시뮬레이션

        bool need_kf = selector.needNewKeyframe(frames_since_last, num_tracked, total_map_points,
                                                avg_parallax);

        if (need_kf)
        {
            cv::Mat dummy_img = cv::Mat::zeros(480, 640, CV_8UC1);
            Keyframe* kf = new Keyframe(frame, dummy_img);
            kf->num_tracked = num_tracked;

            keyframes.push_back(kf);
            last_kf_frame = frame;

            std::cout << "Frame " << frame << ": NEW KEYFRAME" << std::endl;
            std::cout << "  총 Keyframes: " << keyframes.size() << std::endl;
        }

        // 주기적으로 Culling
        if (frame % 50 == 0 && frame > 0)
        {
            std::vector<MapPoint> dummy_map;  // 간단한 시뮬레이션
            culler.cullRedundantKeyframes(keyframes, dummy_map);
            std::cout << "  남은 Keyframes: " << keyframes.size() << "\n" << std::endl;
        }
    }

    // 정리
    for (auto* kf : keyframes)
    {
        delete kf;
    }

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 6: Keyframe Management" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    demoKeyframeManagement();

    std::cout << "\n💡 핵심 내용:" << std::endl;
    std::cout << "   - Keyframe 선택 (시차, 품질, 간격)" << std::endl;
    std::cout << "   - Culling (90% redundancy rule)" << std::endl;
    std::cout << "   - Covisibility graph 관리" << std::endl;
    std::cout << "   - 맵 크기 제어\n" << std::endl;

    std::cout << "다음: Week 7 - Local Bundle Adjustment\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
