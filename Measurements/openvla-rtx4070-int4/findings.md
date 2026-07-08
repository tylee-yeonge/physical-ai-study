# 결과 해석·판단 근거 — openvla-rtx4070-int4

> 이 문서는 측정 수치의 해석과 그 위에서 내린 엔지니어링 판단을 기록한다. §1-§3 은 2026-06 실측 시점의 판단 기록 (출처 표기), §4 는 이해 검증 4블록 (2026.07-08) 에서 본인이 작성하는 영역이다.

## 1. int8 경로 배제 판단 (2026-06)

출처: `Studies/Phase 4/SETUP.md` §1.3

- int8 Bridge 성공률 58.1 ± 5.1% — bf16 (71.3%)·int4 (71.9 ± 4.7%) 대비 유의한 하락 (OpenVLA 논문 Table 2)
- int8 추론 속도 1.2 Hz (A5000) — 속도 하락이 시스템 동역학을 바꿔 성공률 하락으로 이어짐 (논문 §5.4)
- 판단: 성공률·속도 모두 열위이므로 실험에서 배제. 코드 경로는 비교 실험 대비로만 보존 (week8 config)
- 미해소: "비트 수가 크면 덜 손실" 직관과 반대인 이유의 메커니즘 설명 — §4 Block 2 에서 본인 작성

## 2. 3.33 Hz 의 제어 주기 해석 (2026-06)

출처: `Studies/Phase 4/notes.md` 순서 1·3 노트

- 실측 mean 300.3 ms (3.33 Hz) 는 사전 외삽 (메모리 대역폭 기준 약 2-3 Hz, 4070 의 504 GB/s vs A5000 768 GB/s) 의 ±50% 허용 범위 안, 포인트 범위보다 약 9% 빠름
- std 3.8 ms, p95 가 mean 대비 +4.5 ms — 지연 꼬리가 거의 없어 실시간 제어 관점에서 안정적
- quasi-static 단일 task (PickCube 급) 에는 충분 추정 — OpenVLA 원 실험이 유사 속도 대역에서 blocking control 로 실로봇 구동
- 전체 제어 루프 하한은 2 Hz (step 500 ms) 로 설정: predict_action 300 ms + 카메라 캡처·ROS2 통신 headroom 약 200 ms. week8 stress test 에서 processor 전처리 오버헤드는 무시 수준임이 확인됨 (`predict()` 전체 mean 298.8 ms ≈ predict_action 단독)
- 미해소: 제어 계층 구조 (저수준 수백Hz-1kHz vs 정책 수Hz) 와의 연결, action chunking 의 역할 — §4 Block 4 에서 본인 작성

## 3. attention_mask 제외 workaround (2026-06)

출처: `Studies/Phase 4/notes.md` 순서 1·2 노트

```python
# attention_mask 는 전달하지 않는다 -- predict_action 이 빈 토큰(29871) 을 input_ids 에만
# 덧붙여 mask 와 길이가 1 어긋나므로 (eager attention 에서 크래시), generate 가 mask 를
# 알아서 생성하게 둔다
action = vla.predict_action(input_ids=..., pixel_values=..., unnorm_key="bridge_orig", do_sample=False)
```

processor 출력 전체를 `**inputs` 로 넘기는 참고 자료 패턴은 off-by-one 크래시 — 리포 전체에서 해당 패턴을 일괄 청소했다.

## 4. 이해 검증 4블록 — 본인 작성 영역 (2026.07-08)

> 작성 방식: **예측 → 측정 → 오차 설명**. 남의 문서 요약이 아니라 본인 실험 기록으로 쓴다. 통과 기준은 검토 보고서 v1.4 §2.6.

### 4.1 Block 1 — int4 메모리 산수 (2026.07)

- [ ] 7B x 2byte = 14GB 손계산 → int4 예측치 → `memory_allocated` (실측 4.38 GB) / `nvidia-smi` 대조 → 차이 원인 (allocator 예약, activation, CUDA context) 설명

### 4.2 Block 2 — int8 열위 메커니즘 (2026.07)

- [ ] LLM.int8() outlier 채널 fp16 분해 오버헤드 vs NF4 fused kernel — §1 의 미해소 항목 답변 + "fp16 대비 int4 속도" 방향 예측

### 4.3 Block 3 — 300 ms 구간 분해 (2026.08, 재측정과 병합)

- [ ] 추론 경로 (전처리 → vision encoder → action 토큰 7개 autoregressive 생성 → un-norm) 구간별 프로파일링 표 + "해상도 절반이면 어디가 얼마나" 예측-측정-오차

### 4.4 Block 4 — 3.33 Hz 의 조건부 판정 (2026.08, 재측정과 병합)

- [ ] "3.33 Hz 면 충분한가" 를 task 유형별 (정적 pick / 동적 추적) 조건부로 답변 + action chunking 이 간극을 메우는 방식
