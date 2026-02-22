/**
 * Phase 3 Week 1 - Visual Odometry(VO) 개요 데모
 *
 * VO의 기본 개념, 파이프라인, 데이터 구조를 이해하기 위한 데모 프로그램.
 *
 * VO(Visual Odometry)란?
 *   카메라 이미지를 연속으로 분석하여 카메라의 이동(포즈 변화)을 추정하는 기술.
 *   "눈(카메라)으로 내가 얼마나 움직였는지 측정하는 것"
 *
 *   자동차의 바퀴 회전 수로 이동 거리를 측정하는 것이 wheel odometry라면,
 *   카메라 영상으로 이동을 추정하는 것이 visual odometry.
 *
 *   SLAM의 프론트엔드로, 실시간 포즈 추정을 담당한다.
 *   (SLAM = VO + 지도 생성 + 루프 클로저 + 전역 최적화)
 */

#include "vo_types.h"
#include <iostream>
#include <iomanip>

using namespace vo;

// VO 파이프라인 전체 흐름을 단계별로 설명
//
// VO는 연속된 이미지에서 "특징점"을 찾고 추적하여
// 프레임 간 카메라 이동(R, t)을 추정하는 6단계 파이프라인.
//
// 핵심 아이디어:
//   "3D 공간의 같은 점이 연속 이미지에서 다른 위치에 투영된다"
//   → 이 위치 차이(optical flow)로부터 카메라 이동을 역추정
//
// VINS-Mono와의 관계:
//   1~4단계 = VINS의 프론트엔드 (VO, 실시간 포즈 추정)
//   5단계   = VINS의 백엔드 (Bundle Adjustment + Marginalization)
//   → Week 5~8에서 각 단계를 하나씩 구현
void demoPipeline()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Visual Odometry 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "파이프라인 단계:\n" << std::endl;

    // ① 이미지 입력
    //   카메라에서 프레임을 읽어오는 단계.
    //   그레이스케일로 변환하는 이유:
    //     특징점 검출은 밝기 그래디언트(gradient)만 사용하므로
    //     컬러 정보는 불필요하고, 처리 속도도 3배 빨라짐.
    std::cout << "1️⃣  이미지 입력" << std::endl;
    std::cout << "   - 카메라에서 연속 프레임 획득" << std::endl;
    std::cout << "   - Grayscale 변환 (특징점 검출용)\n" << std::endl;

    // ② 특징점 검출
    //   이미지에서 "추적하기 좋은 점"을 찾는 단계.
    //   좋은 특징점의 조건:
    //     - 주변과 구별되는 독특한 패턴 (코너, 블롭 등)
    //     - 다른 각도에서도 재검출 가능 (반복성)
    //     - 조명 변화에 강건 (불변성)
    //   대표 알고리즘:
    //     FAST: 속도 최우선 (VINS-Mono에서 사용)
    //     ORB: FAST + 방향 불변 기술자 (ORB-SLAM에서 사용)
    //   보통 200~500개를 유지 — 너무 적으면 추적 실패, 너무 많으면 느려짐.
    std::cout << "2️⃣  특징점 검출" << std::endl;
    std::cout << "   - FAST, ORB 등으로 코너 검출" << std::endl;
    std::cout << "   - 200~500개 특징점 유지\n" << std::endl;

    // ③ 특징점 추적/매칭
    //   이전 프레임의 특징점이 현재 프레임에서 어디로 이동했는지 찾는 단계.
    //   두 가지 접근:
    //     (1) Optical Flow (Lucas-Kanade):
    //         밝기 항상성(brightness constancy) 가정 하에 픽셀 이동 추적.
    //         → "같은 점은 프레임 간 밝기가 변하지 않는다"
    //         → I(x,y,t) = I(x+dx, y+dy, t+dt)
    //         장점: 기술자 계산 불필요 → 빠름
    //         단점: 큰 이동에 약함 (이미지 피라미드로 보완)
    //     (2) Descriptor 매칭:
    //         각 특징점에 고유한 기술자(descriptor)를 계산한 후,
    //         기술자 간 거리(Hamming/L2)가 가장 가까운 쌍을 매칭.
    //         장점: 큰 이동, 시점 변화에도 가능
    //         단점: 기술자 계산 비용, 오매칭 가능성
    //   VINS-Mono: Optical Flow (KLT) 사용 → Week 2에서 구현
    std::cout << "3️⃣  특징점 추적/매칭" << std::endl;
    std::cout << "   - Optical Flow (Lucas-Kanade)" << std::endl;
    std::cout << "   - 또는 Descriptor 매칭\n" << std::endl;

    // ④ 모션 추정 — 카메라가 얼마나 움직였는지 계산
    //   두 가지 상황에 따라 다른 알고리즘 사용:
    //
    //   (1) 초기화 단계 (3D 맵이 없을 때) → 2D-2D:
    //       Essential Matrix E를 추정하여 R, t를 복원.
    //       p₂ᵀ E p₁ = 0 (에피폴라 구속조건)
    //       → E를 SVD 분해하면 R, t 4가지 해 → Cheirality check로 1개 선택
    //       ⚠️ t는 방향만 알 수 있고 크기(스케일)는 모름!
    //       → Week 2에서 자세히 학습
    //
    //   (2) 추적 단계 (3D 맵이 있을 때) → 3D-2D:
    //       PnP (Perspective-n-Point)로 카메라 포즈 추정.
    //       이미 알고 있는 3D 점과 현재 2D 관측의 대응으로 [R|t] 계산.
    //       ✅ 3D 점의 절대 크기를 알고 있으므로 실제 스케일 복원!
    //       → Week 3에서 자세히 학습
    std::cout << "4️⃣  모션 추정" << std::endl;
    std::cout << "   - 초기화: 2D-2D (Essential Matrix)" << std::endl;
    std::cout << "   - 추적: 3D-2D (PnP)\n" << std::endl;

    // ⑤ Local 최적화 — 추정값을 더 정밀하게 다듬는 단계
    //   Bundle Adjustment (BA):
    //     "여러 프레임의 카메라 포즈와 3D 점을 동시에 최적화"
    //     목적함수: Σ ||p_observed - project(X, R, t, K)||² 최소화
    //     → 재투영 오차(reprojection error)의 합을 최소화
    //     → 비선형 최적화 (Levenberg-Marquardt, Gauss-Newton)
    //   Outlier 제거:
    //     오매칭이나 동적 물체로 인한 잘못된 대응점을 제거.
    //     RANSAC, χ² 테스트 등 사용.
    //   → Week 6~7에서 구현
    std::cout << "5️⃣  Local 최적화 (선택)" << std::endl;
    std::cout << "   - Bundle Adjustment" << std::endl;
    std::cout << "   - Outlier 제거\n" << std::endl;

    // ⑥ 포즈 출력
    //   추정된 카메라 위치/자세를 출력하고 저장.
    //   R (회전): 카메라가 어느 방향을 바라보는지
    //   t (이동): 카메라가 어디에 있는지
    //   → 연속 프레임의 R, t를 누적하면 카메라 궤적(trajectory)이 됨
    //   → 이 궤적이 로봇/자율주행차의 이동 경로
    std::cout << "6️⃣  포즈 출력" << std::endl;
    std::cout << "   - 카메라 위치 (R, t)" << std::endl;
    std::cout << "   - 궤적 저장/시각화\n" << std::endl;

    std::cout << "💡 VINS-Mono 연결:" << std::endl;
    std::cout << "   위 1~4단계 = VINS의 프론트엔드 (VO)" << std::endl;
    std::cout << "   5단계 = VINS의 백엔드 (BA + Marginalization)" << std::endl;
    std::cout << "   💡 이 파이프라인이 quiz_easy 문제 3!\n" << std::endl;
}

