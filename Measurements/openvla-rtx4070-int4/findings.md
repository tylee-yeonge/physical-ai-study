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

> 골격: 실측값 하나 (`memory_allocated` 4.38 GB) 를 첫 원리 (파라미터 수 x dtype 크기) 에서 출발한 예측으로 끝까지 설명한다. 실측 수치의 원본은 methodology.md §4 (2026-07 메모리 상세 실측), 측정 코드는 `scripts/memory_breakdown.py`. 이 절에는 예측 논리와 오차 해석만 본인 문장으로 쓴다.

- [x] **Step 1 — fp16 손계산 (로드 불가의 정량 근거)**

  총 파라미터 수를 모델에서 직접 센다 — OpenVLA 는 Llama-2 7B 백본 + vision encoder (SigLIP + DINOv2) + projector 구조라 정확히 7.0B 가 아니다. 실제 파라미터 수 x 2 byte 로 fp16 요구량을 계산하고, 4070 12GB 에 로드 불가 → int4 채택의 정량 근거로 결론 짓는다.

  -> 실제 파라미터 수 : Llama-2(6.74B) + DINOv2 ViT-L/14(0.303B) + SigLIP SoViT-400M(0.428B) + 이미지 patch 를 visual token 으로 바꾸는 층 (projector, 0.071B) = 7.541B
  -> 7.541B x 2 byte = 15.08 GB => GPU 12GB보다 크기 때문에 로드 불가능

- [x] **Step 2 — int4 예측치 (나이브 → 정제)**

  나이브 계산 (전체 x 0.5 byte) 값을 먼저 적고, 그 값이 실측 4.38 GB 보다 작을 수밖에 없는 이유 두 가지를 반영해 정제 예측을 만든다:
  - bitsandbytes 는 `nn.Linear` 만 양자화한다 — embedding·`lm_head`·LayerNorm 은 fp16 잔존. vision encoder 포함 여부는 Step 3 실측에서 확인
  - NF4 는 블록 (기본 64 파라미터) 마다 absmax scale 을 저장한다 — double quant 포함 오버헤드 추정

  선행 정독 (Block 2 정독분의 앞당김, 약 30분) — 정제 공식의 상수 (블록 64, 2차 블록 256, absmax int8) 는 유도가 아니라 설계 사실이라 출처를 먼저 읽어야 한다:
  - [x] QLoRA 논문 §3 의 두 문단: "4-bit NormalFloat Quantization" (블록 64), "Double Quantization" (absmax 를 블록 256 단위로 int8 재양자화)

  정제 예측 공식 — 각 항의 출처. 값 대입과 해석은 본인 작성:

  | 항 | 계산 | 출처 |
  |---|---|---|
  | int4 본체 | N_q x 0.5 byte | int4 2개가 uint8 1개에 packing — transformers `BitsAndBytesConfig.bnb_4bit_quant_storage` 기본값 `torch.uint8` |
  | absmax scale | N_q / 64 x 1 byte | QLoRA §3 (블록 64) — double quant 로 absmax 가 int8 (1 byte) 저장. bitsandbytes `functional.py` 기본값 `blocksize=64` |
  | 2차 scale | N_q / (64 x 256) x 4 byte | QLoRA §3 Double Quantization — bitsandbytes `functional.py` 의 `quantize_blockwise(absmax - offset, blocksize=256)`. 사실상 무시 가능 |
  | fp16 잔존 | N_f x 2 byte | transformers bitsandbytes 문서 — `lm_head` 등 제외 모듈은 fp16 유지 |

  N_q (양자화 파라미터 수)·N_f (fp16 잔존 파라미터 수) 산출 주의: 양자화된 모델에서 `sum(p.numel())` 은 int4 가중치를 packing 된 uint8 개수 (실제의 절반) 로 세므로 과소집계된다. methodology §4 의 dtype 표에서 역산한다 — N_q = uint8 바이트 x 2, N_f = fp16 바이트 / 2.

  상수를 모델에서 직접 확인하는 법 (논문 교차 검증):

  ```python
  lin = next(m for m in vla.modules() if type(m).__name__ == "Linear4bit")
  qs = lin.weight.quant_state
  print(lin.in_features * lin.out_features, lin.weight.numel())  # 2배 차이 = packing
  print(qs.blocksize, qs.absmax.dtype, qs.state2.blocksize, qs.state2.absmax.dtype)
  # 기대: 64, torch.uint8, 256, torch.float32
  ```

  -> 나이브 계산: 7.541B x 0.5byte(int4) = 3.77GB
  -> 실측 자료: N_q(int4) = uint8 3.638GB * 2 = 7.276B, N_f = float16 0.531GB / 2 = 0.2655GB
  -> 정제 예측: int4 본체 = N_q x 0.5, absmax scale = N_q / 64, 2차 scale = N_q / (64 x 256) x 4, fp16 잔존 = N_f x 2
    => int4 본체 = 3.638GB, absmax scale = 0.1137GB, 2차 scale = 0.0018GB, fp16 잔존 = 0.531GB
    => 합산: 4.284GB

