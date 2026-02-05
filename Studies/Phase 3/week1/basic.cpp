/**
 * Phase 3 Week 1 - VO 기본 데모
 * 
 * VO의 기본 데이터 구조와 파이프라인 개념 이해
 */

#include "vo_types.h"
#include <iostream>
#include <iomanip>

using namespace vo;

void demoPipeline() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Visual Odometry 파이프라인 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "파이프라인 단계:\n" << std::endl;
    
    std::cout << "1️⃣  이미지 입력" << std::endl;
    std::cout << "   - 카메라에서 연속 프레임 획득" << std::endl;
    std::cout << "   - Grayscale 변환 (특징점 검출용)\n" << std::endl;
    
    std::cout << "2️⃣  특징점 검출" << std::endl;
    std::cout << "   - FAST, ORB 등으로 코너 검출" << std::endl;
    std::cout << "   - 200~500개 특징점 유지\n" << std::endl;
    
    std::cout << "3️⃣  특징점 추적/매칭" << std::endl;
    std::cout << "   - Optical Flow (Lucas-Kanade)" << std::endl;
    std::cout << "   - 또는 Descriptor 매칭\n" << std::endl;
    
    std::cout << "4️⃣  모션 추정" << std::endl;
    std::cout << "   - 초기화: 2D-2D (Essential Matrix)" << std::endl;
    std::cout << "   - 추적: 3D-2D (PnP)\n" << std::endl;
    
    std::cout << "5️⃣  Local 최적화 (선택)" << std::endl;
    std::cout << "   - Bundle Adjustment" << std::endl;
    std::cout << "   - Outlier 제거\n" << std::endl;
    
    std::cout << "6️⃣  포즈 출력" << std::endl;
    std::cout << "   - 카메라 위치 (R, t)" << std::endl;
    std::cout << "   - 궤적 저장/시각화\n" << std::endl;
}

void demoDataStructures() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "VO 데이터 구조 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    // Camera
    std::cout << "1️⃣  Camera 구조체" << std::endl;
    Camera cam(600.0, 600.0, 400.0, 300.0);
    std::cout << "   K =\n" << cam.K << "\n" << std::endl;
    
    // Pose
    std::cout << "2️⃣  Pose 구조체" << std::endl;
    Pose pose;
    pose.t = Eigen::Vector3d(1.0, 0.5, 0.0);
    std::cout << "   t = " << pose.t.transpose() << std::endl;
    
    Pose pose_inv = pose.inverse();
    std::cout << "   t_inv = " << pose_inv.t.transpose() << "\n" << std::endl;
    
    // Feature
    std::cout << "3️⃣  Feature 구조체" << std::endl;
    Feature feat(cv::Point2f(320, 240), 0);
    feat.has_3d = true;
    feat.pt_3d = cv::Point3f(0.5, 0.3, 5.0);
    std::cout << "   2D: (" << feat.pt.x << ", " << feat.pt.y << ")" << std::endl;
    std::cout << "   3D: (" << feat.pt_3d.x << ", " << feat.pt_3d.y << ", " << feat.pt_3d.z << ")\n" << std::endl;
    
    // MapPoint
    std::cout << "4️⃣  MapPoint 구조체" << std::endl;
    MapPoint mp(0, Eigen::Vector3d(1.0, 0.5, 3.0));
    mp.observed_by = {0, 1, 2};
    mp.observations = 3;
    std::cout << "   Position: " << mp.position.transpose() << std::endl;
    std::cout << "   Observed by " << mp.observations << " frames\n" << std::endl;
}

void demoVOTypes() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "VO 유형 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "📷 Monocular VO (단안)" << std::endl;
    std::cout << "   ✅ 간단, 저렴" << std::endl;
    std::cout << "   ⚠️  스케일 모호성" << std::endl;
    std::cout << "   사용: 드론, 실내 로봇\n" << std::endl;
    
    std::cout << "📷📷 Stereo VO (스테레오)" << std::endl;
    std::cout << "   ✅ 스케일 복원" << std::endl;
    std::cout << "   ✅ 안정적 depth" << std::endl;
    std::cout << "   ⚠️  무겁고 비쌈" << std::endl;
    std::cout << "   사용: 자율주행\n" << std::endl;
    
    std::cout << "📷🌈 RGB-D VO" << std::endl;
    std::cout << "   ✅ 직접 depth" << std::endl;
    std::cout << "   ✅ 빠른 처리" << std::endl;
    std::cout << "   ⚠️  실외/햇빛 약함" << std::endl;
    std::cout << "   사용: 실내 AR\n" << std::endl;
}

void demoScaleAmbiguity() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "스케일 모호성 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "Monocular VO의 문제점:\n" << std::endl;
    
    std::cout << "실제 이동: 1m" << std::endl;
    std::cout << "추정 이동 (스케일 α):" << std::endl;
    
    for (double scale : {0.5, 1.0, 2.0}) {
        std::cout << "   α=" << scale << " → " << 1.0 * scale << "m" << std::endl;
    }
    
    std::cout << "\n💡 모두 동일한 이미지를 생성!" << std::endl;
    std::cout << "   → 카메라만으로는 구분 불가\n" << std::endl;
    
    std::cout << "해결 방법:" << std::endl;
    std::cout << "   1. IMU와 결합 (VIO)" << std::endl;
    std::cout << "   2. 스테레오 카메라" << std::endl;
    std::cout << "   3. 알려진 물체 크기" << std::endl;
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Phase 3 Week 1: VO 개요 데모" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    demoPipeline();
    demoDataStructures();
    demoVOTypes();
    demoScaleAmbiguity();
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "✅ Week 1 완료!" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    std::cout << "💡 핵심 내용:" << std::endl;
    std::cout << "   - VO = 연속 이미지로 포즈 추정" << std::endl;
    std::cout << "   - SLAM의 프론트엔드" << std::endl;
    std::cout << "   - Monocular VO의 스케일 모호성\n" << std::endl;
    
    std::cout << "다음: Week 2 - 2D-2D 모션 추정\n" << std::endl;
    
    return 0;
}
