# Phase 3: Detection + Depth → PC TensorRT + ROS2 노드 (비공개 리허설)


> **기간**: ~2026.06 초 (week1-7 완료, week8 통합 노드만 1-2일 내 마무리)
> **목표**: Detection + Depth + PC TensorRT/ROS2 통합 = **VLA v1 wrapper 의 난도 낮은 리허설이자 재사용 스캐폴드**
> **상태**: **공개 산출물 아님** (repo 내부 경량 로그). AMR ROS 5년차 기준 commodity 라 별도 공개 어필(velog/LinkedIn) 안 함. 학습/통합 가치(양자화 배포, 추론 노드 스캐폴드)는 VLA v1 에 흡수.
> **언어**: **Python** (학습) + **C++/TensorRT** (PC 배포) + **ROS2** (노드 래퍼)
> **하드웨어**: Ubuntu PC (RTX 4070) — 주 학습/실험/배포 장비 / Jetson Orin Nano — **v3 이후 옵션**
> **주간 시간**: 약 6-8시간 (출장 주 보정)


---


## -> **실습 가이드**: [`Studies/Phase 3/`](../Studies/Phase%203/) (week1-8, 각 weekN/PRACTICE.md)


**학습 내용 (비공개 리허설)**:
- YOLO11 실시간 객체 검출 (PC TensorRT)
- Depth Anything V2 (PC TensorRT)
- 통합 시스템: Detection + Depth → 3D 위치 추정 + ROS2 노드 래퍼 (= VLA wrapper 스캐폴드)


> **공개하지 않는 이유**: 시각 grounding 은 VLA 내부 비전 인코더로 흡수되므로 별도 perception 노드 공개의 명분이 약하다. 어필 자원을 VLA(v1) 에 집중한다. 단 TensorRT + ROS2 노드 통합 경험은 v1 wrapper 의 재사용 스캐폴드로 그대로 쓰인다.
> **fallback 예외**: 부록 E(AMR/AV Perception 착지) 진입 시, 이 비공개 로그를 velog 1편으로 승격 공개해 공백기 산출물로 활용한다.


---


## 학습 환경


| 단계 | 주 장비 | 출장지 가능 여부 |
|---|---|---|
| PyTorch 복습 / YOLO 학습 | Ubuntu PC (원격) | O |
| PC TensorRT 변환 + C++ 추론 | Ubuntu PC (원격) | O |
| Depth Anything V2 실습 + PC TensorRT | Ubuntu PC (원격) | O |
| 통합 시스템 + ROS2 노드 래퍼 | Ubuntu PC (원격) | O |
| Jetson 실기 배포 / 실측 FPS | Jetson (v3 이후 옵션) | 보류 |


- 데이터셋 (COCO, KITTI, Middlebury): Ubuntu PC 디스크 상시 보관
- 상세: [ENVIRONMENT.md](../ENVIRONMENT.md)


> Phase 3 에서는 **PC TensorRT + ROS2 노드까지만**. Jetson 실기 배포는 전체 학습 (Phase 2-7) 완료 후 옵션.


---


## 8주 구성 (Studies/Phase 3 와 정합)


| 주차 | 내용 | 핵심 | 산출 |
|---|---|---|---|
| week1 | PyTorch 기초 재정비 | Tensor / autograd / DataLoader / CNN | 환경 세팅 + ResNet 분류 |
| week2 | 컴퓨터 비전용 라이브러리 | Albumentations / W&B / timm | 증강 파이프라인 + 실험 로깅 |
| week3 | YOLO 이론 | 발전사 / YOLO11(C3k2, PANet, decoupled head) / Detection 지표(mAP, IoU) | 개념 노트 |
| week4 | YOLO11 학습 (Python) | Ultralytics / 커스텀 데이터 / mAP 측정 | 학습된 모델 (mAP > 0.6) |
| week5 | ONNX 변환 및 최적화 | PyTorch → ONNX / ONNX Runtime / quantization | ONNX 모델 + 속도 비교 |
| week6 | PC TensorRT 배포 (C++) | layer fusion / kernel tuning / FP16 / C++ inference + NMS | TensorRT 엔진 + C++ 추론 + latency |
| week7 | Monocular Depth + Depth Anything V2 | MiDaS/DPT/Depth Anything V2 / ONNX & TensorRT / 정확도 검증(AbsRel, RMSE) | Depth TensorRT + 정확도 |
| week8 | **통합 시스템 (Detection + Depth + ROS2)** | 3D 위치 추정(역투영) + ROS2 노드 래퍼(`vision_msgs` publish + TF) + latency | **통합 노드 = VLA wrapper 리허설** |