// VO에서 사용하는 핵심 데이터 구조 데모
//
// VO 시스템은 다음 4가지 데이터를 관리:
//   Camera:   카메라의 물리적 특성 (내부 파라미터 K)
//   Pose:     카메라의 위치와 자세 (SE(3) 군의 원소)
//   Feature:  이미지에서 검출된 특징점 (2D↔3D 연결 가능)
//   MapPoint: 3D 공간의 랜드마크 (여러 프레임에서 관측됨)
//
// 이 구조체들은 vo_types.h에 정의되어 있음.
void demoDataStructures()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "VO 데이터 구조 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ① Camera 구조체 — 카메라의 내부 파라미터(intrinsic)를 저장
    //
    //   K = [fx  0  cx]   fx, fy: 초점거리 (픽셀 단위)
    //       [ 0 fy  cy]   cx, cy: 주점 (이미지 중심 근처)
    //       [ 0  0   1]
    //
    //   K의 역할: 3D 카메라 좌표 → 2D 픽셀 좌표 변환
    //     [u]       [X]         u = fx * (X/Z) + cx
    //     [v] = K · [Y] / Z    v = fy * (Y/Z) + cy
    //     [1]       [Z]
    //
    //   Phase 2에서 캘리브레이션으로 K를 구했고,
    //   이제부터 그 K를 사용하여 모션을 추정!
    std::cout << "1️⃣  Camera 구조체" << std::endl;
    Camera cam(600.0, 600.0, 400.0, 300.0);
    std::cout << "   K =\n" << cam.K << "\n" << std::endl;

    // ② Pose 구조체 — 카메라의 위치와 자세 (SE(3) 변환)
    //
    //   SE(3) = Special Euclidean Group in 3D
    //     = {(R, t) | R ∈ SO(3), t ∈ ℝ³}
    //     = "3D 공간에서의 강체 변환(회전 + 이동)"
    //
    //   R (3×3 회전 행렬): 카메라가 바라보는 방향
    //     성질: R^T R = I, det(R) = 1 (직교 행렬, 행렬식 1)
    //     자유도: 3 (roll, pitch, yaw)
    //
    //   t (3×1 이동 벡터): 카메라의 위치
    //     자유도: 3 (x, y, z)
    //
    //   포즈 역변환: T⁻¹ = (R^T, -R^T · t)
    //     "카메라→월드" 좌표를 "월드→카메라"로 바꾸거나 그 반대.
    //     직관: 카메라가 오른쪽으로 1m 이동 = 세상이 왼쪽으로 1m 이동
    //
    //   포즈 합성: T₁₂ = T₁ · T₂ = (R₁R₂, R₁t₂ + t₁)
    //     "프레임 2에서 프레임 1으로의 변환"
    //     → 연속 프레임의 상대 포즈를 누적하면 전체 궤적!
    std::cout << "2️⃣  Pose 구조체" << std::endl;
    Pose pose;
    pose.t = Eigen::Vector3d(1.0, 0.5, 0.0);
    std::cout << "   t = " << pose.t.transpose() << std::endl;

    // 역변환 예시: (I, [1, 0.5, 0])의 역 = (I, [-1, -0.5, 0])
    //   R이 단위행렬이면 t_inv = -t (단순 부호 반전)
    //   R이 회전이면 t_inv = -R^T · t (회전까지 고려)
    Pose pose_inv = pose.inverse();
    std::cout << "   t_inv = " << pose_inv.t.transpose() << "\n" << std::endl;

    // ③ Feature 구조체 — 이미지에서 검출된 특징점
    //
    //   각 Feature는:
    //     pt (Point2f): 이미지에서의 2D 픽셀 좌표
    //     id: 여러 프레임에 걸쳐 추적할 때 사용하는 고유 식별자
    //     has_3d: 이 특징점에 대응하는 3D 점이 있는지 여부
    //     pt_3d: 대응하는 3D 점의 월드 좌표 (있는 경우)
    //
    //   VO에서 Feature의 생애 주기:
    //     1. 검출: FAST/ORB 등으로 2D 위치 확보
    //     2. 추적: 다음 프레임에서 같은 점을 Optical Flow로 추적
    //     3. 삼각측량: 여러 프레임의 관측으로 3D 위치 복원 → has_3d = true
    //     4. 추적 실패: 점이 화면 밖으로 나가거나 가려지면 폐기
    std::cout << "3️⃣  Feature 구조체" << std::endl;
    Feature feat(cv::Point2f(320, 240), 0);
    feat.has_3d = true;
    feat.pt_3d = cv::Point3f(0.5, 0.3, 5.0);
    std::cout << "   2D: (" << feat.pt.x << ", " << feat.pt.y << ")" << std::endl;
    std::cout << "   3D: (" << feat.pt_3d.x << ", " << feat.pt_3d.y << ", " << feat.pt_3d.z << ")\n"
              << std::endl;

    // ④ MapPoint 구조체 — 3D 공간의 랜드마크 (지도의 점)
    //
    //   각 MapPoint는:
    //     id: 고유 식별자
    //     position (Vector3d): 월드 좌표계에서의 3D 위치
    //     observed_by: 이 점을 관측한 프레임 ID 목록
    //     observations: 관측 횟수
    //
    //   MapPoint vs Feature:
    //     Feature = 하나의 이미지에서 본 2D 점
    //     MapPoint = 여러 이미지의 Feature들이 가리키는 동일한 3D 점
    //     → 하나의 MapPoint에 여러 Feature가 연결됨
    //
    //   관측 횟수(observations)가 많을수록:
    //     - 삼각측량 정밀도 ↑ (다양한 각도에서 관측)
    //     - 신뢰도 ↑ (1~2회만 관측된 점은 노이즈일 가능성)
    //     - BA에서 더 강한 제약 조건 역할
    std::cout << "4️⃣  MapPoint 구조체" << std::endl;
    MapPoint mp(0, Eigen::Vector3d(1.0, 0.5, 3.0));
    mp.observed_by = {0, 1, 2};
    mp.observations = 3;
    std::cout << "   Position: " << mp.position.transpose() << std::endl;
    std::cout << "   Observed by " << mp.observations << " frames\n" << std::endl;
}

