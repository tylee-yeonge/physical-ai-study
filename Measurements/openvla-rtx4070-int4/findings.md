# 결과 해석·판단 근거 — openvla-rtx4070-int4

> 이 문서는 측정 수치의 해석과 그 위에서 내린 엔지니어링 판단을 기록한다. §1-§3 은 2026-06 실측 시점의 판단 기록 (출처 표기), §4 는 이해 검증 4블록 (2026.07-08) 에서 본인이 작성하는 영역이다.

## 1. int8 경로 배제 판단 (2026-06)

출처: `Studies/Phase 4/SETUP.md` §1.3

- int8 Bridge 성공률 58.1 ± 5.1% — bf16 (71.3%)·int4 (71.9 ± 4.7%) 대비 유의한 하락 (OpenVLA 논문 Table 2)
- int8 추론 속도 1.2 Hz (A5000) — 속도 하락이 시스템 동역학을 바꿔 성공률 하락으로 이어짐 (OpenVLA 논문 §5.4)
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
  -> allocator 예약 = 4.55GB - 4.38GB = 0.17GB : PyTorch caching allocator의 동작(cudaMalloc이 비싸서 큰 블록으로 미리 받아두고, 텐서가 free 되어도 블록을 CUDA에 반환하지 않고 캐시함)
  -> activation·KV cache = 4.73GB - 4.38GB = 0.35GB : 추론 중 vision encoder, LLM forward의 activation과 action 토큰 7개를 autoregressive로 생성하는 동안의 KV cache가 일시적으로 할당되었다가 해제되는데 사용한 최대치

사다리 표 — 각 칸이 채워지면 Block 1 완료:

| 단계 | 예측 | 실측 | 오차 해석 |
|---|---|---|---|
| fp16 손계산 | 15.1GB | — (로드 불가) | 현재 갖고있는 GPU(4070 12GB)의 크기를 초과해서 실측이 불가능함 |
| int4 나이브 | 3.77GB | — | 정제 예측과는 0.514GB 차이 발생 - 나이브 계산 대비 fp16 잔존 내용이 파라미터 당 1.5byte 늘어나 0.398GB, scale 오버헤드가 0.116GB(absmax scale = 0.114GB + 2차 scale = 0.002GB) |
| int4 정제 예측 | 4.284GB | 4.38GB (`memory_allocated`) | 예측 공식이 다루지 않는 텐서 종류 0.036GB + 텐서를 열거해도 잡히지 않는 장부상 잔차 0.06GB (합 0.096GB = 4.38 - 4.284) |
| nvidia-smi (로드 직후) | 4.74GB (`memory_reserved`(B): 4.55 + `context`: 0.19) | 4.74GB | 오차 없음, 로드 시점에는 allocator 밖 할당이 없음 |
| nvidia-smi (추론 후) | 5.03GB (`memory_reserved`(C): 4.84 + `context`: 0.19)| 5.05GB | 추론 중 allocator 밖 할당이 생긴 것으로 추정 (cuBLAS workspace 등 후보, 미확인) |

- [x] **Step 5 — 논문 "약 7GB" 와의 간극 해석 (사다리 표 밖 잔여분, Task 2-2 와 병행)**

  SETUP.md §1.3 은 int4 추론 VRAM 을 "약 7GB (논문 실측)" 으로 인용하는데, 본 실측은 디바이스 기준 5.05 GB (추론 후) 다. 

  **원문 확인** — 수치는 OpenVLA 논문 (arXiv 2406.09246 v3) §5.4 "Memory-Efficient Inference via Quantization" 의 **Table 2** 에 있다: bfloat16 16.8 GB / int8 10.2 GB / int4 7.0 GB 

  **코드 확인** 
  - 메모리 측정 코드는 리포에 없음 → 측정 기준은 코드로도 확정 불가, 미확인으로 종결

### 4.2 Block 2 — int8 열위 메커니즘 (2026.07)

> 골격: §1 의 미해소 항목 — "비트 수가 크면 덜 손실" 직관과 반대인 이유 — 를 LLM.int8() 과 NF4 의 계산 경로 차이로 설명한다. Block 1 과 달리 새 측정은 없다: **예측 → 측정 → 오차 설명** 의 측정 자리에 OpenVLA 논문 (arXiv 2406.09246 v3) Table 2·§5.4 의 A5000 공개 수치가 들어간다. fp16 은 4070 에서 로드 불가 (Block 1 Step 1: 15.08 GB > 12 GB) 이므로 fp16 대비 속도도 논문 수치로만 대조한다. 통과 기준은 plan Task 2-1 Step 2: int8 이 int4 보다 느리고 부정확한 메커니즘 답변 + "fp16 대비 int4 속도" 방향 예측. 예상 배분 (총 3-4h): Step 1 약 15분, Step 2 약 1.5-2h, Step 3-5 약 1.5h.

- [x] **Step 1 — 직관이 깔고 있는 가정 체크 (정독 전에 확인)**

  "8bit 이 4bit 보다 정보를 더 담으니 덜 손실" 직관이 깔고 있는 가정을 아래 항목으로 제시한다. 정독 전의 본인 생각과 일치하는 항목에 체크한다 — 체크 상태가 이후 Step 의 판정 대상이고, Step 4 마지막 줄 ("직관이 어느 지점에서 틀렸는가") 은 체크한 가정 중 무엇이 깨졌는지로 답한다.

  - [x] 가정 1: int8 과 int4 는 저장 비트 수만 다르고, forward 계산 경로 (양자화 대상, 계산 dtype, matmul 횟수) 는 같다 (→ Step 2·3 에서 판정)
  - [x] 가정 2: 로봇 성공률은 모델 출력 정확도만으로 결정되고, 추론 속도는 성공률에 영향을 주지 않는다 (→ Step 4 에서 판정)

  -> 직관적으로는 더 작은 자료형으로 데이터를 변환하게 되면 손실되는 부분이 많아 추론이 더 어려워질 것으로 예상한다. 4bit로 처리하게 되면 정확도는 낮아지지만 처리 속도는 올라 갈 것으로 예상한다.

