#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// Phase 2 mini_vo 모듈 (재사용)
#include "camera.h"
#include "descriptor.h"
#include "feature_detector.h"
#include "tracker.h"
#include "triangulator.h"

// Phase 3 mini_slam 모듈
#include "keyframe.h"
#include "local_ba_ceres.h"
#include "local_ba_g2o.h"
#include "map.h"
#include "types.h"

// ─────────────────────────────────────────────────────────────────────────────
// 테스트용 합성 스테레오 이미지 쌍 생성
//
// 체커보드 기본 이미지 + 수평 시프트로 두 번째 이미지 생성
// ─────────────────────────────────────────────────────────────────────────────
static cv::Mat makeSyntheticImage(int width = 640, int height = 480)
{
    cv::Mat img(height, width, CV_8UC1, cv::Scalar(128));

    int cell = 50;
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            int row_cell = r / cell;
            int col_cell = c / cell;
            if ((row_cell + col_cell) % 2 == 0)
                img.at<uchar>(r, c) = 200;
            else
                img.at<uchar>(r, c) = 50;
        }
    }

    cv::Mat noise(height, width, CV_8SC1);
    cv::randn(noise, 0, 5);
    cv::add(img, noise, img, cv::noArray(), CV_8UC1);

    return img;
}

// ─────────────────────────────────────────────────────────────────────────────
// W1: VO 파이프라인 검증 (Phase 2 모듈 재사용 확인)
// ─────────────────────────────────────────────────────────────────────────────
static void demoW1(const cv::Mat& gray, const Camera& cam)
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "  W1: VO 파이프라인 검증" << std::endl;
    std::cout << "========================================" << std::endl;

    // FAST 특징점 검출
    FeatureDetector detector(20, 9, 5);
    auto keypoints = detector.detect(gray);
    std::cout << "  FAST 검출: " << keypoints.size() << " 개" << std::endl;

    // BRIEF 디스크립터
    Descriptor desc(31, 256);
    cv::Mat descriptors = desc.compute(gray, keypoints);
    std::cout << "  BRIEF 디스크립터: " << descriptors.rows << " x "
              << descriptors.cols << std::endl;

    // 카메라 K 확인
    std::cout << "  카메라 K:\n" << cam.K() << std::endl;

    std::cout << "\n  Phase 2 모듈 재사용 확인 완료" << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// W6: 키프레임 + 맵 데모
// ─────────────────────────────────────────────────────────────────────────────
static void demoW6(const cv::Mat& gray, const Camera& cam)
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "  W6: Keyframe + Map 관리" << std::endl;
    std::cout << "========================================" << std::endl;

    slam::Map map;

    // 키프레임 2개 생성 (합성 포즈)
    slam::Pose pose0;  // 원점
    slam::Pose pose1(cv::Mat::eye(3, 3, CV_64F),
                     (cv::Mat_<double>(3, 1) << 0.5, 0.0, 0.0));

    auto* kf0 = new slam::Keyframe(0, gray, pose0);
    auto* kf1 = new slam::Keyframe(1, gray, pose1);

    // 특징점 검출
    FeatureDetector detector(20, 9, 5);
    kf0->keypoints_ = detector.detect(gray);
    kf1->keypoints_ = detector.detect(gray);

    map.addKeyframe(kf0);
    map.addKeyframe(kf1);

    // 합성 맵 포인트 추가
    for (int i = 0; i < 10; i++)
    {
        cv::Point3f pt(static_cast<float>(i) * 0.5f, 0.0f, 5.0f);
        int mp_id = map.addMapPoint(pt);

        // 양쪽 키프레임에서 관측
        map.getMapPoint(mp_id).num_observations = 2;
        map.getMapPoint(mp_id).observer_keyframe_ids = {0, 1};
    }

    // 맵 포인트 ID를 키프레임에 연결
    kf0->map_point_ids_.assign(kf0->keypoints_.size(), -1);
    kf1->map_point_ids_.assign(kf1->keypoints_.size(), -1);
    int assign_count = std::min(10, static_cast<int>(kf0->keypoints_.size()));
    for (int i = 0; i < assign_count; i++)
    {
        kf0->map_point_ids_[i] = i;
        kf1->map_point_ids_[i] = i;
    }

    // 공가시성 갱신
    map.updateCovisibility();

    std::cout << "  키프레임 수: " << map.numKeyframes() << std::endl;
    std::cout << "  맵 포인트 수: " << map.numMapPoints() << std::endl;

    // 공가시성 확인
    auto covis = kf0->getTopCovisibleKeyframes(5);
    std::cout << "  KF0의 공가시성 키프레임: ";
    for (int id : covis) std::cout << id << " ";
    std::cout << std::endl;

    // 키프레임 선택기 테스트
    slam::KeyframeSelector selector(10, 30, 0.8, 5.0);
    bool need = selector.needNewKeyframe(15, 5, 10, 6.0);
    std::cout << "  새 키프레임 필요? " << (need ? "YES" : "NO") << std::endl;

    (void)cam;
    std::cout << "\n  Keyframe + Map 데모 완료" << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// W8~W10: BA 스텁 데모
