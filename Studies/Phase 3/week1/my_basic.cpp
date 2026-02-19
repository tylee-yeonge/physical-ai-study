/**
 * Phase 3 Week 1 - VO 자료구조 직접 구현
 *
 * vo_types.h의 데이터 구조를 활용하여 VO 파이프라인의 기본 개념을 연습합니다.
 * 빌드: cd build && cmake .. && make my_basic
 * 실행: ./my_basic (각 Step 구현 후 실행하여 확인)
 *
 * ┌──────┬───────────────────┬────────┬────────────────┐
 * │ Step │ 함수              │ 난이도  │ 검증 방법        │
 * ├──────┼───────────────────┼────────┼────────────────┤
 * │  1   │ createCamera      │ 쉬움   │ ./my_basic     │
 * │  2   │ composePoses      │ 핵심   │ ./my_basic     │
 * │  3   │ createFrameAndMap │ 보통   │ ./my_basic     │
 * │  4   │ simulateDrift     │ 어려움  │ ./my_basic     │
 * └──────┴───────────────────┴────────┴────────────────┘
 *
 * 💡 각 Step 구현 후 ./my_basic을 실행하면 해당 단계의 테스트 결과를 확인할 수 있습니다.
 */
#include "vo_types.h"
#include <iostream>
#include <cmath>
#include <random>

using namespace vo;

Camera createCamera()
{
    // [Step 1] 카메라 내부 파라미터 생성
    // 힌트: Camera(fx, fy, cx, cy) 생성자 사용
    // 참고: basic.cpp의 demoDataStructures(), K 행렬 출력 확인
    // 기대값: fx=600, fy=600, cx=400, cy=300
    return Camera(600.0, 600.0, 400.0, 300.0);
}

Pose composePoses()
{
    // [Step 2] 포즈 합성: 전진 2회 → 최종 위치 계산
    // 힌트: Pose::operator*()로 합성, Pose::inverse()로 역변환
    // 참고: basic.cpp의 demoDataStructures(), Pose 역변환 확인
    // 기대값: 전진 1m × 2 = (0, 0, 2)

    // 첫 번째 이동: z축으로 1m 전진
    Pose step1;
    step1.t = Eigen::Vector3d(0.0, 0.0, 1.0);

    // 두 번째 이동: z축으로 1m 추가 전진
    Pose step2;
    step2.t = Eigen::Vector3d(0.0, 0.0, 1.0);

    // 합성: step1 * step2 = 총 이동
    return step1 * step2;
}

void createFrameAndMap(Frame& frame, std::vector<MapPoint>& map_points)
{
    // [Step 3] Frame과 MapPoint 생성 + 관측 연결
    // 힌트: Frame에 Feature 추가, MapPoint에 observed_by 설정
    // 참고: basic.cpp의 demoDataStructures()
    // 기대값: 1 프레임, 3 맵포인트, 각 맵포인트가 프레임 0에서 관측됨

    frame.id = 0;
    frame.image = cv::Mat::zeros(480, 640, CV_8UC1);
    frame.timestamp = 0.0;

    // 3D 맵포인트 생성
    std::vector<Eigen::Vector3d> positions = {
        {1.0, 0.5, 5.0}, {-0.5, 1.0, 4.0}, {0.0, -0.5, 6.0}};

    for (int i = 0; i < 3; i++)
    {
        MapPoint mp(i, positions[i]);
        mp.observed_by.push_back(frame.id);
        mp.observations = 1;
        map_points.push_back(mp);

        // 프레임에 대응 특징점 추가 (간단한 투영)
        Feature feat;
        feat.id = i;
        feat.pt = cv::Point2f(320.0f + i * 50.0f, 240.0f + i * 30.0f);
        feat.has_3d = true;
        feat.pt_3d = cv::Point3f(positions[i].x(), positions[i].y(), positions[i].z());
        frame.features.push_back(feat);
    }
}

std::vector<Pose> simulateDrift(int num_frames)
{
    // [Step 4] 포즈 누적 시 드리프트 시뮬레이션
    // 힌트: 매 프레임 z축 전진 + 작은 랜덤 노이즈 추가
    // 참고: basic.cpp의 demoVOvsSLAM(), 드리프트 누적 확인
    // 기대값: 10프레임 후 이상 위치 (0,0,10)에서 벗어남

    std::mt19937 gen(42);
    constexpr double kNoiseStd = 0.02;  // 2% 표준편차
    std::normal_distribution<double> noise(0.0, kNoiseStd);

    std::vector<Pose> trajectory;
    Pose current;
    trajectory.push_back(current);

    for (int i = 0; i < num_frames; i++)
    {
        // 이상적 이동: z축으로 1m 전진
        Pose step;
        step.t = Eigen::Vector3d(noise(gen), noise(gen), 1.0 + noise(gen));

        // 약간의 회전 노이즈 (yaw)
        double angle = noise(gen);
        Eigen::Matrix3d Rz;
        Rz << std::cos(angle), -std::sin(angle), 0,
            std::sin(angle), std::cos(angle), 0,
            0, 0, 1;
        step.R = Rz;

        current = current * step;
        trajectory.push_back(current);
    }

    return trajectory;
}

