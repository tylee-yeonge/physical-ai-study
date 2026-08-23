# Week 4 실습 1 기록: 재머지 + 무결성 검증 (remerge_check)

> 작성일: 2026-08-14
> 산출물: `/workspace/models/openvla-maniskill-ft/` (재머지된 15GB 가중치 + sidecar)
> 판정 요약: 색인 대조 통과 — 재머지가 pod 머지와 같은 구조를 만들었다. week3 `train_log.md` §6 의 "재머지 미검증" 항목이 닫힌다.

## 기록 표 (PRACTICE.md 실습 1 의 "기록할 것")

| 항목 | 값 |
|---|---|
| 재머지 환경 (venv / torch / peft) | `/workspace/venvs/remerge` (uv, Python 3.11.15) / torch 2.2.0+cpu / peft 0.11.1 |
| base 리비전 (캐시 스냅샷) | `47a0ec7fc4ec123775a391911046cf33cf9ed83f` — PRACTICE.md 1-0 기대값과 일치 |
| 색인 대조 (total_size / 텐서 집합) | 15082474368 == 15082474368 / 이름 집합 일치, 한쪽에만 있는 이름 0 |
| 파일 수 / 총 바이트 | 14개 / 15,085,104,997 바이트 (`du -sb`) |
| 4항목 존재 확인 | 아래 §4 — (a)-(d) 전부 확인 |
| 통계 파일 경로 (실습 3 입력) | `/workspace/models/openvla-maniskill-ft/dataset_statistics.json` |

## 1. 재머지 환경

학습 이미지(`openvla-train:v2`)의 버전 핀을 전용 venv 로 재현해 개발 컨테이너 안에서 실행했다. 머지는 bf16 가중치에 수정분을 더하는 CPU 작업이라 GPU 와 CUDA 휠은 쓰지 않았다.

| 패키지 | 재머지 venv | 학습 이미지 핀 |
|---|---|---|
| python | 3.11.15 (uv managed) | 3.11 |
| torch | 2.2.0+cpu | 2.2.0+cu121 |
| torchvision | 0.17.0+cpu | 0.17.0 |
| transformers | 4.40.1 | 4.40.1 |
| peft | 0.11.1 | 0.11.1 |
| timm | 0.9.10 | 0.9.10 |
| accelerate | 0.30.1 | (기록 없음) |
| numpy | 1.26.4 | - |

torch 가 cu121 이 아니라 cpu 휠인 것이 학습 이미지와의 유일한 의미 있는 차이다. 머지 연산은 CPU 에서 수행되므로 결과에 영향이 없고, 그 확인은 §3 색인 대조가 담당한다.

실행: `HF_HUB_OFFLINE=1 /workspace/venvs/remerge/bin/python scripts/practice_remerge.py` (cwd: `week4`). 오프라인 플래그로 캐시 밖 리비전이 새로 내려올 길을 차단했다.

## 2. base 리비전

| 항목 | 값 |
|---|---|
| 로컬 캐시 스냅샷 (유일) | `47a0ec7fc4ec123775a391911046cf33cf9ed83f` |
| `adapter_config.json` 의 `revision` | `null` (`base_model_name_or_path`: `openvla/openvla-7b`) |

어댑터가 base 리비전을 기록하지 않아(`revision: null`) 어댑터 쪽 기록과의 직접 대조는 불가능하다. 캐시 스냅샷이 PRACTICE.md 1-0 의 기대값과 일치하는 것까지가 이 확인의 범위다.

## 3. 색인 대조 (1-3 판정)

pod 머지가 남긴 색인(week3 회수물 `runs/.../model.safetensors.index.json`)과 재머지가 만든 색인을 대조했다.

| 항목 | pod 머지 (회수 색인) | 재머지 | 판정 |
|---|---|---|---|
| `metadata.total_size` | 15082474368 | 15082474368 | 일치 |
| `weight_map` 텐서 이름 집합 | - | - | 일치 (한쪽에만 있는 이름 0개) |

값 수준의 동일성(체크섬)은 pod 사본이 없어 확인할 수 없다. 그 몫은 실습 2 의 VRAM 대조와 실습 3 의 값 대역 검사가 기능적으로 대신한다.

## 4. 4항목 존재 확인 (1-4)

| 항목 | 소재 | 확인 값 |
|---|---|---|
| (a) 머지된 가중치 (15GB 급) | `/workspace/models/openvla-maniskill-ft/` | 파일 14개, 15,085,104,997 바이트 |
| (b) LoRA 어댑터 원본 | `week3/outputs/recovered/adapter-tmp/<EXP>/` | `adapter_model.safetensors` 484,458,600 바이트 + `adapter_config.json` |
| (c) 데이터셋 통계 파일 | `/workspace/models/openvla-maniskill-ft/dataset_statistics.json` | 2,298 바이트 (1-2 에서 회수물로부터 복사) |
| (d) processor / config + 학습 로그 | 설정은 (a) 디렉터리 안, 로그는 `week3/outputs/recovered/` | `preprocessor_config.json` 1,690 바이트, `tokenizer.json` 1,842,976 바이트 등 |

(a) 의 14개 구성: 가중치 조각 `model-0000{1-4}-of-00004.safetensors` 4개 + `model.safetensors.index.json` + `config.json` + `generation_config.json` (이상 재머지 산출) + sidecar 7개 (`dataset_statistics.json`, `preprocessor_config.json`, `tokenizer.json`, `tokenizer.model`, `tokenizer_config.json`, `added_tokens.json`, `special_tokens_map.json` — 1-2 에서 복사).

`du -sb` 값(15,085,104,997)이 색인의 `total_size`(15,082,474,368)보다 약 2.6MB 큰 것은 sidecar 와 safetensors 헤더가 더해진 차이다.

## 5. 재현 절차 포인터

- venv 구성과 실행 절차: `PRACTICE.md` 실습 1 (1-0 - 1-4)
- 재머지 스크립트: `scripts/practice_remerge.py`
- 색인 대조 실행: `scripts/practice.ipynb` (1-3 셀)
- 모델 가중치는 커밋하지 않는다 — 위치와 이 기록만 남긴다
