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

> 골격: §1 의 미해소 항목 — "비트 수가 크면 덜 손실" 직관과 반대인 이유 — 를 LLM.int8() 과 NF4 의 계산 경로 차이로 설명한다. Block 1 과 달리 새 측정은 없다: **예측 → 측정 → 오차 설명** 의 측정 자리에 OpenVLA 논문 (arXiv 2406.09246 v3) Table 2·§5.4 의 A5000 공개 수치가 들어간다. fp16 은 4070 에서 로드 불가 (Block 1 Step 1: 15.08 GB > 12 GB) 이므로 fp16 대비 속도도 논문 수치로만 대조한다. 통과 기준은 plan Task 2-1 Step 2: int8 이 int4 보다 느리고 부정확한 메커니즘 답변 + "fp16 대비 int4 속도" 방향 예측. 예상 배분 (총 3-4h): Step 1 약 15분, Step 2 약 1.5-2h, Step 3-5 약 1.5h.

- [x] **Step 1 — 직관이 깔고 있는 가정 체크 (정독 전에 확인)**

  "8bit 이 4bit 보다 정보를 더 담으니 덜 손실" 직관이 깔고 있는 가정을 아래 항목으로 제시한다. 정독 전의 본인 생각과 일치하는 항목에 체크한다 — 체크 상태가 이후 Step 의 판정 대상이고, Step 4 마지막 줄 ("직관이 어느 지점에서 틀렸는가") 은 체크한 가정 중 무엇이 깨졌는지로 답한다.

  - [x] 가정 1: int8 과 int4 는 저장 비트 수만 다르고, forward 계산 경로 (양자화 대상, 계산 dtype, matmul 횟수) 는 같다 (→ Step 2·3 에서 판정)
  - [x] 가정 2: 로봇 성공률은 모델 출력 정확도만으로 결정되고, 추론 속도는 성공률에 영향을 주지 않는다 (→ Step 4 에서 판정)

  -> 직관적으로는 더 작은 자료형으로 데이터를 변환하게 되면 손실되는 부분이 많아 추론이 더 어려워질 것으로 예상한다. 4bit로 처리하게 되면 정확도는 낮아지지만 처리 속도는 올라 갈 것으로 예상한다.

