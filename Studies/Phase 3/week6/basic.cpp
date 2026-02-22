// Week 6: Keyframe Management — 키프레임 선택, Culling, Covisibility Graph
//
// 이 파일은 SLAM 시스템에서 Keyframe을 관리하는 3가지 핵심 메커니즘을 시연한다:
//   1. Keyframe 선택: 언제 새 Keyframe을 만들 것인가?
//   2. Keyframe Culling: 중복 Keyframe을 언제 제거할 것인가?
//   3. Covisibility Graph: Keyframe 간 관계를 어떻게 관리할 것인가?
//
// Keyframe이란?
//   모든 프레임을 저장하면 메모리/계산량이 폭발적으로 증가.
//   그래서 "중요한 프레임"만 선별하여 저장하고, 이를 기반으로 맵을 구축/최적화.
//   Keyframe은 맵 구축, Bundle Adjustment, Loop Closure의 기본 단위.
//
// ★ Keyframe 관리가 필요한 이유:
//   너무 적으면 → 맵 품질 저하, 추적 불안정
//   너무 많으면 → BA 느려짐, 메모리 낭비, 중복 정보
//   → "적절한 양"을 유지하는 것이 핵심 (보통 100~500개 수준)
//
// 대표 시스템:
//   ORB-SLAM2: 적극적 선택 + 적극적 Culling (Keyframe 자유롭게 생성, 중복은 삭제)
//   VINS-Mono: 보수적 선택 (Sliding Window, 오래된 KF 제거)

#include "keyframe_manager.h"
#include <iostream>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════
// Keyframe 클래스 구현
// ═══════════════════════════════════════════════════════════════

// Keyframe 생성자 — 프레임 ID와 이미지를 저장하고 초기 포즈를 단위행렬로 설정
//
// 각 Keyframe이 가지는 정보:
//   - id: 프레임 번호 (시간순 식별자)
//   - image: 원본 이미지 (특징점 재검출, 시각화에 사용)
//   - R, t: 카메라 포즈 (SE(3) — 월드→카메라 변환)
//   - keypoints: 이 프레임에서 검출된 2D 특징점
//   - descriptors: 특징점 디스크립터 (ORB, BRIEF 등 — 매칭/재인식용)
//   - map_point_indices: 관측하는 맵 포인트의 인덱스 목록
//   - covisible_keyframes: 같은 맵 포인트를 공유하는 다른 KF → Covisibility Graph
Keyframe::Keyframe(int id, const cv::Mat& img) : id(id), image(img.clone()), num_tracked(0)
{
    R = Eigen::Matrix3d::Identity();    // 초기 포즈: 원점
    t = Eigen::Vector3d::Zero();
}

// Covisibility 추가 — 두 Keyframe이 공유하는 맵 포인트 수를 기록
//
// Covisibility = "공동 관측 가능성"
//   KF A와 KF B가 같은 3D 점 100개를 관측하면 → covisibility = 100
//   이 값이 높을수록 두 KF은 비슷한 장면을 보고 있다는 의미.
//
// 용도:
//   1. Local BA 범위 결정 — covisibility가 높은 KF끼리 함께 최적화
//   2. Loop Closure 후보 탐색 — covisibility가 높은 KF에서 먼저 검색
//   3. Relocalization — LOST 상태에서 재위치 결정 시 참조
void Keyframe::addCovisibility(Keyframe* other, int num_shared)
{
    covisible_keyframes[other] = num_shared;
}

// 지정된 최소 공유 수 이상의 Covisible Keyframe 목록 반환
//
// min_shared = 15 (기본값):
//   15개 이상의 맵 포인트를 공유하는 KF만 반환.
//   ★ ORB-SLAM2에서 15는 Covisibility Graph의 edge 생성 임계값.
//   이 값이 낮으면 약한 연결도 포함 → 그래프가 커짐 → BA 범위 확대.
//   이 값이 높으면 강한 연결만 포함 → 그래프가 작음 → BA 범위 축소.
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

// ═══════════════════════════════════════════════════════════════
// KeyframeSelector 구현 — 새 Keyframe 생성 여부 판단
// ═══════════════════════════════════════════════════════════════

