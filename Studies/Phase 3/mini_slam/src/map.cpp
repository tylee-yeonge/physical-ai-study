#include "map.h"

#include <algorithm>
#include <set>

namespace slam {

Map::Map()
    : next_mp_id_(0)
{
}

// ── 키프레임 관리 ──

void Map::addKeyframe(Keyframe* kf)
{
    keyframes_.push_back(kf);
}

Keyframe* Map::getKeyframe(int id) const
{
    for (auto* kf : keyframes_)
    {
        if (kf->id_ == id)
            return kf;
    }
    return nullptr;
}

int Map::numKeyframes() const
{
    return static_cast<int>(keyframes_.size());
}

std::vector<Keyframe*>& Map::keyframes()
{
    return keyframes_;
}

// ── 맵 포인트 관리 ──

int Map::addMapPoint(const cv::Point3f& position)
{
    int id = next_mp_id_++;
    map_points_.emplace_back(id, position);
    return id;
}

MapPoint& Map::getMapPoint(int id)
{
    return map_points_[id];
}

const MapPoint& Map::getMapPoint(int id) const
{
    return map_points_[id];
}

int Map::numMapPoints() const
{
    return static_cast<int>(map_points_.size());
}

const std::vector<MapPoint>& Map::mapPoints() const
{
    return map_points_;
}

void Map::removeOutlierMapPoints()
{
    // 아웃라이어 플래그가 설정된 맵 포인트 제거
    // 단, ID 체계가 깨지므로 실제로는 플래그만 유지하고
    // 새 최적화 시 제외하는 방식이 더 흔하다.
    // 여기서는 플래그 설정된 포인트를 비활성화 처리한다.
    for (auto& mp : map_points_)
    {
        if (mp.is_outlier)
        {
            mp.num_observations = 0;
            mp.observer_keyframe_ids.clear();
        }
    }
}

// ── 공가시성 ──

void Map::updateCovisibility()
{
    // 모든 키프레임 쌍에 대해 공유 맵 포인트 수 계산
    for (size_t i = 0; i < keyframes_.size(); i++)
    {
        // i번 키프레임이 관측하는 맵 포인트 ID 집합
        std::set<int> points_i(keyframes_[i]->map_point_ids_.begin(),
                               keyframes_[i]->map_point_ids_.end());
        points_i.erase(-1);  // 무효 ID 제거

        for (size_t j = i + 1; j < keyframes_.size(); j++)
        {
            int shared = 0;
            for (int mp_id : keyframes_[j]->map_point_ids_)
            {
                if (mp_id >= 0 && points_i.count(mp_id))
                    shared++;
            }

            if (shared > 0)
            {
                keyframes_[i]->addCovisibility(keyframes_[j]->id_, shared);
                keyframes_[j]->addCovisibility(keyframes_[i]->id_, shared);
            }
        }
    }
}

// ── 정리 ──

void Map::clear()
{
    for (auto* kf : keyframes_)
        delete kf;
    keyframes_.clear();
    map_points_.clear();
    next_mp_id_ = 0;
}

Map::~Map()
{
    clear();
}

}  // namespace slam
