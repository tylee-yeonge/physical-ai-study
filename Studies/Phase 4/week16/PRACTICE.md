# Week 16 실습: 산출물 #2 공개 + 회고 + Phase 5 진입 준비


> **실습 목표**: Phase 4 의 모든 산출물 공개 + 회고 + Phase 5 진입 직전 정리.
> **예상 시간**: 4시간


---


## 실습 1: 최종 git push + tag


```bash
cd ~/code/physical-ai-study
git status


# 모든 파일 commit
git add Portfolio/02_VLA_demo/
git commit -m "phase4: release 산출물 #2 (OpenVLA + ROS2 demo + 1분 영상)"


# Phase 4 완료 tag
git tag -a v0.4 -m "Phase 4 complete: VLA papers + OpenVLA ROS2 demo"
git push origin main
git push origin v0.4


# Public 확인
gh repo view --json visibility
# 또는 GitHub UI: Settings -> Visibility = Public
```


---


## 실습 2: 메인 README 업데이트


**파일명**: `~/code/physical-ai-study/README.md`


산출물 #2 row 를 추가:


```markdown
| 2 | 2026.12 | OpenVLA + ROS2 minimal demo + 블로그 2편 | [02_VLA_demo](Portfolio/02_VLA_demo) |
```


```bash
git add README.md
git commit -m "docs: add 산출물 #2 to main portfolio table"
git push
```


---


## 실습 3: LinkedIn post


**파일명**: `~/phase4_notes/week16/linkedin_post.md`


```markdown
# Phase 4 산출물 #2 공개


VLA (Vision-Language-Action) 의 정독 + 실측 + ROS2 통합 minimal demo 를 공개했습니다.


학습 흐름:
- RT-2 정독 (closed) + 블로그 1편
- OpenVLA 정독 (open) + RTX 4070 직접 실측 + 블로그 1편
- OpenVLA HuggingFace inference -> ROS2 토픽 minimal demo
- 1분 시연 영상


실측 결과 (RTX 4070, 4-bit nf4):
- Mean latency: 165 ms
- Throughput: 6 Hz
- GPU memory: 5.3 GB (model) + 1.0 GB (inference)


양산 SW 관점에서의 메시지:
6Hz throughput 은 cm 단위 manipulation 에 충분, 30Hz 실시간 제어는 불가.
양산 통합에는 hierarchical (slow VLA + fast safety) 구조 필수.


블로그 2편:
- RT-2 정독 노트: - OpenVLA + RTX 4070 실측: 

ROS2 패키지 + 1분 영상:
[GitHub link]


다음 산출물 (Phase 7, 2027.08~): Real-to-Sim-to-Real
- 자작 6DOF 팔 (Dynamixel)
- Isaac Sim 디지털 트윈
- OpenVLA fork + LoRA fine-tune
- ROS2 노드 + 안전 인터록
- latency 측정 + Sim/Real gap 영상


#VLA #OpenVLA #ROS2 #PhysicalAI #AMR #양산SW
```


LinkedIn 에 paste + image (thumbnail 추가).


---


## 실습 4: Phase 4 회고


**파일명**: `~/phase4_notes/retro.md`


```markdown
# Phase 4 회고 (2026.09 ~ 2026.12)


## 1. 시간 분배 vs 실제
| Section | 계획 | 실제 |
|---|---|---|
| 6.1 RT-2 (week 1-3) | 3 주 | ___ 주 |
| 6.2 OpenVLA (week 4-7) | 4 주 | ___ 주 |
| 6.3 ROS2 demo (week 8-12) | 5 주 | ___ 주 |
| 6.4 패키징 (week 13-16) | 4 주 | ___ 주 |


## 2. 잘 한 것
- ___
- ___


## 3. 아쉬운 것
- ___
- ___


## 4. 예상 못한 어려움
- ___ (예: OOM 발생 빈도, 모델 다운로드 시간 등)


## 5. 다음 phase 에 적용할 것
- ___
- ___


## 6. 2026.11 분기 재평가 결과
시장 신호 probe 결과:
- 응답률: ___
- 면접관 질문 패턴: ___
- 강조 효과 큰 산출물: ___


판단:
- [ ] 2027.02 실지원 개시 (기본)
- [ ] 시점 앞당김
- [ ] fallback 진입 (2028.03)
- [ ] 모델 갱신 (OpenVLA -> π0 / Helix / GR00T)
```


---


## 실습 5: Phase 5 진입 준비


```bash
# Phase 5 자료 확인
cd ~/code/physical-ai-study/Studies/"Phase 5"
ls
# 기대: week1 ~ week12 + README.md


# 첫 주 (week1: ViT) 의 README 부터 정독
cat "week1/README.md"
```


Phase 5 의 미리 체크할 것:


```markdown
- [ ] week 1~3 (ViT) 의 사전 지식: Transformer 기본 (Phase 4 에서 이미 정독)
- [ ] week 4~6 (CLIP) 의 사전 지식: contrastive learning
- [ ] week 7~9 (DINOv2) 의 사전 지식: self-supervised
- [ ] week 10~12 (SigLIP + 통합) 의 사전 지식: sigmoid loss
```


> Phase 5 의 깊이는 '동작 원리 수준' (아키텍처 다이어그램 + 학습 방식 + 입출력 인터페이스). 너무 깊이 들어가지 말 것.


---


## 실습 체크리스트


- [ ] git push + tag v0.4
- [ ] 메인 README 업데이트
- [ ] LinkedIn post 공개
- [ ] retro.md 작성
- [ ] Phase 5 자료 검토 시작
- [ ] (선택) 2026.11 분기 재평가 결과 반영
- [ ] quiz_easy / quiz_medium


---


## 참고 자료


- [GitHub release 문서](https://docs.github.com/en/repositories/releasing-projects-on-github)
- [LinkedIn post best practices](https://www.linkedin.com/help/linkedin/answer/a516086)
