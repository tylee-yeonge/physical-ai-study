# Week 3: Sim 환경 dry-run (디지털 트윈 활용)

> [goal] **이번 주 목표**: Phase 6 디지털 트윈 + fine-tuned OpenVLA. Sim 에서 안전 dry-run.
> [time] **예상 시간**: 8시간

## 학습 순서
1. Sim 에 inference 노드 연결
2. Sim image -> action -> Sim robot
3. Success rate 측정
4. 퀴즈

## 핵심 개념

### Sim closed loop
```
Isaac Sim camera -> /camera/image_raw
-> vla_node (fine-tuned)
-> /joint_command
-> Isaac Sim articulation
```

### Dry-run 의 가치
- 실 robot 손상 X
- 무한 반복
- 안전 인터록 (week 4-6) 검증 환경

### Success criteria
- ee target 5cm 이내
- grasp 성공
- no collision

## 자체 점검
Q1. Sim closed loop? > image -> action -> joint -> Sim.
Q2. Dry-run 가치? > 손상 X, 무한 반복.
Q3. Success? > 5cm + grasp + no collision.

## 요약
1. Sim closed loop
2. Dry-run 검증
3. Success rate
4. Phase 6 활용
5. 다음: 안전 인터록

[O] [Week 2](../week2/README.md) | [Week 4](../week4/README.md)
