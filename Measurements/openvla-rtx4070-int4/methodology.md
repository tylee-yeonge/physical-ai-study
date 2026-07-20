# 측정 조건·절차 — openvla-rtx4070-int4

> 측정 대상: OpenVLA 7B (int4/nf4) 의 `predict_action` 1회 호출 latency, RTX 4070 12GB
> 측정 코드: [`scripts/practice.ipynb`](scripts/practice.ipynb) (2026-06 실측 당시 실행 기록 포함)
> 원본 데이터: [`raw/openvla_latency_4070_int4.npy`](raw/openvla_latency_4070_int4.npy) (float 배열, ms 단위, n=100)

## 1. 측정 조건 (2026-06 실측 기준 — 기지 사실)

| 항목 | 값 | 출처 |
|---|---|---|
| n | 100 | scripts/practice.ipynb |
| warm-up | 5회 (본 측정과 동일 호출) | scripts/practice.ipynb |
| batch size | 1 | scripts/practice.ipynb |
| 입력 이미지 | 224x224 랜덤 RGB, 매 반복 새 이미지 생성 (cache 효과 방지) | scripts/practice.ipynb |
| prompt | `"In: What action should the robot take to pick up the can?\nOut:"` 고정 | scripts/practice.ipynb |
| 호출 인자 | `input_ids` + `pixel_values` 만 전달 (attention_mask 제외 — 근거는 findings.md §3), `unnorm_key="bridge_orig"`, `do_sample=False` | scripts/practice.ipynb |
| CPU/GPU 동기화 | `torch.cuda.synchronize()` 를 측정 구간 앞뒤 모두 수행 | scripts/practice.ipynb |
| preprocessing 포함 여부 | **제외** — processor 전처리는 측정 구간 밖. 측정은 `predict_action` 단독 | scripts/practice.ipynb |
| ROS2 오버헤드 | 제외 (전체 제어 루프는 week11 dry-run 에서 별도 측정) | Studies/Phase 4 notes.md 순서 1 |

## 2. 결과 통계 (2026-06 실측)

| 통계 | 값 |
|---|---|
| mean | 300.3 ms |
| median (p50) | 301.3 ms |
| std | 3.8 ms |
| min / max | 290.4 / 308.2 ms |
| p95 | 304.8 ms |
| p99 | 305.6 ms |
| throughput | 3.33 Hz |
| `torch.cuda.memory_allocated` (로드 직후) | 4.38 GB — nvidia-smi 관점 VRAM 과의 차이 분석은 findings.md Block 1 | 

## 3. 미기록 항목 — 재측정 (2026.08 내, 휴직 중 GPU 접근성 리스크 대비) 에서 채운다

- [x] VRAM peak (`nvidia-smi` 기준) — §4 메모리 상세 실측 (2026-07-10) 으로 앞당겨 수행 완료. 차이 원인 해석은 findings.md §4.1
- [ ] 구간별 분해 (전처리 / vision encoder / action 토큰 autoregressive 생성 / un-normalization) — findings.md Block 3
- [ ] 실제 카메라 이미지 입력 조건 (랜덤 노이즈 대비 차이 여부)
- [ ] 재실행 스크립트 정리 (notebook → 단일 `.py`, 실행 명령어 1줄 문서화)
- [ ] `summary.csv` 생성 (위 통계의 기계 판독 요약)

## 4. 메모리 상세 실측 (2026-07, findings.md §4.1 Block 1 용)

> §3 의 VRAM peak 항목을 GPU 접근성 리스크 대비로 앞당겨 수행한다. 이 절은 수치의 원본만 담고, 차이 원인 해석은 findings.md §4.1 에 쓴다.
> 실행: `python scripts/memory_breakdown.py` — 로드 조건은 §1·environment.md 와 동일 (nf4, double quant, compute dtype fp16, eager)

측정 절차 — baseline (시점 0: CUDA 초기화 전 디바이스 전체 사용량) 을 먼저 기록하고 3개 시점에서 읽는다. nvidia-smi 값은 프로세스 행 매칭이 아니라 **baseline 대비 디바이스 증가분**이다 — Docker 컨테이너에서는 nvidia-smi 가 보여주는 PID 가 호스트 네임스페이스 값이라 `os.getpid()` 매칭이 구조적으로 불가능하기 때문 (전제: 측정 중 다른 프로세스의 GPU 사용량 일정):

| 시점 | torch 측 | nvidia-smi 측 (baseline 대비 증가분) |
|---|---|---|
| A. CUDA 초기화 직후 (텐서 1개만) | `memory_allocated` (근사 0) | context 단독 크기 |
| B. 모델 로드 직후 | `memory_allocated`, `memory_reserved` | 로드 후 사용량 |
| C. `predict_action` 1회 후 | `max_memory_allocated`, `memory_reserved` | 추론 후 사용량 |

결과 (2026-07-10 실측, 10진 GB):

| 항목 | 값 |
|---|---|
| A. nvidia-smi — CUDA context 단독 | 0.19 GB |
| B. `memory_allocated` | 4.38 GB (2026-06 값 4.38 GB 와 일치) |
| B. `memory_reserved` | 4.55 GB |
| B. nvidia-smi | 4.74 GB (= reserved 4.55 + context 0.19, allocator 밖 잔차 근사 0) |
| C. `max_memory_allocated` | 4.73 GB |
| C. `memory_reserved` | 4.84 GB |
| C. nvidia-smi | 5.05 GB |

dtype 별 합산 (스크립트 출력 — 합계가 B 의 `memory_allocated` 와 일치해야 함):

| 구분 | 크기 |
|---|---|
| torch.uint8 | 3.638 GB |
| torch.float16 | 0.531 GB |
| buffer:torch.float16 | 0.034 GB |
| buffer:torch.float32 | 0.000 GB |
| quant_state (scale 등) | 0.116 GB |
| 합계 | 4.32 GB (`memory_allocated` 4.38 GB 대비 잔차 0.066 GB) |

한계: peak 는 `predict_action` 1회 기준 — n=100 재측정 (2026.08) 때 동일 수준인지 재확인한다.