- [x] **Step 2 — 선행 정독 (해당 절만, 약 1.5-2h)**

  정제 공식의 상수를 출처에서 먼저 읽었던 Block 1 과 같은 이유 — 두 경로의 구조는 유도가 아니라 설계 사실이다. 각 항목을 "읽을 범위 → 답할 질문 → 결과가 쓰이는 곳" 순서로 진행한다. 항목의 "답할 질문" 에 본인 문장으로 답할 수 있으면 그 항목은 끝난 것이고, 답 자체는 Step 3-5 의 해당 칸에 적는다.

  - [x] **(a) LLM.int8() 논문 (arXiv 2208.07339) §2** — absmax·zeropoint 양자화 배경 (약 15-20분)
    - 읽을 범위: §2 (Background) 의 absmax quantization·zeropoint quantization 공식 두 개
    - 답할 질문: Block 1 에서 weight 블록 (64개) 마다 저장하던 absmax 와 같은 연산이 여기서는 activation 에 적용된다 — 그러면 이 양자화는 로드 시 1회가 아니라 언제마다 일어나는가? 그리고 outlier 값 하나가 absmax 를 키우면 같은 벡터의 나머지 값들은 왜 해상도를 잃는가 (§3 에서 outlier 분리가 필요해지는 이유)?
    - 결과가 쓰이는 곳: Step 3 표 "activation 전처리" 행 — "weight 양자화는 정적 (로드 시 1회), activation 양자화는 forward 마다의 런타임 비용" 구분이 이 행의 핵심
    - 기록
      - absmax
        - 1. 숫자 묶음에서 절대값 최대를 찾음
        - 2. 그 값으로 나눠 [-1, 1] 로 정규화한 뒤 127 을 곱해 int8 범위 [-127, 127] 로 스케일링 함
      - LLM.int8()에서는 absmax를 activation에 적용함
        - activation: 추론 시점에 네트워크를 통과하여 흘러가는 중간 계산 결과 텐서
      - Linear 레이어 하나가 하는 일은 Y = X @ W 임
        - W(weight): 학습으로 정해진 파라미터. 디스크에서 로드되는 그 숫자들, 로드 시 1회 호출(추론 중 고정됨)
        - X(activation): 지금 처리 중인 입력이 앞 레이어들을 거쳐 변환된 결과, forward 실행 중에만 호출(입력마다)
        - Y: 이 레이어의 출력 = 다음 레이어의 activation
      - NF4는 weight를 64블록으로 잘라 블록마다 absmax를 계산하고 저장함
        - absmax scale = N_q / 64 * 1byte
        - weight는 로드 후 변하지 않음 -> 그래서 양자화는 로드시 1회만 수행됨
        - forward에서는 저장해둔 absmax를 읽어서 dequant에 재사용만 함
          - 비용이 "저장공간"으로 지불되는 정적 연산임
      - LLM.int8()은 activation에도 absmax 연산이 사용됨
        - LLM.int8()은 진짜 int8 * int8 matmul을 수행하는 방식
        - (matmul) Y = X @ W 에서 W(weight)와 X(activation) 모두 int8이어야 함
        - X는 입력 이미지 & 토큰이 바뀔 때마다, 그리고 decode 중 토큰 하나 생성할 때마다 달라지므로 absmax를 미리 계산할 수 없음
          - 매 forward, Linear 레이어에서 absmax를 새로 계산하고 양자화하는 연산이 반복됨
      - NF4 경로는 activation을 아예 양자화 하지 않음
        - NF4 경로: Linear 레이어 forward 1회가 실제로 실행하는 연산 절차 전체
        - NF4 경로에서 activation은 fp16 그대로 두고, weight를 fp16으로 dequant 한 뒤 fp16 matmul을 함
          - dequant(dequantization, 역 양자화): 정수로 압축 저장된 값을 다시 float로 되돌리는 연산
            - 양자화: float -> 정수 * scale
            - 역 양자화: 정수 * 저장해둔 scale -> float 근사 값
        - NF4 경로가 forward 1회에 하는 일
          - 1. weight는 NF4(4bit)로 GPU에 저장 - 로드 시 1회 양자화된 상태
          - 2. forward가 호출되면, 저장해둔 블록별 absmax를 사용해 weight를 fp16으로 dequant 함
          - 3. 들어온 fp16 activation과 fp16 matmul을 수행
          - 4. 출력도 fp16 
      - "activation에 적용된다"의 근거
        - LLM.int8() 논문 §2.1 에 명시
          - absmax 양자화 공식: Xi8 = [127 * X_f16 / max|X_f16|]
            - 양자화 대상이 X(입력 행렬, activation)임
        - X와 W를 양자화 함을 명시 -> LLM.int8() 논문 §3.1
          - "은닉 상태 X_f16 ∈ R^(b×h) 와 가중치 행렬 W_f16 ∈ R^(h×o)가 주어지면, X_f16의 각 행에 서로 다른 스케일링 상수 cx를, W_f16의 각 열에 cw를 배정할 수 있다"
            - X를 은닉 상태라고 명시함. 따라서 X쪽 absmax 계산은 forward 마다 반복됨.
      - outlier 하나의 absmax를 키우면 나머지가 해상도를 잃는 이유
        - LLM.int8() 논문 §3
          - 텐서 당 하나의 스케일링 상수를 쓰는 양자화 방법의 주된 문제는, 단 하나의 이상치가 나머지 모든 값의 양자화 정밀도를 떨어뜨릴 수 있음


  - [x] **(b) 같은 논문 §3** — vector-wise quantization + mixed-precision decomposition (약 30-40분)
    - 읽을 범위: §3 전체와 분해 절차 그림. 수식은 outlier 열을 분리해 fp16 matmul + int8 matmul 2회를 수행하고 합산하는 식 하나만 따라가면 된다
    - 답할 질문: forward 1회에 어떤 순서로 무엇이 실행되는가 — threshold 초과 열 검사, 초과 열의 fp16 matmul, 나머지의 int8 matmul, int8 결과의 dequant 후 합산. 이 중 int4 (NF4) 경로에는 없는 단계가 무엇인지 목록으로 만든다
    - threshold 기본값 확인 (venv 에서 1줄):

      ```python
      from transformers import BitsAndBytesConfig
      print(BitsAndBytesConfig(load_in_8bit=True).llm_int8_threshold)  # 기대: 6.0
      ```

    - 결과가 쓰이는 곳: Step 3 표 "matmul 횟수·종류"·"출력 후처리" 행

    - 기록
      - forward 1회에 어떤 순서로 무엇이 실행되는가?
        1) fp16 activation X가 도착
        2) X의 열들을 스캔해 threshold(α=6.0) 초과 열 집합 O 확정
        3) X를 O열, 나머지 열로 분할
        4) 나머지 열을 absmax로 int8 양자화 
          - forward 마다 양자화가 수행됨
        5) O 열은 fp16 matmul 수행 (수식의 첫째항)
        6) 나머지 열은 int8 matmul 수행 (수식의 둘째항)
        7) int8 결과(int32 누적)를 S로 dequant
        8) 두 결과 합산 
        - 정리: 분해 -> 이상치는 16비트로 곱함 + 나머지는 스케일링 한 후 int8로 양자화하여 곱함 + 역 양자화 -> 합산

  - [x] **(c) 같은 논문 §4** — outlier 의 규모 의존성 (약 15분)
    - 읽을 범위: §4 (emergent features) 와 규모별 확산 그림 — outlier 가 등장하는 층·시퀀스 비율이 모델 크기에 따라 어떻게 변하는지
    - 답할 질문: 약 6.7B 부터 무엇이 달라지는가 (전 층·전 시퀀스로 확산). OpenVLA 백본은 Llama-2 6.74B (Block 1 Step 1) — 그러면 (b) 의 outlier 분해 경로는 "가끔 발동하는 예외 처리" 인가, "매 forward·전 층에서 항상 지불하는 상수 비용" 인가?
    - 결과가 쓰이는 곳: Step 3 결론 (int8 이 느린 이유가 이 모델에서 항상 성립하는 근거) + Step 4 (무손실 주장의 전제 확인)

    - 기록
      - Q1. 8페이지 4.2 "주요 정량적 결과" (1) 문단 - 6B -> 6.7B 사이에서 변하는 두가지 비율
        1) 영향을 받는 계층 비율: 65% -> 100%
        2) 영향을 받는 시퀀스 차원의 비율: 35% -> 75% => 전체 토큰 위치 중 outlier 특성 열이 실제로 큰 값 (크기 6이상)을 찍는 위치의 비율
      - Q2. Figure 3 캡션 - (a) 파라미터 축과 (b) 퍼플렉시티 축에서 상전이의 모양이 서로 다른 단어로 서술됨. 각각 단어의 의미는? 그리고 (a)의 서술이 "6.7B 근처에 있는 모델"에 대한 판단을 더 강하게 하는 이유는?
        - (a)는 파라미터 크기에 대한 급격한 상전이를 나타냄 -> 특정 시점에서 급격하게 값이 변함
        - (b)는 퍼플렉시티가 감소함에 따른 점진적, 지수적 상전이를 나타냄 -> 전 시점이 연속적으로 값이 변함
        - 퍼플렉시티
          - 언어 모델은 매 순간 "다음 토큰"의 확률 분포를 출력함
          - 검증 텍스트의 실제 정답 토큰들에 모델이 부여한 확률을 모아서 만든 점수
        - (a)의 서술이 "6.7B 근처에 있는 모델"에 대한 판단을 더 강하게 하는 이유
          - 상전이가 급격하므로 문턱을 막 넘은 6.74B는 영향을 받는 계층의 비율은 100%
          - 영향을 받는 시퀀스 차원이 비율은 75%로 변경되었음을 알 수 있고, 따라서 매 forward 및 전 층에서 항상 지불하는 상수 비용이라는 것을 알 수 있음 (-> Q5 에서 어미를 "추정됨" 으로 교정)
          - 만약 상전이가 완만했다면 판정이 애매했을 수 있음
      - Q3. 7페이지 끝 4.1장 - 논문이 outlier를 정의하는 기준 3개는? 그 중 "크기" 기준의 수치를 찾고 5페이지 3.2장의 분해 threshold α값과 비교했을 때 두 수의 관계는?
        - outlier 정의 기준
          1) 특성의 크기가 최소 6.0
          2) 전체 계층의 최소 25%에 영향을 줌
          3) 시퀀스 차원의 최소 6% 영향을 줌
        - threshold α 값은 outlier를 잘 검출할 수 있는 수치로 지정한 것이라 동일
      - Q4. Q3에서 확인했듯 outlier 정의의 크기 기준(6.0)과 분해 threshold α (6.0) 같은 수라면 Q1의 수치("계층 100%에서 outlier 발생")는 step2 (b)의 forward 순서 목록 중 정확히 어느 단계가 얼마나 자주 발동되는지를 말해주는 통계가 됨. 어느 단계인가?        
        - Q1의 "계층 100%에서 outlier 발생"은 어느 층의 forward에서도 스캔 결과 O가 비어있지 않다는 뜻임. 따라서 O ≠ ∅ 일 때만 발동하는 단계들(5. O열 fp16 matmul + 동반: 3. 분리, 마지막 합산)은 특정 층이나 특정 입력에서만이 아니라 모든 층, 매 forward에서 발동함.
      - Q5. 6페이지 3.3장 - 이 통계가 측정된 모델 계열은 무엇인가?
        - OPT: 제로샷 측정, fairseq: 언어 모델링 설정
        - Llama-2는 논문이 나온 후 출시되어 논문에 명시되어있지 않음
          - 계층 100%, 시퀀스 75%라는 통계는 fairseq 계열과 OPT에서 측정된 것이고, Llama-2에서 직접 측정한 적은 없음. 따라서 이 통계를 Llama-2 6.74B에 적용하는 것은 측정한 사실이 아니라 "같은 규모 대역의 Transformer 니까 같은 현상이 있을 것" 이라는 가정에 기댄 추론임. 그러므로 판정문의 어미는 "상수 비용이다 / 임을 알 수 있다"가 아니라 "상수 비용으로 추정된다"(추론의 어미)로 씀

      - 질문의 답
        1) 약 6.7B 부터 무엇이 달라지는가 -> 영향을 받는 계층 비율(65% -> 100%)과 영향을 받는 시퀀스 차원의 비율(35% -> 75%)이 크게 달라짐. 
        2) outlier 분해 경로 -> 계층 100%에서 outlier가 발생하기 때문에 O는 비어있지 않음. O ≠ ∅이기 때문에 매 forward·전 층에서 항상 지불하는 상수 비용으로 추정됨.

  - [x] **(d) QLoRA 논문 §3 재독** — Block 1 에서 읽은 두 문단 (약 10-15분)
    - 읽을 범위: Block 1 Step 2 에서 읽은 "4-bit NormalFloat Quantization"·"Double Quantization" 두 문단. 이번에는 저장 상수 (블록 64 등) 가 아니라 계산 경로를 묻는 질문으로 다시 읽는다
    - 답할 질문: NF4 는 저장 dtype 일 뿐이다 — 그러면 matmul 자체는 어떤 dtype 으로 수행되는가 (이 실험은 fp16, environment.md)? "int4 산술 연산은 존재하지 않는다" 를 논문 문장으로 뒷받침할 수 있는가?
    - compute dtype 확인 (Block 1 의 Linear4bit 코드 재사용):

      ```python
      lin = next(m for m in vla.modules() if type(m).__name__ == "Linear4bit")
      print(lin.compute_dtype)  # 기대: torch.float16 (environment.md 와 일치)
      ```

    - 결과가 쓰이는 곳: Step 3 표 NF4 열 전체 + Step 5 (b) 의 dequant 오버헤드 예측 근거

    - 기록
      - Q1. 3장에서 "storage data type"(저장 dtype)과 "computation data type"(계산 dtype) 두 용어가 함께 등장하는 문장을 찾으시오. 논문은 각각 무엇으로 지정하는가?
        - QLoRA는 하나의 저정밀 저장 데이터 타입(보통 4비트)과 하나의 계산 데이터 타입(보통 BFloat16)을 가짐
      - Q2. 그 문장 주변 - forward / backward pass를 수행하기 위해 무엇을 무엇으로 바꾼다고 되어있는가?
        - 순전파와 역전파를 수행하기 위해 저장 데이터 타입을 계산 데이터 타입으로 역 양자화 함.
      - Q3. matmul의 두 피연산자 {X(activation)와 W(weight)}는 곱해지는 순간 각각 어떤 dtype인가?
        - X와 W의 dtype은 bf16임 (논문 기준. 본 실험은 fp16 -- Q4)
      - Q4. 논문의 계산 dtype과 본인 실험의 계산 dtype이 다름. 이 값은 어디서 정해지는 것인가?
        - 로드 시 'BitsAndBytesConfig'의 'bnb_4bit_compute_dtype'으로 지정
        - 미지정 시 기본 값은 torch.float32 -> fp16은 기본 값이 아니라 명시적 선택
        - 런타임 도달 확인: 'lin.compute_dtype' -> torch.float16
      - 질문 답변
        - matmul 은 계산 dtype으로 수행됨. 논문 기준 - BFloat16, 본인 실험 기준 - torch.float16. 미지정 시 기본값은 float32 — 본 실험의 fp16 은 명시적 선택
        - 순전파와 역전파를 수행할 때 저장 데이터 타입을 계산 데이터 타입으로 역 양자화 하기 때문에 int4 산술 연산은 존재하지 않음

  - [x] **(e) bitsandbytes 0.49.2 코드로 두 forward 경로 대조** (약 30-40분)

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
        - F.int8_double_quant(A.to(torch.float16), threshold=state.threshold)
        - F.int8_vectorwise_quant(A.to(torch.float16), threshold=state.threshold)        
      2. threshold 초과 열을 분리하는 지점 — (b) 의 분해가 코드로 보이는 지점
        - if state.threshold > 0.0:
      3. matmul 이 2회인 것 — int8 matmul 과 outlier fp16 matmul
        - torch.ops.bitsandbytes.int8_mixed_scaled_mm
      4. int8 matmul 출력을 fp16 으로 되돌리는 dequant
        - 논문의 분리/두 matmul/dequant/합산이 하나의 fused 커널로 묶여 있음
          - torch.ops.bitsandbytes.int8_mixed_scaled_mm
        - else 분기, threshold=0 으로 분해를 끈 경우
          - torch.ops.bitsandbytes.int8_scaled_mm.default
        - mixed: 혼합 정밀도, scaled: 스케일 복원(=dequant), mm: 행렬 곱
    - `matmul_4bit` 에서 찾을 것:
      1. 단일 토큰 분기 — `if A.numel() == A.shape[-1] and A.requires_grad == False and A.device.type != "hpu":`
        - 참 (토큰 1개) + hidden 이 blocksize 배수 -> `F.gemv_4bit(A, B.t(), out, state=quant_state)`.
          decode (action 토큰 7개를 1개씩 생성) 가 이 경로. 4096 % 64 == 0 이라 배수 조건 통과
        - 거짓 (다중 토큰) -> `MatMul4Bit.apply(...)`. prefill (visual 256 + prompt) 이 이 경로
        - 배수가 아니면 warn 후 `MatMul4Bit.apply` 로 폴백 (본 모델은 해당 없음)
      2. weight dequant 후 matmul — `matmul_4bit` 본문에는 없음 (라우팅 함수라 dequant 를 직접
        수행하지 않는다). 두 목적지에서 각각 확인:
        - else 경로 (`MatMul4Bit.forward`): `torch.nn.functional.linear(A, F.dequantize_4bit(B, quant_state).to(A.dtype).t(), bias)`
          -- 주석도 `# 1. Dequantize` / `# 2. MatmulnN` 으로 두 단계를 명시. B(=weight) 를
          dequantize_4bit 로 푼 뒤 `.to(A.dtype)` 로 A 와 같은 dtype (본 실험 fp16) 으로 맞춰
          linear 에 넘긴다 -- 곱셈 시점에 int4 피연산자는 없음
        - GEMV 경로 (`F.gemv_4bit`): Python 레벨에 dequant 호출이 없고 quant_state (absmax 등
          dequant 재료) 를 커널에 넘긴다 -> dequant 가 커널 내부에 fused.
          int8 쪽 `int8_mixed_scaled_mm` 과 같은 패턴
        - 결론: 두 경로 어디에도 int4 끼리 곱하는 호출이 없다. (d) 의 논문 문장
          (저장 dtype -> 계산 dtype 역양자화 후 16비트 matmul) 의 코드 버전

    - 결과가 쓰이는 곳: 찾은 지점 번호를 Step 3 표의 해당 행에 근거로 인용한다 (예: "int8 경로 2·3 → matmul 2회")

