/**
 * Phase 5 Week 9 - ONNX & TensorRT 변환 중급 퀴즈
 *
 * TensorRT 추론 코드 분석 및 메모리/성능 관련 심화 문제입니다.
 * 직접 코드를 분석하고 결과를 예측해보세요.
 *
 * 컴파일: g++ -std=c++17 -o quiz_medium quiz_medium.cpp
 * 실행: ./quiz_medium
 */

#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <numeric>

void problem1_preprocessing_order() {
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: TensorRT 전처리 파이프라인 순서" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "질문: OpenCV로 읽은 이미지를 TensorRT Depth 모델에 입력하기 위한" << std::endl;
    std::cout << "      올바른 전처리 순서를 나열하시오." << std::endl;
    std::cout << std::endl;
    std::cout << "  전처리 단계들 (순서가 섞여 있음):" << std::endl;
    std::cout << "  (a) ImageNet mean/std로 정규화" << std::endl;
    std::cout << "  (b) NHWC -> NCHW 변환" << std::endl;
    std::cout << "  (c) 384x512로 resize" << std::endl;
    std::cout << "  (d) BGR -> RGB 색상 변환" << std::endl;
    std::cout << "  (e) uint8 -> float32 변환 (0~1 범위)" << std::endl;
    std::cout << std::endl;
    std::cout << "보기:" << std::endl;
    std::cout << "  A) (c) -> (d) -> (e) -> (a) -> (b)" << std::endl;
    std::cout << "  B) (d) -> (c) -> (a) -> (e) -> (b)" << std::endl;
    std::cout << "  C) (c) -> (e) -> (d) -> (b) -> (a)" << std::endl;
    std::cout << "  D) (e) -> (c) -> (d) -> (a) -> (b)" << std::endl;
    std::cout << std::endl;

    // TODO: 정답을 작성하세요
}

void problem2_memory_calculation() {
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: Jetson GPU 메모리 사용량 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "질문: Jetson Orin Nano (8GB 통합 메모리)에서 아래 구성으로" << std::endl;
    std::cout << "      동시에 Depth + YOLO를 돌릴 때, 여유 메모리는 약 얼마인가?" << std::endl;
    std::cout << std::endl;
    std::cout << "  - 시스템 OS/GUI: ~2.0 GB" << std::endl;
    std::cout << "  - Depth TRT (FP16): ~350 MB" << std::endl;
    std::cout << "  - YOLO TRT (FP16): ~200 MB" << std::endl;
    std::cout << "  - 입출력 버퍼: ~50 MB" << std::endl;
    std::cout << "  - CUDA 런타임: ~500 MB" << std::endl;
    std::cout << std::endl;

    // 직접 계산
    float total_gb = 8.0f;
    float system_gb = 2.0f;
    float depth_gb = 350.0f / 1024.0f;
    float yolo_gb = 200.0f / 1024.0f;
    float buffer_gb = 50.0f / 1024.0f;
    float cuda_gb = 500.0f / 1024.0f;

    float used_gb = system_gb + depth_gb + yolo_gb + buffer_gb + cuda_gb;
    float free_gb = total_gb - used_gb;

    std::cout << "  계산 과정:" << std::endl;
    std::cout << "    총 메모리:    " << total_gb << " GB" << std::endl;
    std::cout << "    시스템:       " << system_gb << " GB" << std::endl;
    std::cout << "    Depth TRT:    " << depth_gb << " GB" << std::endl;
    std::cout << "    YOLO TRT:     " << yolo_gb << " GB" << std::endl;
    std::cout << "    입출력 버퍼:   " << buffer_gb << " GB" << std::endl;
    std::cout << "    CUDA 런타임:  " << cuda_gb << " GB" << std::endl;
    std::cout << "    ─────────────────────────" << std::endl;
    std::cout << "    사용 합계:    " << used_gb << " GB" << std::endl;
    std::cout << "    여유 메모리:   " << free_gb << " GB" << std::endl;
    std::cout << std::endl;

    std::cout << "  이 여유 메모리로 추가 모델을 더 올릴 수 있을까요?" << std::endl;
    std::cout << "  (정답은 quiz_solutions/medium_sol.cpp 참고)" << std::endl;
}

