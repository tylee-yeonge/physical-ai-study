/**
 * Phase 5 Week 9 - ONNX & TensorRT 변환 기초 퀴즈
 *
 * TensorRT, ONNX, FP16 최적화에 대한 개념 확인 문제입니다.
 * 주석으로 답을 작성하고, quiz_solutions/easy_sol.cpp와 비교하세요.
 *
 * 컴파일: g++ -std=c++17 -o quiz_easy quiz_easy.cpp
 * 실행: ./quiz_easy
 */

#include <iostream>
#include <string>

void problem1_pipeline_order() {
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1: 모델 변환 파이프라인 순서" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "질문: Depth Anything 모델을 Jetson에서 실시간 추론하기 위한" << std::endl;
    std::cout << "      올바른 변환 순서는?" << std::endl;
    std::cout << std::endl;
    std::cout << "보기:" << std::endl;
    std::cout << "  A) ONNX -> PyTorch -> TensorRT" << std::endl;
    std::cout << "  B) TensorRT -> ONNX -> PyTorch" << std::endl;
    std::cout << "  C) PyTorch -> ONNX -> TensorRT" << std::endl;
    std::cout << "  D) PyTorch -> TensorRT -> ONNX" << std::endl;
    std::cout << std::endl;

    // TODO: 여기에 정답을 작성하세요
    // std::string answer = "?";
}

void problem2_fp16_benefit() {
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2: FP16 변환의 이점" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "질문: TensorRT에서 FP32 -> FP16 변환 시 기대되는 효과가 아닌 것은?" << std::endl;
    std::cout << std::endl;
    std::cout << "보기:" << std::endl;
    std::cout << "  A) 추론 속도 약 2배 향상" << std::endl;
    std::cout << "  B) GPU 메모리 사용량 감소" << std::endl;
    std::cout << "  C) 모델의 학습 정확도 향상" << std::endl;
    std::cout << "  D) Tensor Core 활용 가능" << std::endl;
    std::cout << std::endl;

    // TODO: 여기에 정답을 작성하세요
}

void problem3_input_size() {
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3: 입력 크기 고정의 이유" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "질문: TensorRT 변환 시 입력 크기를 384x512로 고정하는 주된 이유는?" << std::endl;
    std::cout << std::endl;
    std::cout << "보기:" << std::endl;
    std::cout << "  A) 카메라 해상도가 384x512이기 때문" << std::endl;
    std::cout << "  B) 정적 크기에서 커널 튜닝, 메모리 할당 등 최적화가 가능하기 때문" << std::endl;
    std::cout << "  C) 동적 크기를 TensorRT가 지원하지 않기 때문" << std::endl;
    std::cout << "  D) ONNX 변환 시 동적 크기가 불가능하기 때문" << std::endl;
    std::cout << std::endl;

    // TODO: 여기에 정답을 작성하세요
}

void problem4_memory_layout() {
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4: GPU 메모리 계산" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "질문: 입력 텐서 [1, 3, 384, 512]를 FP32로 GPU에 올릴 때" << std::endl;
    std::cout << "      필요한 메모리(바이트)는 얼마인가?" << std::endl;
    std::cout << std::endl;
    std::cout << "  힌트: FP32 = 4바이트/값" << std::endl;
    std::cout << "        총 값의 수 = 1 * 3 * 384 * 512 = ?" << std::endl;
    std::cout << std::endl;
    std::cout << "보기:" << std::endl;
    std::cout << "  A) 약 1.18 MB" << std::endl;
    std::cout << "  B) 약 2.36 MB" << std::endl;
    std::cout << "  C) 약 4.72 MB" << std::endl;
    std::cout << "  D) 약 0.59 MB" << std::endl;
    std::cout << std::endl;

    // TODO: 직접 계산해보세요
    int batch = 1, channels = 3, height = 384, width = 512;
    int total_elements = batch * channels * height * width;
    float memory_bytes = total_elements * 4.0f;  // FP32 = 4 bytes
    float memory_mb = memory_bytes / (1024.0f * 1024.0f);

    std::cout << "  계산 과정:" << std::endl;
    std::cout << "    총 요소 수: " << total_elements << std::endl;
    std::cout << "    메모리: " << memory_mb << " MB" << std::endl;
}

void problem5_trt_optimization() {
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5: TensorRT 최적화 기법" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "질문: TensorRT의 Layer Fusion 최적화에 대한 설명으로 올바른 것은?" << std::endl;
    std::cout << std::endl;
    std::cout << "보기:" << std::endl;
    std::cout << "  A) Conv + BN + ReLU를 하나의 커널로 합쳐 메모리 접근과 커널 실행 횟수를 줄인다" << std::endl;
    std::cout << "  B) 모든 레이어를 하나의 거대한 행렬 연산으로 변환한다" << std::endl;
    std::cout << "  C) 불필요한 레이어를 자동으로 삭제한다" << std::endl;
    std::cout << "  D) 입력 이미지를 여러 개의 작은 패치로 분할하여 병렬 처리한다" << std::endl;
    std::cout << std::endl;

    // TODO: 여기에 정답을 작성하세요
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Week 9 Quiz - Easy (C++)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_pipeline_order();
    problem2_fp16_benefit();
    problem3_input_size();
    problem4_memory_layout();
    problem5_trt_optimization();

    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