- [x] **Step 3 — `memory_allocated` (4.38 GB) 대조**

  스크립트의 dtype 별 합산 표 (methodology §4) 가 4.38 GB 와 일치하는지 확인하고, Step 2 정제 예측과의 오차를 설명한다. 부수 확인: vision encoder 가 양자화됐는지가 이 표에서 드러난다.

  -> 측정 내용: memory_allocated (로드 직후) => 4.38 GB
  -> 정제 예측: 4.284GB
    => 측정 내용(4.38GB)와 정제 예측(4.284GB)는 일치하지 않음
    => dtype 별 합산(4.32GB) - 정제 예측(4.284GB) = buffer - 공식이 다루지 않는 비파라미터 텐서
    => memory_allocated(4.38GB) - dtype 별 합산(4.32GB) = 표 집계 밖 미설명 잔차

- [x] **Step 4 — `nvidia-smi` 대조 (차이 원인 3분해)**

  nvidia-smi 프로세스 사용량과 `memory_allocated` 의 차이를 3개 delta 로 분리하고, 각 delta 가 왜 생기는지 본인 문장으로 설명한다:

  | 원인 | 대응 delta | 수치 출처 |
  |---|---|---|
  | CUDA context | smi(A, 텐서 1개만) 또는 smi(B) - `memory_reserved`(B) | methodology §4 시점 A·B |
  | allocator 예약 | `memory_reserved`(B) - `memory_allocated`(B) | methodology §4 시점 B |
  | activation·KV cache | `max_memory_allocated`(C) - `memory_allocated`(B) | methodology §4 시점 C |

  -> CUDA context = 0.19GB : torch의 allocator 밖에서 CUDA 드라이버/런타임이 잡는 메모리
  -> allocator 예약 = 4.55GB - 4.38GB = 0.17GB : PyTorch caching allocator의 동작(cudaMalloc이 비싸스 큰 블록으로 미리 받아두고, 텐서가 free 되어도 블록을 CUDA에 반환하지 않고 캐시함)
  -> activation·KV cache = 4.73GB - 4.38GB = 0.35GB : 추론 중 vision encoder, LLM forward의 activation과 action 토큰 7개를 autoregressive로 생성하는 동안의 KV cache가 일시적으로 할당되었다가 해제되는데 사용한 최대치

사다리 표 — 각 칸이 채워지면 Block 1 완료:

| 단계 | 예측 | 실측 | 오차 해석 |
|---|---|---|---|
| fp16 손계산 | 15.1GB | — (로드 불가) | 현재 갖고있는 GPU(4070 12GB)의 크기를 초과해서 실측이 불가능함 |
| int4 나이브 | 3.77GB | — | 정제 예측과는 0.646GB 차이 발생 - 나이브 계산 대비 fp16 잔존 내용이 파라미터 당 1.5byte를 늘어나 0.398GB, scale 오버헤드가 0.11GB(absmax scale = 0.113GB + 2차 scale = 0.002GB) |
| int4 정제 예측 | 4.284GB | 4.38GB (`memory_allocated`) | 예측 공식이 다루지 않는 텐서 종류 0.036GB + 텐서를 열거해도 잡히지 않는 장부상 잔차 0.066GB (표시 자리수에 의한 오차 0.006) |
| nvidia-smi (로드 직후) | 4.74GB (`memory_reserved`(B): 4.55 + `context`: 0.19) | 4.74GB | 오차 없음, 로드 시점에는 allocator 밖 할당이 없음 |
| nvidia-smi (추론 후) | 5.03GB (`memory_reserved`(C): 4.84 + `context`: 0.19)| 5.05GB | 추론 중 allocator 밖 할당이 생긴 것으로 추정 (cuBLAS workspace 등 후보, 미확인) |

- [x] **Step 5 — 논문 "약 7GB" 와의 간극 해석 (사다리 표 밖 잔여분, Task 2-2 와 병행)**

  SETUP.md §1.3 은 int4 추론 VRAM 을 "약 7GB (논문 실측)" 으로 인용하는데, 본 실측은 디바이스 기준 5.05 GB (추론 후) 다. 

  **원문 확인** — 수치는 논문 (arXiv 2406.09246 v3) §5.4 "Memory-Efficient Inference via Quantization" 의 **Table 2** 에 있다: bfloat16 16.8 GB / int8 10.2 GB / int4 7.0 GB 

  **코드 확인** 
  - 메모리 측정 코드는 리포에 없음 → 측정 기준은 코드로도 확정 불가, 미확인으로 종결

### 4.2 Block 2 — int8 열위 메커니즘 (2026.07)

- [ ] LLM.int8() outlier 채널 fp16 분해 오버헤드 vs NF4 fused kernel — §1 의 미해소 항목 답변 + "fp16 대비 int4 속도" 방향 예측

### 4.3 Block 3 — 300 ms 구간 분해 (2026.08, 재측정과 병합)

- [ ] 추론 경로 (전처리 → vision encoder → action 토큰 7개 autoregressive 생성 → un-norm) 구간별 프로파일링 표 + "해상도 절반이면 어디가 얼마나" 예측-측정-오차

### 4.4 Block 4 — 3.33 Hz 의 조건부 판정 (2026.08, 재측정과 병합)

- [ ] "3.33 Hz 면 충분한가" 를 task 유형별 (정적 pick / 동적 추적) 조건부로 답변 + action chunking 이 간극을 메우는 방식
