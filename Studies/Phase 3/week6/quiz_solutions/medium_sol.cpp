/**
 * Solutions - Medium Quiz (Week 6: TensorRT 배포)
 *
 * 컴파일: g++ -std=c++17 -o medium_sol medium_sol.cpp
 * 실행: ./medium_sol
 */

#include <iostream>
#include <string>

int main()
{
    std::cout << std::string(37, '=') << std::endl;
    std::cout << "Week 6 Quiz - Medium 정답" << std::endl;
    std::cout << std::string(37, '=') << std::endl;

    std::cout << "\nQ1. Layer Fusion 분석:" << std::endl;
    std::cout << std::endl;
    std::cout << "  원본 (10개 레이어):" << std::endl;
    std::cout << "  Conv3x3 -> BatchNorm -> SiLU ->" << std::endl;
    std::cout << "  Conv1x1 -> BatchNorm ->" << std::endl;
    std::cout << "  Concat ->" << std::endl;
    std::cout << "  Conv3x3 -> BatchNorm -> SiLU ->" << std::endl;
    std::cout << "  MaxPool" << std::endl;
    std::cout << std::endl;
    std::cout << "  Fusion 후 (5개 레이어):" << std::endl;
    std::cout << "  [FusedConvBNSiLU] ->       (Conv3x3+BN+SiLU 융합)" << std::endl;
    std::cout << "  [FusedConvBN] ->            (Conv1x1+BN 융합)" << std::endl;
    std::cout << "  Concat ->                   (융합 불가, 유지)" << std::endl;
    std::cout << "  [FusedConvBNSiLU] ->        (Conv3x3+BN+SiLU 융합)" << std::endl;
    std::cout << "  MaxPool                     (융합 불가, 유지)" << std::endl;
    std::cout << std::endl;
    std::cout << "  결과: 10개 -> 5개 (50% 감소)" << std::endl;
    std::cout << std::endl;
    std::cout << "  메모리 관점에서의 속도 향상 원리:" << std::endl;
    std::cout << "  - Fusion 전: Conv 결과를 GPU 메모리에 쓰고, BN이 다시 읽고," << std::endl;
    std::cout << "    BN 결과를 쓰고, SiLU가 다시 읽음 (6번 메모리 접근)" << std::endl;
    std::cout << "  - Fusion 후: 하나의 커널에서 Conv+BN+SiLU를 연속 계산" << std::endl;
    std::cout << "    중간 결과는 GPU 레지스터/공유메모리에 유지 (1번 메모리 접근)" << std::endl;
    std::cout << "  - GPU는 연산보다 메모리 대역폭이 병목!" << std::endl;
    std::cout << "    -> 메모리 접근 감소 = 큰 속도 향상" << std::endl;

    std::cout << "\n" << std::string(37, '-') << std::endl;

    std::cout << "\nQ2. CUDA 메모리 관리 버그 (4개):" << std::endl;
    std::cout << std::endl;
    std::cout << "  버그 1: buffers[1] GPU 메모리 미할당" << std::endl;
    std::cout << "  수정: cudaMalloc(&buffers[1], output_size); 추가" << std::endl;
    std::cout << std::endl;
    std::cout << "  버그 2: cudaMemcpy 방향 오류 (입력: H->D인데 D->H로 되어있음)" << std::endl;
    std::cout << "  수정: cudaMemcpy(buffers[0], input, size, cudaMemcpyHostToDevice);"
              << std::endl;
    std::cout << "        (CPU에서 GPU로 복사해야 함)" << std::endl;
    std::cout << std::endl;
    std::cout << "  버그 3: cudaMemcpy 방향 오류 (출력: D->H인데 H->D로 되어있음)" << std::endl;
    std::cout << "  수정: cudaMemcpy(output, buffers[1], size, cudaMemcpyDeviceToHost);"
              << std::endl;
    std::cout << "        (GPU에서 CPU로 복사해야 함)" << std::endl;
    std::cout << std::endl;
    std::cout << "  버그 4: 리소스 해제 누락 (context, runtime, buffers[1])" << std::endl;
    std::cout << "  수정 (TRT 10에서는 destroy() 메서드가 제거됐으므로 delete 사용):" << std::endl;
    std::cout << "    cudaFree(buffers[0]);" << std::endl;
    std::cout << "    cudaFree(buffers[1]);    // 추가!" << std::endl;
    std::cout << "    delete context;          // 추가! (구버전 TRT 8.x: context->destroy())" << std::endl;
    std::cout << "    delete engine;           //         (구버전 TRT 8.x: engine->destroy())" << std::endl;
    std::cout << "    delete runtime;          // 추가! (구버전 TRT 8.x: runtime->destroy())" << std::endl;
    std::cout << std::endl;
    std::cout << "  추가 참고: 해제 순서도 중요!" << std::endl;
    std::cout << "    GPU 메모리 해제 -> context -> engine -> runtime 순서" << std::endl;

    std::cout << "\n" << std::string(37, '-') << std::endl;

    std::cout << "\nQ3. 성능 최적화 전략:" << std::endl;
    std::cout << std::endl;
    std::cout << "  1. 가장 큰 병목: TensorRT 추론 (15ms) + 카메라 읽기 (10ms)" << std::endl;
    std::cout << std::endl;
    std::cout << "  2. Multi-threading으로 해결 가능한 부분:" << std::endl;
    std::cout << "     Thread 1: 카메라 읽기 (10ms) -> 병렬화로 대기 시간 제거" << std::endl;
    std::cout << "     Thread 2: 전처리 + 추론 + 후처리 + 시각화" << std::endl;
    std::cout << "     -> 카메라 읽기 10ms가 추론과 겹치므로 실질적 0ms" << std::endl;
    std::cout << std::endl;
    std::cout << "  3. FP32 -> FP16 추론 시간:" << std::endl;
    std::cout << "     FP16은 약 2배 빠름: 15ms -> ~8ms" << std::endl;
    std::cout << std::endl;
    std::cout << "  4. 모든 최적화 적용 후 예상:" << std::endl;
    std::cout << "     ┌──────────────────┬──────────┐" << std::endl;
    std::cout << "     | 단계              | 소요 시간 |" << std::endl;
    std::cout << "     ├──────────────────┼──────────┤" << std::endl;
    std::cout << "     | 카메라 읽기       |   0 ms   | (병렬 스레드)" << std::endl;
    std::cout << "     | 전처리 (CPU)      |   8 ms   |" << std::endl;
    std::cout << "     | cudaMemcpy H->D  |   3 ms   |" << std::endl;
    std::cout << "     | TensorRT 추론    |   8 ms   | (FP16)" << std::endl;
    std::cout << "     | cudaMemcpy D->H  |   2 ms   |" << std::endl;
    std::cout << "     | NMS 후처리       |   2 ms   |" << std::endl;
    std::cout << "     | 시각화           |   5 ms   |" << std::endl;
    std::cout << "     ├──────────────────┼──────────┤" << std::endl;
    std::cout << "     | 총합              |  28 ms   |" << std::endl;
    std::cout << "     | FPS              |  35 FPS  |" << std::endl;
    std::cout << "     └──────────────────┴──────────┘" << std::endl;
    std::cout << "     -> 30 FPS 목표 달성!" << std::endl;
    std::cout << std::endl;
    std::cout << "  추가 최적화 가능:" << std::endl;
    std::cout << "  - CUDA 전처리 (CPU 8ms -> GPU 2ms)" << std::endl;
    std::cout << "  - CUDA 스트림으로 전처리와 추론 파이프라이닝" << std::endl;
    std::cout << "  - 시각화 스레드 분리 (5ms 제거)" << std::endl;

    std::cout << "\n" << std::string(37, '=') << std::endl;

    return 0;
}
