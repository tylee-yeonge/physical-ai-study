// Phase 2 Week 3 - 기초 퀴즈 정답 (요약)
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "Week 3 기초 퀴즈 핵심 정답\n" << std::endl;

    cv::Mat image = cv::Mat::zeros(400, 600, CV_8UC1);

    // 문제 1: FAST threshold
    std::cout << "문제 1: 임계값 ↑ → 특징점 개수 ↓" << std::endl;
    std::cout << "   - 더 강한 코너만 검출됨\n" << std::endl;

    // 문제 2: ORB descriptor
    std::cout << "문제 2: ORB = 256 bits = 32 bytes" << std::endl;
    std::cout << "   - 이진 디스크립터 → 해밍거리로 빠른 매칭\n" << std::endl;

    // 문제 3: NMS
    std::cout << "문제 3: NMS로 중복 제거" << std::endl;
    std::cout << "   - 더 균등한 분포 → 더 나은 포즈 추정\n" << std::endl;

    // 문제 4: Speed
    std::cout << "문제 4: FAST가 ORB보다 빠름" << std::endl;
    std::cout << "   - FAST: 코너만, ORB: 코너 + 디스크립터\n" << std::endl;

    // 문제 5: Harris 응답
    std::cout << "문제 5: Harris 응답 R = det(M) - k * trace(M)^2" << std::endl;
    double k = 0.04;

    // 케이스 1: 코너 M = [500, 10; 10, 480]
    double det_a = 500.0 * 480.0 - 10.0 * 10.0;      // 239900
    double trace_a = 500.0 + 480.0;                     // 980
    double R_a = det_a - k * trace_a * trace_a;         // 239900 - 0.04*960400 = 201484
    std::cout << "   케이스 1 (코너): det=" << det_a << ", trace=" << trace_a
              << ", R=" << R_a << " → 코너\n" << std::endl;

    // 케이스 2: 에지 M = [800, 5; 5, 10]
    double det_b = 800.0 * 10.0 - 5.0 * 5.0;          // 7975
    double trace_b = 800.0 + 10.0;                      // 810
    double R_b = det_b - k * trace_b * trace_b;         // 7975 - 0.04*656100 = -18269
    std::cout << "   케이스 2 (에지): det=" << det_b << ", trace=" << trace_b
              << ", R=" << R_b << " → 에지\n" << std::endl;

    // 케이스 3: 평면 M = [3, 1; 1, 2]
    double det_c = 3.0 * 2.0 - 1.0 * 1.0;              // 5
    double trace_c = 3.0 + 2.0;                          // 5
    double R_c = det_c - k * trace_c * trace_c;          // 5 - 0.04*25 = 4
    std::cout << "   케이스 3 (평면): det=" << det_c << ", trace=" << trace_c
              << ", R=" << R_c << " → |R|이 작아 평면에 가까움" << std::endl;

    std::cout << "\n   k 값 변화 (케이스 1):" << std::endl;
    std::vector<double> k_values = {0.02, 0.04, 0.06, 0.10};
    for (double kv : k_values)
    {
        double R_k = det_a - kv * trace_a * trace_a;
        std::cout << "     k=" << kv << " → R=" << R_k << std::endl;
    }
    std::cout << "   → k가 커지면 R이 감소, 코너 판별이 엄격해짐" << std::endl;

    return 0;
}
