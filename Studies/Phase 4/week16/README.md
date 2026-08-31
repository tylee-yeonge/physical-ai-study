# Week 16: 산출물 v1 공개 + 다음 phase 진입 준비


> **[v1 범위 조정 — 2026-06]** 이 주차(v1 외부 공개 + LinkedIn 공지)는 **v1 에서 제외하고 v2(Phase 6)로 이관**했다. v1 은 레포에 결과 기록만 남기고 외부 공개(vla-lab/LinkedIn)는 하지 않는다 — 첫 외부 공개는 실제 팔이 결합되는 v2 에서 수행한다. 아래 공개 체크리스트·LinkedIn 패턴·회고 틀은 v2 공개 시 참조용으로 보존한다. (Phase 4 회고 자체는 순서 5 의 8월 초 체크포인트에서 가볍게 수행 가능.) 결정 배경: `Roadmap/Phase 4.md` "산출물 v1" 절.


> **이번 주 목표**: 산출물 v1 (Portfolio/01_VLA_v1/) 를 GitHub Public 으로 공개 + LinkedIn 공지. Phase 5 진입 직전 정리. **Phase 4 마지막 주**.
> **예상 시간**: 4시간 (publishing 2h + LinkedIn / 회고 2h)
> **핵심 질문**: "내 산출물 v1 가 GitHub 검색 / LinkedIn 에 노출되는가? 면접관이 자연스럽게 발견할 수 있는가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | 최종 git push | `PRACTICE.md` 1 | Portfolio/01_VLA_v1/ |
| 2 | 메인 README 업데이트 | `PRACTICE.md` 2 | physical-ai-study/README.md |
| 3 | LinkedIn 공지 | `PRACTICE.md` 3 | post 작성 |
| 4 | 회고 | `PRACTICE.md` 4 | Phase 4 lessons learned |
| 5 | Phase 5 진입 준비 | `PRACTICE.md` 5 | 다음 학습 자료 검토 |
| 6 | 퀴즈 | quiz_easy / quiz_medium | release / 회고 |


---


## 이번 주 = Phase 4 의 마지막


본 주가 끝나면:
- **산출물 v1 완료**: vla-lab 문서 2편 + ROS2 minimal demo + 1분 영상
- **2026 하반기** 시점 본인이 가진 패: perception 로그 (Detection+Depth+TRT+ROS2, 비공개) + v1 (VLA) + v2 선행 (자작 팔 Stage 1)
- **2027 실지원** 의 핵심 카드 준비


다음 Phase: **Phase 5 (Foundation Model 기초)** 진입 (2027.02-04).


---


## 핵심 개념


### 1. GitHub Public 공개 체크리스트


```
- [ ] physical-ai-study repo 가 Public 인가
- [ ] Portfolio/01_VLA_v1/ 가 README 의 main entry 에 link
- [ ] secret / token 파일 commit 안 됨
- [ ] .gitignore 가 .env / credentials / __pycache__ 포함
- [ ] LICENSE 파일 (MIT)
- [ ] CITATION 파일 (선택)
- [ ] 모든 commit message 가 conventional commit 형식
```


### 2. 메인 README 의 산출물 v1 section


```markdown
# physical-ai-study


> AMR ROS 양산 SW + Physical AI 통합 - AMR ROS 실무 5년 (2021.06~) SW 엔지니어의 학습 + 산출물 레포


## Portfolio (산출물)


| 산출물 | 시점 | 내용 | Link |
|---|---|---|---|
| (로그) | - | Detection + Depth + PC TensorRT + ROS2 (Phase 3, 비공개) | repo 내부 |
| v1 | 2026 하반기 | OpenVLA + ROS2 + sim 단일 task 루프 + vla-lab 문서 | [01_VLA_v1](Portfolio/01_VLA_v1) |
| v2 선행 | 2027 | 자작 팔 Stage 1 (2~3DOF + URDF) | [Hardware-Arm/Stage1](...) |
| v3 | 2027 후반~ | (예정) Real-to-Sim-to-Real (정점) | (작성중) |


(자세한 로드맵: [Roadmap/](Roadmap/))
```


### 3. LinkedIn 공지 post 의 표준 패턴


```
Phase 4 산출물 v1 공개:
OpenVLA -> ROS2 Minimal Demo


VLA 의 정독 (RT-2 / OpenVLA) 과 실측 (RTX 4070, 4-bit nf4, 165ms latency)
을 ROS2 환경에서 minimal demo 로 통합했습니다.


vla-lab 문서 2편:
- RT-2 정독 노트 - OpenVLA + RTX 4070 실측 

ROS2 패키지 + 1분 영상:
[GitHub link]


다음 산출물:
Real-to-Sim-to-Real (Phase 7, 2027.08~) - 자작 6DOF 팔 + Isaac Sim + OpenVLA


#VLA #OpenVLA #ROS2 #PhysicalAI #AMR
```


