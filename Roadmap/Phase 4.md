# Phase 4: VLA 논문 reading + OpenVLA → ROS2 minimal demo

> [time] **기간**: 약 4개월 (2026.09~12, F안)
> [goal] **목표**: VLA 의 아키텍처 다이어그램을 막힘없이 읽을 수 있는 수준 + OpenVLA inference 를 ROS2 토픽으로 받는 minimal demo
> [code] **언어**: **Python** + **ROS2 (rclpy)**
> [tool] **하드웨어**: Ubuntu PC (RTX 4070) — HuggingFace inference / ROS2 노드 / 시각화
> [time] **주간 시간**: 약 8~12시간 (출장 주 50% 보정)

---

## -> **실습 가이드**: `Studies/Phase 4/weekN/` (각 week 별 README + PRACTICE + quiz 미리 작성됨. **진입 시 (2026.09) 다시 체크**)

**핵심 산출물 (#2)**:
- RT-2 + OpenVLA 블로그 2편 (각 1편)
- OpenVLA HuggingFace inference → ROS2 토픽 minimal demo + 1분 영상

**산출물 #2** (2026.12까지 `physical-ai-study` 레포 공개):
- 블로그 2편 (RT-2, OpenVLA 의 아키텍처 / 학습 / 데이터 / inference 흐름)
- ROS2 패키지: OpenVLA inference → `vla_action` 토픽 publish + 시각화

> [!] **선정 논문 (2편)**: RT-2, OpenVLA — 2026.11 분기 재평가에서 π0 / Helix / GR00T 등으로 갱신 가능. 본 마스터는 *시점 기준 OpenVLA 가 표준* 이라는 가정으로 작성.

> [!] **#4 결정타의 예고편**: 본 Phase 의 ROS2 minimal demo 가 Phase 7 의 *Real-to-Sim-to-Real* 산출물 #4 의 토대. 여기서 익힌 inference 파이프라인 + ROS2 통합이 Phase 7 에서 자작 팔과 결합됨.

---

## [tool] 학습 환경

| 단계 | 주 장비 | 출장지 가능 여부 |
|---|---|---|
| RT-2 / OpenVLA 논문 정독 | 디바이스 무관 | O |
| HuggingFace inference 셋업 | Ubuntu PC (원격) | O |
| ROS2 패키지 작성 | Ubuntu PC (원격) | O |
| 블로그 작성 | 디바이스 무관 | O |

- VRAM 요구사항: OpenVLA 7B 기준 ~14GB. RTX 4070 12GB 로는 *4-bit quantization* 필요할 수 있음. 또는 더 작은 변종 시도.
- 상세: [ENVIRONMENT.md](../ENVIRONMENT.md)

---

## [list] Section 6.0: 시작 전 (Phase 3 산출물 #1 공개 후)

- [ ] OpenVLA HuggingFace 모델 카드 + 라이선스 확인
- [ ] PC VRAM 점검 (7B 모델 inference 가능성, 4-bit quantization 옵션 확인)
- [ ] 블로그 플랫폼 선정 (Velog / Medium / 본 레포의 `Studies/Phase 4/blog/`)
- [x] **Studies/Phase 4/ week 1~16 자료** 미리 작성됨. 진입 시 (2026.09) 본문 다시 체크.

---

## [list] Section 6.1: RT-2 정독 + 블로그 1편 (3주)

> Vision-Language → Action 의 원조 (Google DeepMind, 2023)

| 주차 | 내용 | 핵심 |
|------|------|------|
| 1 | RT-2 논문 1회독 + Architecture Diagram 정독 | PaLI-X / PaLM-E 기반 |
| 2 | Co-fine-tuning + Action tokenization 이해 | "Action 도 토큰" |
| 3 | 블로그 1편 작성 (RT-2) | 핵심 아키텍처 + 데이터 + 한계 |

---

## [list] Section 6.2: OpenVLA 정독 + 블로그 1편 (4주)

> open-source VLA 의 표준 (Stanford, 2024)

| 주차 | 내용 | 핵심 |
|------|------|------|
| 4 | OpenVLA 논문 1회독 + Architecture | Llama 7B + DINOv2 + SigLIP |
| 5 | OpenX-Embodiment 데이터 + Fine-tuning 흐름 | 970K episodes |
| 6 | OpenVLA HuggingFace 모델 카드 + 환경 셋업 | `transformers` + GPU |
| 7 | 블로그 1편 작성 (OpenVLA) | RT-2 와 비교 + open-source 의 의미 |

---

## [list] Section 6.3: OpenVLA HuggingFace → ROS2 minimal demo (5주)

> Brain ↔ Body 의 첫 통합. Phase 7 의 예고편.

| 주차 | 내용 | 핵심 |
|------|------|------|
| 8 | HuggingFace inference 셋업 | 4-bit quantization (필요 시) |
| 9 | inference 입력/출력 인터페이스 정리 | image + instruction → action |
| 10 | ROS2 패키지 골격 작성 | `vla_node` 노드 + `vla_action` 토픽 |
| 11 | image subscribe → inference → action publish | 단순 동영상 입력으로 dry-run |
| 12 | Rerun 시각화 + 1분 영상 제작 | Demo 영상 마감 |

**ROS2 인터페이스 예시**:
```
Input  : /camera/image_raw (sensor_msgs/Image), /vla/instruction (std_msgs/String)
Output : /vla/action (geometry_msgs/Twist 또는 custom msg)
```

---

## [list] Section 6.4: 블로그 마무리 + 패키징 (4주)

| 주차 | 내용 | 핵심 |
|------|------|------|
| 13 | 블로그 2편 퇴고 + 다이어그램 작성 | 면접관 진입점 |
| 14 | ROS2 demo README + 환경 세팅 가이드 | 재현성 |
| 15 | 1분 데모 영상 마감 + 자막 | 패키징 |
| 16 | 산출물 #2 공개 (`physical-ai-study/Portfolio/02_VLA_demo/`) | 2026.12 |

---

## [O] Phase 4 완료 체크리스트

### 논문 reading
- [ ] RT-2 아키텍처 다이어그램을 막힘없이 설명 가능
- [ ] OpenVLA 아키텍처 다이어그램을 막힘없이 설명 가능
- [ ] RT-2 와 OpenVLA 의 차이 (open-source / 모델 크기 / 데이터) 정리

### 블로그
- [ ] RT-2 블로그 1편 공개
- [ ] OpenVLA 블로그 1편 공개

### ROS2 minimal demo
- [ ] OpenVLA HuggingFace inference 동작 (PC RTX 4070)
- [ ] `vla_node` ROS2 패키지 빌드 + 실행
- [ ] image subscribe → inference → action publish 흐름 확인
- [ ] 1분 데모 영상 제작

### 산출물 #2 공개
- [ ] `physical-ai-study` 레포 산출물 #2 디렉토리 정리
- [ ] README + 영상 + 블로그 링크

---

## [goal] Phase 4 완료 기준

> "RT-2 와 OpenVLA 의 아키텍처를 막힘없이 설명할 수 있고, OpenVLA HuggingFace inference 를 ROS2 토픽으로 받는 minimal demo 를 동작시킬 수 있다."

---

## [ref] 참고 자료

### 논문
- RT-2 (Google DeepMind, 2023): https://robotics-transformer2.github.io/
- OpenVLA (Stanford, 2024): https://openvla.github.io/

### 도구
- HuggingFace transformers: https://huggingface.co/docs/transformers
- OpenVLA HuggingFace 모델: https://huggingface.co/openvla
- ROS2 (Humble 또는 Iron): https://docs.ros.org/

---

## [?] 다음 단계

Phase 4 완료 후 (2026.12):
- **Phase 5: Foundation Model 기초** (ViT / CLIP / DINOv2 / SigLIP, 동작 원리 수준, 2026.12~2027.02)
- 동시기 진행: **Hardware-Arm Stage 1 마무리** (산출물 #2.5)
- 동시기 진행: **정찰 지원 2~3건** (2026.11~12)
- **6개월 분기 재평가 #1 (2026.11)** — Phase 4 진행률 / VLA 모델 갱신 검토 / 정찰 지원 반응