- [x] **Step 3 — 속도 메커니즘: Linear 1개 forward 의 경로 대조표**

  **선행 개념 (표를 채우기 전에 확인)**

  - **경로**(path): Linear 레이어 forward 1회가 실제로 밟는 코드 실행 절차 전체. 두 열은 서로 다른 두 함수다 — int8 열은 `MatMul8bitLt.forward`, NF4 열은 `matmul_4bit` (Step 2 (e) 에서 읽은 그 소스). 로드 설정 (`load_in_8bit` / `load_in_4bit`) 이 어느 클래스를 쓸지 정하고, 그 클래스의 forward 가 곧 경로다
  - **표의 칸에 넣을 것**: 코드가 아니라 "그 경로가 이 관점에서 무엇을 하는가" 한두 구절 + 근거 번호. 코드 전문은 이미 Step 2 (e) 기록에 있으므로 표에서는 번호로 참조한다 (예: "fused 커널 1회 — (e) int8 3")
  - **"읽어야 하는 바이트" 의 뜻**: 저장 용량이 아니라 forward 마다 GPU 가 실제로 읽어 오는 트래픽. decode 는 토큰 1개를 낼 때마다 전체 가중치를 다시 읽으므로, 저장 크기 차이가 매 forward 반복된다

  **칸별 재료 위치** (내용이 아니라 어느 기록에서 가져올지)

  | 관점 | int8 칸 | NF4 칸 |
  |---|---|---|
  | 가중치 메모리 읽기 | N_q × 1 byte (Block 1 의 N_q = 7.28B) | N_q × 0.5 byte + scale 오버헤드 (Block 1 Step 2 의 absmax·2차 scale 항) |
  | activation 전처리 | (a) 의 결론 + (e) int8 1 — 무조건 실행인지 조건부인지 포함 | (d) 의 결론 — X 를 건드리는 호출이 있는가 없는가 |
  | matmul 횟수·종류 | (e) int8 3 (fused 커널) + (c) 의 발동 빈도 | (e) int4 1 (decode/prefill 두 경로) + (e) int4 2 |
  | 출력 후처리 | (e) int8 4 | (d) + (e) int4 2 — 해당 단계가 존재하는가 |

  아래 표의 빈 칸을 위 재료로 채우고, "int8 이 int4 보다 느린 이유" 를 본인 문장 2-3줄로 결론 짓는다.

  | 관점 | LLM.int8() 경로 (`MatMul8bitLt.forward`) | NF4 경로 (`matmul_4bit`) |
  |---|---|---|
  | 가중치 메모리 읽기 | N_q × 1 byte | N_q × 0.5 byte + scale 오버헤드 |
  | activation 전처리 | 매 forward 무조건 absmax 로 int8 양자화 -- 조건 분기 없이 실행되고, weight 쪽 양자화는 `if` 안이라 캐시됨 (정적 1회 vs 매 forward 반복) — (a), (e) int8 1 | 없음 -- X 는 fp16 그대로 matmul 에 들어감. 4bit 경로에 A 를 양자화하는 호출이 없고, `.to(A.dtype)` 로 W 를 A 에 맞춘다 — (d), (e) int4 2 |
  | matmul 횟수·종류 | int8_mixed_scaled_mm 하나에 int8 matmul + outlier fp16 + dequant 가 fused. Python 호출은 1회 — (e) int8 3, 발동 빈도는 (c) | decode → gemv_4bit (dequant 커널 내부 fused), prefill → MatMul4Bit (dequantize_4bit → F.linear) — (e) int4 1·2 |
  | 출력 후처리 | 같은 fused 호출의 scaled 부분 (int32 → fp16 스케일 복원) — (e) int8 4 | 없음 — linear/gemv 의 출력이 이미 fp16 — (d), (e) int4 2 |

  **결론에 들어가야 할 것** (2-3줄, 아래 세 가지가 모두 있어야 한다)

  1. 가중치 트래픽 방향 — 어느 쪽이 몇 배를 읽는가, 그것이 memory-bound 구간에서 뜻하는 것
  2. int8 만 지불하는 런타임 추가 연산 목록 + 그것이 이 모델에서 항상 지불된다는 근거 ((c))
  3. 어미 — int8 을 직접 실측한 것이 아니라 논문 수치와 코드 구조에서 도출한 판단임을 반영 ((c) 에서 정한 원칙)

  **피할 함정**: "matmul 이 2회라 2배 느리다" 로 쓰지 않는다. (e) 에서 확인했듯 Python 레벨 호출은 1회 (fused) 이고, outlier 열은 약 0.1% 라 fp16 곱셈 자체는 작다. 비용은 호출 횟수가 아니라 커널이 추가로 수행하는 작업 (스캔·양자화·분해·dequant) 과 그에 따르는 메모리 왕복에서 온다.

  - 결론 (by claude)
    - int8 이 int4 보다 느린 이유는 비트 수가 아니라 경로 구조에 있다. 
      - (1) decode 는 memory-bound 인데 int8 은 가중치를 int4 의 2배 (N_q x 1 byte vs 0.5 byte) 읽는다. 
      - (2) 그 위에 int8 만 지불하는 런타임 연산이 매 forward 얹힌다 -- activation absmax 양자화, threshold 스캔, outlier 열 분해, 출력 dequant. NF4 경로에는 이 네 가지가 전부 없고 weight dequant 만 있다. 
      - (3) 이 추가 연산은 6.7B 급에서 outlier 가 전 층에 나타나므로 예외가 아니라 상수 비용으로 추정된다.
      - 비용의 크기는 matmul 호출 횟수가 아니라(실제로 Python 레벨 호출은 fused 되어 1회다) 커널이 추가로 수행하는 작업과 그에 따르는 메모리 왕복에서 온다. 
      - 본 판단은 int8 직접 실측이 아니라 논문 수치 (1.2 Hz, Table 2) 와 설치본 코드 구조에서 도출한 것이다.