- [ ] **Step 2 — 선행 정독 (해당 절만, 약 1.5-2h)**

  정제 공식의 상수를 출처에서 먼저 읽었던 Block 1 과 같은 이유 — 두 경로의 구조는 유도가 아니라 설계 사실이다. 각 항목을 "읽을 범위 → 답할 질문 → 결과가 쓰이는 곳" 순서로 진행한다. 항목의 "답할 질문" 에 본인 문장으로 답할 수 있으면 그 항목은 끝난 것이고, 답 자체는 Step 3-5 의 해당 칸에 적는다.

  - [ ] **(a) LLM.int8() 논문 (arXiv 2208.07339) §2** — absmax·zeropoint 양자화 배경 (약 15-20분)
    - 읽을 범위: §2 (Background) 의 absmax quantization·zeropoint quantization 공식 두 개
    - 답할 질문: Block 1 에서 weight 블록 (64개) 마다 저장하던 absmax 와 같은 연산이 여기서는 activation 에 적용된다 — 그러면 이 양자화는 로드 시 1회가 아니라 언제마다 일어나는가? 그리고 outlier 값 하나가 absmax 를 키우면 같은 벡터의 나머지 값들은 왜 해상도를 잃는가 (§3 에서 outlier 분리가 필요해지는 이유)?
    - 결과가 쓰이는 곳: Step 3 표 "activation 전처리" 행 — "weight 양자화는 정적 (로드 시 1회), activation 양자화는 forward 마다의 런타임 비용" 구분이 이 행의 핵심

  - [ ] **(b) 같은 논문 §3** — vector-wise quantization + mixed-precision decomposition (약 30-40분)
    - 읽을 범위: §3 전체와 분해 절차 그림. 수식은 outlier 열을 분리해 fp16 matmul + int8 matmul 2회를 수행하고 합산하는 식 하나만 따라가면 된다
    - 답할 질문: forward 1회에 어떤 순서로 무엇이 실행되는가 — threshold 초과 열 검사, 초과 열의 fp16 matmul, 나머지의 int8 matmul, int8 결과의 dequant 후 합산. 이 중 int4 (NF4) 경로에는 없는 단계가 무엇인지 목록으로 만든다
    - threshold 기본값 확인 (venv 에서 1줄):

      ```python
      from transformers import BitsAndBytesConfig
      print(BitsAndBytesConfig(load_in_8bit=True).llm_int8_threshold)  # 기대: 6.0
      ```

    - 결과가 쓰이는 곳: Step 3 표 "matmul 횟수·종류"·"출력 후처리" 행

  - [ ] **(c) 같은 논문 §4** — outlier 의 규모 의존성 (약 15분)
    - 읽을 범위: §4 (emergent features) 와 규모별 확산 그림 — outlier 가 등장하는 층·시퀀스 비율이 모델 크기에 따라 어떻게 변하는지
    - 답할 질문: 약 6.7B 부터 무엇이 달라지는가 (전 층·전 시퀀스로 확산). OpenVLA 백본은 Llama-2 6.74B (Block 1 Step 1) — 그러면 (b) 의 outlier 분해 경로는 "가끔 발동하는 예외 처리" 인가, "매 forward·전 층에서 항상 지불하는 상수 비용" 인가?
    - 결과가 쓰이는 곳: Step 3 결론 (int8 이 느린 이유가 이 모델에서 항상 성립하는 근거) + Step 4 (무손실 주장의 전제 확인)

  - [ ] **(d) QLoRA 논문 §3 재독** — Block 1 에서 읽은 두 문단 (약 10-15분)
    - 읽을 범위: Block 1 Step 2 에서 읽은 "4-bit NormalFloat Quantization"·"Double Quantization" 두 문단. 이번에는 저장 상수 (블록 64 등) 가 아니라 계산 경로를 묻는 질문으로 다시 읽는다
    - 답할 질문: NF4 는 저장 dtype 일 뿐이다 — 그러면 matmul 자체는 어떤 dtype 으로 수행되는가 (이 실험은 fp16, environment.md)? "int4 산술 연산은 존재하지 않는다" 를 논문 문장으로 뒷받침할 수 있는가?
    - compute dtype 확인 (Block 1 의 Linear4bit 코드 재사용):

      ```python
      lin = next(m for m in vla.modules() if type(m).__name__ == "Linear4bit")
      print(lin.compute_dtype)  # 기대: torch.float16 (environment.md 와 일치)
      ```

    - 결과가 쓰이는 곳: Step 3 표 NF4 열 전체 + Step 5 (b) 의 dequant 오버헤드 예측 근거

  - [ ] **(e) bitsandbytes 0.49.2 코드로 두 forward 경로 대조** (약 30-40분)

    (a)-(d) 가 논문의 주장이라면 이 항목은 실제 설치본에서의 확인이다. 아래를 실행해 두 소스를 출력한다:

    ```python
    import inspect
    import bitsandbytes as bnb
    # int8 경로: forward 마다 activation 양자화 + outlier 열 분리 + matmul 2회
    print(inspect.getsource(bnb.autograd._functions.MatMul8bitLt.forward))
    # int4 경로: 단일 토큰 (decode) 이면 dequant fused GEMV 1회, 아니면 dequant 후 fp16 matmul
    print(inspect.getsource(bnb.matmul_4bit))
    ```

    - `MatMul8bitLt.forward` 에서 찾을 것 (함수명은 버전마다 달라지므로 이름이 아니라 역할로 찾는다):
      1. activation 을 int8 로 양자화하는 호출 — (a) 의 "forward 마다" 가 코드로 보이는 지점
      2. threshold 초과 열을 분리하는 지점 — (b) 의 분해가 코드로 보이는 지점
      3. matmul 이 2회인 것 — int8 matmul 과 outlier fp16 matmul
      4. int8 matmul 출력을 fp16 으로 되돌리는 dequant
    - `matmul_4bit` 에서 찾을 것:
      1. 단일 토큰 분기 — A 의 원소 수가 마지막 차원 크기와 같은지 (= 토큰 1개) 를 검사하는 조건. decode (action 토큰 생성) 가 어느 경로를 타는지 확정
      2. 두 경로 모두에서 weight 가 fp16 으로 dequant 된 뒤 matmul 되는 것 — "int4 끼리의 matmul" 이 어디에도 없음을 확인 ((d) 의 코드 버전)
    - 결과가 쓰이는 곳: 찾은 지점 번호를 Step 3 표의 해당 행에 근거로 인용한다 (예: "int8 경로 2·3 → matmul 2회")

