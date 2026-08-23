# Week 4 실습 4 기록: 버전 호환성 + 결정성 검증 (compat_check)

> 작성일: 2026-08-14
> 실행 위치: `scripts/practice.ipynb` (로드: 실습 2 셀, 결정성: 실습 4 셀)
> 판정 요약: 로드 성공 (가중치 초기화 경고 없음, deprecation 계열 경고만 존재) + 결정성 통과 (3회 시행 최대 편차 0.00e+00). README §6 의 "안 열렸을 경우" 선택지는 해당 없음.

## 기록 표 (PRACTICE.md 실습 4 의 "기록할 것")

| 항목 | 값 |
|---|---|
| 버전 대조 | 아래 §1 — transformers 4.40.1 일치, torch 는 2.2.0 vs 2.12.0 으로 다름 |
| 로드 성공 여부 | 성공 — 4/4 shard, 4bit NF4 적재 후 4.38 GB (baseline 과 +0.00 GB) |
| 경고 메시지 전문 | 아래 §2 — 전부 deprecation/위젯 경고, **가중치 관련 경고 0건** |
| 결정성 판정 | 결정적 — 3회 시행 bit 동일, 최대 편차 0.00e+00 (기준 1e-6 미만) |
| 안 열렸을 경우 선택지 (README §6) | 해당 없음 — 첫 시도에서 열렸다 |

## 1. 버전 대조 표 (학습 환경 vs 추론 환경)

학습 측은 week3 `outputs/image_build.md` 의 `openvla-train:v2` 기록, 추론 측은 실습 4 셀의 런타임 출력이다 (`outputs/local_versions.txt` 와 일치). 추론 환경은 `Phase 4/.venv-vla` (python 3.12).

| 패키지 | 학습 이미지 (openvla-train:v2) | 추론 환경 (.venv-vla) | 대조 |
|---|---|---|---|
| python | 3.11 | 3.12 | 다름 |
| torch | 2.2.0+cu121 | 2.12.0+cu130 | 다름 (메이저 격차) |
| transformers | 4.40.1 | 4.40.1 | 일치 |
| tokenizers | 기록 없음 (transformers 전이 의존성) | 0.19.1 | 대조 불가 |
| timm | 0.9.10 | 0.9.16 | 다름 |
| accelerate | 기록 없음 | 1.0.1 | 대조 불가 |
| peft | 0.11.1 | 미사용 | 추론은 머지 완료 가중치를 쓰므로 불필요 |
| bitsandbytes | 미설치 (학습은 양자화 경로를 안 씀) | 0.49.2 | 추론에서만 사용 (4bit NF4) |
| flash-attn | 2.5.5 | 미설치 | 추론은 `attn_implementation="eager"` 로 대체 |

읽는 법 (PRACTICE.md 4-1 의 원칙): 이 표는 판정이 아니라 기록이다. 판정은 로드 성공(§2)과 출력 대역·결정성(§3)이 한다. 그 기준으로 보면 torch 메이저 격차와 timm 마이너 격차에도 체크포인트가 열리고 정상 출력했다. 격차 항목 중 체크포인트 직렬화와 remote code 를 직접 쥐는 **transformers 가 4.40.1 로 정확히 일치**하는 것이 이 조합이 통과한 배경으로 가장 유력하다.

## 2. 로드 성공 여부와 경고 메시지 전문

`/workspace/models/openvla-maniskill-ft/` 를 4bit NF4 (`bnb_4bit_use_double_quant=True`, compute dtype fp16) 로 적재했다. shard 4/4 로드 완료, 적재 후 VRAM 4.38 GB — 실습 2 baseline(4.38 GB) 대비 +0.00 GB 로 양자화 적용이 확인된 상태의 로드다.

로드 과정에서 나온 경고 전문:

```
tqdm/auto.py:21: TqdmWarning: IProgress not found. Please update jupyter and ipywidgets.
See https://ipywidgets.readthedocs.io/en/stable/user_install.html

huggingface_hub/file_download.py:949: FutureWarning: `resume_download` is deprecated
and will be removed in version 1.0.0. Downloads always resume when possible.
If you want to force a new download, use `force_download=True`.
(동일 경고 2회)

bitsandbytes/backends/cuda/ops.py:213: FutureWarning: _check_is_size will be removed
in a future PyTorch release along with guard_size_oblivious. Use _check(i >= 0) instead.
(추론 시 ops.py:468 에서도 동일 계열 경고)

Loading checkpoint shards: 100%|##########| 4/4 [00:08<00:00,  2.24s/it]
```

핵심 관찰: "Some weights were not used" / "newly initialized" 계열, 즉 **가중치가 학습된 값 대신 랜덤값으로 채워졌다는 경고가 한 건도 없다.** 위 경고 전문을 남기는 목적(나중에 결과가 이상할 때의 첫 단서)에 비추면, 남은 경고는 전부 라이브러리 deprecation 및 jupyter 위젯 안내로 모델 가중치와 무관하다.

## 3. 결정성 판정

같은 입력(고정 seed 무작위 이미지 + 동일 프롬프트)을 `unnorm_key="maniskill_pickcube"`, `do_sample=False` 로 3회 추론했다.

| trial | action (소수 5자리 반올림) |
|---|---|
| 0 | [-5.5e-04, 1.187e-02, -1.184e-02, -5.0e-05, 2.3e-04, 7.8e-04, 0.0] |
| 1 | 위와 동일 |
| 2 | 위와 동일 |

- 최대 편차: 0.00e+00 (기준: 1e-6 미만이면 결정적)
- 판정: **결정적** — 4bit 양자화 상태에서도 greedy 디코딩 출력이 bit 단위로 같다. week5 의 "동일 조건 N회" 측정 전제가 성립한다.

## 4. 재현 절차 포인터

- 실행 코드: `scripts/practice.ipynb` — 로드는 실습 2 셀, 버전 출력과 3회 시행은 실습 4 셀 (PRACTICE.md 실습 4 의 `practice_compat_check.py` 내용과 동일)
- 학습 측 버전 원본: week3 `outputs/image_build.md` §"이미지에 들어간 버전"
- 추론 환경 버전 스냅샷: `outputs/local_versions.txt` (실습 0 에서 생성)
- 결정성 시행의 입력은 실습 3 셀의 `inputs` 를 재사용하므로, 단독 재현 시 실습 3 셀을 먼저 실행해야 한다