### 4. Phase 4 회고 (lessons learned)


```markdown
# Phase 4 회고 (2026.06 ~ 2026.09)


## 잘 한 것
- vla-lab 문서 2편 (RT-2 + OpenVLA) 시리즈로 일관성
- RTX 4070 실측 (165ms, 6Hz) 의 정량적 증거
- ROS2 통합으로 양산 SW 색깔 명확


## 아쉬운 것
- (실제 학습 후 채울 부분)
- 처음 ROS2 통합 시 OOM 발생 빈도 ___
- 자막 작업이 예상보다 오래 걸림 ___


## 다음에 적용할 것
- ___
- ___


## 시장 시그널 (2026.06~ 시장 신호 probe 결과)
- ___ 건 지원 / ___ 건 응답
- 면접관 질문 패턴: ___
- 강조한 산출물: ___
```


### 5. Phase 5 진입 준비


```
- [ ] Studies/Phase 5/ 자료 확인 (week 1~12)
- [ ] ViT / CLIP / DINOv2 / SigLIP 의 사전 지식 점검
- [ ] HuggingFace transformers 환경 그대로 사용 가능
- [ ] 분기 재평가 (2026.11) 결과 반영 - 모델 갱신 여부
```


### 6. 분기 재평가 (2026.11)


본 phase 의 마지막 주 (2026.12) 가까이 분기 재평가 (#1) 시점:


```
재평가 항목:
- 시장 신호 probe 결과 (JD 격차 / 커피챗 / 실지원 1건 반응)
- Phase 4 진행률 (산출물 v1 완성)
- 자작 팔 Stage 1 완성 여부 (산출물 v2 선행)
- 시장 시그널 (1순위 회사 채용 활성도, OpenVLA 후속 모델)


판단:
- 시그널 좋음 + v1+스파이크 확보 -> 2027 실지원 개시 (기본선)
- 시그널 약함 + 시장 정체 -> 2028.03 fallback + Jetson 옵션
- OpenVLA 가 한 세대 뒤 -> 2027.05 재평가 시 모델 갱신
```


---


## 자체 점검


**Q1. Phase 4 의 산출물 v1 의 3 요소는?**
> vla-lab 문서 2편 (RT-2 + OpenVLA) + OpenVLA → ROS2 minimal demo + 1분 영상.


**Q2. 산출물 v1 의 발행 후 즉시 해야 할 것은?**
> LinkedIn 공지 + 본 레포 메인 README 의 Portfolio 표 업데이트.


**Q3. Phase 5 진입 직전 검토할 것은?**
> Studies/Phase 5/ 의 week 자료 + 분기 재평가 결과 + 모델 갱신 검토.


**Q4. 회고 의 가장 중요한 부분?**
> "다음에 적용할 것" - 추상적 lessons 가 아닌 구체적 next action.


**Q5. 2026.11 분기 재평가의 핵심 판단?**
> 스파이크 결과 + 시장 신호 probe 반응을 보고 2028.03 fallback 진입 여부 / 모델 갱신 결정 (실지원은 2027 기본 개시).


---


## 이번 주 실습 & Phase 5 준비


### 이번 주 실습 과제
1. Portfolio/01_VLA_v1/ 최종 git push
2. 메인 README 업데이트 (Portfolio 표)
3. LinkedIn post 공개
4. Phase 4 회고 작성 (`~/phase4_notes/retro.md`)
5. 분기 재평가 #1 결과 반영 (있다면)
6. Phase 5 자료 검토 시작
7. quiz_easy / quiz_medium


### Phase 5 진입 준비 (2027.01 말 ~ 2027.02 초)
- ViT 의 patch embedding 사전 지식 점검
- CLIP / SigLIP 의 차이 한 줄 정리
- DINOv2 의 self-supervised 학습 사전 지식
- HuggingFace transformers + datasets 라이브러리 그대로 사용


---


## 이번 주 핵심 요약


1. **산출물 v1 공개** = Phase 4 의 종착점.
2. **LinkedIn 공지** 로 시장 노출.
3. **메인 README 업데이트** Portfolio 표 채움.
4. **회고** 다음 phase 에 적용할 구체 action.
5. **2026.11 분기 재평가** 반영.


---


- 이전: [Week 15 - 영상 마감 + 자막](../week15/README.md)


다음: [Phase 5 - Foundation Model 기초 (Studies/Phase 5/)](../../Phase%205/README.md)
