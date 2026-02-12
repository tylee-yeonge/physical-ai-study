/**
 * Phase 5 Week 9 - ONNX & TensorRT 변환 기초 퀴즈 정답 및 해설
 *
 * 컴파일: g++ -std=c++17 -o easy_sol easy_sol.cpp
 * 실행: ./easy_sol
 */

#include <iostream>
#include <string>

void problem1_solution()
{
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 1 정답: C) PyTorch -> ONNX -> TensorRT" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "해설:" << std::endl;
    std::cout << "  [PyTorch .pt] --> [ONNX .onnx] --> [TensorRT .trt]" << std::endl;
    std::cout << "       |               |                |" << std::endl;
    std::cout << "     학습용          중간 포맷         배포용" << std::endl;
    std::cout << "    (Python)      (프레임워크       (GPU 최적화)" << std::endl;
    std::cout << "                   독립적)" << std::endl;
    std::cout << std::endl;
    std::cout << "  - PyTorch: 학습 시 사용하는 프레임워크" << std::endl;
    std::cout << "  - ONNX: Open Neural Network Exchange, 프레임워크 간 호환 포맷" << std::endl;
    std::cout << "  - TensorRT: NVIDIA GPU에 특화된 추론 최적화 엔진" << std::endl;
    std::cout << std::endl;
    std::cout << "  ONNX는 '중간 언어' 역할을 합니다." << std::endl;
    std::cout << "  PyTorch, TensorFlow 등 다양한 프레임워크에서 ONNX로 변환 가능하며,"
              << std::endl;
    std::cout << "  ONNX에서 TensorRT, OpenVINO 등 다양한 추론 엔진으로 변환할 수 있습니다."
              << std::endl;
}

void problem2_solution()
{
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 2 정답: C) 모델의 학습 정확도 향상" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "해설:" << std::endl;
    std::cout << "  FP16 변환의 이점:" << std::endl;
    std::cout << "  A) 속도 ~2배 향상: 데이터 크기 절반 -> 메모리 대역폭 2배 효율" << std::endl;
    std::cout << "  B) 메모리 감소: 4바이트 -> 2바이트 (50% 절약)" << std::endl;
    std::cout << "  D) Tensor Core: NVIDIA GPU의 FP16 전용 연산 유닛 활용" << std::endl;
    std::cout << std::endl;
    std::cout << "  C는 틀렸습니다!" << std::endl;
    std::cout << "  FP16은 '추론 최적화'입니다. 학습 정확도와는 관련 없습니다." << std::endl;
    std::cout << "  오히려 FP16 변환 시 약간의 정밀도 손실(< 1%)이 발생할 수 있습니다."
              << std::endl;
    std::cout << "  하지만 Depth Estimation에서는 이 손실이 무시할 수 있는 수준입니다."
              << std::endl;
    std::cout << std::endl;
    std::cout << "  FP32: [1비트 부호][8비트 지수][23비트 가수]" << std::endl;
    std::cout << "  FP16: [1비트 부호][5비트 지수][10비트 가수]" << std::endl;
}

void problem3_solution()
{
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 3 정답: B) 정적 크기에서 최적화 가능" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "해설:" << std::endl;
    std::cout << "  TensorRT는 정적(고정) 입력 크기에서 다음 최적화를 수행합니다:" << std::endl;
    std::cout << std::endl;
    std::cout << "  1. 커널 자동 튜닝 (Kernel Auto-Tuning)" << std::endl;
    std::cout << "     -> 특정 크기에 최적인 CUDA 커널을 선택" << std::endl;
    std::cout << "  2. 메모리 사전 할당" << std::endl;
    std::cout << "     -> 미리 정확한 크기의 메모리를 할당하여 오버헤드 제거" << std::endl;
    std::cout << "  3. Layer Fusion 최적화" << std::endl;
    std::cout << "     -> 고정 크기에서만 가능한 레이어 병합 최적화 적용" << std::endl;
    std::cout << std::endl;
    std::cout << "  참고:" << std::endl;
    std::cout << "  - TensorRT는 동적 크기도 지원합니다 (C가 틀린 이유)" << std::endl;
    std::cout << "    (--minShapes, --optShapes, --maxShapes 옵션)" << std::endl;
    std::cout << "  - 하지만 고정 크기가 더 빠릅니다" << std::endl;
    std::cout << "  - ViT 패치 크기(14)와의 호환성도 고려해야 합니다" << std::endl;
}

void problem4_solution()
{
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 4 정답: B) 약 2.36 MB" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "해설:" << std::endl;
    std::cout << "  입력 텐서: [1, 3, 384, 512]" << std::endl;
    std::cout << std::endl;

    int total = 1 * 3 * 384 * 512;
    float bytes = total * 4.0f;
    float mb = bytes / (1024.0f * 1024.0f);

    std::cout << "  총 요소 수: 1 x 3 x 384 x 512 = " << total << std::endl;
    std::cout << "  FP32 메모리: " << total << " x 4 bytes = " << static_cast<int>(bytes)
              << " bytes" << std::endl;
    std::cout << "             = " << mb << " MB" << std::endl;
    std::cout << std::endl;
    std::cout << "  참고 - FP16이라면:" << std::endl;
    std::cout << "    " << total << " x 2 bytes = " << mb / 2.0f << " MB" << std::endl;
    std::cout << std::endl;
    std::cout << "  메모리 계산은 TensorRT 추론 시 버퍼 할당에 필수적인 지식입니다." << std::endl;
    std::cout << "  입력 + 출력 + 중간 텐서의 메모리를 모두 합산해야 합니다." << std::endl;
}

void problem5_solution()
{
    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "문제 5 정답: A) Conv + BN + ReLU를 하나의 커널로 합침" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << std::endl;

    std::cout << "해설:" << std::endl;
    std::cout << "  Layer Fusion은 연속된 레이어들을 하나의 CUDA 커널로 합치는 기법입니다."
              << std::endl;
    std::cout << std::endl;
    std::cout << "  Before fusion (3번의 커널 실행 + 2번의 메모리 읽기/쓰기):" << std::endl;
    std::cout << "    GPU Mem -> Conv -> GPU Mem -> BN -> GPU Mem -> ReLU -> GPU Mem" << std::endl;
    std::cout << std::endl;
    std::cout << "  After fusion (1번의 커널 실행):" << std::endl;
    std::cout << "    GPU Mem -> [Conv+BN+ReLU] -> GPU Mem" << std::endl;
    std::cout << std::endl;
    std::cout << "  효과:" << std::endl;
    std::cout << "  - 커널 실행 횟수 감소 (커널 시작 오버헤드 제거)" << std::endl;
    std::cout << "  - 중간 텐서의 GPU 메모리 읽기/쓰기 제거" << std::endl;
    std::cout << "  - GPU 메모리 대역폭 절약" << std::endl;
    std::cout << std::endl;
    std::cout << "  TensorRT가 자동으로 수행하는 다른 최적화:" << std::endl;
    std::cout << "  - Kernel Auto-Tuning: GPU에 맞는 최적 커널 선택" << std::endl;
    std::cout << "  - Precision Calibration: FP32 -> FP16/INT8 변환" << std::endl;
    std::cout << "  - Memory Optimization: 텐서 재사용, 메모리 풀링" << std::endl;
}

int main()
{
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "  Week 9 Quiz - Easy 정답 (C++)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    problem1_solution();
    problem2_solution();
    problem3_solution();
    problem4_solution();
    problem5_solution();

    std::cout << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
