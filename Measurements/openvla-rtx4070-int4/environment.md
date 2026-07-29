# 실행 환경 — openvla-rtx4070-int4

> 측정일: 2026-06 (Phase 4 순서 1, week6 실습 1-2)
> 측정 기록 commit: `1652c81` (chore(phase4): record week6 latency run outputs in notebook)

## 0. 이 문서가 답하는 질문

> **"이 수치를 다시 얻으려면 무엇이 같아야 하는가."**

측정값은 장비와 소프트웨어 조합에 붙어 있는 값이다. 같은 코드를 다른 GPU 나 다른 라이브러리 버전에서 돌리면 다른 숫자가 나오고, 심하면 **로드 자체가 실패한다.** 그래서 실험 디렉터리는 조건(`methodology.md`) 과 해석(`findings.md`) 전에 환경부터 고정해 적는다.

이 문서에는 표만 있고 해석이 없는 것이 정상이다 — 수치의 뜻은 [`methodology.md`](methodology.md), 판단은 [`findings.md`](findings.md) 가 담당한다.

읽기 전에 알아 둘 용어:

| 용어 | 뜻 |
|---|---|
| **driver** | GPU 하드웨어를 운영체제가 쓸 수 있게 해 주는 소프트웨어 |
| **CUDA** | NVIDIA GPU 에서 계산을 돌리기 위한 플랫폼·툴킷 |
| **venv** | 파이썬 가상 환경. 프로젝트별로 패키지 버전을 격리하는 폴더 |
| **dependency** | 내 코드가 동작하기 위해 필요한 외부 라이브러리 |
| **remote code** | 표준 라이브러리에 없는 모델 고유 코드. 모델 저장소에서 함께 받아 실행한다 |
| **양자화**(quantization) | 가중치를 더 적은 비트로 표현해 메모리를 줄이는 것 |
| **nf4** | 4-bit 양자화 방식의 한 종류 (NormalFloat 4-bit) |
| **double quant** | 양자화에 쓰는 보조 상수(scale)까지 한 번 더 양자화해 메모리를 더 줄이는 옵션 |
| **compute dtype** | 저장은 4-bit 로 하되 실제 곱셈·덧셈을 수행할 때 쓰는 자료형 |
| **attention 구현** | Transformer 의 attention 을 어느 커널로 계산할지의 선택. 구현에 따라 속도·수치가 달라진다 |
| **메모리 대역폭** | GPU 가 1초에 메모리에서 읽어 올 수 있는 바이트 양 (GB/s) |

## Hardware

| 항목 | 값 |
|---|---|
| GPU | NVIDIA RTX 4070 12GB (메모리 대역폭 504 GB/s) |
| 호스트 | Ubuntu PC (원격 접속 메인 장비) |

두 값이 뒤 문서에서 쓰이는 자리:

- **12GB** — 이 용량이 int4 채택의 이유다. 같은 모델을 fp16 으로 올리면 약 15GB 가 필요해 애초에 로드되지 않는다 (계산은 `findings.md` §4.1 Step 1)
- **504 GB/s** — 가중치를 읽어 오는 속도의 상한이다. 토큰을 하나씩 생성하는 구간은 계산량보다 읽기량이 병목이라, 이 값이 속도의 하한선을 정한다 (`findings.md` §4.2 Step 5, §4.3)

즉 hardware 표는 참고 정보가 아니라 **뒤의 모든 예측 계산에 대입되는 입력**이다.

## Software

| 항목 | 버전 | 비고 |
|---|---|---|
| NVIDIA driver | 580.159.03 | |
| CUDA | 13.0 | |
| Python | 3.12.3 | `.venv-vla` 공용 venv |
| PyTorch | 2.12.0 | |
| transformers | 4.40.1 | OpenVLA remote code 요구 고정 버전 |
| tokenizers | 0.19.1 | transformers 4.40.1 페어 |
| timm | 0.9.16 | 고정 |
| accelerate | 1.0.1 | 1.2.0+ 는 4-bit 모델 `.to()` 충돌 |
| bitsandbytes | 0.49.2 | nf4 양자화 |
| attention 구현 | eager | `attn_implementation="eager"` |