// KeyframeSelector 생성자 — 4가지 기준 파라미터 설정
//
// ORB-SLAM2 스타일의 Keyframe 선택 기준:
//   min_frames_ = 20:  마지막 KF 이후 최소 20프레임 경과 (너무 빈번한 생성 방지)
//   max_frames_ = 30:  30프레임 이상 KF 없으면 강제 생성 (맵 갱신 보장)
//   min_tracked_ratio_ = 0.5:  추적 품질 50% 이하면 KF 생성 (맵 보강 필요)
//   min_parallax_ = 10.0:  시차 10px 이상이면 KF 생성 (삼각측량 가능)
KeyframeSelector::KeyframeSelector(int min_frames, int max_frames, double min_tracked_ratio,
                                   double min_parallax)
    : min_frames_(min_frames),
      max_frames_(max_frames),
      min_tracked_ratio_(min_tracked_ratio),
      min_parallax_(min_parallax)
{
}

// 새 Keyframe이 필요한지 판단 — 4가지 기준의 조합
//
// 판단 로직 (ORB-SLAM2 기반):
//   전제: num_frames >= min_frames (최소 간격 미충족 시 무조건 false)
//   조건 A: num_frames >= max_frames        → 최대 간격 도달 (강제 생성)
//   조건 B: tracked_ratio < min_tracked_ratio → 추적 품질 저하 (맵 보강 필요)
//   조건 C: avg_parallax > min_parallax      → 충분한 시차 (삼각측량 가능)
//
// ★ 왜 이런 복합 조건이 필요한가?
//   - 조건 A만: 일정 간격으로 기계적 생성 → 실제 필요와 무관
//   - 조건 B만: 추적 불량 시에만 → 좋은 삼각측량 기회를 놓침
//   - 조건 C만: 시차 기반 → 빠른 이동 시 KF 폭증
//   → 3가지를 OR로 결합하여 다양한 상황에 대응
bool KeyframeSelector::needNewKeyframe(int num_frames_since_last, int num_tracked,
                                       int total_map_points, double avg_parallax)
{
    // 기본 조건: 최소 간격 미충족 → 아직 이름
    //   ORB-SLAM2에서 이 조건 + "Local Mapping이 idle 상태"도 추가됨
    if (num_frames_since_last < min_frames_)
    {
        return false;
    }

    // 조건 A: 최대 간격 — 30프레임 이상 KF가 없으면 강제 생성
    //   맵이 너무 오래 갱신되지 않으면 환경 변화에 대응 불가
    if (num_frames_since_last >= max_frames_)
    {
        std::cout << "  Keyframe: 최대 간격 도달 (" << num_frames_since_last << " frames)"
                  << std::endl;
        return true;
    }

    // 조건 B: 추적 품질 — 현재 추적 점이 전체 맵의 50% 미만이면 KF 생성
    //   tracked_ratio = num_tracked / total_map_points
    //   비율이 낮다 = 현재 보이는 맵 포인트가 적다 = 새 맵 포인트 필요
    //   ★ 이 비율은 카메라가 새 영역으로 이동했음을 간접적으로 나타냄
    if (total_map_points > 0)
    {
        double tracked_ratio = (double)num_tracked / total_map_points;
        if (tracked_ratio < min_tracked_ratio_)
        {
            std::cout << "  Keyframe: 추적 품질 저하 (" << tracked_ratio * 100 << "%)" << std::endl;
            return true;
        }
    }

    // 조건 C: 시차 — 평균 시차가 10px 이상이면 KF 생성
    //   시차가 크다 = 카메라가 충분히 이동 = 삼각측량으로 정확한 3D 점 생성 가능
    //   ★ VINS-Mono는 주로 이 조건만 사용 (시차 기반 KF 선택)
    if (avg_parallax > min_parallax_)
    {
        std::cout << "  Keyframe: 시차 충분 (" << avg_parallax << " px)" << std::endl;
        return true;
    }

    return false;
}

// ═══════════════════════════════════════════════════════════════
// KeyframeCuller 구현 — 중복 Keyframe 제거
// ═══════════════════════════════════════════════════════════════

// KeyframeCuller 생성자
//   redundancy_threshold = 0.9 (90%):
//   KF의 맵 포인트 중 90%가 다른 3개 이상의 KF에서도 관측되면 "중복"으로 판단
//   ★ 이것이 ORB-SLAM2의 유명한 "90% Rule"
KeyframeCuller::KeyframeCuller(double redundancy_threshold)
    : redundancy_threshold_(redundancy_threshold)
{
}

