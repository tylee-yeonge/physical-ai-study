# Week 9: 4가지 Sim/Real Gap 정량 (Phase 6 인프라 활용)

> [goal] **이번 주 목표**: latency / 반복성 / force / 시각 4 gap 의 Phase 7 시점 측정.
> [time] **예상 시간**: 6시간

## 학습 순서
1. Phase 6 인프라 재사용 + Phase 7 환경에서 측정
2. 4 gap 표 갱신
3. 보고서 작성
4. 퀴즈

## 핵심 개념

### 4 Gap (Phase 6 의 측정 패턴 + Phase 7 환경)

| Gap | Phase 6 측정 | Phase 7 (LoRA fine-tune 후) |
|---|---|---|
| Latency | 165 ms inference | 같음 (LoRA 영향 X) |
| 반복성 | Sim 0.05 / Real 2.3 mm | 같음 |
| Force | Friction 5x | 같음 |
| Image | DR 후 38 distance | 같음 |

Phase 7 의 새로움:
- closed loop 의 e2e success rate
- Real 의 작업 완료율

### 산출물 #4 의 정량 표 (template)

```markdown
| Metric | Sim | Real | Gap |
|---|---|---|---|
| Latency (closed loop) | 215 ms | 184 ms | -31 |
| Success rate | 80% | 65% | -15%p |
| Repeatability (std) | 0.05mm | 2.3mm | 46x |
| Force fidelity | Friction sim | Real measure | 5x |
| Image (RGB hist) | After DR | Real | 38 dist |
```

이 표가 Phase 7 산출물 #4 의 핵심.

## 자체 점검
Q1. 4 gap 의 source? > Phase 6 week 8-11.
Q2. Phase 7 의 새로움? > Closed loop success rate.
Q3. 보고서 활용? > 산출물 #4 의 영상에 인용.
Q4. Sim/Real 차이? > Latency / 반복성 / force / image.
Q5. 양산 차별화 증거? > 정량 수치 모두 (latency / 안전 / 비용).

## 요약
1. 4 gap 재측정 (LoRA 후)
2. 정량 표 통합
3. 산출물 #4 의 핵심 보고서
4. "VLA latency / 안전 메커니즘 / 양산 비용" 증거 종합
5. 다음: 영상 마감

[O] [Week 8](../week8/README.md) | [Week 10](../week10/README.md)
