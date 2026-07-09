# Studies/Phase 4.5 — VLA v1.5: OpenVLA LoRA adaptation (before/after)


> **학습 기간**: Section 0 은 2026.08 (GPU 반납 전 전진 배치), Sections 1-3 은 2026.09-11 — 상세: [`Roadmap/Phase 4.5.md`](../../Roadmap/Phase%204.5.md)
> **마스터 가이드**: [`Roadmap/Phase 4.5.md`](../../Roadmap/Phase%204.5.md)
> **자료 미리 작성됨** — 진입 시 다시 체크 후 학습 시작


---


## 0. 시작하기 전 — 필수 진입점


Phase 4.5 의 모든 week 를 시작하기 *전에* [`SETUP.md`](SETUP.md) 를 1회 수행한다.

**왜 별도 단계가 필요한가**: v1.5 는 **학습(RunPod RTX 4090) + 추론/eval(로컬 4070 4-bit)** 분업을 전제로 한다. 이 분업·버전 매칭의 단일 진실 공급원은 [`Studies/Phase 4/SETUP.md`](../Phase%204/SETUP.md) 이며, 본 Phase 의 SETUP 은 그 위에 **adaptation 전용 추가분**(sim 데이터 생성, LoRA 학습 스크립트, eval harness)만 얹는다.


---


## 1. 본 Phase 의 한 줄 정체성


v1(zero-shot) 베이스라인 위에서 OpenVLA 를 sim 데이터로 LoRA adaptation 하고, 동일 task 성공률을 **before/after 로 정량 비교**한다. **성공률 상승이 목표가 아니라 adaptation 을 설계-실행-분석한 서사 자체가 산출물**이다 (Roadmap §성공 기준).


---


## 디렉토리 구조 (예정)


```
Studies/Phase 4.5/
  week1/ ~ week2/  # sim 데이터 생성 + OpenVLA 포맷 변환
  week3/ ~ week4/  # RunPod LoRA 학습 + 로컬 머지/양자화
  week5/ ~ week6/  # eval harness (N회) + before/after 분석 + 블로그/공개
  lora/            # LoRA 어댑터 (gitignore — *.pt/*.safetensors)
  eval/            # eval harness + 결과 표/그래프 (outputs/ gitignore)
  blog/            # adaptation 설계-실행-분석 블로그 초고
```


## week 자료 형식 (Phase 3-4 패턴)


각 week 디렉토리:
- `README.md` — 이번 주 목표 + 학습 순서 + 핵심 개념 + must/nice 2단
- `PRACTICE.md` — 실습 단계별 가이드 (진입 시 작성/갱신)
- `quiz_easy.py` / `quiz_medium.py` — 개념 / 코드 퀴즈 (진입 시 작성)


> 본 디렉토리는 **스캐폴드** 상태다. 각 week 의 PRACTICE/quiz 는 v1 완료 후 본 Phase 진입 시점에 작성·갱신한다 (다른 Phase 의 "미리 작성됨"보다 가벼운 상태 — v1.5 는 v1 자산 재사용 비중이 커서 진입 시 실측에 맞춰 채우는 편이 낫다).


---


## 참고


- 컴퓨트 분업 / 버전 매칭 / 가중치 전송: [`Studies/Phase 4/SETUP.md`](../Phase%204/SETUP.md) (단일 진실 공급원)
- v1 베이스라인(sim 환경 / 성공 task / 성공률 표): [`Studies/Phase 4/`](../Phase%204/)
