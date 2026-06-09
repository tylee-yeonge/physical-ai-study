# Week 8: Sim/Real 비교 영상 (동일 명령)


> **이번 주 목표**: 같은 instruction 으로 Sim 과 Real 동시 실행. Side-by-side 영상.
> **예상 시간**: 8시간


## 학습 순서
1. Sim + Real 동시 실행 인프라
2. 같은 random seed 또는 같은 init pose
3. Side-by-side 영상 capture
4. 차이 분석 + 보고서
5. 퀴즈


## 핵심 개념


### 동시 실행
```
Terminal 1: Isaac Sim (Phase 6 의 디지털 트윈)
Terminal 2: vla_node (LoRA fine-tuned)
Terminal 3: Real 자작 팔 dynamixel_hardware
Terminal 4: Sim 의 카메라가 동시에 vla_node 의 input?
            Or Real 카메라가 vla_node 의 input + Sim 은 mirroring?
```


본 phase 권장: **Real 카메라 input -> vla_node -> 두 robot (Sim mirror + Real)** 동시.


### Side-by-side 영상
- 좌측: Real 자작 팔
- 우측: Isaac Sim 디지털 트윈
- 하단: Rerun 의 latency / joint 차트


영상 구성 (1분):
- 0:00-0:10 Intro
- 0:10-0:25 System
- 0:25-0:50 실시간 실행 (3 trials)
- 0:50-0:55 결과 + 통계
- 0:55-1:00 Next


### 차이 분석
- Joint 차이 시계열
- ee 위치 차이 (Sim vs Real)
- Phase 6 의 4 gap 활용


## 자체 점검
Q1. 동시 실행 흐름? > Real camera -> vla_node -> Sim mirror + Real.
Q2. 영상 구성? > Real | Sim | metrics.
Q3. 차이 분석? > Joint / ee 시계열, Phase 6 의 4 gap.
Q4. 보고서 입력? > Phase 7 v3 의 영상의 한 segment.


## 요약
1. Sim/Real 동시 실행
2. Side-by-side 영상
3. 차이 시계열 분석
4. Phase 6 의 4 gap 인용
5. 산출물 v3 의 핵심 컨텐츠


- [Week 7](../week7/README.md) | [Week 9](../week9/README.md)
