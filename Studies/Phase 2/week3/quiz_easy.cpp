/**
 * Phase 2 Week 3 - 특징점 검출 기초 퀴즈
 *
 * FAST, ORB 등의 특징점 검출 개념을 확인합니다.
 */

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>

/**
 * 문제 1: FAST 파라미터 이해
 *
 * FAST 검출기의 임계값(threshold)을 변경하면서
 * 검출되는 특징점 개수의 변화를 관찰하세요.
 */
void problem1_fast_threshold()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: FAST 임계값과 특징점 개수" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // 테스트 이미지 생성
    cv::Mat image = cv::imread("test_image.jpg", cv::IMREAD_GRAYSCALE);
    if (image.empty())
    {
        // 이미지 없으면 체커보드 생성
        image = cv::Mat::zeros(600, 800, CV_8UC1);
        for (int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                if ((i + j) % 2 == 0)
                {
                    cv::rectangle(image, cv::Point(j * 50, i * 50),
                                  cv::Point((j + 1) * 50, (i + 1) * 50), cv::Scalar(255), -1);
                }
            }
        }
    }

    std::vector<int> thresholds = {10, 20, 30, 40, 50};

    std::cout << "📊 임계값에 따른 특징점 개수:\n" << std::endl;

    for (int threshold : thresholds)
    {
        std::vector<cv::KeyPoint> keypoints;

        // TODO: cv::FAST 함수로 keypoints에 검출 결과를 저장하세요
        // 파라미터: 이미지, 결과 벡터, 임계값, NMS 사용 여부

        std::cout << "   Threshold = " << threshold << " → 특징점: " << keypoints.size() << "개"
                  << std::endl;
    }

    std::cout << "\n💡 질문:" << std::endl;
    std::cout << "   1. 임계값이 증가하면 특징점 개수는 어떻게 변하나요?" << std::endl;
    std::cout << "   2. 실시간 SLAM에 적합한 임계값은?" << std::endl;
    std::cout << "\n힌트: 임계값 ↑ → 더 강한 코너만 검출 → 개수 ↓" << std::endl;
}

/**
 * 문제 2: ORB 디스크립터 크기
 *
 * ORB 디스크립터의 크기를 확인하고,
 * 이진 디스크립터의 특징을 이해하세요.
 */
void problem2_orb_descriptor()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: ORB 디스크립터 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat image = cv::Mat::zeros(400, 600, CV_8UC1);
    cv::circle(image, cv::Point(300, 200), 100, cv::Scalar(255), -1);

    cv::Ptr<cv::ORB> orb = cv::ORB::create(100);
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;

    orb->detectAndCompute(image, cv::noArray(), keypoints, descriptors);

    std::cout << "검출 결과:" << std::endl;
    std::cout << "   - 특징점 개수: " << keypoints.size() << std::endl;
    std::cout << "   - 디스크립터 크기: " << descriptors.size() << std::endl;
    std::cout << "   - 디스크립터 타입: " << descriptors.type() << std::endl;

    if (!descriptors.empty())
    {
        std::cout << "\n첫 번째 디스크립터(일부):" << std::endl;
        std::cout << "   ";
        for (int i = 0; i < std::min(8, descriptors.cols); i++)
        {
            std::cout << (int)descriptors.at<uchar>(0, i) << " ";
        }
        std::cout << "..." << std::endl;
    }

    std::cout << "\n💡 질문:" << std::endl;
    std::cout << "   1. ORB 디스크립터는 몇 바이트인가요?" << std::endl;
    std::cout << "   2. 이진 디스크립터의 장점은?" << std::endl;
    std::cout << "\n힌트: ORB = 256 bits = 32 bytes, 해밍거리로 빠른 매칭" << std::endl;
}

/**
 * 문제 3: NMS(Non-Maximum Suppression) 효과
 *
 * NMS 적용 전후의 특징점 개수와 분포를 비교하세요.
 */
void problem3_nms_effect()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: NMS 효과 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat image = cv::Mat::zeros(400, 600, CV_8UC1);
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 12; j++)
        {
            if ((i + j) % 2 == 0)
            {
                cv::rectangle(image, cv::Point(j * 50, i * 50),
                              cv::Point((j + 1) * 50, (i + 1) * 50), cv::Scalar(255), -1);
            }
        }
    }

    std::vector<cv::KeyPoint> kp_without_nms, kp_with_nms;

    // TODO: NMS 없이 FAST 검출 (threshold=20, nonmaxSuppression=false)

    // TODO: NMS 적용 FAST 검출 (threshold=20, nonmaxSuppression=true)

    std::cout << "결과:" << std::endl;
    std::cout << "   NMS 없음: " << kp_without_nms.size() << "개" << std::endl;
    std::cout << "   NMS 적용: " << kp_with_nms.size() << "개" << std::endl;

    if (!kp_without_nms.empty() && !kp_with_nms.empty())
    {
        double reduction = 100.0 * (1.0 - double(kp_with_nms.size()) / kp_without_nms.size());
        std::cout << "   → 감소율: " << reduction << "%\n" << std::endl;
    }

    std::cout << "💡 질문:" << std::endl;
    std::cout << "   1. NMS는 왜 필요한가요?" << std::endl;
    std::cout << "   2. SLAM에서 NMS를 안 하면 어떤 문제가 생기나요?" << std::endl;
    std::cout << "\n힌트: 중복 특징점 제거 → 더 균등한 분포 → 더 나은 포즈 추정" << std::endl;
}

/**
 * 문제 4: 검출기 속도 비교
 *
 * FAST와 ORB의 검출 시간을 측정하고 비교하세요.
 */
void problem4_speed_comparison()
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: 검출기 속도 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    cv::Mat image = cv::Mat::zeros(600, 800, CV_8UC3);
    for (int i = 0; i < 50; i++)
    {
        cv::Point center(rand() % 800, rand() % 600);
        int radius = 10 + rand() % 30;
        cv::circle(image, center, radius, cv::Scalar(rand() % 255, rand() % 255, rand() % 255), -1);
    }

    // TODO: FAST 시간 측정
    // std::chrono::high_resolution_clock으로 시작/종료 시간을 기록하세요
    // FAST 검출 후 소요 시간(ms)을 계산하세요

    // TODO: ORB 시간 측정
    // ORB::create()로 검출기 생성 후 detectAndCompute 시간을 측정하세요

    std::cout << "💡 질문:" << std::endl;
    std::cout << "   1. 어떤 검출기가 더 빠른가요?" << std::endl;
    std::cout << "   2. 실시간 SLAM (30 FPS)에 적합한 것은?" << std::endl;
    std::cout << "\n힌트: FAST는 이름처럼 매우 빠름, ORB는 디스크립터 계산 추가" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Phase 2 Week 3 Quiz - Easy" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_fast_threshold() problem2_orb_descriptor() problem3_nms_effect()
            problem4_speed_comparison()

                std::cout
        << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