#ifndef MY_BASIC_LIB_ONLY
int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  [내 구현] VO 자료구조 - 단계별 테스트" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ── Step 1: createCamera ──
    std::cout << "Step 1: createCamera" << std::endl;
    Camera cam = createCamera();
    std::cout << "   fx=" << cam.fx << ", fy=" << cam.fy << std::endl;
    std::cout << "   cx=" << cam.cx << ", cy=" << cam.cy << std::endl;
    bool cam_ok = (cam.fx == 600.0 && cam.fy == 600.0 && cam.cx == 400.0 && cam.cy == 300.0);
    std::cout << "   " << (cam_ok ? "✅ K 행렬 정상" : "❌ 기대: fx=600, fy=600, cx=400, cy=300")
              << std::endl;

    // ── Step 2: composePoses ──
    std::cout << "\nStep 2: composePoses" << std::endl;
    Pose composed = composePoses();
    std::cout << "   최종 위치: " << composed.t.transpose() << std::endl;
    bool pose_ok = (std::abs(composed.t.z() - 2.0) < 0.001);
    std::cout << "   " << (pose_ok ? "✅ 전진 1m × 2 = (0,0,2)" : "❌ 기대: (0, 0, 2)")
              << std::endl;

    // 역변환 검증
    Pose inv = composed.inverse();
    Pose identity = composed * inv;
    double id_err = identity.t.norm();
    std::cout << "   합성 × 역변환 = t=" << identity.t.transpose()
              << (id_err < 0.001 ? " ✅ 항등" : " ❌ 기대: (0,0,0)") << std::endl;

    // ── Step 3: createFrameAndMap ──
    std::cout << "\nStep 3: createFrameAndMap" << std::endl;
    Frame frame;
    std::vector<MapPoint> map_points;
    createFrameAndMap(frame, map_points);

    std::cout << "   Frame ID: " << frame.id
              << ", 특징점: " << frame.features.size() << "개" << std::endl;
    std::cout << "   MapPoint: " << map_points.size() << "개" << std::endl;

    bool frame_ok = (frame.id == 0 && frame.features.size() == 3 && map_points.size() == 3);
    std::cout << "   " << (frame_ok ? "✅ 프레임+맵 정상" : "❌ 기대: 1프레임, 3맵포인트")
              << std::endl;

    // 관측 연결 확인
    bool obs_ok = true;
    for (const auto& mp : map_points)
    {
        if (mp.observed_by.empty() || mp.observed_by[0] != 0)
            obs_ok = false;
    }
    std::cout << "   " << (obs_ok ? "✅ 관측 연결 정상" : "❌ 기대: 모든 맵포인트가 프레임 0 관측")
              << std::endl;

    // ── Step 4: simulateDrift ──
    std::cout << "\nStep 4: simulateDrift" << std::endl;
    constexpr int kNumFrames = 10;
    auto trajectory = simulateDrift(kNumFrames);

    std::cout << "   궤적 (" << trajectory.size() << " 포즈):" << std::endl;
    Eigen::Vector3d ideal_pos(0, 0, 0);
    for (size_t i = 0; i < trajectory.size(); i++)
    {
        const auto& pose = trajectory[i];
        if (i > 0)
            ideal_pos.z() += 1.0;
        Eigen::Vector3d error = pose.t - ideal_pos;
        if (i % 3 == 0 || i == trajectory.size() - 1)
        {
            std::cout << "   Frame " << i << ": t=" << pose.t.transpose()
                      << "  오차=" << error.norm() << std::endl;
        }
    }

    // 최종 드리프트 확인
    Eigen::Vector3d final_pos = trajectory.back().t;
    Eigen::Vector3d expected(0, 0, static_cast<double>(kNumFrames));
    double drift = (final_pos - expected).norm();
    std::cout << "   이상 위치: " << expected.transpose() << std::endl;
    std::cout << "   실제 위치: " << final_pos.transpose() << std::endl;
    std::cout << "   드리프트: " << drift << "m" << std::endl;
    std::cout << "   " << (drift > 0.01 ? "✅ 드리프트 발생 확인!" : "❌ 노이즈가 너무 작음")
              << std::endl;

    // ── 요약 ──
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  💡 핵심 확인 사항:" << std::endl;
    std::cout << "   - Step 2: Pose 합성은 SE(3) 그룹 연산" << std::endl;
    std::cout << "   - Step 4: 작은 노이즈도 누적되면 큰 드리프트!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    return 0;
}
#endif
