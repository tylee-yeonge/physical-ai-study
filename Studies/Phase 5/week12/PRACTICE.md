# Week 12 실습: Phase 4 보강 + Phase 5 회고 + Phase 6 준비


> **예상 시간**: 4시간


---


## 실습 1: Phase 4 demo 보강 결정


판단:
- 시간 여유 있음 -> v2 영상 제작
- 시간 부족 -> 노트 / vla-lab 문서 추가만


v2 영상의 추가 요소:
- CLIP zero-shot top class 시각화
- DINOv2 patch feature PCA 시각화
- Vision encoder component-level latency


---


## 실습 2: CLIP ROS2 통합 (선택)


```bash
# week 6 의 clip_node 빌드
cd ~/ros2_ws
colcon build --packages-select vla_node --symlink-install
source install/setup.bash


# 통합 실행
ros2 run vla_node vla_inference_node &
ros2 run vla_node clip_node --ros-args -p "labels:=cup,mouse,keyboard,robot arm" &
ros2 run vla_node rerun_logger &


# image bag 재생
ros2 bag play my_test_bag --loop --clock
```


Rerun 시각화:
- camera/image
- vla/action (line chart)
- clip/top_class (text log)


---


## 실습 3: Phase 5 회고 작성


`~/phase5_notes/retro.md`:


```markdown
# Phase 5 회고


## 시간 분배
[표 채우기]


## 잘 한 것
- 4 vision foundation 모델의 정리
- OpenVLA backbone 완전 이해
- ___


## 아쉬운 것
- ___


## 다음 phase (6, 7) 에 적용할 것
- ___ (구체적 action)


## 분기 재평가 #2 준비 (2027.05)
- Phase 5 학습 효과 평가
- 자작 팔 Stage 2 완성도
- VLA 모델 갱신 검토 (π0, Helix, GR00T 시점)
```


---


## 실습 4: Phase 6 환경 점검


```bash
# Isaac Sim 4.x 시스템 요구사항
# - RTX 30 series 이상 (4070 OK)
# - VRAM 8GB+ (4070 12GB OK)
# - RAM 32GB+
# - Ubuntu 22.04
# - python 3.10


# 사전 download (sim 환경)
# Isaac Sim 의 omniverse-launcher 설치
# 또는 standalone conda 환경
```


`~/phase5_notes/week12/phase6_prep.md`:


```markdown
# Phase 6 진입 준비


## 시스템 점검
- [ ] RTX 4070: OK
- [ ] VRAM 12GB: OK
- [ ] RAM 32GB: ___
- [ ] Disk 100GB+: ___


## 자작 팔 Stage 2 진입 준비
- [ ] Stage 1 의 URDF
- [ ] Stage 2 BOM 발주 (Dynamixel XM430 x 3, ~150만)
- [ ] Studies/Hardware-Arm/stage2/ 자료 검토


## Phase 6 자료 검토
- [ ] Studies/Phase 6/week1~12 README 훑기
- [ ] week 1: Isaac Sim 환경 셋업 시간 예상
```


---


## 실습 5: 분기 재평가 #2 (2027.05) 캘린더 등록


```
캘린더 알림: 2027.05.01
제목: 6개월 분기 재평가 #2
체크 항목:
- Phase 5 종료 시점
- 자작 팔 Stage 2 완성도 (~2027.06)
- VLA 모델 선정 재검토
- Phase 6 (Isaac Sim) 진입 준비도


판단 매트릭스:
- 시그널 좋음 + Phase 6 정상 -> 실지원 지속 (2027 개시)
- 신 VLA 모델 등장 -> 모델 갱신
- 자작 팔 지연 -> Phase 6 진입 늦음
```


---


## 체크리스트


- [ ] (선택) Phase 4 demo v2 영상 제작
- [ ] (선택) CLIP ROS2 통합 시연
- [ ] retro.md 작성
- [ ] phase6_prep.md 작성
- [ ] 분기 재평가 #2 일정 등록
- [ ] git commit (Phase 5 의 모든 산출물)
- [ ] quiz