> week8 통합 노드가 본 Phase 의 핵심 산출이자 VLA v1 wrapper 의 스캐폴드다. 빌드/실행 후 repo 에 비공개 로그로 커밋한다(velog 공개 안 함).


---


## Section 자체 점검


### Detection
1. mAP@0.5 와 mAP@0.5:0.95 의 차이는?
2. TensorRT 가 빠른 이유 3가지는? (layer fusion / kernel auto-tuning / precision)
3. ONNX 를 거치는 이유는? (프레임워크 독립적 중간 표현)


### Depth
1. Monocular Depth 가 Stereo 보다 부정확한 이유는?
2. Relative depth 와 Metric depth 의 차이는?
3. Depth Anything V2 의 장점은? (합성 데이터 활용 일반화)


### 통합
1. Depth map 과 Detection bbox 를 어떻게 융합하는가? (bbox 중심 픽셀의 depth → 역투영)
2. ROS2 노드 래퍼가 VLA v1 wrapper 와 무엇을 공유하는가? (inference → 토픽 publish 스캐폴드)


---


## Phase 3 완료 기준


> "PC TensorRT 위에서 Detection + Depth → 3D 위치 추정 + ROS2 노드 래퍼를 구동하고(week8 통합 노드), 이를 repo 내부 로그로 커밋한다. 이 통합 노드가 VLA v1 wrapper 의 재사용 스캐폴드로 쓸 수 있는 상태면 완료. 공개 어필은 하지 않는다."


### 완료 체크리스트
- [ ] YOLO11 커스텀 데이터 학습 (mAP > 0.6) + ONNX + PC TensorRT C++ 추론
- [ ] Depth Anything V2 ONNX & PC TensorRT + 정확도 검증
- [ ] week8 통합 노드 (Detection + Depth → 3D 위치 + ROS2 `vision_msgs` publish + TF) 빌드/실행
- [ ] latency 측정 (전체 파이프라인 ms 단위)
- [ ] repo 비공개 로그 커밋 + 짧은 노트 (velog/LinkedIn 공개 안 함)


---


## 참고 자료


| 이름 | 용도 | 링크 |
|------|------|------|
| Ultralytics YOLO11 | 객체 검출 | https://github.com/ultralytics/ultralytics |
| Depth Anything V2 | Depth 추정 | https://github.com/DepthAnything/Depth-Anything-V2 |
| TensorRT | PC/Jetson 최적화 | https://developer.nvidia.com/tensorrt |


| 데이터셋 | 용도 |
|---------|------|
| COCO | Detection 학습/평가 |
| KITTI Depth | Depth 학습/검증 |
| NYU Depth V2 | 실내 Depth |


---


## 팁


1. **작은 모델부터**: YOLO11n (nano)부터 시작, 성능 필요하면 확장
2. **데이터가 80%**: 라벨링 품질이 성능 차이의 대부분
3. **TensorRT 디버깅**: Verbose 모드로 최적화 과정 관찰
4. **Depth는 상대적**: Metric depth 필요 시 스케일 조정 필수
5. **통합 노드를 VLA 관점으로**: week8 노드를 "inference → ROS2 토픽" 스캐폴드로 설계해 v1 에서 재사용


---


## [?] 다음 단계


Phase 3 마무리 후 (2026.06):
- **Phase 4: VLA v1** (2026.06-09, 메인 단독) — pretrained OpenVLA zero-shot 추론 + ROS2 wrapper + 카메라/bag dry-run → **산출물 v1 (2026 하반기 레포 기록, 외부 공개는 v2 로 이관)**
- **하드웨어 스파이크 (2026.10, 2-3주)** — 2-DOF Dynamixel + ROS2 파이프라인 리스크 검증 (산출물 아님). 조달은 v1 과 병렬로 지금 착수.
- 병행: **시장 신호 probe** (가시성 기준 분해 — JD 정독 2026.07-08 / LinkedIn 헤드라인·커피챗 2026.09~)
- **6개월 분기 재평가 #1 (2026.11)** (스파이크 결과 / v1 결과 / probe 반응). **육아휴직(2026.09-2027.02) 중이라 정찰 지원은 안 하며, 본격 실지원은 복직(2027.03) 직후 개시.**


> Jetson 실기 배포 자료는 `Archive/Phase3-jetson-legacy/` 에 있음. Jetson 배포는 v3 이후 옵션.