// VO 유형(Monocular, Stereo, RGB-D) 비교
//
// 카메라 종류에 따라 VO의 특성이 크게 달라진다.
// 핵심 차이: "깊이(depth) 정보를 어떻게 얻는가?"
//
//   Monocular: 깊이 없음 → 삼각측량으로 추정 (스케일 모호)
//   Stereo:    두 카메라 간 시차(disparity)로 깊이 계산
//   RGB-D:     ToF/패턴 투영으로 직접 깊이 측정
void demoVOTypes()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "VO 유형 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // Monocular VO (단안 카메라)
    //   카메라 1대로 동작. 가장 간단하고 저렴.
    //   ⚠️ 치명적 한계: 스케일 모호성 (Scale Ambiguity)
    //     → 실제 이동이 1m인지 10m인지 카메라만으로는 구분 불가
    //     → IMU와 결합(VIO)하거나 알려진 물체 크기로 보완
    //   대표 시스템: VINS-Mono (단안 + IMU), DSO
    //   적합한 환경: 드론, 실내 로봇, 스마트폰 AR
    std::cout << "📷 Monocular VO (단안)" << std::endl;
    std::cout << "   ✅ 간단, 저렴" << std::endl;
    std::cout << "   ⚠️  스케일 모호성" << std::endl;
    std::cout << "   사용: 드론, 실내 로봇\n" << std::endl;

    // Stereo VO (스테레오 카메라)
    //   좌/우 카메라 2대를 일정 간격(baseline)으로 고정.
    //   같은 점의 좌/우 픽셀 위치 차이(disparity d)로 깊이 계산:
    //     depth = (fx × baseline) / d
    //   ✅ 실제 스케일 복원 가능 (baseline을 알고 있으므로)
    //   ⚠️ baseline이 클수록 먼 거리 정확, 가까운 거리 부정확
    //   대표 시스템: ORB-SLAM2 (stereo), KITTI benchmark
    //   적합한 환경: 자율주행, 실외 로봇
    std::cout << "📷📷 Stereo VO (스테레오)" << std::endl;
    std::cout << "   ✅ 스케일 복원" << std::endl;
    std::cout << "   ✅ 안정적 depth" << std::endl;
    std::cout << "   ⚠️  무겁고 비쌈" << std::endl;
    std::cout << "   사용: 자율주행\n" << std::endl;

    // RGB-D VO (깊이 카메라)
    //   RGB 이미지 + 깊이 이미지를 동시에 제공.
    //   깊이 측정 원리:
    //     Structured Light: 적외선 패턴 투영 (RealSense D400)
    //     ToF (Time of Flight): 빛의 왕복 시간 측정 (Azure Kinect)
    //   ✅ 깊이를 직접 측정하므로 삼각측량 불필요 → 초기화가 빠름
    //   ⚠️ 실외/햇빛 환경에서 적외선 간섭 → 깊이 측정 실패
    //   ⚠️ 유효 거리: 보통 0.3~10m (먼 거리에서 정밀도 급감)
    //   대표 시스템: ORB-SLAM2 (RGB-D), ElasticFusion
    //   적합한 환경: 실내 AR/VR, 로봇 내비게이션
    std::cout << "📷🌈 RGB-D VO" << std::endl;
    std::cout << "   ✅ 직접 depth" << std::endl;
    std::cout << "   ✅ 빠른 처리" << std::endl;
    std::cout << "   ⚠️  실외/햇빛 약함" << std::endl;
    std::cout << "   사용: 실내 AR\n" << std::endl;

    std::cout << "💡 이 비교가 quiz_easy 문제 4!\n" << std::endl;
}