// ─────────────────────────────────────────────────────────────────────────────
static void demoBA(const Camera& cam)
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "  W8~W10: Bundle Adjustment (stub)" << std::endl;
    std::cout << "========================================" << std::endl;

    slam::Map map;
    cv::Mat gray = cv::Mat::zeros(100, 100, CV_8UC1);

    // 최소한의 키프레임 + 맵 포인트 설정
    auto* kf0 = new slam::Keyframe(0, gray, slam::Pose());
    map.addKeyframe(kf0);
    map.addMapPoint(cv::Point3f(1.0f, 2.0f, 5.0f));

    // Ceres BA (스텁)
    slam::LocalBACeres ba_ceres(cam.K(), 20);
    ba_ceres.optimize(map);

    // g2o BA (스텁)
    slam::LocalBAG2O ba_g2o(cam.K(), 10, true);
    ba_g2o.optimize(map);

    std::cout << "\n  BA 스텁 데모 완료 (W8~W10에서 구현 예정)" << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// 미구현 모듈 안내
// ─────────────────────────────────────────────────────────────────────────────
static void showPendingModules()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "  미구현 모듈 (TODO)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  W2:  E Matrix RANSAC 튜닝 (Phase 2 Epipolar 확장)" << std::endl;
    std::cout << "  W3:  PnP RANSAC 직접 구현" << std::endl;
    std::cout << "  W5:  mini_vo → mini_slam 완전 통합" << std::endl;
    std::cout << "  W8:  Ceres Local BA 구현" << std::endl;
    std::cout << "  W9:  g2o Local BA + Schur" << std::endl;
    std::cout << "  W10: g2o BA 통합 + 비교" << std::endl;
    std::cout << "  W12: 스케일 드리프트 정량 측정" << std::endl;
    std::cout << "  W13: 최종 데모 + GT 비교" << std::endl;
    std::cout << std::endl;
    std::cout << "  해당 주차 학습 후 각 파일에 직접 구현하세요." << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    std::cout << "\n╔══════════════════════════════════════╗" << std::endl;
    std::cout << "║       mini_slam — Phase 3            ║" << std::endl;
    std::cout << "╚══════════════════════════════════════╝" << std::endl;

    // 이미지 로드
    cv::Mat gray;
    if (argc >= 2)
    {
        cv::Mat img = cv::imread(argv[1]);
        if (img.empty())
        {
            std::cerr << "이미지를 열 수 없습니다: " << argv[1] << std::endl;
            return 1;
        }
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        std::cout << "\n  입력 이미지: " << argv[1]
                  << " (" << gray.cols << "x" << gray.rows << ")" << std::endl;
    }
    else
    {
        gray = makeSyntheticImage();
        std::cout << "\n  합성 체커보드 이미지 사용 (640x480)" << std::endl;
        std::cout << "  실제 이미지 사용: ./mini_slam <image_path>" << std::endl;
    }

    // 카메라 파라미터 (TUM fr1 기준)
    Camera cam(517.3, 516.5, 318.6, 255.3);

    // 데모 실행
    demoW1(gray, cam);
    demoW6(gray, cam);
    demoBA(cam);
    showPendingModules();

    std::cout << "\n  Done." << std::endl;
    return 0;
}
