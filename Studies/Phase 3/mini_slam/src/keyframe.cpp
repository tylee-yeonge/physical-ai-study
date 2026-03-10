#include "keyframe.h"

#include <algorithm>

namespace slam {

// ─────────────────────────────────────────────────────────────
// Keyframe
// ─────────────────────────────────────────────────────────────

Keyframe::Keyframe(int id, const cv::Mat& image, const Pose& pose)
    : id_(id), image_(image.clone()), pose_(pose)
{
}

void Keyframe::addCovisibility(int other_kf_id, int num_shared_points)
{
    covisibility_[other_kf_id] = num_shared_points;
}

std::vector<int> Keyframe::getTopCovisibleKeyframes(int n) const
{
    // 공유 맵 포인트 수 기준으로 내림차순 정렬
    std::vector<std::pair<int, int>> sorted_covis(covisibility_.begin(),
                                                   covisibility_.end());
    std::sort(sorted_covis.begin(), sorted_covis.end(),
              [](const auto& a, const auto& b)
              {
                  return a.second > b.second;
              });

    std::vector<int> result;
    int count = std::min(n, static_cast<int>(sorted_covis.size()));
    for (int i = 0; i < count; i++)
    {
        result.push_back(sorted_covis[i].first);
    }
    return result;
}

// ─────────────────────────────────────────────────────────────
// KeyframeSelector
// ─────────────────────────────────────────────────────────────

KeyframeSelector::KeyframeSelector(int min_frames,
                                   int max_frames,
                                   double min_tracked_ratio,
                                   double min_parallax_deg)
    : min_frames_(min_frames),
      max_frames_(max_frames),
      min_tracked_ratio_(min_tracked_ratio),
      min_parallax_deg_(min_parallax_deg)
{
}

bool KeyframeSelector::needNewKeyframe(int frames_since_last,
                                       int num_tracked,
                                       int total_map_points,
                                       double avg_parallax_deg) const
{
    // 최소 간격 미충족 → 생성 안 함
    if (frames_since_last < min_frames_)
        return false;

    // 최대 간격 초과 → 무조건 생성
    if (frames_since_last >= max_frames_)
        return true;

    // 추적 비율이 낮으면 새 키프레임 필요 (특징점이 부족해지고 있음)
    if (total_map_points > 0)
    {
        double tracked_ratio = static_cast<double>(num_tracked) / total_map_points;
        if (tracked_ratio < min_tracked_ratio_)
            return true;
    }

    // 시차가 충분하면 새 키프레임 생성
    if (avg_parallax_deg >= min_parallax_deg_)
        return true;

    return false;
}

// ─────────────────────────────────────────────────────────────
// KeyframeCuller
// ─────────────────────────────────────────────────────────────

KeyframeCuller::KeyframeCuller(double redundancy_threshold)
    : redundancy_threshold_(redundancy_threshold)
{
}

int KeyframeCuller::cullRedundant(std::vector<Keyframe*>& keyframes,
                                  const std::vector<MapPoint>& map_points) const
{
    // 90% rule: 키프레임이 관측하는 맵 포인트의 90% 이상이
    // 다른 3개 이상 키프레임에서도 관측되면 중복으로 판단
    int removed = 0;
    const int kMinObservers = 3;

    // 첫 번째 키프레임은 제거하지 않음 (앵커)
    for (size_t i = 1; i < keyframes.size(); /* i 조절은 내부에서 */)
    {
        Keyframe* kf = keyframes[i];
        int total_points = 0;
        int redundant_points = 0;

        for (int mp_id : kf->map_point_ids_)
        {
            if (mp_id < 0 || mp_id >= static_cast<int>(map_points.size()))
                continue;

            const MapPoint& mp = map_points[mp_id];
            if (mp.is_outlier)
                continue;

            total_points++;

            // 이 맵 포인트를 관측하는 다른 키프레임이 3개 이상이면 중복
            if (mp.num_observations >= kMinObservers)
                redundant_points++;
        }

        double ratio = (total_points > 0)
                            ? static_cast<double>(redundant_points) / total_points
                            : 0.0;

        if (ratio > redundancy_threshold_)
        {
            delete kf;
            keyframes.erase(keyframes.begin() + i);
            removed++;
        }
        else
        {
            i++;
        }
    }

    return removed;
}

}  // namespace slam