// 스케일 모호성(Scale Ambiguity) 데모
//
// Monocular VO의 가장 근본적인 한계.
//
// 원리:
//   카메라가 본 이미지만으로는 "실제 크기"를 알 수 없다.
//   예: 1m 크기 물체가 1m 앞에 있는 것과
//       10m 크기 물체가 10m 앞에 있는 것은
//       카메라에 완전히 동일한 이미지를 생성!
//
// 수학적 설명:
//   Essential Matrix E에서 복원한 t는 항상 ||t|| = 1로 정규화됨.
//   즉, 이동 방향은 알 수 있지만 이동 거리는 알 수 없음.
//   → t_real = α · t_estimated  (α = 알 수 없는 스케일 계수)
//
// 해결 방법:
//   1. IMU 결합 (VIO): 가속도계로 실제 이동 거리 측정 → α 결정
//      VINS-Mono가 바로 이 방식!
//   2. 스테레오 카메라: 이미 baseline이라는 절대 길이 참조
//   3. 알려진 물체: 바닥 타일, 차선 폭 등 크기를 아는 물체 활용
void demoScaleAmbiguity()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "스케일 모호성 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "Monocular VO의 문제점:\n" << std::endl;

    // 실제 1m 이동에 대해 다양한 스케일 계수를 적용
    //   α=0.5 → 0.5m로 추정 (과소추정)
    //   α=1.0 → 1.0m로 추정 (정확)
    //   α=2.0 → 2.0m로 추정 (과대추정)
    //   → 세 경우 모두 동일한 이미지를 생성하므로 구분 불가!
    std::cout << "실제 이동: 1m" << std::endl;
    std::cout << "추정 이동 (스케일 α):" << std::endl;

    for (double scale : {0.5, 1.0, 2.0})
    {
        std::cout << "   α=" << scale << " → " << 1.0 * scale << "m" << std::endl;
    }

    std::cout << "\n💡 모두 동일한 이미지를 생성!" << std::endl;
    std::cout << "   → 카메라만으로는 구분 불가\n" << std::endl;

    std::cout << "해결 방법:" << std::endl;
    std::cout << "   1. IMU와 결합 (VIO)" << std::endl;
    std::cout << "   2. 스테레오 카메라" << std::endl;
    std::cout << "   3. 알려진 물체 크기" << std::endl;

    std::cout << "\n💡 이 개념이 quiz_easy 문제 2, quiz_medium 문제 1!\n" << std::endl;
}