- [ ] **Step 3 — 속도 메커니즘: Linear 1개 forward 의 경로 대조표**

  아래 표의 빈 칸을 Step 2 정독 내용으로 채우고, "int8 이 int4 보다 느린 이유" 를 본인 문장 2-3줄로 결론 짓는다. 관점: 읽어야 하는 가중치 바이트 (N_q = 7.28B 기준, Block 1 Step 2 재사용), 런타임 추가 연산, 커널/matmul 횟수.

  | 관점 | LLM.int8() 경로 | NF4 경로 |
  |---|---|---|
  | 가중치 메모리 읽기 | | |
  | activation 전처리 | | |
  | matmul 횟수·종류 | | |
  | 출력 후처리 | | |

  -> (본인 작성)

- [ ] **Step 4 — 부정확 메커니즘: 성공률 58.1% 의 인과 경로**

  LLM.int8() 논문은 언어 벤치마크에서 성능 무손실을 주장하는데, OpenVLA Table 2 의 int8 성공률은 58.1% 로 유의하게 낮다. 두 사실이 모순되지 않는 이유를 본인 문장으로 쓴다 — §1 에 이관된 판단 (논문 §5.4: 속도 하락이 시스템 동역학을 바꿈) 을 "표현 손실 지표 (perplexity)" 와 "closed-loop 지표 (성공률)" 의 구분으로 다시 서술하고, "비트 수 직관" 이 정확히 어느 지점에서 틀렸는지 한 줄로 마무리한다.

  -> (본인 작성)

- [ ] **Step 5 — "fp16 대비 int4 속도" 방향 예측 → Table 2 대조**

  예측을 먼저 적는다. 경쟁하는 두 효과 중 어느 쪽이 이기는지, 왜인지:
  - (a) 가중치 메모리 트래픽 약 1/4 — decode 는 forward 당 토큰 1개라 memory-bound 에 가까움 (실측 확인은 Block 3 으로 연결)
  - (b) dequant 오버헤드 — 매 forward 마다 커널 안에서 반복

  방향 (빠르다/느리다/비슷하다) 과 근거를 적은 뒤, 논문 수치와 대조하고 오차를 설명한다:
  - 뽑을 수치: A5000 기준 bf16 과 int4 의 추론 속도 (Hz) 2개 — §1 이 int8 1.2 Hz 를 인용한 자리와 같은 곳. Table 2 에 속도 열이 없으면 §5.4 본문에서 찾는다
  - 대조 방법: 절대값이 아니라 int4/bf16 비율로 비교한다 — A5000 과 4070 은 대역폭이 달라 절대값은 이식되지 않는다
  - prefill (vision token 포함 수백 개) 과 decode (action 토큰 7개) 의 비중이 결과에 미치는 영향은 Block 3 과 연결되는 지점으로 한 줄 남긴다

  -> 예측:
  -> 논문 수치 대조:
  -> 오차 설명:

