# Week 12: Sim/Real gap 종합 보고서 + Phase 7 진입


> **이번 주 목표**: Phase 6 의 4 gap 통합 보고서. Phase 7 산출물 #4 의 직접 입력.
> **예상 시간**: 5시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 4 gap 통합 보고서 | `PRACTICE.md` 1 | week 8-11 결과 |
| 2 | Rerun 통합 시각화 | `PRACTICE.md` 2 | 동시 표시 |
| 3 | Phase 6 회고 | `PRACTICE.md` 3 | retro |
| 4 | Phase 7 진입 준비 | `PRACTICE.md` 4 | |
| 5 | 퀴즈 | | |


---


## Phase 6 마지막 주


Phase 7 (2027.08~, Real-to-Sim-to-Real) 의 진입.


```
Phase 6 산출물:
- Isaac Sim + 디지털 트윈
- 4 gap 측정 인프라
- DR 시작


Phase 7 입력:
- 본 phase 의 모든 인프라
- OpenVLA + ROS2 (Phase 4)
- LoRA (Phase 5)
- 자작 6DOF 팔 (Hardware-Arm Stage 2)


Phase 7 산출물 #4:
- Real-to-Sim-to-Real 영상 (2027.08~)
```


---


## 핵심 개념


### 1. 종합 보고서 (templates)


```markdown
# Sim/Real Gap Report (자작 6DOF 팔, 2027.07)


## 1. Latency
| Component | Sim | Real | Gap |
|---|---|---|---|
| Step / response | 35 ms | 8 ms | -27 ms |
| Closed loop | 210 ms | 195 ms | -15 ms |


## 2. 반복성 (100회)
| | Sim std | Real std |
|---|---|---|
| EE | 0.05 mm | 2.3 mm |
| Joint | 0.001 rad | 0.005 rad |


## 3. Force/Torque
Friction parameter mismatch 가 주 원인.


## 4. Image (RGB histogram distance)
| Metric | Default | After DR |
|---|---|---|
| Hist dist | 145 | 38 |
| Noise var | Sim 12 vs Real 320 | Sim 280 |
```


### 2. Rerun 통합


```
camera image (Real, Sim)
joint state (Real, Sim)
latency timeseries
torque timeseries
ee position trace
```


### 3. Phase 7 진입 점검


```
- [ ] Isaac Sim 안정
- [ ] 자작 팔 Stage 2 완성
- [ ] OpenVLA inference 통합 가능
- [ ] LoRA 환경 준비
- [ ] Sim/Real gap 측정 코드 모두 동작
```


### 4. 분기 재평가 #2


2027.07 시점:
- VLA 모델 갱신
- Phase 7 시점
- 2028.03 fallback 진입 여부 (실지원은 2027.02 개시)


---


## 자체 점검


**Q1. 4 gap?** > Latency/반복성/Force/Image.
**Q2. 가장 큰 gap?** > Image (noise variance, DR 전 후).
**Q3. Phase 7 입력?** > Phase 6 인프라 + OpenVLA + 자작 팔.
**Q4. 분기 재평가 #2?** > 2027.05, VLA 갱신 + Phase 7.
**Q5. 실지원 시점?** > 2027.02 개시, 2028.03 은 fallback 분기.


---


## 실습 + Phase 7


### 이번 주: 종합 보고서 + Rerun + retro + quiz
### Phase 7 (2027.08~): 산출물 #4 강화 카드


---


## Phase 6 핵심 요약


1. **4 gap 측정 완료**
2. **종합 보고서** Phase 7 입력
3. **Domain Randomization 시작**
4. **Rerun 통합 시각화**
5. **Phase 7 진입 준비도 완성**


- [Week 11](../week11/README.md) | [Phase 7](../../Phase%207/README.md)
