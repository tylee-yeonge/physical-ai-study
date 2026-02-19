/**
 * Phase 3 Week 6 - Keyframe 관리 직접 구현
 *
 * keyframe_manager.h의 클래스들을 직접 구현해보세요.
 *
 * ┌──────┬──────────────────────────┬────────┬──────────────┐
 * │ Step │ 함수                     │ 난이도  │ 검증 방법     │
 * ├──────┼──────────────────────────┼────────┼──────────────┤
 * │  1   │ addCovisibility          │ 쉬움   │ ./my_basic   │
 * │  2   │ getCovisibleKeyframes    │ 쉬움   │ ./my_basic   │
 * │  3   │ needNewKeyframe          │ 핵심   │ ./my_basic   │
 * │  4   │ isRedundant              │ 핵심   │ ./my_basic   │
 * │  5   │ cullRedundantKeyframes   │ 어려움  │ ./my_basic   │
 * └──────┴──────────────────────────┴────────┴──────────────┘
 */
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

// [Step 1] addCovisibility — covisible_keyframes에 추가
// 힌트: covisible_keyframes[other] = num_shared
// 참고: basic.cpp demoCovisibility()
void Keyframe::addCovisibility(Keyframe* other, int num_shared)
{
    // TODO: covisible_keyframes 맵에 (other, num_shared) 추가
}