완료 판정 표 — 각 행이 본인 문장으로 채워지면 Block 2 완료:

| 질문 | 답변 요지 (본인) | 대조 근거 |
|---|---|---|
| int8 이 int4 보다 느린 메커니즘 | | Step 3 표 + Table 2 (int8 1.2 Hz) |
| int8 성공률 58.1% 의 인과 경로 | | 논문 §5.4 + LLM.int8() 무손실 주장 |
| fp16 대비 int4 속도 방향 | | Table 2·§5.4 (bf16 vs int4, A5000) |
| §1 미해소 항목 한 줄 답 | | 위 세 행의 종합 |

### 4.3 Block 3 — 300 ms 구간 분해 (2026.08, 재측정과 병합)

> 골격: 실측값 하나 (mean 300.3 ms) 를 구간으로 쪼개 병목을 특정하고, 병목 가설을 "해상도 절반" 개입으로 검증한다. Block 1 과 같은 분업 — 측정 조건·수치 원본은 methodology.md 재측정 절 (신설) 과 raw/·scripts/ 에 두고, 이 절에는 예측 논리와 오차 해석만 본인 문장으로 쓴다. methodology §3 의 미기록 항목 (notebook → `.py`, `summary.csv`, 실제 카메라 이미지 조건) 과 같은 세션에서 병합 수행. 통과 기준은 plan Task 2-2 Step 1: 구간 분해표 + "해상도 절반이면 어디가 얼마나" 예측-측정-오차. 예상 배분 (총 4-6h): Step 1 약 1h, Step 2-3 약 2-3h, Step 4-5 약 1.5h.

- [ ] **Step 1 — 측정 전 예측: 300 ms 의 구간 배분 손계산**

  구간은 5개: 전처리 (processor) / vision encoder / LLM prefill 1회 / LLM decode 7회 (action 토큰 7개 = xyz 3 + rpy 3 + gripper 1) / un-norm. 예측치는 아래 사다리 표의 "예측" 열에 채운다. 각 구간의 예측 근거:
  - 전처리: week8 stress test 에서 무시 수준 확인 (§2) — 근사 0 예측
  - vision encoder: visual token 수 손계산 — patch 14 기준 (224/14)^2 = 256. 실제 token 수는 Step 2 훅에서 projector 출력 shape 으로 교차 검증
  - decode 하한 공식 (roofline): 토큰당 ≈ LLM 상주 가중치 바이트 / 대역폭 504 GB/s (§2). LLM 바이트는 Block 1 수치에서 vision encoder 분 (0.731B, N_q 에 포함 — Block 1 Step 3 에서 양자화 확인됨) 을 빼고 재산출 — decode 는 vision encoder 가중치를 읽지 않는다
  - prefill: 가중치 읽기는 decode 1회와 동일 + 시퀀스 약 280 토큰 (visual 256 + prompt, 정확한 길이는 input_ids 로 확인) 의 연산 — compute 비중이 커지는 구간
  - un-norm: 7원소 벡터 연산 — 근사 0

  하한 합계와 실측 300.3 ms 의 간극 자체가 Step 3 오차 설명의 대상이다 (후보: eager attention, dequant 커널 효율, 커널 launch·CPU 측 generate 루프 오버헤드 — 예단하지 말고 실측 후 배분).