- [x] **Step 4 — 부정확 메커니즘: 성공률 58.1% 의 인과 경로**

  **선행 개념 (쓰기 전에 확인)**

  - **퍼플렉시티**(perplexity): 언어 모델이 검증 텍스트의 정답 토큰에 부여한 확률로 만드는 점수. 낮을수록 좋다. 모델이 한 번의 출력에서 얼마나 정확한 분포를 내는지를 재는 **정적 지표** — 출력이 다음 입력에 영향을 주지 않는다
  - **open-loop vs closed-loop**: 언어 벤치마크는 open-loop 다 (모델 출력이 다음 입력을 바꾸지 않는다). 로봇 제어는 closed-loop 다 — 출력한 action 이 로봇을 움직이고, 움직인 결과가 다음 관측이 되어 되돌아온다. 그래서 closed-loop 에서는 **출력의 정확도뿐 아니라 출력이 나오는 시점**도 결과를 바꾼다
  - **성공률이 재는 것**: 한 번의 출력 품질이 아니라 에피소드 전체가 목표를 달성했는가. 매 스텝의 관측이 얼마나 낡았는지, 그 사이 환경이 얼마나 변했는지가 누적되어 반영된다

  **답의 구조 (세 부분)**

  1. **두 지표가 다른 것을 잰다** — LLM.int8() 의 무손실 주장은 어느 지표에 대한 것인가, OpenVLA 의 58.1% 는 어느 지표인가. 한쪽에서 무손실인 것이 다른 쪽에서 하락과 모순되지 않는 이유
  2. **속도가 정확도로 전환되는 경로** — §1 에 이관된 판단 (OpenVLA 논문 §5.4: 1.2 Hz 로의 속도 하락이 시스템 동역학을 바꿈) 을 위 closed-loop 개념으로 다시 서술. 추론이 느려지면 관측과 실행 사이 지연이 커지고, 그 지연이 무엇을 낡게 만들어 실패로 이어지는가
  3. **직관이 틀린 지점** — Step 1 에서 체크한 가정 2 ("추론 속도는 성공률에 영향을 주지 않는다") 가 깨지는 자리를 한 줄로. 가정 1 은 Step 3 에서 이미 판정했으므로 여기서는 가정 2 가 대상이다

  **근거로 인용할 것**: (c) 의 결론 (분해 경로가 이 규모에서 상수 비용 -> int8 이 느려지는 구조적 이유), Step 3 표, OpenVLA 논문 §5.4, OpenVLA Table 2 의 58.1%·1.2 Hz. 어미는 (c) 와 같은 원칙 — 본인이 int8 을 실측한 것이 아니다.

  - LLM.int8()의 int8 무손실 주장의 경우 혼합 정밀도 분해 + 벡터 단위 양자화를 적용한 경우 퍼플렉시티와 제로샷 정확도에 대해 fp32에 비해 손실이 없다라는 의미였음. OpenVLA에서는 int8을 사용하는 경우 bridge 성공률이 58.1%라고 제시하고 있음.
    - 주장이 모순되어 보이지만 실제로 모순이 아닌 이유는 서로가 다른 지표를 가지고 판단하기 때문임.
  - int8 양자화는 추가된 양자화 연산의 오버헤드 때문에 대부분의 GPU에서 추론을 느리게 함(출처 - OpenVLA §5.4).
    - LLM.int8()의 분해 경로에서 파라미터 수가 6.7B 이상이 되면 outlier가 발생하는 조건이 되어 상수 비용이 될 것으로 추정됨.
    - 평가에 사용한 A5000 GPU에서 모델이 1.2Hz로 동작하여 약 800ms 추론 소요 시간을 나타내며, closed-loop 시스템 내 팔을 이동시키는 동역학이 달라지는 결과를 가져옴.
    - 추론 속도가 낮을수록 팔 등을 이동한 후 궤적을 갱신할 수 있는 시간이 늘어나 궤적 보정 간격이 길어져 구간 내 오차가 더 오래 방치되어 실패할 확률이 높아짐.
    - 결과적으로 성공률이 71.3% -> 58.1%로 낮아짐.
      - 속도 요인과 양자화 표현 손실 요인의 기여도는 OpenVLA 논문에 분리 측정된 바 없어 미확인
  - 따라서 "로봇 성공률은 모델 출력 정확도만으로 결정되고, 추론 속도는 성공률에 영향을 주지 않는다"는 가정은 맞지 않음을 확인.


