# 실행 환경 — openvla-rtx4070-int4

> 측정일: 2026-06 (Phase 4 순서 1, week6 실습 1-2)
> 측정 기록 commit: `1652c81` (chore(phase4): record week6 latency run outputs in notebook)

## Hardware

| 항목 | 값 |
|---|---|
| GPU | NVIDIA RTX 4070 12GB (메모리 대역폭 504 GB/s) |
| 호스트 | Ubuntu PC (원격 접속 메인 장비) |

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

버전 고정의 근거 주석: `Studies/Phase 4/week8/requirements.txt`, 매트릭스: `Studies/Phase 4/SETUP.md` §7.1.

## 모델

| 항목 | 값 |
|---|---|
| 모델 | `openvla/openvla-7b` (HuggingFace) |
| 양자화 | bitsandbytes 4-bit nf4, double quant, compute dtype fp16 |
| 로드 결과 | RTX 4070 12GB 에 OOM 없이 안착 |