// VO vs SLAM 비교 — 두 기술의 핵심 차이점
//
// VO (Visual Odometry):
//   "이전 프레임 대비 얼마나 움직였는가?" → 로컬 추정
//   최근 N개 프레임만 사용 (슬라이딩 윈도우)
//   → 오차가 누적(drift)되어 시간이 지날수록 부정확해짐
//   → 계산이 가볍고 실시간 처리에 적합
//
// SLAM (Simultaneous Localization and Mapping):
//   "전체 지도에서 내가 어디에 있는가?" → 전역 추정
//   모든 프레임과 3D 점을 관리 (전역 지도)
//   핵심 기능:
//     Loop Closure: 이전에 방문한 장소를 재인식 → 누적 오차 보정
//     전역 BA: 모든 포즈와 3D 점을 한꺼번에 최적화
//   → 오차를 보정할 수 있지만, 계산/메모리 비용이 높음
//
// VO는 SLAM의 "프론트엔드"이고,
// Loop Closure + 전역 최적화가 SLAM의 "백엔드"
void demoVOvsSLAM()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "VO vs SLAM 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "┌────────────┬───────────┬────────────┐" << std::endl;
    std::cout << "│   항목      │    VO     │    SLAM    │" << std::endl;
    std::cout << "├────────────┼───────────┼────────────┤" << std::endl;
    std::cout << "│ Loop Close │    ❌     │     ✅     │" << std::endl;
    std::cout << "│ 오차 누적   │  계속 증가 │  보정 가능  │" << std::endl;
    std::cout << "│ 지도 생성   │    ❌     │     ✅     │" << std::endl;
    std::cout << "│ 계산량      │   가벼움   │   무거움   │" << std::endl;
    std::cout << "│ 메모리      │ 적음(로컬) │ 많음(전역) │" << std::endl;
    std::cout << "└────────────┴───────────┴────────────┘\n" << std::endl;

    // 드리프트(drift) 누적 시연
    //   매 프레임 1%의 오차가 발생한다고 가정.
    //   100 프레임 후: 1.01^100 ≈ 2.70배로 오차 누적!
    //   → VO만으로는 긴 궤적에서 정확한 위치 추정이 불가능
    //   → SLAM의 Loop Closure가 이 누적 오차를 한 번에 보정
    std::cout << "📊 드리프트 누적 시연 (매 프레임 1% 오차):" << std::endl;
    double cum_error = 1.0;
    constexpr int kFrameStep = 20;
    constexpr int kMaxFrame = 100;
    for (int frame = 0; frame <= kMaxFrame; frame += kFrameStep)
    {
        if (frame > 0)
            cum_error *= std::pow(1.01, kFrameStep);
        std::cout << "   Frame " << std::setw(3) << frame << ": "
                  << std::fixed << std::setprecision(2) << cum_error << "x" << std::endl;
    }

    std::cout << "\n💡 VO는 드리프트를 막을 수 없음 → SLAM의 Loop Closure 필요!" << std::endl;
    std::cout << "   💡 이 내용이 quiz_easy 문제 1, quiz_medium 문제 2!\n" << std::endl;
}