- [ ] **Step 5 — "fp16 대비 int4 속도" 방향 예측 → Table 2 대조**

  **선행 개념 (예측을 쓰기 전에 확인)**

  - **memory-bound vs compute-bound**: GPU 연산의 소요 시간은 "데이터를 읽어 오는 시간" 과 "계산하는 시간" 중 **긴 쪽**이 지배한다. 읽어 오는 쪽이 병목이면 memory-bound (이때는 읽는 바이트를 줄이면 빨라진다), 계산이 병목이면 compute-bound (이때는 바이트를 줄여도 별 효과가 없다)
  - **decode 가 memory-bound 인 이유**: 토큰 1개를 낼 때 계산량은 작은데 (행렬×벡터) 가중치는 전부 읽어야 한다. 읽는 양은 그대로인데 계산할 거리가 적으니 읽기가 병목이 된다. 반대로 prefill 은 수백 토큰을 한 번에 처리 (행렬×행렬) 하므로 같은 가중치 읽기로 훨씬 많은 계산을 하고, compute 비중이 커진다
  - **그래서 이 예측이 성립하는 조건**: (a) 의 이득 (바이트 1/4) 은 memory-bound 일 때만 속도로 전환된다. 두 구간 (prefill·decode) 의 비중이 결과를 좌우하는 이유가 이것이고, 실측 확인이 Block 3 으로 넘어가는 이유이기도 하다

  예측을 먼저 적는다. 경쟁하는 두 효과 중 어느 쪽이 이기는지, 왜인지:
  - (a) 가중치 메모리 트래픽 약 1/4 — decode 는 forward 당 토큰 1개라 memory-bound 에 가까움 (실측 확인은 Block 3 으로 연결)
  - (b) dequant 오버헤드 — 매 forward 마다 커널 안에서 반복 ((e) int4 1·2 에서 확인한 두 경로 모두에서 발생)

  방향 (빠르다/느리다/비슷하다) 과 근거를 적은 뒤, 논문 수치와 대조하고 오차를 설명한다:
  - 뽑을 수치: A5000 기준 bf16 과 int4 의 추론 속도 (Hz) 2개 — §1 이 int8 1.2 Hz 를 인용한 자리와 같은 곳 (OpenVLA 논문). Table 2 에 속도 열이 없으면 OpenVLA §5.4 본문에서 찾고, 확정한 소재를 §1·완료 판정 표와 일치시킨다
  - 대조 방법: 절대값이 아니라 int4/bf16 비율로 비교한다 — A5000 과 4070 은 대역폭이 달라 절대값은 이식되지 않는다
  - prefill (vision token 포함 수백 개) 과 decode (action 토큰 7개) 의 비중이 결과에 미치는 영향은 Block 3 과 연결되는 지점으로 한 줄 남긴다

  -> 예측:
  -> 논문 수치 대조:
  -> 오차 설명:

완료 판정 표 — 각 행이 본인 문장으로 채워지면 Block 2 완료:

| 질문 | 답변 요지 (본인) | 대조 근거 |
|---|---|---|
| int8 이 int4 보다 느린 메커니즘 | | Step 3 표 + OpenVLA §5.4·Table 2 (int8 1.2 Hz -- 정확한 소재는 Step 5 에서 확정) |
| int8 성공률 58.1% 의 인과 경로 | | OpenVLA 논문 §5.4 + LLM.int8() 무손실 주장 |
| fp16 대비 int4 속도 방향 | | Table 2·§5.4 (bf16 vs int4, A5000) |
| §1 미해소 항목 한 줄 답 | | 위 세 행의 종합 |

### 4.3 Block 3 — 300 ms 구간 분해 (2026.08, 재측정과 병합)

> 골격: 실측값 하나 (mean 300.3 ms) 를 구간으로 쪼개 병목을 특정하고, 병목 가설을 "해상도 절반" 개입으로 검증한다. Block 1 과 같은 분업 — 측정 조건·수치 원본은 methodology.md 재측정 절 (신설) 과 raw/·scripts/ 에 두고, 이 절에는 예측 논리와 오차 해석만 본인 문장으로 쓴다. methodology §3 의 미기록 항목 (notebook → `.py`, `summary.csv`, 실제 카메라 이미지 조건) 과 같은 세션에서 병합 수행. 통과 기준은 plan Task 2-2 Step 1: 구간 분해표 + "해상도 절반이면 어디가 얼마나" 예측-측정-오차. 예상 배분 (총 4-6h): Step 1 약 1h, Step 2-3 약 2-3h, Step 4-5 약 1.5h.

