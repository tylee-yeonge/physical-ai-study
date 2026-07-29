# 측정 조건·절차 — openvla-rtx4070-int4

> 측정 대상: OpenVLA 7B (int4/nf4) 의 `predict_action` 1회 호출 latency, RTX 4070 12GB
> 측정 코드: [`scripts/practice.ipynb`](scripts/practice.ipynb) (2026-06 실측 당시 실행 기록 포함)
> 원본 데이터: [`raw/openvla_latency_4070_int4.npy`](raw/openvla_latency_4070_int4.npy) (float 배열, ms 단위, n=100)

## 0. 이 문서가 답하는 질문

> **"이 숫자는 정확히 무엇을 잰 값인가."**

`300 ms` 같은 수치는 그 자체로는 의미가 확정되지 않는다. 전처리를 포함했는지, 몇 번 재서 평균한 것인지, GPU 가 실제로 일을 끝낼 때까지 기다렸는지에 따라 같은 코드에서도 전혀 다른 값이 나온다. 이 문서는 그 조건을 전부 적어 **수치의 의미를 확정**한다.

문서 구성:

| 절 | 내용 |
|---|---|
| §1 | latency 측정 조건 (무엇을 어떻게 재었는가) |
| §2 | latency 결과 통계 |
| §3 | 아직 기록하지 못한 항목과 그것을 채울 계획 |
| §4 | 메모리 상세 실측 (조건 + 결과 수치) |

해석은 이 문서에 쓰지 않는다 — "왜 그 값인가", "그래서 무엇을 판단했는가" 는 [`findings.md`](findings.md) 가 담당한다. 두 문서가 같은 수치를 각자 적으면 나중에 한쪽만 고쳐 어긋나므로, **수치의 원본은 여기 한 곳**이다.

### 0.1 미리 풀어 두는 용어

| 용어 | 뜻 |
|---|---|
| **latency** | 한 번의 호출이 끝날 때까지 걸린 시간 |
| **`predict_action`** | 이미지와 지시문을 받아 로봇 action 7개를 내놓는 OpenVLA 메서드 |
| **n** | 측정 반복 횟수. 여기서는 100회 |
| **warm-up** | 본 측정 전에 버리는 예비 실행. 첫 호출에는 커널 로딩·메모리 할당 비용이 섞인다 |
| **batch size** | 한 번에 함께 처리하는 입력 개수. 1 이면 한 장씩 |
| **prompt** | 모델에 주는 지시 문장 틀 |
| **`torch.cuda.synchronize()`** | GPU 에 쌓인 작업이 전부 끝날 때까지 CPU 를 대기시키는 호출 |
| **preprocessing** | 이미지·문장을 모델 입력 형식으로 바꾸는 전처리 단계 |
| **processor** | 그 전처리를 수행하는 객체 |
| **mean / median** | 평균 / 중앙값. 두 값이 가까우면 분포가 한쪽으로 치우치지 않았다는 신호 |
| **std**(표준편차) | 값이 평균에서 얼마나 퍼져 있는지. 작으면 매 호출이 일정하다 |
| **p95 / p99** | 정렬했을 때 하위 95% / 99% 지점. "느린 쪽 꼬리" 의 크기를 본다 |
| **throughput** | 초당 처리 횟수 (Hz). `1 / mean` 이다 |
| **`memory_allocated`** | PyTorch 가 텐서에 실제로 할당한 GPU 메모리 |
| **`memory_reserved`** | PyTorch 가 미리 확보해 들고 있는 메모리 (할당분 + 여유분) |
| **`max_memory_allocated`** | 실행 중 기록된 `memory_allocated` 의 최댓값 (peak) |
| **CUDA context** | GPU 를 쓰기 위해 드라이버가 잡는 기본 메모리. PyTorch 통계에 안 잡힌다 |
| **baseline delta** | 절대값이 아니라 "측정 시작 시점 대비 증가분" 으로 내 몫을 계산하는 방식 |

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

### 1.1 각 조건이 왜 그 값인가

표의 항목은 임의로 고른 값이 아니다. 각각이 특정 오측정을 막는다.

| 조건 | 막는 오측정 |
|---|---|
| n = 100 | 1-2회만 재면 우연한 느린 호출 하나가 평균을 흔든다 |
| warm-up 5회 | 첫 호출에는 CUDA 커널 로딩·메모리 할당·자동 튜닝이 섞여 실제보다 훨씬 느리게 나온다 |
| batch size 1 | 로봇 제어는 카메라 한 장을 받아 한 번 판단하는 구조다. 배치를 키우면 throughput 은 좋아지지만 **제어 지연과 무관한 숫자**가 된다 |
| 매 반복 새 이미지 | 같은 이미지를 반복하면 캐시나 최적화가 개입해 실제보다 빨라질 수 있다 |
| prompt 고정 | 문장 길이가 바뀌면 토큰 수가 바뀌고 계산량도 바뀐다 |
| `do_sample=False` | 확률적 샘플링을 끈다. 같은 입력에 같은 출력이 나와 측정이 재현된다 |
| sync 앞뒤 모두 | 아래 1.2 참조 |
| preprocessing 제외 | 아래 1.3 참조 |