// 중복 Keyframe 제거 — 뒤에서부터 순회하며 중복 KF 삭제
//
// 왜 Culling이 필요한가?
//   KF를 적극적으로 생성하면 맵 품질은 좋아지지만 KF 수가 계속 증가.
//   BA 비용은 KF 수에 비례 → KF가 많으면 BA가 느려짐.
//   → 정보가 중복되는 KF을 제거하여 맵 크기를 유지.
//
// ★ 첫 번째 KF(인덱스 0)는 절대 제거하지 않음!
//   첫 KF = 좌표계 원점 = 전체 맵의 기준점.
//   이것을 제거하면 모든 포즈와 맵 포인트의 기준이 사라짐.
//
// Covisibility 업데이트:
//   삭제되는 KF을 참조하는 다른 KF의 covisibility에서도 제거해야
//   dangling pointer(무효 포인터)를 방지.
void KeyframeCuller::cullRedundantKeyframes(std::vector<Keyframe*>& keyframes,
                                            const std::vector<MapPoint>& map_points)
{
    int num_culled = 0;

    // 뒤에서부터 순회 — erase 시 인덱스 밀림 방지
    for (int i = keyframes.size() - 1; i > 0; i--)
    {  // i > 0: 첫 KF 보호
        Keyframe* kf = keyframes[i];

        if (isRedundant(kf, map_points))
        {
            // 삭제 전: 다른 KF의 covisibility에서 이 KF 참조 제거
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

// Keyframe 중복 판단 — 90% Rule
//
// 알고리즘:
//   1. 이 KF이 관측하는 모든 맵 포인트를 순회
//   2. 각 맵 포인트가 3개 이상의 다른 KF에서도 관측되면 "중복 관측"
//   3. 중복 관측 비율 > 90%이면 이 KF는 중복 → 삭제 대상
//
// 직관:
//   "이 KF에서만 볼 수 있는 점"이 10% 미만이면,
//   이 KF을 삭제해도 맵 포인트 대부분이 살아남음 → 정보 손실 최소.
//
// ★ "3개 이상"의 의미:
//   맵 포인트가 최소 3개 KF에서 관측되어야 삼각측량이 안정적.
//   (2개 KF만이면 baseline이 하나뿐 → 불안정)
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

        // 3개 이상 다른 Keyframe에서 관측 → 이 KF 없어도 충분히 관측됨
        if (mp.num_observations >= 3)
        {
            redundant_obs++;
        }
    }

    if (total_obs == 0)
        return false;

    // redundancy > 90% → 중복 KF로 판단
    double redundancy = (double)redundant_obs / total_obs;
    return redundancy > redundancy_threshold_;
}

// ═══════════════════════════════════════════════════════════════
// CovisibilityGraph 구현 — Keyframe 간 관계 관리
// ═══════════════════════════════════════════════════════════════

// Covisibility 업데이트 — 두 KF이 공유하는 맵 포인트 수를 계산
//
// Covisibility Graph:
//   노드 = Keyframe
//   엣지 = 공유 맵 포인트 수 (가중치)
//   임계값 = 15 (15개 미만이면 연결하지 않음)
//
// ★ 이 그래프의 실제 활용:
//   - Local BA: 현재 KF과 covisibility 높은 KF만 최적화 → 실시간 가능
//   - Loop Closure: covisibility 그래프를 따라가며 후보 검색
//   - Map Merging: 두 맵의 KF 간 covisibility로 병합 결정
//
// 💡 시간 복잡도:
//   현재 구현은 O(M²) (M = 맵 포인트 수) — 교육용으로 단순화.
//   실제 ORB-SLAM2에서는 맵 포인트가 자신을 관측하는 KF 목록을 유지하여 O(M)으로 동작.
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

    // 15개 이상 공유해야 연결 (양방향)
    //   ★ 15 = ORB-SLAM2의 covisibility 임계값
    //   양방향: KF1→KF2, KF2→KF1 모두 기록 (무방향 그래프)
    if (num_shared >= 15)
    {
        kf1->addCovisibility(kf2, num_shared);
        kf2->addCovisibility(kf1, num_shared);
    }
}