- [ ] **Step 1 — 측정 전 예측: 300 ms 의 구간 배분 손계산**

  **선행 개념 (예측을 세우기 전에 확인)**

  - **prefill 과 decode**: 자기회귀 생성은 두 국면으로 나뉜다. prefill 은 입력 전체 (visual token + prompt) 를 한 번에 통과시켜 첫 출력 토큰을 만드는 국면이고, decode 는 그 뒤 토큰을 1개씩 이어 붙이는 국면이다. OpenVLA 의 action 7개는 decode 7회에 해당한다
  - **KV cache**: decode 가 매번 전체 시퀀스를 다시 계산하지 않도록, prefill 에서 만든 각 층의 key·value 를 저장해 두고 재사용하는 메모리. Block 1 Step 4 의 "activation·KV cache 0.35 GB" 가 이것이다
  - **roofline 하한**: "이 연산이 아무리 빨라도 이보다는 느리다" 는 이론 하한. decode 는 memory-bound (Block 2 Step 5 선행 개념) 이므로 하한 = 읽어야 하는 가중치 바이트 / 메모리 대역폭. 실측이 하한의 몇 배인지가 효율의 지표이고, 그 간극의 정체를 밝히는 것이 Step 3 의 과제다
  - **왜 vision encoder 분을 빼는가**: decode 는 이미지 처리를 다시 하지 않는다 (prefill 에서 만든 visual token 이 KV cache 에 있다). 따라서 decode 하한 계산의 분자에는 LLM 가중치만 들어간다

  구간은 5개: 전처리 (processor) / vision encoder / LLM prefill 1회 / LLM decode 7회 (action 토큰 7개 = xyz 3 + rpy 3 + gripper 1) / un-norm. 예측치는 아래 사다리 표의 "예측" 열에 채운다. 각 구간의 예측 근거:
  - 전처리: week8 stress test 에서 무시 수준 확인 (§2) — 근사 0 예측
  - vision encoder: visual token 수 손계산 — patch 14 기준 (224/14)^2 = 256. 실제 token 수는 Step 2 훅에서 projector 출력 shape 으로 교차 검증
  - decode 하한 공식 (roofline): 토큰당 ≈ LLM 상주 가중치 바이트 / 대역폭 504 GB/s (§2). LLM 바이트는 Block 1 수치에서 vision encoder 분 (0.731B, N_q 에 포함 — Block 1 Step 3 에서 양자화 확인됨) 을 빼고 재산출 — decode 는 vision encoder 가중치를 읽지 않는다
  - prefill: 가중치 읽기는 decode 1회와 동일 + 시퀀스 약 280 토큰 (visual 256 + prompt, 정확한 길이는 input_ids 로 확인) 의 연산 — compute 비중이 커지는 구간
  - un-norm: 7원소 벡터 연산 — 근사 0

  하한 합계와 실측 300.3 ms 의 간극 자체가 Step 3 오차 설명의 대상이다 (후보: eager attention, dequant 커널 효율, 커널 launch·CPU 측 generate 루프 오버헤드 — 예단하지 말고 실측 후 배분).