- [ ] **Step 2 — 계측 설계 (스크립트 작성, `scripts/` 저장)**

  `predict_action` 내부를 수정하지 않고 구간을 나누는 방법을 정하고 스크립트로 만든다 (notebook → `.py` 전환과 병합 — methodology §3). 파일명은 기존 관례 (`scripts/memory_breakdown.py`) 를 따라 `scripts/latency_breakdown.py` 권장. 작업 순서:

  1. 경계 모듈 확인: `[n for n, _ in vla.named_children()]` 로 최상위 자식 이름을 출력해 vision_backbone / projector / language_model 에 해당하는 실제 모듈명을 확정한다 — hook 은 이 세 모듈에 건다
  2. hook + sync 골격 — CUDA 호출은 비동기 큐라 sync 없이는 경계 시각이 의미 없다 (자가 검증 문항 "synchronize" 의 답이 되는 지점):

     ```python
     import time
     marks = []  # (라벨, 시각) 누적 -- 실행 후 인접 쌍의 차로 구간 산출

     def mark(label):
         torch.cuda.synchronize()
         marks.append((label, time.perf_counter()))

     vla.vision_backbone.register_forward_pre_hook(lambda m, a: mark("vision_in"))
     vla.vision_backbone.register_forward_hook(lambda m, a, o: mark("vision_out"))
     # projector, language_model 도 같은 패턴
     ```

  3. prefill/decode 분리: language_model 은 `generate` 안에서 여러 번 호출된다 — hook 이 쌓은 language_model 구간 중 1회차가 prefill, 2회차 이후가 decode. 호출 횟수가 예상 (prefill 1 + action 토큰 7 = 8회) 과 일치하는지 먼저 확인하고, 다르면 그 이유를 기록한다
  4. hook 이 못 잡는 구간 처리: 전처리 (processor) 는 `predict_action` 밖이므로 호출 전후에서 직접 mark. un-norm 은 내부 말미라 hook 지점이 없다 — "전체 실측 − 잡힌 구간 합" 의 잔차로 귀속하고 예측 (근사 0) 과 비교
  5. 검증 기준: 구간 합 ≈ sync 미삽입 단독 실측 (재측정 mean). sync 는 파이프라인을 세우므로 hook 있는 실행과 없는 실행을 분리해 둘 다 기록하고, 차이를 계측 오버헤드로 명시
  6. n=100·warm-up 5회 등 조건은 methodology §1 과 동일 유지 — 2026-06 값과 비교 가능해야 재측정의 의미가 있다

- [ ] **Step 3 — 실측 + 오차 설명**

  구간별 실측치를 methodology 재측정 절에 기록하고 (raw/ 에 구간별 배열 저장, `summary.csv` 생성), 사다리 표의 예측 대비 오차를 구간별로 본인 문장으로 설명한다. decode 실측 (토큰당 ms) 은 Block 2 Step 5 의 memory-bound 가설 검증이기도 하다 — 하한 대비 몇 배인지 명시. 부수 한 줄: decode 가 대역폭 지배가 맞으면 Jetson 이식 예측 (대역폭 비례 외삽) 이 이 수치에서 나온다 (자가 검증 문항 "Jetson").

- [ ] **Step 4 — "해상도 절반이면 어디가 얼마나" 예측 → 측정 → 오차**

  사전 확인이 먼저다: 112x112 pixel_values 가 vision encoder 를 통과하는지 (ViT positional embedding 이 224 고정일 수 있음). 확인 방법:
  - 경로 구분: processor 에 112x112 이미지를 주는 방식은 processor 가 자체 리사이즈하면 무효다 — processor 출력 pixel_values 의 shape 을 print 해 실제로 112 인지 먼저 확인하고, 아니면 pixel_values 직접 조작 (`torch.nn.functional.interpolate` 로 224 → 112 축소) 으로 전환한다. OpenVLA 는 SigLIP + DINOv2 이중 인코더라 pixel_values 채널 구성이 3채널이 아닐 수 있다 — shape 을 보고 판단
  - 단독 통과 시험: 축소한 pixel_values 를 `vla.vision_backbone` 에 단독으로 넣어 본다. 예외가 나면 traceback 이 가리키는 지점 (positional embedding 크기 불일치인지) 을 기록

  통과 시 — 측정 전에 예측을 먼저 적는다. 예측을 만들며 답할 질문:
  - visual token 수는 몇 개가 되는가 — Step 1 에서 224 기준으로 했던 patch 산수를 112 로 반복
  - 그 변화가 세 구간의 비용에 어떻게 전파되는가 — vision encoder 의 연산량, prefill 의 시퀀스 길이, decode 가 읽는 가중치 바이트는 각각 token 수에 의존하는가?
  - 그래서 아래 표의 세 행 중 어디가 줄고 어디가 그대로여야 하는가?

  측정 후 아래 표를 채운다. 통과 불가 시 — 시도한 방법과 실패 지점을 기록하고 예측만 남긴 채 "구조 제약으로 측정 불가" 로 종결 (Block 1 Step 5 의 미확인 종결과 같은 방식).

  | 구간 | 변화 예측 | 변화 실측 | 오차 해석 |
  |---|---|---|---|
  | vision encoder | | | |
  | LLM prefill | | | |
  | LLM decode | | | |

