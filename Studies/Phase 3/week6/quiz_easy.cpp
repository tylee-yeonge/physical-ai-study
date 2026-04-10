/**
 * Quiz Easy - Week 6: TensorRT 배포 (Section 5.2)
 * 4문제 - 기본 개념 확인
 *
 * 컴파일: g++ -std=c++17 -o quiz_easy quiz_easy.cpp
 * 실행: ./quiz_easy
 */

#include <iostream>
#include <string>

void problem1_tensorrt_optimization()
{
    std::cout << "\n" << std::string(28, '-') << std::endl;
    std::cout << "문제 1: TensorRT 최적화 기법" << std::endl;
    std::cout << std::string(28, '-') << "\n" << std::endl;

    std::cout << "TensorRT의 3가지 핵심 최적화 기법을 설명하세요." << std::endl;
    std::cout << std::endl;
    std::cout << "  1. Layer Fusion이란? ___________" << std::endl;
    std::cout << "  2. Kernel Auto-Tuning이란? ___________" << std::endl;
    std::cout << "  3. Precision Calibration이란? ___________" << std::endl;
    std::cout << std::endl;
}

void problem2_trtexec_command()
{
    std::cout << "\n" << std::string(28, '-') << std::endl;
    std::cout << "문제 2: trtexec 변환 명령어" << std::endl;
    std::cout << std::string(28, '-') << "\n" << std::endl;

    std::cout << "ONNX 모델을 FP16 TensorRT 엔진으로 변환하는" << std::endl;
    std::cout << "trtexec 명령어를 완성하세요." << std::endl;
    std::cout << std::endl;
    std::cout << "  trtexec --onnx=yolov8n.onnx \\" << std::endl;
    std::cout << "          --saveEngine=_________ \\" << std::endl;
    std::cout << "          --_________" << std::endl;
    std::cout << std::endl;
    std::cout << "  a) --saveEngine=yolov8n.trt --int8" << std::endl;
    std::cout << "  b) --saveEngine=yolov8n_fp16.trt --fp16" << std::endl;
    std::cout << "  c) --saveEngine=yolov8n.engine --half" << std::endl;
    std::cout << "  d) --saveEngine=yolov8n_fp16.trt --float16" << std::endl;
    std::cout << std::endl;
    std::cout << "  답: _____" << std::endl;
    std::cout << std::endl;
}

void problem3_engine_portability()
{
    std::cout << "\n" << std::string(28, '-') << std::endl;
    std::cout << "문제 3: 엔진 이식성" << std::endl;
    std::cout << std::string(28, '-') << "\n" << std::endl;

    std::cout << "다음 중 올바른 설명을 모두 고르세요." << std::endl;
    std::cout << std::endl;
    std::cout << "  a) Jetson에서 빌드한 .trt 파일은 RTX 4090에서도 실행 가능하다" << std::endl;
    std::cout << "  b) TensorRT 엔진은 GPU 아키텍처별로 다시 빌드해야 한다" << std::endl;
    std::cout << "  c) .onnx 파일은 어떤 GPU에서든 동일하게 사용 가능하다" << std::endl;
    std::cout << "  d) TensorRT 버전이 다르면 같은 GPU라도 엔진이 호환되지 않을 수 있다"
              << std::endl;
    std::cout << std::endl;
    std::cout << "  답: _____" << std::endl;
    std::cout << std::endl;
}

void problem4_nms_concept()
{
    std::cout << "\n" << std::string(28, '-') << std::endl;
    std::cout << "문제 4: NMS (Non-Maximum Suppression)" << std::endl;
    std::cout << std::string(28, '-') << "\n" << std::endl;

    std::cout << "NMS에서 IoU threshold를 0.45에서 0.7로 높이면" << std::endl;
    std::cout << "어떤 변화가 발생하나요?" << std::endl;
    std::cout << std::endl;
    std::cout << "  a) 검출 수가 줄어든다 (더 많이 제거)" << std::endl;
    std::cout << "  b) 검출 수가 늘어난다 (덜 제거)" << std::endl;
    std::cout << "  c) 변화 없다" << std::endl;
    std::cout << "  d) 정확도만 올라간다" << std::endl;
    std::cout << std::endl;
    std::cout << "  답: _____" << std::endl;
    std::cout << std::endl;
}

int main()
{
    std::cout << std::string(35, '=') << std::endl;
    std::cout << "Week 6 Quiz - Easy (TensorRT 배포)" << std::endl;
    std::cout << std::string(35, '=') << std::endl;

    problem1_tensorrt_optimization();
    problem2_trtexec_command();
    problem3_engine_portability();
    problem4_nms_concept();

    std::cout << std::string(35, '=') << std::endl;
    std::cout << "정답은 quiz_solutions/easy_sol.cpp 참고" << std::endl;
    std::cout << std::string(35, '=') << std::endl;

    return 0;
}
