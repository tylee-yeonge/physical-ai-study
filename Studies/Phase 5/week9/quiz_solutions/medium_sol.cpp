/**
 * Phase 5 Week 9 - ONNX & TensorRT 변환 중급 퀴즈 정답 및 해설
 *
 * 컴파일: g++ -std=c++17 -o medium_sol medium_sol.cpp
 * 실행: ./medium_sol
 */

#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <numeric>

void problem1_solution()
{
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 정답: A) (c) -> (d) -> (e) -> (a) -> (b)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "해설:" << std::endl;
    std::cout << "  올바른 전처리 순서:" << std::endl;
    std::cout << std::endl;
    std::cout << "  (c) Resize 384x512" << std::endl;
    std::cout << "      -> 모델 입력 크기에 맞춤" << std::endl;
    std::cout << "      -> cv::resize(image, resized, cv::Size(512, 384))" << std::endl;
    std::cout << std::endl;
    std::cout << "  (d) BGR -> RGB" << std::endl;
    std::cout << "      -> OpenCV는 BGR, 모델은 RGB를 기대" << std::endl;
    std::cout << "      -> cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB)" << std::endl;
    std::cout << std::endl;
    std::cout << "  (e) uint8 -> float32 (0~1)" << std::endl;
    std::cout << "      -> rgb.convertTo(rgb, CV_32FC3, 1.0f / 255.0f)" << std::endl;
    std::cout << std::endl;
    std::cout << "  (a) ImageNet mean/std 정규화" << std::endl;
    std::cout << "      -> (pixel - mean) / std" << std::endl;
    std::cout << "      -> mean = [0.485, 0.456, 0.406]" << std::endl;
    std::cout << "      -> std  = [0.229, 0.224, 0.225]" << std::endl;
    std::cout << std::endl;
    std::cout << "  (b) NHWC -> NCHW" << std::endl;
    std::cout << "      -> OpenCV는 HWC 형식, TensorRT는 CHW 형식" << std::endl;
    std::cout << "      -> cv::split()로 채널 분리 후 순서대로 복사" << std::endl;
    std::cout << std::endl;
    std::cout << "  핵심: 정규화는 float 변환 이후에 해야 합니다." << std::endl;
    std::cout << "  NCHW 변환은 GPU에 올리기 직전(마지막)에 수행합니다." << std::endl;
}

void problem2_solution()
{
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 정답: Jetson 메모리 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    float total_gb = 8.0f;
    float system_gb = 2.0f;
    float depth_gb = 350.0f / 1024.0f;
    float yolo_gb = 200.0f / 1024.0f;
    float buffer_gb = 50.0f / 1024.0f;
    float cuda_gb = 500.0f / 1024.0f;

    float used_gb = system_gb + depth_gb + yolo_gb + buffer_gb + cuda_gb;
    float free_gb = total_gb - used_gb;

    std::cout << "해설:" << std::endl;
    std::cout << "  총 메모리:    " << total_gb << " GB" << std::endl;
    std::cout << "  사용 합계:    " << used_gb << " GB" << std::endl;
    std::cout << "  여유 메모리:   " << free_gb << " GB (약 " << free_gb << " GB)" << std::endl;
    std::cout << std::endl;
    std::cout << "  Jetson Orin Nano는 통합 메모리(Unified Memory)입니다." << std::endl;
    std::cout << "  CPU와 GPU가 같은 물리 메모리를 공유합니다." << std::endl;
    std::cout << std::endl;
    std::cout << "  약 " << free_gb << " GB 여유가 있으므로:" << std::endl;
    std::cout << "  - 소규모 추가 모델 (예: Semantic Segmentation) 가능" << std::endl;
    std::cout << "  - 하지만 메모리 부족 시 OOM Killer가 프로세스 종료" << std::endl;
    std::cout << "  - 안전을 위해 항상 1GB 이상 여유를 유지하는 것을 권장" << std::endl;
    std::cout << std::endl;
    std::cout << "  메모리 절약 팁:" << std::endl;
    std::cout << "  1. FP16 사용 (FP32 대비 ~40% 절약)" << std::endl;
    std::cout << "  2. 작은 모델 선택 (ViT-S > ViT-B > ViT-L)" << std::endl;
    std::cout << "  3. Zero-copy 활용 (cudaHostAllocMapped)" << std::endl;
    std::cout << "  4. 불필요한 GUI 끄기 (sudo systemctl set-default multi-user.target)"
              << std::endl;
}

