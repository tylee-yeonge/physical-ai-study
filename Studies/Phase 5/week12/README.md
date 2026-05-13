# Week 12: Phase 4 demo 보강 + Phase 5 회고 + Phase 6 진입


> **이번 주 목표**: Phase 4 의 ROS2 demo 영상을 CLIP 시각화 / DINOv2 feature 등으로 보강 (선택). Phase 5 회고 + Phase 6 진입 직전 정리. **Phase 5 마지막 주**.
> **예상 시간**: 5시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | Phase 4 demo 보강 결정 | `PRACTICE.md` 1 | 선택, 시간 여유 |
| 2 | (선택) CLIP zero-shot ROS2 통합 | `PRACTICE.md` 2 | week 6 의 코드 |
| 3 | Phase 5 회고 | `PRACTICE.md` 3 | retro.md |
| 4 | Phase 6 진입 준비 | `PRACTICE.md` 4 | Isaac Sim 환경 점검 |
| 5 | 6개월 분기 재평가 #2 (2027.05) 준비 | `PRACTICE.md` 5 | meta-rule |


---


## 본 주가 Phase 5 의 끝


본 주 이후:
- **Phase 5 종료** (2027.02 추정)
- 가능 시 **Phase 4 demo 보강** (산출물 #2 의 v1.1)
- **Phase 6 진입** (Isaac Sim + 디지털 트윈, 2027.02~05)
- **자작 팔 Stage 2** 와 병행 (2027.02~04)


---


## 핵심 개념


### 1. Phase 4 demo 보강 (선택)


본 phase 가 끝나는 2027.02 시점에 Phase 4 영상이 발행된 지 약 2개월 경과. 시청자 / 면접관 피드백 반영 가능:


- CLIP zero-shot top class 를 영상에 추가
- DINOv2 patch feature 시각화 추가
- "vision encoder 의 component-level latency" 새로운 측정


업데이트 영상을 **vla_demo_v2.mp4** 로 발행.


### 2. (선택) CLIP ROS2 통합


week 6 의 `clip_node.py` 를 vla_node 와 함께 실행:


```bash
ros2 run vla_node vla_inference_node &
ros2 run vla_node clip_node &
ros2 run vla_node rerun_logger &
```


Rerun 에서 3 layer 시각화:
- camera image
- CLIP top class
- OpenVLA action


### 3. Phase 5 회고


`~/phase5_notes/retro.md`:


```markdown
# Phase 5 회고 (2026.12 ~ 2027.02)


## 1. 시간 분배 vs 실제
| Section | 계획 | 실제 |
|---|---|---|
| ViT (week 1-3) | 3 주 | ___ |
| CLIP (week 4-6) | 3 주 | ___ |
| DINOv2 (week 7-9) | 3 주 | ___ |
| SigLIP + 통합 (week 10-12) | 3 주 | ___ |


## 2. 잘 한 것
- ___
- ___


## 3. 아쉬운 것
- ___


## 4. 다음 phase 에 적용할 것
- ___


## 5. Phase 6 / 7 진입 준비도
- [ ] Isaac Sim 환경 점검
- [ ] 자작 팔 Stage 2 진입 (2027.02)
- [ ] 6개월 분기 재평가 #2 (2027.05) 준비
```


### 4. Phase 6 진입 직전 점검


```
환경:
- [ ] Isaac Sim 시스템 요구사항 확인
       (RTX 4070 12GB 가능, 단 ROS2 와 동시 부하 주의)
- [ ] conda env phase6 별도 권장
- [ ] Studies/Phase 6/ 의 week 자료 검토


자작 팔 Stage 2:
- [ ] 추가 hardware 발주 검토 (Dynamixel XM430 x 3, ~100~150만원)
- [ ] Stage 1 의 URDF 를 Stage 2 의 base 로
```


### 5. 6개월 분기 재평가 #2 (2027.05 예정)


본 phase 종료 ~ 3개월 후가 재평가 시점:


```
재평가 항목:
- Phase 5 (Foundation Model) 종료 시점
- 자작 팔 Stage 2 완성도 (~2027.04 예정)
- VLA 모델 선정 재검토:
  - OpenVLA 유지?
  - π0 / Helix / GR00T 로 갱신?
- Phase 6 (Isaac Sim) 진입 준비도


판단:
- 시그널 좋음 + Phase 6 정상 -> 원안 (2027.07 본격 지원)
- 신 VLA 모델 등장 -> 모델 갱신
- 자작 팔 지연 -> Phase 6 진입 늦음
```


### 6. Phase 5 종합 산출물 list


```
~/phase5_notes/
  week1/vit_one_pager.md
  week2/vit_benchmark.md
  week3/openvla_breakdown.md
  week4/clip_one_pager.md
  week5/...
  week6/clip_classifier.py
  week7/dinov2_one_pager.md
  week8/dinov2_pca.png
  week9/openvla_arch_v2.md
  week10/siglip_one_pager.md
  week11/phase5_summary.md
  week12/retro.md
```


각 week 의 노트가 누적되어 phase 종합 노트가 됨.


---


## 자체 점검


**Q1. Phase 4 demo 보강의 정당성?**
> 2개월 동안 추가 학습 / 데이터 / 피드백 반영. v1.1 영상으로 LinkedIn 재공개 가능.


**Q2. Phase 5 회고의 핵심 질문?**
> "Phase 6/7 진입 준비도" 와 "다음 phase 에 적용할 것".


**Q3. 6개월 분기 재평가 #2 의 핵심 결정?**
> VLA 모델 갱신 여부 (OpenVLA 유지 or π0 등). 시장 변화 큼.


**Q4. Phase 6 진입 직전 환경 점검?**
> Isaac Sim 시스템 요구사항 + RTX 4070 OOM 가능성 + conda env 분리.


**Q5. Phase 5 의 가장 큰 자산?**
> "OpenVLA backbone 의 모든 component 를 설명 가능" 의 수준. Phase 6/7 의 토대.


---


## 실습 + Phase 6 준비


### 이번 주 실습 과제
1. (선택) Phase 4 demo v2 영상 제작
2. CLIP ROS2 통합 (week 6 코드)
3. Phase 5 회고 (`retro.md`)
4. Phase 6 진입 직전 환경 점검
5. 6개월 분기 재평가 #2 일정 캘린더 등록
6. quiz


### Phase 6 진입 (2027.02~)
- Isaac Sim 환경 셋업
- URDF 임포트 + 디지털 트윈 첫 사이클
- Sim/Real gap 측정 인프라


---


## Phase 5 핵심 요약 (마지막 주)


1. **4 vision foundation model** 의 정리 (ViT/CLIP/DINOv2/SigLIP).
2. **OpenVLA vision backbone** ~630M (DINOv2 + SigLIP + projector).
3. **Phase 4 demo 보강** 선택 (v1.1).
4. **Phase 6 진입 직전** 환경 점검 + 자작 팔 Stage 2.
5. **6개월 분기 재평가 #2** (2027.05) 준비.


---


- 이전: [Week 11](../week11/README.md)


다음: [Phase 6 - Isaac Sim + 디지털 트윈 (Studies/Phase 6/)](../../Phase%206/README.md)