- [ ] **Step 5 — 측정 조건 부수 검증 3종 (자가 검증 문항 선행)**

  - warm-up 근거: cold start 포함 첫 10회 latency 나열 — 몇 회부터 정상 상태인지, 5회로 충분한 근거를 수치로
  - synchronize 필요성: sync 제거 1회 시연 — 측정값이 무엇으로 붕괴하는지 (launch 시간만 남음) 기록
  - n=100 분산: 재측정 p50/p95/p99 를 2026-06 값 (methodology §2) 과 대조 — 재현성 판단

사다리 표 — 각 칸이 채워지면 Step 4 표·methodology §3 체크박스와 함께 Block 3 완료:

| 구간 | 예측 | 실측 | 오차 해석 |
|---|---|---|---|
| 전처리 (processor) | | | |
| vision encoder | | | |
| LLM prefill (1회) | | | |
| LLM decode (7 토큰) | | | |
| un-norm | | | |
| 합계 | | 재측정 mean 과 대조 | 계측 오버헤드 포함 |

### 4.4 Block 4 — 3.33 Hz 의 조건부 판정 (2026.08, 재측정과 병합)

> 골격: §2 의 미해소 항목 — 제어 계층 구조와의 연결, action chunking 의 역할 — 을 답하고 "3.33 Hz 면 충분한가" 를 task 유형별 조건부로 판정한다. 새 측정 없음 — 입력은 Block 3 의 구간 분해 결과, §2 의 전체 루프 하한 (2 Hz, step 500 ms), 공개 문헌이다. 판정 기준은 predict 단독 3.33 Hz 가 아니라 시스템 전체 루프 2 Hz 로 잡는다. 통과 기준은 plan Task 2-2 Step 2: task 유형별 조건부 답변. 예상 배분 (총 2-3h): 정독 약 1h, 작성 약 1-1.5h.

