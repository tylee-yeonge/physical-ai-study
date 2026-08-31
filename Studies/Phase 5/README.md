# Studies/Phase 5 — Foundation Model 기초 (ViT / CLIP / DINOv2 / SigLIP)

> **⚠ 구판 (2026-08-31 판정)**: 본 디렉토리 전체는 OpenVLA 세대 전제로 작성됐다 (OpenVLA/OpenX 언급 22파일). 커리큘럼 자체 (ViT/CLIP/DINOv2/SigLIP 12주) 가 **재평가 #1 안건** (액션 토크나이저·flow-matching 교체 + 압축 여부 — master roadmap §5-1) 이므로 **결정 전 재작성 금지, 결정 후 재작성 전에는 사용 금지.** 주당 예상 시간 (5-10h) 도 예산 (주 6-8h) 초과 — 재작성 시 재산정.

> **학습 기간**: 2027.02-04 (3개월, 12주)
> **마스터 가이드**: [`Roadmap/Phase 5.md`](../../Roadmap/Phase%205.md)
> **자료 미리 작성됨** — 진입 시 (2027.02) 다시 체크 후 학습 시작


---


## 디렉토리 구조 (예정)


```
Studies/Phase 5/
  week1/ ~ week3/ # ViT
  week4/ ~ week6/ # CLIP
  week7/ ~ week9/ # DINOv2
  week10/ ~ week12/ # SigLIP + Phase 4 demo 보강
```


## week 자료 형식 (Phase 3 패턴)


- `README.md` / `PRACTICE.md` / `quiz_easy.py` / `quiz_medium.py` / `quiz_solutions/` / `requirements.txt`


## 학습 원칙
- **"동작 원리 수준"** — 아키텍처 다이어그램 + 학습 방식 + 입출력 인터페이스 설명 가능. **직접 학습 / fine-tune 은 하지 않음**.
- 각 모델별 *mini-demo 1개* 또는 짧은 노트 — Phase 4 의 ROS2 demo 보강에 활용
