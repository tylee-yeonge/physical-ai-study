# Week 1: Isaac Sim 4.x 설치 + Workstation 셋업


> **이번 주 목표**: Isaac Sim 4.x 를 Ubuntu PC (RTX 4070) 에 설치하고 첫 hello-world Stage 동작.
> **예상 시간**: 10시간
> **핵심 질문**: "Isaac Sim 이 RTX 4070 12GB 에서 정상 동작하는가? Python API 가 호출되는가?"


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 시스템 요구사항 | `PRACTICE.md` 1 | GPU / RAM / Disk |
| 2 | Isaac Sim 4.x 설치 | `PRACTICE.md` 2 | Omniverse Launcher |
| 3 | Conda + Python API | `PRACTICE.md` 3 | standalone |
| 4 | hello-world Stage | `PRACTICE.md` 4 | empty + cube |
| 5 | 퀴즈 | | |


---


## Phase 6 의 위치


Phase 6 = Phase 7 의 사전 준비:


```
Phase 6: 자작 팔 URDF -> Isaac Sim 디지털 트윈 + Sim/Real gap 측정
Phase 7: OpenVLA + ROS2 + 자작 팔 + 디지털 트윈 = Real-to-Sim-to-Real (산출물 #4)
```


본 phase 자체 산출물 없음. Phase 7 의 한 축.


---


## 핵심 개념


### 1. Isaac Sim 의 큰 그림


NVIDIA Omniverse Isaac Sim 4.x:
- USD (Universal Scene Description) 기반
- PhysX 5 물리 엔진
- Photo-realistic rendering
- ROS2 Bridge 지원


### 2. 시스템 요구사항


| 항목 | 최소 | 권장 (본 phase) |
|---|---|---|
| GPU | RTX 30 | RTX 4070+ |
| VRAM | 8 GB | 12 GB+ |
| RAM | 16 GB | 32 GB+ |
| Disk | 50 GB | 100 GB+ |
| OS | Ubuntu 22.04 | Ubuntu 22.04 |


### 3. 설치 방법


```bash
# venv + pip 방법 (권장)
python3 -m venv .venv
source .venv/bin/activate
pip install --extra-index-url https://pypi.nvidia.com isaacsim==4.5.0.0 \
    isaacsim-extscache-physics==4.5.0.0
```


### 4. USD / Stage / Prim


- USD: 3D scene file format
- Stage: USD 의 in-memory representation
- Prim: USD 의 unit (object, light, transform)


### 5. Headless vs GUI


- GUI: 시각 디버깅, 로컬
- Headless: ssh, 출장지 가능, 본 phase 권장


### 6. ROS2 Bridge (week 3)


Sim -> ROS2 topics:
- /clock
- /joint_states
- /camera/image_raw
- /tf


### 7. RTX 4070 hardware budget


- Isaac Sim 단독: ~ 4 GB VRAM (간단 scene)
- + OpenVLA int4: 5 GB -> total ~ 9 GB (가능)
- + 자작 팔 ROS2: 추가 1 GB


본 phase 권장: Sim 만 단독 (12GB 여유), OpenVLA 동시 실행은 Phase 7 에서.


---


## 자체 점검


**Q1. Isaac Sim system requirement?**
> RTX 30+, VRAM 8GB+, Ubuntu 22.04.


**Q2. USD 와 Stage?**
> USD file format, Stage 가 in-memory representation.


**Q3. Headless 가치?**
> 출장지 ssh 원격 실행 가능.


**Q4. ROS2 Bridge?**
> Sim 의 데이터를 ROS2 topic 으로.


**Q5. RTX 4070 에서 Isaac Sim + OpenVLA 동시?**
> 어려움 (~ 14GB). 본 phase 는 Sim 단독.


---


## 실습 + 다음


### 이번 주
- 시스템 점검
- Isaac Sim 설치
- hello-world cube
- quiz


### 다음 주 (week 2)
- USD / Stage 더 자세히
- Camera / Light 추가


---


## 핵심 요약


1. **Isaac Sim 4.x** NVIDIA Robot 시뮬레이션
2. **USD + Stage + Prim**
3. **RTX 4070 OK**, OpenVLA 동시 OOM 주의
4. **Headless 모드** 출장지
5. **ROS2 Bridge** Sim/Real 인터페이스


- 이전: [Phase 5](../../../Roadmap/Phase%205.md) | 다음: [Week 2](../week2/README.md)