void problem3_fps_analysis() {
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: FPS 측정 결과 분석" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "아래는 Depth TRT 엔진의 100프레임 벤치마크 결과입니다:" << std::endl;
    std::cout << std::endl;

    // 시뮬레이션 데이터
    std::vector<float> inference_times;
    // 처음 10프레임은 워밍업 (느림)
    for (int i = 0; i < 10; i++) {
        inference_times.push_back(120.0f + i * 5.0f);
    }
    // 나머지 90프레임은 안정적
    for (int i = 0; i < 90; i++) {
        inference_times.push_back(55.0f + (i % 10) * 1.0f);
    }

    // 전체 평균
    float total_avg = std::accumulate(inference_times.begin(), inference_times.end(), 0.0f)
                      / inference_times.size();

    // 워밍업 제외 평균
    float stable_sum = 0.0f;
    for (int i = 10; i < 100; i++) {
        stable_sum += inference_times[i];
    }
    float stable_avg = stable_sum / 90.0f;

    std::cout << "  전체 100프레임 평균 추론 시간: " << total_avg << " ms" << std::endl;
    std::cout << "  워밍업 제외 90프레임 평균:     " << stable_avg << " ms" << std::endl;
    std::cout << std::endl;

    std::cout << "질문 1: 전체 100프레임 기준 FPS는?" << std::endl;
    std::cout << "  FPS = 1000 / " << total_avg << " = " << 1000.0f / total_avg << std::endl;
    std::cout << std::endl;

    std::cout << "질문 2: 워밍업 제외 기준 FPS는?" << std::endl;
    std::cout << "  FPS = 1000 / " << stable_avg << " = " << 1000.0f / stable_avg << std::endl;
    std::cout << std::endl;

    std::cout << "질문 3: 왜 워밍업(warm-up)을 제외하고 측정해야 하는가?" << std::endl;
    std::cout << std::endl;
    std::cout << "보기:" << std::endl;
    std::cout << "  A) GPU 클럭이 아직 최대로 올라가지 않았기 때문" << std::endl;
    std::cout << "  B) CUDA 커널 최초 로딩, 메모리 할당, JIT 컴파일 등" << std::endl;
    std::cout << "     초기화 오버헤드가 포함되어 있기 때문" << std::endl;
    std::cout << "  C) OS가 아직 프로세스 우선순위를 설정하지 않았기 때문" << std::endl;
    std::cout << "  D) 네트워크 통신 지연이 있기 때문" << std::endl;
    std::cout << std::endl;

    // TODO: 정답을 작성하세요
}

void problem4_code_analysis() {
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: TensorRT 추론 코드 버그 찾기" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "아래 TensorRT 추론 코드에서 잘못된 부분을 찾으시오:" << std::endl;
    std::cout << std::endl;
    std::cout << "  // 추론 코드 (버그 있음)" << std::endl;
    std::cout << "  void infer(const cv::Mat& image) {" << std::endl;
    std::cout << "      preprocess(image, input_host.data());" << std::endl;
    std::cout << "      " << std::endl;
    std::cout << "      cudaMemcpyAsync(buffers_[0], input_host.data()," << std::endl;
    std::cout << "                      input_size_ * sizeof(float)," << std::endl;
    std::cout << "                      cudaMemcpyHostToDevice, stream_);" << std::endl;
    std::cout << "      " << std::endl;
    std::cout << "      context_->enqueueV2(buffers_, stream_, nullptr);" << std::endl;
    std::cout << "      " << std::endl;
    std::cout << "      cudaMemcpyAsync(output_host.data(), buffers_[1]," << std::endl;
    std::cout << "                      output_size_ * sizeof(float)," << std::endl;
    std::cout << "                      cudaMemcpyDeviceToHost, stream_);" << std::endl;
    std::cout << "      " << std::endl;
    std::cout << "      // <<<< 여기서 바로 output_host 사용" << std::endl;
    std::cout << "      cv::Mat depth = postprocess(output_host.data());" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;
    std::cout << "보기:" << std::endl;
    std::cout << "  A) preprocess에서 BGR -> RGB 변환을 안 했다" << std::endl;
    std::cout << "  B) cudaMemcpyAsync 후 cudaStreamSynchronize를 호출하지 않아" << std::endl;
    std::cout << "     비동기 복사가 완료되기 전에 output_host를 접근한다" << std::endl;
    std::cout << "  C) buffers_ 인덱스가 잘못되었다 (입출력이 뒤바뀜)" << std::endl;
    std::cout << "  D) sizeof(float) 대신 sizeof(half)를 써야 한다" << std::endl;
    std::cout << std::endl;

    // TODO: 정답을 작성하세요
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Week 9 Quiz - Medium (C++)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_preprocessing_order();
    problem2_memory_calculation();
    problem3_fps_analysis();
    problem4_code_analysis();

    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