### 1.2 `synchronize()` 가 없으면 무엇이 잘못되는가

GPU 연산을 호출하면 CPU 는 "이 작업을 해 달라" 고 큐에 넣고 **기다리지 않고 다음 줄로 넘어간다.** 그래서 sync 없이 시간을 재면 GPU 가 계산한 시간이 아니라 **CPU 가 큐에 작업을 넣는 데 걸린 시간**이 찍힌다. 그 값은 실제보다 훨씬 작고, 측정으로서 무의미하다.

측정 구간 앞뒤 모두에 넣는 이유: 앞의 sync 는 이전 작업이 남아 있지 않도록 비우고, 뒤의 sync 는 이번 작업이 끝날 때까지 기다린다. 둘 다 있어야 구간이 실제 연산 경계와 일치한다.

### 1.3 전처리를 제외한 것의 의미

이 300 ms 는 **모델 추론 단독** 값이고, 실제 시스템의 한 사이클은 여기에 카메라 캡처·전처리·통신·실행이 더 붙는다. 즉 이 수치를 그대로 "제어 주기" 로 읽으면 안 된다.

다만 전처리 몫이 무시할 만하다는 별도 확인이 있다 — `predict()` 전체를 잰 값이 `predict_action` 단독과 거의 같았다 (`findings.md` §2). 전체 루프를 어떻게 잡았는지도 같은 절에 있다.

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

### 2.1 이 표를 읽는 법

숫자 하나하나가 아니라 **항목 사이의 관계**를 본다.

| 볼 것 | 이 표에서의 값 | 뜻 |
|---|---|---|
| mean 과 median 이 가까운가 | 300.3 vs 301.3 | 분포가 한쪽으로 치우치지 않았다. 특이하게 느린 호출이 평균을 끌어올리지 않았다 |
| std 가 mean 대비 얼마나 작은가 | 3.8 / 300.3 (약 1.3%) | 매 호출이 거의 같은 시간에 끝난다 |
| p95·p99 가 mean 에서 얼마나 떨어졌나 | +4.5 / +5.3 ms | 느린 쪽 꼬리가 거의 없다 |
| min 과 max 의 폭 | 290.4 - 308.2 (약 18 ms) | 최악의 호출도 최선 대비 6% 정도만 느리다 |
| throughput | 3.33 Hz | `1000 / 300.3`. 초당 3.33회 판단할 수 있다는 뜻 |

**제어 관점에서 왜 p95·p99 를 보는가**: 평균이 좋아도 100번 중 5번이 크게 느리면 그 순간 로봇이 멈칫한다. 실시간 제어에서는 평균보다 **최악 지연**이 설계 기준이 되므로 꼬리를 함께 기록한다. 이 측정에서는 꼬리가 거의 없어 안정적이라는 판단이 나왔고, 그 해석은 `findings.md` §2 에 있다.

마지막 행의 4.38 GB 는 시간 측정이 아니라 메모리 값이다. 이 수치의 상세 분해가 §4 이고, "왜 nvidia-smi 로 본 값과 다른가" 의 해석이 `findings.md` §4.1 이다.

## 3. 미기록 항목 — 재측정 (2026.08 내, 휴직 중 GPU 접근성 리스크 대비) 에서 채운다

이 절은 **아직 없는 것을 숨기지 않고 목록으로 남기는 자리**다. 빠진 항목을 적어 두지 않으면 나중에 "안 한 것" 과 "하고 기록을 잊은 것" 을 구분할 수 없다.

- [x] VRAM peak (`nvidia-smi` 기준) — §4 메모리 상세 실측 (2026-07-10) 으로 앞당겨 수행 완료. 차이 원인 해석은 findings.md §4.1
- [ ] 구간별 분해 (전처리 / vision encoder / action 토큰 autoregressive 생성 / un-normalization) — findings.md Block 3
- [ ] 실제 카메라 이미지 입력 조건 (랜덤 노이즈 대비 차이 여부)
- [ ] 재실행 스크립트 정리 (notebook → 단일 `.py`, 실행 명령어 1줄 문서화)
- [ ] `summary.csv` 생성 (위 통계의 기계 판독 요약)

각 항목이 채워지면 무엇이 가능해지는가:

| 항목 | 채워지면 |
|---|---|
| 구간별 분해 | 300 ms 중 어디가 병목인지 특정된다. 최적화나 다른 장비로의 이식 예측이 그 위에서 가능해진다 |
| 실제 카메라 이미지 | 랜덤 노이즈로 잰 값이 실제 장면에서도 유지되는지 확인된다 (내용에 따라 계산량이 달라지지 않는지) |
| notebook → `.py` | 재현이 "노트북을 열어 순서대로 실행" 이 아니라 명령 한 줄이 된다. 제 3자 검증의 문턱이 낮아진다 |
| `summary.csv` | 사람이 읽는 표 외에 기계가 읽는 요약이 생겨, 나중에 다른 측정과 자동 대조할 수 있다 |

## 4. 메모리 상세 실측 (2026-07, findings.md §4.1 Block 1 용)

