# Week 6: Inference -> 안전 -> 모터 통합 + 오버헤드 측정

> [goal] **이번 주 목표**: 전체 파이프라인 통합 + 인터록 오버헤드 정량.
> [time] **예상 시간**: 8시간

## 학습 순서
1. 전체 파이프라인 통합
2. 인터록 오버헤드 측정
3. Real 자작 팔 실행 (안전 보장)
4. 퀴즈

## 핵심 개념

### 전체 파이프라인
```
/camera/image_raw -> vla_node -> /inference/joint_command
                                  -> safety_node (C++)
                                  -> /joint_command
                                  -> dynamixel_hardware
                                  -> motor 동작
```

### 오버헤드 측정
```
Latency: image header -> motor write
Components:
- Inference: 165 ms
- IK: 5 ms
- Safety: 1 ms
- Motor write: 8 ms
- Total: ~ 180 ms
```

### Real 실행 - 안전 우선
- 첫 실행: low speed (50% velocity limit)
- 충돌 감지 확인
- e-stop 손이 닿는 위치
- 위험한 trajectory 미리 sim 에서 검증

## 자체 점검
Q1. 통합 흐름? > inference -> IK -> safety -> motor.
Q2. 안전 오버헤드? > ~ 1 ms.
Q3. 총 latency? > ~ 180 ms.
Q4. 첫 Real 실행 시 주의? > low speed + e-stop 위치.
Q5. 양산 차별화 증거? > "VLA latency 200ms / 안전 메커니즘" 의 정량 측정.

## 요약
1. **전체 파이프라인 통합** vla -> ik -> safety -> motor
2. **Total latency ~ 180 ms**
3. **Safety overhead ~ 1 ms**
4. **첫 Real 실행 안전 우선**
5. **"VLA latency / 안전 메커니즘" 직접 증거**

[O] [Week 5](../week5/README.md) | [Week 7](../week7/README.md)