// Local Keyframes 가져오기 — Covisibility 기반 Local BA 범위 결정
//
// Local BA = 현재 KF 주변의 covisible KF만 대상으로 최적화.
// 전체 맵을 최적화하면 느리므로, 관련있는 KF만 추려서 빠르게 최적화.
//
// 알고리즘:
//   1. 현재 KF의 covisible KF을 공유 포인트 수 내림차순 정렬
//   2. 상위 max_keyframes개만 반환
//
// ★ 실제 ORB-SLAM2에서는 1차 covisible + 2차 covisible(이웃의 이웃)까지 포함
std::vector<Keyframe*> CovisibilityGraph::getLocalKeyframes(Keyframe* curr_kf, int max_keyframes)
{
    // Covisibility가 높은 순서로 정렬
    std::vector<std::pair<Keyframe*, int>> covis_vec;
    for (auto& [kf, count] : curr_kf->covisible_keyframes)
    {
        covis_vec.push_back({kf, count});
    }

    // 공유 포인트 수 내림차순 정렬 → 가장 관련 높은 KF부터
    std::sort(covis_vec.begin(), covis_vec.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // 상위 max_keyframes개만 반환
    std::vector<Keyframe*> result;
    for (int i = 0; i < std::min((int)covis_vec.size(), max_keyframes); i++)
    {
        result.push_back(covis_vec[i].first);
    }

    return result;
}

// ═══════════════════════════════════════════════════════════════
// 교육 블록
// ═══════════════════════════════════════════════════════════════

// ── 교육 블록 1: Keyframe 선택 기준 ──
//
// Keyframe을 "언제" 만들 것인가?
// 이것은 SLAM 시스템 설계에서 가장 중요한 결정 중 하나.
//
// 💡 quiz_easy Q1: Keyframe을 사용하는 주요 이유 3가지
// 💡 quiz_easy Q2: ORB-SLAM 기본 조건 (20프레임 간격)
// 💡 quiz_medium Q1: ORB-SLAM vs VINS의 KF 선택 차이
void demoKeyframeSelection()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Keyframe 선택 기준 4가지" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Keyframe = 맵 구축/최적화에 사용되는 '중요한 프레임'\n" << std::endl;

    // ORB-SLAM2 Keyframe 선택 기준:
    //   전제: num_frames >= 20 AND Local Mapping이 idle 상태
    //     → 20프레임 미만이면 무조건 거절 (너무 빈번한 생성 방지)
    //     → Local Mapping이 바쁘면 새 KF을 처리할 수 없으므로 대기
    //   조건A: num_frames >= 30 → 강제 생성 (맵 갱신 보장)
    //   조건B: tracked < 50% → 추적 품질 저하 (새 맵 포인트 필요)
    //   조건C: parallax > 10px → 삼각측량 가능 (좋은 3D 점 생성)
    std::cout << "ORB-SLAM 선택 기준:" << std::endl;
    std::cout << "  기본: num_frames >= 20 AND Local Mapping idle" << std::endl;
    std::cout << "  조건A: num_frames >= 30    (최대 간격)" << std::endl;
    std::cout << "  조건B: tracked < 0.5       (추적 품질 저하)" << std::endl;
    std::cout << "  조건C: parallax > 10px     (충분한 시차)\n" << std::endl;

    // VINS-Mono:
    //   시차 기반만 사용 — avg_parallax > 10px이면 KF 생성
    //   Sliding Window 방식 → 오래된 KF를 제거 (고정 크기 윈도우 유지)
    //   ORB-SLAM과의 차이: VINS는 IMU가 있으므로 KF가 적어도 안정적
    std::cout << "VINS-Mono 선택 기준:" << std::endl;
    std::cout << "  avg_parallax > 10px  (시차 기반)\n" << std::endl;

    // ── 수치 시연: 조건별 판단 결과 ──
    KeyframeSelector selector(20, 30, 0.5, 10.0);

    struct TestCase
    {
        const char* desc;
        int frames;     // 마지막 KF 이후 프레임 수
        int tracked;    // 현재 추적된 점
        int total;      // 전체 맵 포인트
        double parallax;    // 평균 시차 (px)
    };

    // 각 테스트 케이스의 판단 이유:
    //   "너무 이른 프레임": frames=5 < min_frames_=20 → 무조건 false
    //   "최대 간격 도달": frames=30 >= max_frames_=30 → 조건A true
    //   "추적 품질 저하": tracked/total=30% < 50% → 조건B true
    //   "시차 충분": parallax=15 > 10 → 조건C true
    //   "양호 (불필요)": 모든 조건 미충족 → false
    TestCase cases[] = {
        {"너무 이른 프레임", 5, 80, 100, 5.0},
        {"최대 간격 도달", 30, 80, 100, 5.0},
        {"추적 품질 저하", 25, 30, 100, 3.0},
        {"시차 충분", 22, 80, 100, 15.0},
        {"양호 (KF 불필요)", 22, 80, 100, 5.0},
    };

    for (auto& tc : cases)
    {
        bool need = selector.needNewKeyframe(tc.frames, tc.tracked, tc.total, tc.parallax);
        std::cout << "  " << tc.desc << ": " << (need ? "KF 필요!" : "불필요") << std::endl;
    }
}

