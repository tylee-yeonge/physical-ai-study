#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "camera.h"
#include "descriptor.h"
#include "feature_detector.h"
#include "feature_matcher.h"
#include "epipolar.h"
#include "pose_recovery.h"
#include "tracker.h"
#include "triangulator.h"

// ─────────────────────────────────────────────────────────────────────────────
// 테스트용 합성 이미지 생성
//
// 코너가 많은 체커보드 패턴 — FAST 검출 확인에 적합
// ─────────────────────────────────────────────────────────────────────────────
static cv::Mat makeSyntheticImage(int width = 640, int height = 480)
{
    cv::Mat img(height, width, CV_8UC1, cv::Scalar(128));

    // 체커보드 (50x50 픽셀 셀)
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

    // 랜덤 노이즈 추가 (FAST 검출 현실화)
    cv::Mat noise(height, width, CV_8SC1);
    cv::randn(noise, 0, 5);
    cv::add(img, noise, img, cv::noArray(), CV_8UC1);

    return img;
}

// ─────────────────────────────────────────────────────────────────────────────
// W3 데모: FAST 직접 구현
// ─────────────────────────────────────────────────────────────────────────────
static void demoW3(const cv::Mat& gray)
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "  W3: FAST 직접 구현 (FeatureDetector)" << std::endl;
    std::cout << "========================================" << std::endl;

    FeatureDetector detector(/*threshold=*/20, /*n=*/9, /*nms_radius=*/5);

    // 직접 구현 vs OpenCV 비교 출력
    detector.compareWithOpenCV(gray);

    // 시각화
    auto my_kps = detector.detect(gray);
    cv::Mat vis;
    cv::cvtColor(gray, vis, cv::COLOR_GRAY2BGR);
    cv::drawKeypoints(vis, my_kps, vis, cv::Scalar(0, 255, 0),
                      cv::DrawMatchesFlags::DRAW_OVER_OUTIMG);

    // 상단 왼쪽에 정보 표시
    std::string text = "FAST (directly implemented): " + std::to_string(my_kps.size()) + " keypoints";
    cv::putText(vis, text, {10, 30}, cv::FONT_HERSHEY_SIMPLEX, 0.7,
                cv::Scalar(0, 255, 255), 2);

    cv::imwrite("w3_fast_result.png", vis);
    std::cout << "\n  결과 저장: w3_fast_result.png" << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// W3 데모: BRIEF 직접 구현
// ─────────────────────────────────────────────────────────────────────────────
static void demoW3Brief(const cv::Mat& gray)
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "  W3: BRIEF 직접 구현 (Descriptor)" << std::endl;
    std::cout << "========================================" << std::endl;

    FeatureDetector detector(20, 9, 5);
    auto keypoints = detector.detect(gray);

    Descriptor desc(/*patch_size=*/31, /*n_pairs=*/256);
    desc.compareWithOpenCV(gray, keypoints);

    // 완성된 경우: 디스크립터 시각화 (첫 키포인트 비트열 일부 출력)
    auto kps_copy = keypoints;
    cv::Mat descriptors = desc.compute(gray, kps_copy);
    if (!descriptors.empty() && descriptors.rows > 0)
    {
        std::cout << "\n  첫 번째 키포인트 디스크립터 (앞 8바이트):" << std::endl;
        std::cout << "  ";
        for (int j = 0; j < std::min(8, descriptors.cols); j++)
        {
            uchar byte = descriptors.at<uchar>(0, j);
            // 바이트를 이진수로 출력
            for (int b = 7; b >= 0; b--)
                std::cout << ((byte >> b) & 1);
            std::cout << " ";
        }
        std::cout << std::endl;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// W4~W8: 미구현 안내
// ─────────────────────────────────────────────────────────────────────────────
static void showPendingModules()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "  미구현 모듈 (TODO)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  W4: FeatureMatcher  — BF + Ratio Test + RANSAC" << std::endl;
    std::cout << "  W5: Epipolar        — 8-Point Algorithm" << std::endl;
    std::cout << "  W6: PoseRecovery    — E → R, t (SVD + Cheirality)" << std::endl;
    std::cout << "  W7: Triangulator    — DLT 삼각측량" << std::endl;
    std::cout << "  W8: Tracker         — LK 광류 (AᵀA + 피라미드)" << std::endl;
    std::cout << std::endl;
    std::cout << "  해당 주차 학습 후 각 파일에 직접 구현하세요." << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    std::cout << "\n╔══════════════════════════════════════╗" << std::endl;
    std::cout << "║         mini_vo — Phase 2            ║" << std::endl;
    std::cout << "╚══════════════════════════════════════╝" << std::endl;

    // 이미지 로드 (인자로 경로 지정 가능, 없으면 합성 이미지 사용)
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
        std::cout << "  실제 이미지 사용: ./mini_vo <image_path>" << std::endl;
    }

    // 카메라 파라미터 (TUM fr1 기준)
    Camera cam(517.3, 516.5, 318.6, 255.3);
    std::cout << "\n  카메라 K:\n" << cam.K() << std::endl;

    // 각 주차 데모 실행
    demoW3(gray);
    demoW3Brief(gray);
    showPendingModules();

    std::cout << "\n  Done." << std::endl;
    return 0;
}
