#ifndef MINI_SLAM_MAP_H_
#define MINI_SLAM_MAP_H_

#include "keyframe.h"
#include "types.h"

#include <vector>

namespace slam {

/**
 * @brief SLAM 맵 관리자 (P3-W6)
 *
 * 키프레임과 맵 포인트를 저장·관리한다.
 * 키프레임 추가/제거, 맵 포인트 추가/제거, 공가시성 갱신을 담당한다.
 */
class Map
{
public:
    Map();

    // ── 키프레임 관리 ──

    /** @brief 키프레임 추가 (소유권은 Map이 가짐) */
    void addKeyframe(Keyframe* kf);

    /** @brief ID로 키프레임 검색 (없으면 nullptr) */
    Keyframe* getKeyframe(int id) const;

    /** @brief 전체 키프레임 수 */
    int numKeyframes() const;

    /** @brief 전체 키프레임 목록 */
    std::vector<Keyframe*>& keyframes();

    // ── 맵 포인트 관리 ──

    /** @brief 맵 포인트 추가, 부여된 ID 반환 */
    int addMapPoint(const cv::Point3f& position);

    /** @brief ID로 맵 포인트 참조 */
    MapPoint& getMapPoint(int id);
    const MapPoint& getMapPoint(int id) const;

    /** @brief 전체 맵 포인트 수 */
    int numMapPoints() const;

    /** @brief 전체 맵 포인트 목록 (const) */
    const std::vector<MapPoint>& mapPoints() const;

    /** @brief 아웃라이어 맵 포인트 제거 */
    void removeOutlierMapPoints();

    // ── 공가시성 ──

    /** @brief 공가시성 그래프 갱신 (모든 키프레임 간) */
    void updateCovisibility();

    // ── 정리 ──

    /** @brief 모든 키프레임 메모리 해제 */
    void clear();

    ~Map();

private:
    std::vector<Keyframe*> keyframes_;
    std::vector<MapPoint> map_points_;
    int next_mp_id_;
};

}  // namespace slam

#endif  // MINI_SLAM_MAP_H_