// 메인 함수 — Week 1의 모든 데모를 순서대로 실행
//
// 학습 흐름:
//   1. demoPipeline(): VO 6단계 파이프라인 전체 그림 파악
//   2. demoDataStructures(): VO에서 사용하는 데이터 구조 이해
//   3. demoVOTypes(): 카메라 유형별 VO 특성 비교
//   4. demoScaleAmbiguity(): Monocular VO의 근본 한계 이해
//   5. demoVOvsSLAM(): VO와 SLAM의 관계 이해
#ifndef BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 1: VO 개요 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    demoPipeline();
    demoDataStructures();
    demoVOTypes();
    demoScaleAmbiguity();
    demoVOvsSLAM();

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ Week 1 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "💡 핵심 내용:" << std::endl;
    std::cout << "   - VO = 연속 이미지로 포즈 추정" << std::endl;
    std::cout << "   - SLAM의 프론트엔드" << std::endl;
    std::cout << "   - Monocular VO의 스케일 모호성\n" << std::endl;

    std::cout << "💡 다음 단계 (README.md 학습 순서 참고):" << std::endl;
    std::cout << "   1. README.md에서 VO/SLAM 이론 읽기 → quiz_easy 문제 1~2" << std::endl;
    std::cout << "   2. my_basic.cpp Step 1~2 구현 → quiz_easy 문제 3~4" << std::endl;
    std::cout << "   3. my_basic.cpp Step 3~4 구현 → quiz_medium 문제 1~3" << std::endl;
    std::cout << "   4. PRACTICE.md에서 추가 실습\n" << std::endl;

    std::cout << "다음: Week 2 - 2D-2D 모션 추정\n" << std::endl;

    return 0;
}
#endif  // BASIC_LIB_ONLY
