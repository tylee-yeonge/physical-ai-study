# Studies/Phase 3 — Detection + Depth (PC TensorRT + ROS2)


> **학습 기간**: 2026.06~08 (2개월, 8주)
> **마스터 가이드**: [`Roadmap/Phase 3.md`](../../Roadmap/Phase%203.md)


---


## 디렉토리 구조


```
Studies/Phase 3/
  week1/ # PyTorch 기초 재정비
  week2/ # 컴퓨터 비전용 라이브러리 (Albumentations / W&B / timm)
  week3/ # YOLO 이론 (발전사 / YOLO11 / Detection 지표)
  week4/ # YOLO11 학습 (Python)
  week5/ # ONNX 변환 및 최적화
  week6/ # PC TensorRT 배포 (C++)
  week7/ # Monocular Depth 이론 + Depth Anything V2
  week8/ # 통합 시스템 (Detection + Depth + ROS2 노드 래퍼)
```


## week 자료 형식


각 week 디렉토리:
- `README.md` — 이번 주 목표 + 학습 순서 + 핵심 개념
- `PRACTICE.md` — 실습 단계별 가이드
- `quiz_easy.{py,cpp}` / `quiz_medium.{py,cpp}` — 개념 / 코드 퀴즈
- `quiz_solutions/` — 답
- `requirements.txt` 또는 `CMakeLists.txt` — 의존성 (week 별 Python / C++ 여부에 따라)


## 참고


- 정책, 산출물 #1 공개 일정, 주차별 상세 학습 내용은 모두 [`Roadmap/Phase 3.md`](../../Roadmap/Phase%203.md) 에 있음. 본 디렉토리는 학습 노트 모음.
- 기존 12주 로드맵 중 Jetson 실기 배포 (week 9~12) 자료는 [`Archive/Phase3-jetson-legacy/`](../../Archive/Phase3-jetson-legacy/) 로 이동됨. Jetson 배포는 Phase 7 이후 옵션 #5.