// [Step 2] getCovisibleKeyframes — min_shared 이상 공유하는 KF 반환
// 힌트: covisible_keyframes를 순회하며 count >= min_shared인 것만
// 기대값: min_shared=15이면, 15 이상 공유하는 KF만 반환
std::vector<Keyframe*> Keyframe::getCovisibleKeyframes(int min_shared)
{
    // TODO: 결과 벡터 생성 → 맵 순회 → 조건 필터 → 반환
    return {};
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

// [Step 3] needNewKeyframe — 새 Keyframe 필요 여부 판단 (ORB-SLAM 방식)
// 힌트: 기본 조건 → 조건A(최대간격) → 조건B(추적품질) → 조건C(시차)
// 참고: basic.cpp demoKeyframeSelection() + README §2
// 기대값: (5, 80, 100, 5.0) → false, (30, 80, 100, 5.0) → true
bool KeyframeSelector::needNewKeyframe(int num_frames_since_last, int num_tracked,
                                       int total_map_points, double avg_parallax)
{
    // TODO:
    // 1. 기본 조건: num_frames < min_frames_ → false
    // 2. 조건A: num_frames >= max_frames_ → true
    // 3. 조건B: tracked_ratio < min_tracked_ratio_ → true
    // 4. 조건C: avg_parallax > min_parallax_ → true
    return false;
}

// ====================
// KeyframeCuller
// ====================

KeyframeCuller::KeyframeCuller(double redundancy_threshold)
    : redundancy_threshold_(redundancy_threshold)
{
}

// [Step 4] isRedundant — KF가 중복인지 판단 (90% Rule)
// 힌트: map_point_indices 순회 → num_observations >= 3이면 redundant
//       redundancy > threshold → 중복
// 참고: basic.cpp demoCullingRule() + README §3
// 기대값: 10개 중 9개가 3+ 관측 → redundancy = 0.9 → 중복!
bool KeyframeCuller::isRedundant(const Keyframe* kf, const std::vector<MapPoint>& map_points)
{
    // TODO:
    // 1. redundant_obs, total_obs 카운트
    // 2. map_point_indices 순회
    // 3. mp.num_observations >= 3 → redundant_obs++
    // 4. redundancy > redundancy_threshold_ → true
    return false;
}

// [Step 5] cullRedundantKeyframes — 중복 KF 제거 + covisibility 정리
// 힌트: 뒤에서부터 순회 (첫 KF 보호) → isRedundant → covisibility 정리 → 삭제
// 참고: basic.cpp demoKeyframeManagement()
// 주의: 첫 KF(index 0)는 좌표계 원점이므로 절대 제거하지 않음!
void KeyframeCuller::cullRedundantKeyframes(std::vector<Keyframe*>& keyframes,
                                            const std::vector<MapPoint>& map_points)
{
    // TODO:
    // 1. 뒤에서부터 순회 (i = size-1 → 1, 첫 KF 제외)
    // 2. isRedundant() 호출
    // 3. 중복이면: covisible_keyframes 정리 → delete → erase
}

// ====================
// CovisibilityGraph (보너스)
// ====================

void CovisibilityGraph::updateCovisibility(Keyframe* kf1, Keyframe* kf2,
                                           const std::vector<MapPoint>& map_points)
{
    // TODO: 공유 맵 포인트 개수 세기 → 임계값(15) 이상이면 addCovisibility
}

std::vector<Keyframe*> CovisibilityGraph::getLocalKeyframes(Keyframe* curr_kf, int max_keyframes)
{
    // TODO: covisibility 높은 순서로 정렬 → 상위 max_keyframes개 반환
    return {};
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] Keyframe 관리 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat img = cv::Mat::zeros(480, 640, CV_8UC1);

    // ── Step 1 테스트: addCovisibility ──
    std::cout << "[Step 1] addCovisibility" << std::endl;
    Keyframe kf0(0, img), kf1(1, img), kf2(2, img);
    kf0.addCovisibility(&kf1, 25);
    kf0.addCovisibility(&kf2, 10);
    bool step1 = (kf0.covisible_keyframes.size() == 2);
    std::cout << "  KF0 covisible 수: " << kf0.covisible_keyframes.size()
              << (step1 ? " ✅" : " ❌ (기대: 2)") << "\n" << std::endl;

    // ── Step 2 테스트: getCovisibleKeyframes ──
    std::cout << "[Step 2] getCovisibleKeyframes" << std::endl;
    auto covisible = kf0.getCovisibleKeyframes(15);
    bool step2 = (covisible.size() == 1 && covisible[0] == &kf1);
    std::cout << "  KF0 covisible (>=15): " << covisible.size()
              << (step2 ? " ✅" : " ❌ (기대: 1, KF1만)") << "\n" << std::endl;

    // ── Step 3 테스트: needNewKeyframe ──
    std::cout << "[Step 3] needNewKeyframe" << std::endl;
    KeyframeSelector selector(20, 30, 0.5, 10.0);
    bool t1 = !selector.needNewKeyframe(5, 80, 100, 5.0);     // 너무 이름 → false
    bool t2 = selector.needNewKeyframe(30, 80, 100, 5.0);     // 최대 간격 → true
    bool t3 = selector.needNewKeyframe(25, 30, 100, 3.0);     // 추적 저하 → true
    bool t4 = selector.needNewKeyframe(22, 80, 100, 15.0);    // 시차 충분 → true
    bool t5 = !selector.needNewKeyframe(22, 80, 100, 5.0);    // 양호 → false
    bool step3 = t1 && t2 && t3 && t4 && t5;
    std::cout << "  너무 이름: " << (t1 ? "✅" : "❌") << std::endl;
    std::cout << "  최대 간격: " << (t2 ? "✅" : "❌") << std::endl;
    std::cout << "  추적 저하: " << (t3 ? "✅" : "❌") << std::endl;
    std::cout << "  시차 충분: " << (t4 ? "✅" : "❌") << std::endl;
    std::cout << "  양호:      " << (t5 ? "✅" : "❌") << std::endl;
    std::cout << "  종합: " << (step3 ? "✅" : "❌") << "\n" << std::endl;

    // ── Step 4 테스트: isRedundant ──
    std::cout << "[Step 4] isRedundant" << std::endl;
    // 맵 포인트 10개 생성: 9개는 3+ 관측, 1개는 1 관측
    std::vector<MapPoint> map_points(10);
    for (int i = 0; i < 9; i++)
        map_points[i].num_observations = 3;
    map_points[9].num_observations = 1;

    Keyframe kf_test(10, img);
    for (int i = 0; i < 10; i++)
        kf_test.map_point_indices.push_back(i);

    KeyframeCuller culler(0.9);
    // isRedundant는 private이므로 간접 테스트 — cullRedundantKeyframes에서 확인
    std::cout << "  (Step 5에서 간접 테스트)\n" << std::endl;

    // ── Step 5 테스트: cullRedundantKeyframes ──
    std::cout << "[Step 5] cullRedundantKeyframes" << std::endl;
    // KF 3개 생성: KF0(보호), KF1(중복 아님), KF2(중복)
    std::vector<Keyframe*> keyframes;
    Keyframe* ckf0 = new Keyframe(0, img);
    Keyframe* ckf1 = new Keyframe(1, img);
    Keyframe* ckf2 = new Keyframe(2, img);

    // KF2만 중복: 모든 맵포인트가 3+ 관측
    std::vector<MapPoint> mps(5);
    for (auto& mp : mps)
        mp.num_observations = 4;

    ckf2->map_point_indices = {0, 1, 2, 3, 4};
    // KF1은 1 관측 맵포인트 포함 → 중복 아님
    std::vector<MapPoint> mps_extra = mps;
    mps_extra.push_back(MapPoint());
    mps_extra[5].num_observations = 1;
    ckf1->map_point_indices = {0, 5};

    keyframes = {ckf0, ckf1, ckf2};

    culler.cullRedundantKeyframes(keyframes, mps);
    bool step5 = (keyframes.size() == 2);  // KF2만 제거
    std::cout << "  남은 KF 수: " << keyframes.size()
              << (step5 ? " ✅" : " ❌ (기대: 2, KF2만 제거)") << std::endl;

    // 정리
    for (auto* kf : keyframes)
        delete kf;

    // ── 종합 결과 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  종합: Step1 " << (step1 ? "✅" : "❌")
              << " Step2 " << (step2 ? "✅" : "❌")
              << " Step3 " << (step3 ? "✅" : "❌")
              << " Step5 " << (step5 ? "✅" : "❌") << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