각 항목이 어떤 역할을 하고 왜 그 버전에 묶여 있는지:

| 항목 | 역할 | 버전이 고정된 이유 |
|---|---|---|
| transformers | 모델을 불러오고 실행하는 본체 라이브러리 | OpenVLA 는 자체 모델 코드를 함께 받아 실행하는데, 그 코드가 이 버전의 내부 구조를 전제로 쓰여 있다. 올리면 로드가 깨진다 |
| tokenizers | 문장을 토큰(숫자) 으로 쪼개는 구성 요소 | transformers 와 짝으로 맞물려 있어 한쪽만 올릴 수 없다 |
| timm | 이미지 모델(vision encoder) 구현 모음 | OpenVLA 의 vision encoder 가 이 라이브러리 구조를 쓴다 |
| accelerate | 모델을 GPU 에 배치하고 옮기는 유틸리티 | 1.2.0 이상에서 4-bit 로 적재한 모델에 `.to()` 를 호출하면 충돌한다 |
| bitsandbytes | 4-bit 양자화를 실제로 수행하는 라이브러리 | nf4 저장·역양자화 커널이 여기 있다 |
| attention 구현 `eager` | attention 을 표준 파이썬·torch 연산으로 계산 | 최적화 커널 대신 기본 구현을 쓴다. 측정 조건의 일부이므로 재측정 때도 같은 값을 쓴다 |

**버전을 "올리면 좋은 것" 으로 보면 안 되는 이유**가 이 표에 있다. 여기서 버전은 성능 옵션이 아니라 **동작 조건**이다. 하나를 올리면 로드 실패나 조용한 동작 변화가 생기고, 그러면 이 디렉터리의 모든 수치가 다른 환경의 값이 되어 비교 근거를 잃는다.

버전 고정의 근거 주석: `Studies/Phase 4/week8/requirements.txt`, 매트릭스: `Studies/Phase 4/SETUP.md` §7.1.

## 모델

| 항목 | 값 |
|---|---|
| 모델 | `openvla/openvla-7b` (HuggingFace) |
| 양자화 | bitsandbytes 4-bit nf4, double quant, compute dtype fp16 |
| 로드 결과 | RTX 4070 12GB 에 OOM 없이 안착 |

표를 문장으로 풀면 이렇다.

- **`openvla-7b`** — 파라미터 약 70억 개 규모의 VLA(Vision-Language-Action) 정책 모델이다. 이름은 7B 지만 실제 파라미터 수는 그보다 많다 (백본 언어 모델 + vision encoder 2개 + 연결층 — 정확한 산출은 `findings.md` §4.1 Step 1)
- **양자화 3항목** — `nf4` 는 4-bit 저장 방식, `double quant` 는 그 4-bit 를 위한 보조 상수까지 압축하는 옵션, `compute dtype fp16` 은 "저장은 4-bit 이지만 곱셈은 fp16 으로 한다" 는 뜻이다. 즉 **연산은 16비트로 하고 저장만 4비트로 줄인다** (근거 코드 확인은 `findings.md` §4.2 Step 2)
- **로드 결과** — 이 한 줄이 이 실험의 첫 성과다. 12GB 카드에 15GB 짜리 모델을 올리는 문제를 양자화로 해결했다는 사실이고, 실제 사용량 4.38 GB 의 분해는 `methodology.md` §4 와 `findings.md` §4.1 에 있다

이 3항목은 이후 Phase 4.5 에서 **fine-tuned 모델을 측정할 때도 그대로 유지해야 하는 조건**이다. 양자화 설정이 다르면 before/after 비교에서 "adaptation 효과" 와 "양자화 손실" 이 섞인다.