void problem3_solution()
{
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 정답: B) 초기화 오버헤드 포함" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "해설:" << std::endl;
    std::cout << "  워밍업(Warm-up) 단계에서 발생하는 오버헤드:" << std::endl;
    std::cout << std::endl;
    std::cout << "  1. CUDA 커널 최초 로딩" << std::endl;
    std::cout << "     -> 첫 실행 시 GPU에 커널 코드를 전송" << std::endl;
    std::cout << "     -> JIT(Just-In-Time) 컴파일 발생 가능" << std::endl;
    std::cout << std::endl;
    std::cout << "  2. GPU 메모리 할당" << std::endl;
    std::cout << "     -> 첫 cudaMalloc이 실제 물리 메모리를 할당" << std::endl;
    std::cout << "     -> 이후에는 캐시된 메모리 풀에서 빠르게 할당" << std::endl;
    std::cout << std::endl;
    std::cout << "  3. GPU 클럭 스케일링" << std::endl;
    std::cout << "     -> 유휴 상태에서 저클럭으로 동작" << std::endl;
    std::cout << "     -> 부하 감지 후 클럭을 올리는 데 시간 소요" << std::endl;
    std::cout << std::endl;
    std::cout << "  4. TensorRT 내부 캐시 초기화" << std::endl;
    std::cout << "     -> 연산 버퍼, 중간 텐서 등의 초기 설정" << std::endl;
    std::cout << std::endl;
    std::cout << "  정확한 벤치마크를 위한 규칙:" << std::endl;
    std::cout << "  - Warm-up: 최소 10프레임 실행 후 버림" << std::endl;
    std::cout << "  - 측정: 100프레임 이상 평균" << std::endl;
    std::cout << "  - cudaDeviceSynchronize(): 비동기 완료 대기 필수" << std::endl;
    std::cout << "  - 전처리 + 추론 + 후처리 모두 포함" << std::endl;
}

void problem4_solution()
{
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 정답: B) cudaStreamSynchronize 누락" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "해설:" << std::endl;
    std::cout << "  cudaMemcpyAsync는 비동기(asynchronous) 복사입니다." << std::endl;
    std::cout << "  함수가 리턴해도 실제 복사가 완료되지 않았을 수 있습니다." << std::endl;
    std::cout << std::endl;
    std::cout << "  잘못된 코드:" << std::endl;
    std::cout << "    cudaMemcpyAsync(output_host, ...);" << std::endl;
    std::cout << "    // 바로 output_host 사용 -> 데이터가 불완전!" << std::endl;
    std::cout << "    postprocess(output_host.data());" << std::endl;
    std::cout << std::endl;
    std::cout << "  올바른 코드:" << std::endl;
    std::cout << "    cudaMemcpyAsync(output_host, ...);" << std::endl;
    std::cout << "    cudaStreamSynchronize(stream_);  // 복사 완료 대기!" << std::endl;
    std::cout << "    postprocess(output_host.data());  // 이제 안전" << std::endl;
    std::cout << std::endl;
    std::cout << "  비동기 연산의 핵심:" << std::endl;
    std::cout << "  - Async 함수는 GPU에 작업을 '명령'만 하고 바로 리턴" << std::endl;
    std::cout << "  - GPU가 실제 작업을 완료하는 것은 나중" << std::endl;
    std::cout << "  - CPU에서 결과를 사용하려면 반드시 Synchronize 필요" << std::endl;
    std::cout << "  - Synchronize 없이 접근하면 불완전하거나 이전 데이터를 읽음" << std::endl;
    std::cout << std::endl;
    std::cout << "  동기화 방법들:" << std::endl;
    std::cout << "  - cudaStreamSynchronize(stream): 특정 스트림 대기" << std::endl;
    std::cout << "  - cudaDeviceSynchronize(): 모든 스트림 대기" << std::endl;
    std::cout << "  - cudaEventSynchronize(event): 특정 이벤트 대기" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Week 9 Quiz - Medium 정답 (C++)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();

    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