// ── 교육 블록 2: 90% Culling Rule ──
//
// ORB-SLAM2의 Keyframe Culling 전략.
// "이 KF의 맵 포인트 90%가 다른 3+ KF에서도 관측되면 삭제"
//
// 💡 quiz_easy Q3: Culling의 목적
// 💡 quiz_easy Q4: 90% rule의 의미
// 💡 quiz_medium Q4: 첫 KF를 제거하지 않는 이유
void demoCullingRule()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 90% Culling Rule 수치 예시" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "기준: KF의 맵 포인트 중 90%가 다른 3+ KF에서도 관측\n" << std::endl;

    // 수치 예시: KF A가 10개 맵 포인트를 관측
    //   관측 3+ KF인 포인트 9개 = 중복 90% → 삭제!
    //   관측 3+ KF인 포인트 8개 = 중복 80% → 유지
    //
    // ★ "3개 이상 다른 KF에서 관측"의 의미:
    //   이 맵 포인트는 이미 충분한 KF에서 관측되고 있어,
    //   현재 KF을 삭제해도 이 포인트의 삼각측량/BA에 영향 없음.
    std::cout << "예시: Keyframe A가 10개 맵 포인트를 관측" << std::endl;
    std::cout << "  Point 1: 4개 KF에서 관측  → 중복" << std::endl;
    std::cout << "  Point 2: 3개 KF에서 관측  → 중복" << std::endl;
    std::cout << "  Point 3: 5개 KF에서 관측  → 중복" << std::endl;
    std::cout << "  Point 4: 1개 KF에서 관측  → 고유!" << std::endl;
    std::cout << "  ..." << std::endl;
    std::cout << "  중복 9/10 = 90%  → KF A 제거!" << std::endl;
    std::cout << "  중복 8/10 = 80%  → KF A 유지\n" << std::endl;

    // 첫 KF 보호 이유:
    //   좌표계 원점 = 모든 포즈의 기준점
    //   삭제하면 전체 맵의 좌표계가 무효화됨
    std::cout << "💡 첫 KF는 절대 제거하지 않음" << std::endl;
    std::cout << "   → 좌표계 원점, 전체 맵의 기준점!" << std::endl;
}