- [ ] **Step 1 — 선행 정독 (해당 절만, 약 1h)**

  Block 2 Step 2 와 같은 진행 방식 — 항목마다 "읽을 범위 → 답할 질문 → 결과가 쓰이는 곳". 답할 질문에 본인 문장으로 답할 수 있으면 항목 완료이고, 답 자체는 Step 2-4 의 칸에 적는다.

  - [ ] **(a) ACT 논문 (arXiv 2304.13705) — action chunking + temporal ensembling (약 30-40분)**
    - 읽을 범위: Method 중 action chunking 과 temporal ensembling 을 정의하는 절과 청크 실행 타임라인 그림. CVAE 등 모델 구조 세부는 이 Block 과 무관 — 읽지 않는다
    - 답할 질문: 청크 크기 k 일 때 정책 호출 주파수와 action 실행 주파수는 각각 어떻게 되는가? 청크를 실행하는 도중 들어온 새 관측은 행동에 반영되는가, 다음 호출까지 버려지는가? temporal ensembling 은 무엇을 평균하고, 그것이 완화하는 문제는 무엇인가?
    - 결과가 쓰이는 곳: Step 4 (실행 주파수 vs 반응 주파수 구분이 이 질문들의 답으로 만들어진다) + 완료 판정 표 "chunking 이 메우는 것" 행
  - [ ] **(b) OpenVLA 논문 §5.4 — blocking controller 구동 조건 (약 15분)**
    - 읽을 범위: §5.4 에서 실로봇 구동 방식을 서술하는 문장 (blocking controller 언급 대역) — 절 전체가 아니라 해당 문장 전후
    - 답할 질문: blocking control 에서 관측 → 추론 → 실행은 어떻게 순차화되는가 — 추론 300 ms 동안 로봇과 환경은 각각 무엇을 하고 있는가? 이 전제가 유지되는 task 조건과 무너지는 조건은 무엇인가?
    - 결과가 쓰이는 곳: Step 2 표 "정책" 행 + Step 3 의 판정 기준 (반응 시간 정의의 출발점)
  - [ ] **(c) (선택) ros2_control 의 controller 주기 관례 (약 10분)**
    - 읽을 범위: 정독이 아니라 수치 확인 — controller_manager update rate 의 관례적 대역. 본인 ROS 경력에서 다룬 실제 수치가 있으면 그것으로 대체 가능
    - 답할 질문: 본인이 다뤄 본 저수준 루프는 몇 Hz 였고, Step 2 표의 어느 층에 해당하는가?
    - 결과가 쓰이는 곳: Step 2 표 "서보/전류 루프"·"궤적 보간·모션 컨트롤러" 행 — 문헌 수치가 아니라 본인 경험 수치로 채워도 되는 행

- [ ] **Step 2 — 제어 계층 표: 3.33 Hz 가 어느 층의 숫자인지**

  각 층의 주기·담당과 "정책이 느려도 되는 이유 / 느리면 안 되는 지점" 을 채운다. §2 미해소 전반부의 답:

  | 층 | 주기 | 담당 | 3.33 Hz 와의 관계 |
  |---|---|---|---|
  | 서보/전류 루프 | 수백 Hz - 1 kHz | | |
  | 궤적 보간·모션 컨트롤러 | | | |
  | 정책 (OpenVLA) | 3.33 Hz (루프 2 Hz) | | |

- [ ] **Step 3 — 판정 기준 정의 + task 유형별 판정**

  "충분한가" 를 정량 기준으로 바꾼다: 관측 → 행동 완료까지의 반응 시간 (루프 500 ms 기준) 동안 환경이 얼마나 변하는가 vs 허용 오차 (그리퍼 개구 여유 등). 정적 pick (PickCube: 물체 정지, 환경 변화 근사 0) / 동적 추적 (등속 이동 물체: 500 ms x 물체 속도만큼 관측이 낡음) 각각에 수치 예를 하나씩 만들어 판정한다.

  -> (본인 작성)

- [ ] **Step 4 — action chunking 이 메우는 것과 못 메우는 것**

  실행 주파수 (청크 내 action 을 하위 층이 소화하는 속도) 와 반응 주파수 (새 관측이 행동에 반영되는 속도) 를 구분해 답한다 — chunking 이 어느 쪽을 올리고 어느 쪽은 그대로인지, temporal ensembling 이 무엇을 완화하는지. §2 미해소 후반부의 답. 마지막에 Step 3 의 판정이 chunking 도입 시 어떻게 바뀌는지 한 줄.

  -> (본인 작성)

완료 판정 표 — 각 행이 본인 문장으로 채워지면 Block 4 완료:

| 질문 | 답변 요지 (본인) | 근거 |
|---|---|---|
| 3.33 Hz — 정적 pick | | Step 3 수치 예 |
| 3.33 Hz — 동적 추적 | | Step 3 수치 예 |
| chunking 이 메우는 것 / 못 메우는 것 | | ACT 논문 + Step 4 |
| §2 미해소 항목 한 줄 답 | | Step 2-4 종합 |