- [ ] **Step 2 — 계측 설계 (스크립트 작성, `scripts/` 저장)**

  **선행 개념 (스크립트를 쓰기 전에 확인)**

  - **forward hook**: PyTorch 모듈에 "이 모듈이 실행되기 직전/직후에 이 함수를 불러 달라" 고 등록하는 장치. 모듈 소스를 고치지 않고 경계 시각을 기록할 수 있어, `predict_action` 내부를 수정하지 않는다는 제약을 푸는 수단이다. `register_forward_pre_hook` 은 직전, `register_forward_hook` 은 직후
  - **CUDA 비동기 실행**: GPU 연산을 호출하면 CPU 는 큐에 작업을 넣고 곧바로 다음 줄로 넘어간다 — GPU 가 끝날 때까지 기다리지 않는다. 그래서 sync 없이 `time.perf_counter()` 로 재면 "GPU 가 일한 시간" 이 아니라 "CPU 가 큐에 넣는 데 걸린 시간" 이 찍힌다
  - **`torch.cuda.synchronize()`**: 큐에 쌓인 GPU 작업이 전부 끝날 때까지 CPU 를 대기시킨다. 이걸 넣어야 mark 시각이 실제 연산 경계와 일치한다. 대신 파이프라인이 매번 비워지므로 측정 자체가 약간의 오버헤드를 만든다 — 그래서 hook 있는 실행과 없는 실행을 둘 다 기록해 차이를 계측 오버헤드로 명시한다 (아래 5번)
  - **warm-up 이 필요한 이유**: 첫 호출에는 CUDA 커널 로딩·메모리 할당·autotune 이 섞여 들어간다. 정상 상태 (steady state) 수치를 재려면 앞의 몇 회를 버려야 하고, 몇 회를 버려야 충분한지는 Step 5 에서 수치로 근거를 만든다

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

  **선행 개념 (정독 전에 확인 — 이 Block 은 용어 두 쌍이 축이다)**

  - **action chunking**: 정책을 한 번 호출할 때 다음 1스텝이 아니라 앞으로의 k 스텝 action 을 한꺼번에 예측하는 방식. 예측한 k 개를 하위 층이 차례로 실행하는 동안 정책은 쉬므로, 정책 호출 횟수가 1/k 로 줄어든다
  - **temporal ensembling**: 서로 다른 시점의 호출들이 같은 시각의 action 을 중복 예측하게 해 두고 그것들을 평균하는 기법. chunking 이 만드는 부작용 (청크 경계에서 행동이 튀는 것) 을 완화한다
  - **실행 주파수 vs 반응 주파수** (이 Block 의 핵심 구분): 실행 주파수는 하위 층이 action 을 소화해 내보내는 속도이고, 반응 주파수는 **새 관측이 행동에 반영되는** 속도다. chunking 은 이 둘 중 한쪽만 올린다 — 어느 쪽인지가 Step 4 의 답
  - **blocking control**: 관측 -> 추론 -> 실행을 겹치지 않고 순차로 돌리는 제어 방식. 추론하는 동안 로봇은 멈춰 있고, 실행하는 동안 새 관측은 받지 않는다. 이 전제에서는 추론이 느려도 "관측이 낡는" 문제가 생기지 않지만, 대신 전체 사이클이 그만큼 길어진다

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

  **판정의 형태 (수치 예를 만드는 방법)**

  "충분한가" 는 그 자체로는 답할 수 없는 질문이다 — 무엇에 대해 충분한지를 정하지 않았기 때문이다. 그래서 다음 형태로 바꾼다: **반응 시간 동안 환경이 변한 양 vs 허용 오차**. 변한 양이 허용 오차보다 작으면 충분, 크면 불충분이다. 예를 만들 때 채워야 할 세 값은 (1) 반응 시간 (루프 500 ms 기준), (2) 그 시간 동안의 환경 변화량 (물체 속도 x 시간), (3) 허용 오차 (그리퍼 개구 여유 등 물리 치수) 다. 세 값이 정해지면 판정은 산수로 나온다.

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
