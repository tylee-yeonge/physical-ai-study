/**
 * Solutions - Easy Quiz (Week 6: TensorRT 배포)
 *
 * 컴파일: g++ -std=c++17 -o easy_sol easy_sol.cpp
 * 실행: ./easy_sol
 */

#include <iostream>
#include <string>

int main() {
    std::cout << std::string(35, '=') << std::endl;
    std::cout << "Week 6 Quiz - Easy 정답" << std::endl;
    std::cout << std::string(35, '=') << std::endl;

    std::cout << "\nQ1. TensorRT 3가지 핵심 최적화 기법:" << std::endl;
    std::cout << std::endl;
    std::cout << "  1. Layer Fusion (레이어 합치기):" << std::endl;
    std::cout << "     Conv + BatchNorm + ReLU 등 여러 레이어를 하나로 합침" << std::endl;
    std::cout << "     -> GPU 커널 호출 횟수 감소" << std::endl;
    std::cout << "     -> 중간 텐서의 메모리 읽기/쓰기 제거" << std::endl;
    std::cout << std::endl;
    std::cout << "  2. Kernel Auto-Tuning (커널 자동 최적화):" << std::endl;
    std::cout << "     각 레이어에 대해 수백 개의 커널 후보를 실행" << std::endl;
    std::cout << "     현재 GPU 아키텍처에서 가장 빠른 커널을 자동 선택" << std::endl;
    std::cout << "     -> 빌드 시간이 오래 걸리지만 추론 시 최적 성능" << std::endl;
    std::cout << std::endl;
    std::cout << "  3. Precision Calibration (정밀도 조정):" << std::endl;
    std::cout << "     FP32 -> FP16 -> INT8로 정밀도를 낮춤" << std::endl;
    std::cout << "     -> 메모리 절약 + 연산 속도 향상" << std::endl;
    std::cout << "     -> FP16은 정확도 손실 거의 없음 (< 1%)" << std::endl;

    std::cout << "\nQ2. 정답: b) --saveEngine=yolov8n_fp16.trt --fp16" << std::endl;
    std::cout << "    해설:" << std::endl;
    std::cout << "    trtexec --onnx=yolov8n.onnx \\" << std::endl;
    std::cout << "            --saveEngine=yolov8n_fp16.trt \\" << std::endl;
    std::cout << "            --fp16" << std::endl;
    std::cout << "    -> --fp16 플래그로 FP16 정밀도 활성화" << std::endl;
    std::cout << "    -> --half, --float16은 존재하지 않는 옵션" << std::endl;

    std::cout << "\nQ3. 정답: b, c, d" << std::endl;
    std::cout << "    해설:" << std::endl;
    std::cout << "    a) 오류: Jetson .trt는 RTX에서 실행 불가" << std::endl;
    std::cout << "       (GPU 아키텍처가 다르기 때문)" << std::endl;
    std::cout << "    b) 맞음: TensorRT 엔진은 GPU 아키텍처별 빌드 필요" << std::endl;
    std::cout << "    c) 맞음: ONNX는 프레임워크/하드웨어 독립적 포맷" << std::endl;
    std::cout << "    d) 맞음: TensorRT 버전 호환성 문제 존재" << std::endl;

    std::cout << "\nQ4. 정답: b) 검출 수가 늘어난다 (덜 제거)" << std::endl;
    std::cout << "    해설:" << std::endl;
    std::cout << "    IoU threshold 0.45 -> 0.7:" << std::endl;
    std::cout << "    -> IoU > 0.7인 박스만 '같은 객체'로 간주하여 제거" << std::endl;
    std::cout << "    -> IoU 0.5~0.7인 박스는 '다른 객체'로 유지" << std::endl;
    std::cout << "    -> 결과: 더 많은 박스가 남음 (검출 수 증가)" << std::endl;
    std::cout << "    -> 주의: 같은 객체에 여러 박스가 남을 수 있음" << std::endl;

    std::cout << "\n" << std::string(35, '=') << std::endl;

    return 0;
}