// ── 교육 블록 3: Covisibility Graph 활용 ──
//
// Covisibility Graph = Keyframe 간의 관계를 표현하는 가중 그래프
//   노드 = Keyframe
//   엣지 가중치 = 공유하는 맵 포인트 수
//
// 💡 quiz_easy Q5: Covisibility graph의 용도
// 💡 quiz_medium Q3: Covisibility가 높은 KF의 의미
void demoCovisibility()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📖 Covisibility Graph 활용" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Covisibility = 같은 맵 포인트를 공유하는 KF 관계\n" << std::endl;

    // 실제 그래프 구축 시연
    //   4개 KF (KF0~KF3) 간의 covisibility:
    //     KF0 -- 25 -- KF1 -- 20 -- KF3
    //       \           /
    //       15        10
    //         \     /
    //          KF2
    //
    //   숫자 = 공유 맵 포인트 수
    //   KF0-KF1: 25개 공유 → 같은 장면의 인접 프레임
    //   KF1-KF3: 20개 공유 → 가까운 위치
    //   KF0-KF2: 15개 → 중간 정도 관련
    //   KF1-KF2: 10개 → 약한 관련 (임계값 15 미만이면 edge 없음)
    cv::Mat dummy_img = cv::Mat::zeros(480, 640, CV_8UC1);
    Keyframe kf0(0, dummy_img), kf1(1, dummy_img), kf2(2, dummy_img), kf3(3, dummy_img);

    kf0.addCovisibility(&kf1, 25);
    kf0.addCovisibility(&kf2, 15);
    kf1.addCovisibility(&kf0, 25);
    kf1.addCovisibility(&kf2, 10);
    kf1.addCovisibility(&kf3, 20);
    kf2.addCovisibility(&kf0, 15);
    kf2.addCovisibility(&kf1, 10);
    kf3.addCovisibility(&kf1, 20);

    std::cout << "그래프:" << std::endl;
    std::cout << "  KF0 --25-- KF1 --20-- KF3" << std::endl;
    std::cout << "   \\         /" << std::endl;
    std::cout << "   15      10" << std::endl;
    std::cout << "     \\   /" << std::endl;
    std::cout << "      KF2\n" << std::endl;

    // Local BA 범위 결정:
    //   KF1에서 min_shared >= 15인 KF만 추출 → KF0(25), KF3(20)
    //   KF2는 공유 10개이므로 제외 (Local BA 범위 밖)
    //
    // ★ Local BA에 포함 = 포즈를 함께 최적화
    //   포함되지 않은 KF은 고정(fixed)으로 처리 → 기준점 역할
    auto local = kf1.getCovisibleKeyframes(15);
    std::cout << "KF1의 Local BA 대상 (>=15 공유):" << std::endl;
    for (auto* kf : local)
    {
        std::cout << "  KF" << kf->id << std::endl;
    }

    std::cout << "\n💡 Covisibility 용도:" << std::endl;
    std::cout << "  1. Local BA 범위 결정" << std::endl;
    std::cout << "  2. Loop Closure 후보 탐색" << std::endl;
    std::cout << "  3. Relocalization 재시도" << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// 통합 시뮬레이션 — 200 프레임에서 KF 선택 + Culling 과정
// ═══════════════════════════════════════════════════════════════

// Keyframe Management 전체 시뮬레이션
//
// 200 프레임을 순회하며:
//   1. 매 프레임 Keyframe 필요 여부 판단
//   2. 필요하면 새 KF 생성
//   3. 50 프레임마다 Culling 수행
//
// 관찰 포인트:
//   - 얼마나 자주 KF가 생성되는가? (보통 20~30 프레임마다)
//   - Culling으로 몇 개가 제거되는가? (중복이 많으면 적극 제거)
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

        // 시뮬레이션 데이터 — 실제로는 추적 결과에서 가져옴
        int num_tracked = 80 + rand() % 40;  // 80~120개 추적
        int total_map_points = 150;
        // 시차: 프레임 간격에 비례 (간격이 클수록 시차 증가)
        double avg_parallax = frames_since_last * 0.5;

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

        // 주기적으로 Culling — 50 프레임마다 중복 KF 제거
        if (frame % 50 == 0 && frame > 0)
        {
            std::vector<MapPoint> dummy_map;  // 간단한 시뮬레이션 (빈 맵)
            culler.cullRedundantKeyframes(keyframes, dummy_map);
            std::cout << "  남은 Keyframes: " << keyframes.size() << "\n" << std::endl;
        }
    }

    // 메모리 해제 — new로 생성한 Keyframe은 반드시 delete
    for (auto* kf : keyframes)
    {
        delete kf;
    }

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ 데모 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

// ═══════════════════════════════════════════════════════════════
// main
// ═══════════════════════════════════════════════════════════════

#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 6: Keyframe Management" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 교육 블록
    demoKeyframeSelection();    // 블록 1: KF 선택 기준 (ORB-SLAM vs VINS)
    demoCullingRule();          // 블록 2: 90% Culling Rule
    demoCovisibility();         // 블록 3: Covisibility Graph 활용

    // 통합 시뮬레이션
    demoKeyframeManagement();   // 200 프레임 KF 관리 시뮬레이션

    // 다음 단계 안내
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "📌 다음 단계:" << std::endl;
    std::cout << "  1. quiz_easy    → Keyframe 목적, 선택 조건, Culling" << std::endl;
    std::cout << "  2. quiz_medium  → ORB-SLAM vs VINS, Covisibility" << std::endl;
    std::cout << "  3. my_basic.cpp → 직접 구현 (5 Step)" << std::endl;
    std::cout << "  4. Week 7       → Local Bundle Adjustment" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
