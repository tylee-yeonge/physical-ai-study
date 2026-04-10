/**
 * Quiz Medium - Week 6: TensorRT 배포 (Section 5.2)
 * 3문제 - 심화 이해
 *
 * 컴파일: g++ -std=c++17 -o quiz_medium quiz_medium.cpp
 * 실행: ./quiz_medium
 */

#include <iostream>
#include <string>

void problem1_layer_fusion_analysis()
{
    std::cout << "\n" << std::string(28, '-') << std::endl;
    std::cout << "문제 1: Layer Fusion 분석" << std::endl;
    std::cout << std::string(28, '-') << "\n" << std::endl;

    std::cout << "다음 YOLO 네트워크 구조에서 TensorRT가 적용할 수 있는" << std::endl;
    std::cout << "Layer Fusion을 모두 찾고, Fusion 후 레이어 수를 계산하세요." << std::endl;
    std::cout << std::endl;
    std::cout << "  원본 레이어 (10개):" << std::endl;
    std::cout << "  Conv3x3 -> BatchNorm -> SiLU ->" << std::endl;
    std::cout << "  Conv1x1 -> BatchNorm ->" << std::endl;
    std::cout << "  Concat ->" << std::endl;
    std::cout << "  Conv3x3 -> BatchNorm -> SiLU ->" << std::endl;
    std::cout << "  MaxPool" << std::endl;
    std::cout << std::endl;
    std::cout << "  질문:" << std::endl;
    std::cout << "  1. 어떤 레이어들이 융합될 수 있나요?" << std::endl;
    std::cout << "  2. Fusion 후 총 레이어 수는?" << std::endl;
    std::cout << "  3. Fusion이 속도를 높이는 원리를 메모리 관점에서 설명하세요." << std::endl;
    std::cout << std::endl;
}

void problem2_cuda_memory_bug()
{
    std::cout << "\n" << std::string(28, '-') << std::endl;
    std::cout << "문제 2: CUDA 메모리 관리 버그 찾기" << std::endl;
    std::cout << std::string(28, '-') << "\n" << std::endl;

    std::cout << "다음 C++ TensorRT 코드에서 버그를 3개 이상 찾고 수정하세요." << std::endl;
    std::cout << std::endl;
    std::cout << "  void runInference(const char* engine_file) {" << std::endl;
    std::cout << "      // 1. 엔진 로드" << std::endl;
    std::cout << "      IRuntime* runtime = createInferRuntime(logger);" << std::endl;
    std::cout << "      ICudaEngine* engine = runtime->deserializeCudaEngine(data, size);"
              << std::endl;
    std::cout << "      IExecutionContext* context = engine->createExecutionContext();"
              << std::endl;
    std::cout << std::endl;
    std::cout << "      // 2. GPU 메모리 할당" << std::endl;
    std::cout << "      void* buffers[2];" << std::endl;
    std::cout << "      cudaMalloc(&buffers[0], input_size);" << std::endl;
    std::cout << "      // buffers[1]은 할당하지 않음" << std::endl;
    std::cout << std::endl;
    std::cout << "      // 3. 추론 루프" << std::endl;
    std::cout << "      while (running) {" << std::endl;
    std::cout << "          cudaMemcpy(buffers[0], input, size, cudaMemcpyDeviceToHost);"
              << std::endl;
    std::cout << "          context->executeV2(buffers);" << std::endl;
    std::cout << "          cudaMemcpy(output, buffers[1], size, cudaMemcpyHostToDevice);"
              << std::endl;
    std::cout << "      }" << std::endl;
    std::cout << std::endl;
    std::cout << "      // 4. 정리" << std::endl;
    std::cout << "      cudaFree(buffers[0]);" << std::endl;
    std::cout << "      engine->destroy();" << std::endl;
    std::cout << "      // runtime, context는 정리하지 않음" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;
    std::cout << "  버그와 수정 방법: _____" << std::endl;
    std::cout << std::endl;
}

void problem3_performance_optimization()
{
    std::cout << "\n" << std::string(28, '-') << std::endl;
    std::cout << "문제 3: 성능 최적화 전략" << std::endl;
    std::cout << std::string(28, '-') << "\n" << std::endl;

    std::cout << "Jetson Orin Nano에서 다음과 같은 성능 결과를 얻었습니다." << std::endl;
    std::cout << "30 FPS 목표를 달성하기 위한 최적화 방안을 제시하세요." << std::endl;
    std::cout << std::endl;
    std::cout << "  현재 성능:" << std::endl;
    std::cout << "  ┌──────────────────┬──────────┐" << std::endl;
    std::cout << "  │ 단계              │ 소요 시간 │" << std::endl;
    std::cout << "  ├──────────────────┼──────────┤" << std::endl;
    std::cout << "  │ 카메라 읽기       │  10 ms   │" << std::endl;
    std::cout << "  │ 전처리 (CPU)      │   8 ms   │" << std::endl;
    std::cout << "  │ cudaMemcpy H→D   │   3 ms   │" << std::endl;
    std::cout << "  │ TensorRT 추론    │  15 ms   │" << std::endl;
    std::cout << "  │ cudaMemcpy D→H   │   2 ms   │" << std::endl;
    std::cout << "  │ NMS 후처리       │   2 ms   │" << std::endl;
    std::cout << "  │ 시각화           │   5 ms   │" << std::endl;
    std::cout << "  ├──────────────────┼──────────┤" << std::endl;
    std::cout << "  │ 총합              │  45 ms   │" << std::endl;
    std::cout << "  │ FPS              │  22 FPS  │" << std::endl;
    std::cout << "  └──────────────────┴──────────┘" << std::endl;
    std::cout << std::endl;
    std::cout << "  질문:" << std::endl;
    std::cout << "  1. 가장 큰 병목 구간은 어디인가요?" << std::endl;
    std::cout << "  2. Multi-threading으로 해결할 수 있는 부분은?" << std::endl;
    std::cout << "  3. FP32 → FP16 변환 시 추론 시간은 얼마로 줄어드나요?" << std::endl;
    std::cout << "  4. 위 최적화를 모두 적용하면 예상 FPS는?" << std::endl;
    std::cout << std::endl;
}

int main()
{
    std::cout << std::string(37, '=') << std::endl;
    std::cout << "Week 6 Quiz - Medium (TensorRT 배포)" << std::endl;
    std::cout << std::string(37, '=') << std::endl;

    problem1_layer_fusion_analysis();
    problem2_cuda_memory_bug();
    problem3_performance_optimization();

    std::cout << std::string(37, '=') << std::endl;
    std::cout << "정답은 quiz_solutions/medium_sol.cpp 참고" << std::endl;
    std::cout << std::string(37, '=') << std::endl;

    return 0;
}
