# Phase 5: Foundation Model 기초

> [time] **기간**: 약 3개월 (2026.12~2027.02, F안 — v2 의 4개월에서 압축)
> [goal] **목표**: ViT / CLIP / DINOv2 / SigLIP 의 *동작 원리 수준* — 아키텍처 다이어그램 + 학습 방식 + 입출력 인터페이스 설명 가능. **직접 학습 / fine-tune 은 하지 않음**.
> [code] **언어**: **Python** (HuggingFace transformers)
> [tool] **하드웨어**: Ubuntu PC (RTX 4070)
> [time] **주간 시간**: 약 8~12시간 (출장 주 50% 보정)

---

## -> **실습 가이드**: `Studies/Phase 5/weekN/` (각 week 별 README + PRACTICE + quiz 미리 작성됨. **진입 시 (2026.12) 다시 체크**)

**산출물**: 없음 (사전 지식 Phase). 단, 각 모델별 짧은 노트 또는 mini-demo 1개 — Phase 4 의 ROS2 demo 보강에 활용 가능.

> [!] **목적**: Phase 4 의 OpenVLA 가 *어떤 vision encoder + LLM 토대* 위에 서 있는지 이해. Phase 6, 7 의 인코더 선택 및 latency 분석의 사전 지식.
>
> "동작 원리 수준" 의 의미: 아키텍처 다이어그램 + 학습 방식 + 입력/출력 인터페이스를 설명할 수 있는 수준. 직접 학습 / fine-tune 은 *하지 않음*.

---

## [tool] 학습 환경

- 주 장비: Ubuntu PC (RTX 4070)
- 도구: HuggingFace transformers, datasets, mini 시각화
- 출장지 가능 여부: O (전 주차)
- 상세: [ENVIRONMENT.md](../ENVIRONMENT.md)

---

## [list] Section 7.0: 시작 전 (Phase 4 종료 후)

- [ ] **6개월 분기 재평가 #1 결과 반영** (2026.11) — VLA 모델 변경 결정 시 본 Phase 의 인코더 선택도 갱신
- [x] **Studies/Phase 5/ week 1~12 자료** 미리 작성됨. 진입 시 (2026.12) 본문 다시 체크.

---

## [list] Section 7.1: ViT — Vision Transformer (3주)

> Patch embedding + Self-attention. 모든 vision Transformer 의 토대.

| 주차 | 내용 | 핵심 |
|------|------|------|
| 1 | ViT 논문 정독 + Architecture | Patch → Linear → Transformer |
| 2 | HuggingFace `transformers` 의 ViT 모델 inference | Pretrained 모델로 ImageNet 분류 |
| 3 | OpenVLA 의 backbone 과의 관계 정리 | Phase 4 와 연결 |

---

## [list] Section 7.2: CLIP — Vision-Language Contrastive (3주)

> Vision + Text 의 공통 표현 공간. VLA 의 "이해" 부분의 토대.

| 주차 | 내용 | 핵심 |
|------|------|------|
| 4 | CLIP 논문 정독 + Contrastive Learning | Image ↔ Text alignment |
| 5 | HuggingFace CLIP inference + similarity | image embedding ↔ text embedding |
| 6 | open-vocabulary detection / classification mini-demo | "label-free" 의 의미 |

---

## [list] Section 7.3: DINOv2 — Self-Supervised Vision (3주)

> Label 없이 학습된 vision encoder. OpenVLA 의 vision backbone.

| 주차 | 내용 | 핵심 |
|------|------|------|
| 7 | DINOv2 논문 정독 + Self-Distillation | Teacher-Student |
| 8 | HuggingFace DINOv2 inference + feature 시각화 | Patch-level features |
| 9 | OpenVLA 의 vision backbone 으로서의 역할 정리 | Phase 4 의 architecture 다이어그램 재정리 |

---

## [list] Section 7.4: SigLIP + Phase 4 demo 보강 (3주)

> CLIP 의 sigmoid 변종 (스케일 친화적). OpenVLA 의 또 다른 backbone.

| 주차 | 내용 | 핵심 |
|------|------|------|
| 10 | SigLIP 논문 정독 + Sigmoid loss 의 의미 | CLIP softmax 대비 |
| 11 | OpenVLA 의 DINOv2 + SigLIP 통합 이해 | dual-encoder |
| 12 | Phase 4 ROS2 demo 보강 + Phase 6 진입 준비 | 인코더 latency 측정 추가 |

---

## [O] Phase 5 완료 체크리스트

- [ ] ViT 아키텍처 + Patch embedding 설명 가능
- [ ] CLIP 의 contrastive learning 설명 가능
- [ ] DINOv2 의 self-distillation 설명 가능
- [ ] SigLIP 와 CLIP 의 차이 설명 가능
- [ ] OpenVLA 의 vision encoder (DINOv2 + SigLIP) 의 역할 정리
- [ ] 각 모델별 mini-demo 또는 짧은 노트 1개

---

## [goal] Phase 5 완료 기준

> "OpenVLA 가 *어떤 vision encoder + LLM 토대* 위에 서 있는지 아키텍처 다이어그램으로 설명할 수 있고, 각 컴포넌트의 학습 방식을 이해한다."

---

## [ref] 참고 자료

- ViT: https://arxiv.org/abs/2010.11929
- CLIP: https://arxiv.org/abs/2103.00020
- DINOv2: https://arxiv.org/abs/2304.07193
- SigLIP: https://arxiv.org/abs/2303.15343
- HuggingFace transformers: https://huggingface.co/docs/transformers

---

## [?] 다음 단계

Phase 5 완료 후 (2027.02):
- **Phase 6: Isaac Sim + 디지털 트윈** (2027.02~05) — 자작 팔 Stage 2 와 병행
- **Hardware-Arm Stage 2** (2027.02~04) — 6DOF 확장 + teleop + 안전 인터록
- **6개월 분기 재평가 #2 (2027.05)** — VLA 모델 갱신 검토 (OpenVLA 유지 or π0/Helix)
