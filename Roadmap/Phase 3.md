# Phase 3: Detection + Depth → PC TensorRT + ROS2 노드


> **기간**: 약 2개월 (2026.06-08)
> **목표**: Detection + Depth 의 핵심 + PC TensorRT latency 측정을 갖춘 독립 완성품 #1 (Foundation Model latency 감각의 토대이기도 함)
> **언어**: **Python** (학습) + **C++/TensorRT** (PC 배포) + **ROS2** (노드 래퍼)
> **하드웨어**: Ubuntu PC (RTX 4070) — 주 학습/실험/배포 장비 / Jetson Orin Nano — **Phase 7 이후 옵션 #5**
> **주간 시간**: 약 6-8시간 (출장 주 보정)


---


## -> **실습 가이드**: [`Studies/Phase 3/PRACTICE.md`](../Studies/Phase%203/PRACTICE.md)


**핵심 산출물 (#1)**:
- YOLO11 실시간 객체 검출 (PC TensorRT)
- Depth Anything V2 (PC TensorRT)
- 통합 시스템: Detection + Depth → 3D 위치 추정 + ROS2 노드 래퍼


**산출물 #1**: YOLO11 + Depth Anything V2 → **PC TensorRT 추론 + ROS2 노드 래퍼 + latency 측정(ms) + 분석 글 + 1분 데모 영상** (2026.08까지 `physical-ai-study` 레포에 공개. #4 없이도 단독으로 면접에 들고 갈 수 있는 독립 완성품. Jetson 실기 배포는 Phase 7 이후 옵션 #5)


> **언어 전략**: 학습은 Python, 배포는 C++/TensorRT (PC), 통합은 ROS2.


> **포트폴리오 통합 원칙**: `physical-ai-study` 단일 레포에 *Studies/* (학습 노트) 와 *Portfolio/* (산출물 디렉토리) 를 디렉토리 수준에서 분리. 채용 담당자 진입점은 README → 산출물 디렉토리.


---


## 학습 환경


| 단계 | 주 장비 | 출장지 가능 여부 |
|---|---|---|
| PyTorch 복습 / YOLO 학습 | Ubuntu PC (원격) | O |
| PC TensorRT 변환 + C++ 추론 | Ubuntu PC (원격) | O |
| Depth Anything V2 실습 + PC TensorRT | Ubuntu PC (원격) | O |
| 통합 시스템 + ROS2 노드 래퍼 | Ubuntu PC (원격) | O |
| Jetson 실기 배포 / 실측 FPS | Jetson (Phase 7 이후 옵션 #5) | 보류 |


- 데이터셋 (COCO, KITTI, Middlebury): Ubuntu PC 디스크 상시 보관
- 상세: [ENVIRONMENT.md](../ENVIRONMENT.md)


> Phase 3 에서는 **PC TensorRT + ROS2 노드까지만**. Jetson 실기 배포는 전체 학습 (Phase 2-7) 완료 후 옵션 (#5).


---


## Section 5.0: 시작 전 (0주, 2026.05 말~6월 초)


> Phase 3 진입 전에 반드시 완료. 학습이 채용 라인에서 이탈하지 않게 하는 출발점.


- [ ] 1순위 3개사 JD 정독 — **VLA 모델 직접 개발 코스닥 상장사 / 대기업 SW 자회사 (VLA 자율주행) / 신생 휴머노이드 스타트업**
- [ ] 2순위 3개사 JD 정독 — 대기업 자율주행 SW 자회사 (CV/ML) / ADAS 양산 SW 중견기업 / Dynamixel 제조사 + 휴머노이드 양산 상장사 (모터 직접 매칭)
- [ ] 공통 요구 스택 추출 → 학습 우선순위 매핑 표 작성
- [ ] **Phase 3 산출물 #1 스펙 1페이지 확정** (타깃 모델 / 데이터 / 수치 목표 / 데모 형태, Jetson 제외)
- [x] `physical-ai-study` 레포 Public 전환 완료 (별도 Portfolio repo 사용 안 함)
- [ ] **TensorRT C++ Quick Start 1회 따라하기** — Section 5.2 Week 6 (PC TensorRT 배포) 학습 곡선 단축용 사전 워밍업
  - 공식 가이드: https://docs.nvidia.com/deeplearning/tensorrt/latest/getting-started/quick-start-guide.html
  - 샘플 코드 (Semantic Segmentation, C++): https://github.com/NVIDIA/TensorRT/tree/main/quickstart/SemanticSegmentation
  - 사전 환경: CUDA + TensorRT (Ubuntu PC 에서 진행 — Jetson 불필요)
  - 예상 시간: 2-3 시간 (빌드/실행 + 코드 1회 정독)
  - 완료 기준: `trtexec` 로 ONNX → engine 변환 1회 + C++ 샘플로 추론 1회 성공


---


## Section 5.1: PyTorch 복습 (1-2주, 압축 옵션)


> 이미 PyTorch에 익숙하면 Week 1+2를 1주로 압축. 절약된 1주를 Week 12 GitHub 공개·블로그 정리에 투입.


### Week 1: PyTorch 기초 재정비


#### 딥러닝 환경 세팅
- [ ] **PC (학습 + PC TensorRT 배포)**
  - [ ] CUDA 11.8+ 설치
  - [ ] PyTorch 2.0+ 설치 (GPU 버전)
  - [ ] TensorRT 8.6+ 설치 (`trtexec` 동작 확인)
  - [ ] conda 가상환경 구성
  - [ ] Jupyter Lab 세팅


> Jetson Orin Nano 배포는 **Phase 7 이후 옵션 (#5)** 으로 분리. Phase 3 에서는 PC TensorRT 만 진행.


#### PyTorch 핵심 복습
- [ ] Tensor 연산
- [ ] autograd (자동 미분)
- [ ] Dataset & DataLoader
- [ ] 간단한 CNN 학습 (CIFAR-10)


#### 실습
- [ ] ResNet-18로 이미지 분류
- [ ] 학습 과정 시각화 (TensorBoard)
- [ ] Checkpoint 저장/로드
- [ ] GPU 메모리 관리


### Week 2: 컴퓨터 비전용 라이브러리


#### Albumentations (Data Augmentation)
- [ ] 설치: `pip install albumentations`
- [ ] 기본 변환: Resize, Normalize, Flip
- [ ] Detection용 변환: BBox 변환 포함
- [ ] Compose 파이프라인 구성


#### Weights & Biases (실험 관리)
- [ ] 가입 및 설치
- [ ] 기본 로깅: loss, accuracy
- [ ] 이미지 로깅
- [ ] Sweep (하이퍼파라미터 탐색)


#### torchvision vs timm
- [ ] torchvision: 기본 모델, Detection API
- [ ] timm: 최신 모델 (EfficientNet, ViT 등)
- [ ] Pretrained weights 사용법


### Section 5.1 자체 점검
1. PyTorch Dataset과 DataLoader의 역할 차이는?
2. GPU 메모리 부족 시 해결 방법 3가지는?
3. Albumentations의 장점은?


---


## Section 5.2: Object Detection (4주)


### Week 3: YOLO 이론


#### YOLO 발전사
- [ ] YOLOv1-v3: Anchor 기반
- [ ] YOLOv5-v8: Anchor-free로 전환, Ultralytics 생태계
- [ ] **YOLO11** (2024): Ultralytics 최신, C3k2 블록, 경량화
- [ ] YOLO26 (2025): 최신이지만 커뮤니티 평가 미흡 (참고)


#### YOLO11 핵심 개념
- [ ] **Backbone**: C3k2 블록 (경량화된 CSP)
- [ ] **Neck**: PANet (Multi-scale fusion)
- [ ] **Head**: Decoupled head (Classification + Localization)
- [ ] **Loss**: CIoU loss + BCE loss
- [ ] nano(n) ~ xlarge(x) 변형, 멀티태스크 지원


#### Detection 지표
- [ ] Precision, Recall
- [ ] IoU (Intersection over Union)
- [ ] **mAP** (mean Average Precision)
  - mAP@0.5
  - mAP@0.5:0.95 (COCO metric)
- [ ] FPS (Frames Per Second)


### Week 4: YOLO11 학습 (Python)


#### Ultralytics 사용법
```bash
pip install ultralytics
```


#### 커스텀 데이터셋 준비
- [ ] 라벨링 도구: Roboflow, LabelImg
- [ ] YOLO 포맷:
  ```
  <class_id> <x_center> <y_center> <width> <height>
  ```
  (모두 정규화 0-1)
- [ ] 데이터셋 구조:
  ```
  dataset/
  +-- images/
  | +-- train/
  | +-- val/
  +-- labels/
      +-- train/
      +-- val/
  ```


#### 학습 실습
- [ ] COCO128 데이터셋으로 빠른 실험
- [ ] 커스텀 데이터 학습 (AMR 관련 객체)
- [ ] Hyperparameter tuning (lr, batch_size)
- [ ] Augmentation 전략


#### 평가
- [ ] mAP 측정
- [ ] Confusion matrix 분석
- [ ] 오검출 (False Positive) 패턴 파악


### Week 5: ONNX 변환 및 최적화


#### ONNX 개요
- [ ] 왜 ONNX? (프레임워크 독립적)
- [ ] PyTorch → ONNX 변환 과정
- [ ] ONNX Runtime 추론


#### 변환 실습
- [ ] YOLO11 → ONNX
  ```python
  model.export(format='onnx', simplify=True)
  ```
- [ ] Dynamic shape vs Static shape
- [ ] Opset version 선택


#### ONNX Runtime 검증
- [ ] Python으로 추론
- [ ] 속도 비교: PyTorch vs ONNX
- [ ] Quantization (선택)
  - FP32 → FP16 → INT8


### Week 6: PC TensorRT 배포


#### TensorRT 이론
- [ ] Layer fusion (레이어 합치기)
- [ ] Kernel auto-tuning
- [ ] FP16, INT8 precision
- [ ] **속도 향상**: 2-5배


#### ONNX → TensorRT 변환 (PC)
```bash
trtexec --onnx=yolo11n.onnx \
        --saveEngine=yolo11n.trt \
        --fp16 \
        --workspace=4096
```


#### C++ Inference 구현 (PC)
- [ ] TensorRT API 사용
- [ ] 전처리 (OpenCV)
- [ ] NMS (Non-Maximum Suppression) 구현
- [ ] 후처리 및 시각화


#### 성능 측정 (PC, RTX 4070 기준)
- [ ] Latency 분석 (전처리/추론/후처리) — #1 의 측정 증거 + Foundation Model latency 감각
- [ ] Multi-threading (카메라 읽기 병렬화)


> Jetson 실기 배포 (30+ FPS 목표) 는 **Phase 7 이후 옵션 (#5)** 으로 분리.


### Section 5.2 자체 점검
1. mAP@0.5와 mAP@0.5:0.95의 차이는?
2. TensorRT가 빠른 이유 3가지는?
3. Jetson에서 FP16을 쓰는 이유는?


---


## Section 5.3: Depth Estimation (4주)


### Week 7: Monocular Depth 이론


#### 왜 Depth Estimation인가?
- [ ] 스테레오 없이 단안 카메라로 Depth 추정
- [ ] AMR: 장애물까지 거리 파악
- [ ] 3D Perception의 기초


#### 방법론
- [ ] **Supervised**: Depth GT 필요 (KITTI, NYU Depth)
- [ ] **Self-supervised**: Stereo 또는 Video로 학습
- [ ] **Zero-shot**: 사전학습만으로 일반화 (Depth Anything)


#### 핵심 모델
- [ ] MiDaS (2020): Inverse depth 예측
- [ ] DPT (2021): Vision Transformer 기반
- [ ] Depth Anything (2024): 대규모 데이터 학습, 일반화 성능 우수
- [ ] **Depth Anything V2** (2024): 합성 데이터 활용, V1 대비 정확도/속도 향상


### Week 8: Depth Anything V2 사용


#### 모델 다운로드
```python
from transformers import pipeline


pipe = pipeline(task="depth-estimation",
               model="depth-anything/Depth-Anything-V2-Small-hf")
```


#### Inference
- [ ] 단일 이미지 Depth 추정
- [ ] Depth map 시각화 (colormap)
- [ ] Metric depth vs Relative depth 이해


#### Fine-tuning (선택)
- [ ] KITTI Depth 데이터셋
- [ ] Custom 데이터 (stereo 카메라로 GT 생성)
- [ ] Loss: Scale-invariant log loss


### Week 9: ONNX & PC TensorRT 변환 (Depth Anything V2)


#### Depth Anything V2 → ONNX
- [ ] Hugging Face 모델 변환
- [ ] Input size 고정 (384×512 권장)
- [ ] 출력 shape 확인


#### PC TensorRT 최적화
- [ ] FP16 변환
- [ ] 메모리 사용량 체크
- [ ] PC (RTX 4070) 추론 속도 + Latency 분포 측정 (Foundation Model 사전 학습)


### Week 10: Depth 정확도 검증


#### Ground Truth 생성
- [ ] Phase 2 스테레오 카메라 활용
- [ ] Disparity → Depth 계산
- [ ] Depth Anything V2 결과와 비교


#### 정량 평가
- [ ] Absolute Relative Error (AbsRel)
- [ ] RMSE (Root Mean Square Error)
- [ ] δ < 1.25 (Threshold accuracy)


#### 정성 평가
- [ ] 실제 환경에서 Depth map 시각화
- [ ] 경계선, 먼 물체, 반사 표면 등 취약점 파악


### Section 5.3 자체 점검
1. Monocular Depth가 Stereo보다 부정확한 이유는?
2. Relative depth와 Metric depth의 차이는?
3. Depth Anything의 장점은?


---


## Section 5.4: 통합 시스템 (2주)


### Week 11: Detection + Depth 융합


#### 3D 위치 추정 파이프라인
```
1. YOLO Detection → 2D bbox
2. Depth Estimation → Depth map
3. 역투영 (Unprojection) → 3D 좌표
```


#### 구현
- [ ] Detection bbox 중심 픽셀 추출
- [ ] Depth map에서 해당 픽셀의 depth 값
- [ ] 내부 파라미터 K로 3D 변환:
  ```python
  X = (u - cx) / fx * depth
  Y = (v - cy) / fy * depth
  Z = depth
  ```


#### ROS2 노드 래퍼 (필수)
- [ ] Detection 결과 publish (`vision_msgs/Detection3DArray`)
- [ ] Depth map publish (`sensor_msgs/Image`)
- [ ] 통합 결과 publish + TF로 좌표계 변환
- [ ] `ros2 run` 으로 실행 가능한 패키지 형태


### Week 12: 최종 데모 및 최적화


#### 통합 시스템 최적화
- [ ] YOLO + Depth 병렬 처리
- [ ] PC TensorRT 추론 안정화


#### 최종 성능 목표 (PC, RTX 4070)
- [ ] Latency 측정 (전체 파이프라인 ms 단위) — Foundation Model latency 사전 학습
- [ ] 정확도: Detection mAP > 0.6, Depth AbsRel < 0.15


#### 데모 영상 제작
- [ ] 실시간 Detection + Depth 시각화 (Rerun.io)
- [ ] 3D 좌표 표시
- [ ] 다양한 실내/실외 환경


#### 산출물 #1 공개 (2026.08까지)
- [ ] `physical-ai-study` 레포에 산출물 #1 디렉토리 추가 (Studies/ 와 별도 Portfolio 구조 분리)
- [ ] **README**: 환경 세팅 + 실행 방법 + 수치 성능 표 (mAP, AbsRel, Latency)
- [ ] **1분 데모 영상** (Detection + Depth + 3D 위치 + ROS2 노드 시연)
- [ ] **기술 블로그 1개** (YOLO/Depth 학습 + PC TensorRT + ROS2 노드 정리)


### Section 5.4 자체 점검
1. Depth map과 Detection bbox를 어떻게 융합하는가?
2. 실시간성을 위한 최적화 기법 3가지는?
3. AMR에서 이 시스템을 어떻게 활용할 수 있는가?


---


## Phase 3 완료 체크리스트


### Object Detection
- [ ] YOLO11 커스텀 데이터 학습
- [ ] mAP > 0.6 달성
- [ ] ONNX 변환 및 검증
- [ ] **PC TensorRT 변환 + C++ 추론** (Jetson 30+ FPS 는 Phase 7 이후 옵션 #5)


### Depth Estimation
- [ ] Depth Anything V2 사용법 이해
- [ ] ONNX & PC TensorRT 변환
- [ ] PC 추론 속도 + Latency 분포 측정
- [ ] 정확도 검증


### 통합 시스템 + ROS2
- [ ] Detection + Depth 융합 구현
- [ ] 3D 위치 추정 동작
- [ ] **ROS2 노드 래퍼** (필수) — `vision_msgs` publish + TF
- [ ] Demo 영상 제작 (1분)


### 산출물 #1 공개
- [ ] `physical-ai-study` 레포 산출물 #1 디렉토리 정리 (README, 코드)
- [ ] 블로그 포스팅 1-2개
- [ ] Demo 영상 1분


---


## Phase 3 완료 기준


> "PC TensorRT 위에서 Detection + Depth → 3D 위치 추정 + ROS2 노드 래퍼를 구동하고, 이를 1분 데모 영상으로 증명. Jetson 30+ FPS 는 Phase 7 이후 옵션 (#5)."


---


## 참고 자료


### 모델 & 라이브러리


| 이름 | 용도 | 링크 |
|------|------|------|
| Ultralytics YOLO11 | 객체 검출 | https://github.com/ultralytics/ultralytics |
| Depth Anything V2 | Depth 추정 | https://github.com/DepthAnything/Depth-Anything-V2 |
| TensorRT | Jetson 최적화 | https://developer.nvidia.com/tensorrt |


### 데이터셋


| 데이터셋 | 용도 |
|---------|------|
| COCO | Detection 학습/평가 |
| KITTI Depth | Depth 학습/검증 |
| NYU Depth V2 | 실내 Depth |


### 강의


| 강의 | 용도 |
|------|------|
| PyTorch Tutorials | 기초 |
| Ultralytics Docs | YOLOv8 |
| NVIDIA TensorRT Docs | 배포 최적화 |


---


## 팁


1. **작은 모델부터**: YOLO11n (nano)부터 시작, 성능 필요하면 확장
2. **데이터가 80%**: 라벨링 품질이 성능 차이의 대부분
3. **Jetson 메모리 주의**: Swap 설정, 모델 크기 조절
4. **TensorRT 디버깅**: Verbose 모드로 최적화 과정 관찰
5. **Depth는 상대적**: Metric depth 필요 시 스케일 조정 필수


---


## [?] 다음 단계


Phase 3 완료 후 (2026.08):
- **하드웨어 스파이크 (2026.08-09, 2-3주)** — 2-DOF Dynamixel + ROS2 파이프라인 리스크 검증 (산출물 아님)
- **Phase 4: VLA 논문 reading + OpenVLA → ROS2 minimal demo** (2026.09-12, 메인 단독 트랙)
- 산출물 #2 목표: RT-2 + OpenVLA 블로그 2편 + ROS2 토픽 demo (2026.12)
- 병행: **시장 신호 probe** (JD 정독 + 커피챗 + LinkedIn 헤드라인, 2026.06~)
- 2026.11 **6개월 분기 재평가 #1** (스파이크 결과 / Phase 4 진행 / probe 반응)
- Hardware-Arm Stage 1 본 빌드는 Phase 4 종료 후 (2027.01-02) — 한 구간 1트랙 원칙


> 기존 Phase 4 (3D Perception/KITTI/BEV) 는 `Archive/Perception-3D-legacy/` 로 이동 예정 (F안). 본 레포의 새 Phase 4 는 VLA 트랙.