> §3 의 VRAM peak 항목을 GPU 접근성 리스크 대비로 앞당겨 수행한다. 이 절은 수치의 원본만 담고, 차이 원인 해석은 findings.md §4.1 에 쓴다.
> 실행: `python scripts/memory_breakdown.py` — 로드 조건은 §1·environment.md 와 동일 (nf4, double quant, compute dtype fp16, eager)

### 4.0 이 측정이 필요한 이유

"모델이 GPU 메모리를 얼마나 쓰는가" 는 **어느 도구로 보느냐에 따라 다른 값이 나온다.** PyTorch 가 보고하는 값과 `nvidia-smi` 가 보여주는 값이 다르고, 둘 다 틀린 것이 아니라 **서로 다른 것을 세고 있다.**

| 도구 | 세는 것 | 빠지는 것 |
|---|---|---|
| `torch.cuda.memory_allocated` | PyTorch 가 텐서에 할당한 양 | CUDA context, PyTorch 가 확보만 해 둔 여유분 |
| `torch.cuda.memory_reserved` | PyTorch 가 확보한 양 (할당 + 여유) | CUDA context |
| `nvidia-smi` | 디바이스 전체 사용량 | 없음 (전부 포함) |

그래서 세 값을 **세 시점에서 함께** 읽어야 차이를 항목별로 분해할 수 있다. 그 분해가 `findings.md` §4.1 Step 4 의 과제다.

측정 절차 — baseline (시점 0: CUDA 초기화 전 디바이스 전체 사용량) 을 먼저 기록하고 3개 시점에서 읽는다. nvidia-smi 값은 프로세스 행 매칭이 아니라 **baseline 대비 디바이스 증가분**이다 — Docker 컨테이너에서는 nvidia-smi 가 보여주는 PID 가 호스트 네임스페이스 값이라 `os.getpid()` 매칭이 구조적으로 불가능하기 때문 (전제: 측정 중 다른 프로세스의 GPU 사용량 일정):

| 시점 | torch 측 | nvidia-smi 측 (baseline 대비 증가분) |
|---|---|---|
| A. CUDA 초기화 직후 (텐서 1개만) | `memory_allocated` (근사 0) | context 단독 크기 |
| B. 모델 로드 직후 | `memory_allocated`, `memory_reserved` | 로드 후 사용량 |
| C. `predict_action` 1회 후 | `max_memory_allocated`, `memory_reserved` | 추론 후 사용량 |

세 시점을 이렇게 잡은 이유: **각 시점 사이의 증가분이 곧 한 항목의 크기**가 된다. A 는 모델과 무관한 기본 비용, A→B 는 가중치, B→C 는 추론 중 임시로 쓰는 메모리다. 시점을 하나만 재면 이 항목들이 한 덩어리로 섞여 분해가 불가능하다.

`nvidia-smi` 를 증가분으로 쓰는 이유도 실무적이다. 원래는 "내 프로세스 행" 을 찾아 그 값을 읽는 것이 정확하지만, 컨테이너 안에서는 `nvidia-smi` 가 호스트 기준 프로세스 번호를 보여 주므로 내 프로세스와 매칭되지 않는다. 그래서 **측정 전후 디바이스 전체 값의 차이**로 우회하고, 그 대가로 "측정 중 다른 프로세스가 GPU 를 쓰지 않는다" 는 전제를 함께 적는다.

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

> B 의 `memory_allocated` 가 2026-06 값과 일치한 것은 부수 확인이 아니라 **재현성 확인**이다. 한 달 뒤 다른 스크립트로 다시 재어 같은 값이 나왔으므로, 이 수치는 그날의 우연이 아니다.

dtype 별 합산 (스크립트 출력 — 합계가 B 의 `memory_allocated` 와 일치해야 함):

| 구분 | 크기 |
|---|---|
| torch.uint8 | 3.638 GB |
| torch.float16 | 0.531 GB |
| buffer:torch.float16 | 0.034 GB |
| buffer:torch.float32 | 0.000 GB |
| quant_state (scale 등) | 0.116 GB |
| 합계 | 4.32 GB (`memory_allocated` 4.38 GB 대비 잔차 0.066 GB) |

이 표의 각 줄이 무엇인지:

| 줄 | 정체 |
|---|---|
| `torch.uint8` | 4-bit 로 양자화된 가중치. int4 두 개가 uint8 한 개에 담겨 저장되므로 dtype 이 uint8 로 잡힌다 |
| `torch.float16` | 양자화되지 않고 16비트로 남은 가중치 (embedding·출력층·정규화 층 등) |
| `buffer:...` | 학습 대상이 아닌 상수 텐서 (모델이 들고 다니는 고정값) |
| `quant_state` | 4-bit 값을 원래 크기로 되돌릴 때 쓰는 배율(scale) 등의 부가 데이터 |

합계가 `memory_allocated` 와 정확히 맞지 않는 것 자체가 분석 대상이다 — 그 잔차를 무엇으로 설명할지가 `findings.md` §4.1 Step 3 의 내용이다.

한계: peak 는 `predict_action` 1회 기준 — n=100 재측정 (2026.08) 때 동일 수준인지 재확인한다.
