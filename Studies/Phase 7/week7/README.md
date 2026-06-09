# Week 7: 전체 파이프라인 Latency 측정


> **이번 주 목표**: image -> inference -> 안전 -> 모터의 전체 latency 정량.
> **예상 시간**: 6시간


## 학습 순서
1. End-to-end latency 측정
2. Component 분해
3. Histogram + 통계
4. 퀴즈


## 핵심 개념


### Latency breakdown (목표)
```
| Component | Mean (ms) | p95 |
| Image capture -> inference 입력 | 5 | 10 |
| OpenVLA inference | 165 | 220 |
| IK | 5 | 8 |
| Safety check | 1 | 2 |
| Motor write | 8 | 15 |
| Total end-to-end | ~ 184 | ~ 255 |
```


### 측정 방법
- header.stamp 모든 단계
- 시작/끝 timestamp 차이


### Real-to-Sim-to-Real 의 latency 관점
- Real: 184 ms
- Sim equivalent: ~ 210 ms (Sim step 35 ms 추가)
- Real 이 더 빠름 (Sim 의 step time overhead)


## 자체 점검
Q1. End-to-end 측정 방법? > image header -> motor write timestamp.
Q2. Total 기대? > ~ 184 ms.
Q3. 가장 큰 component? > Inference (165).
Q4. Sim vs Real latency? > Sim 이 step time 으로 약간 느림.


## 요약
1. End-to-end latency 184 ms
2. Inference dominant (90%)
3. Sim 약간 느림
4. Phase 7 v3 의 정량 증거


- [Week 6](../week6/README.md) | [Week 8](../week8/README.md)
