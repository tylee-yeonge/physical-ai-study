# Visual SLAM & Perception 로드맵

> 🎯 **목표**: SLAM 기초 → **AI Perception** → **이직** (Perception Engineer)
> ⏰ **기간**: Stage 1 (이직 전) + Stage 2 (이직 후) + Stage 2+ (장기 확장)
> 👶 **전제**: 5개월 딸과 함께하는 직장인 아빠, AMR ROS Application 개발자

---

## 📊 전체 로드맵

```mermaid
graph LR
    subgraph Stage1[Stage 1]
        P0[Phase 0]
        P1[Phase 1]
        P2[Phase 2]
        P3[Phase 3]
        P4[Phase 4]
        P5[Phase 5]
        P6[Phase 6]
    end

    subgraph Career[Career]
        Portfolio[Portfolio]
        Job[Job]
    end

    subgraph Stage2[Stage 2]
        BEV[BEV]
        Blender[Blender]
        Isaac[Isaac Sim]
        Multi[Multi-modal]
    end

    subgraph VLATrack[VLA Track]
        VLA[VLA]
    end

    subgraph Stage2Plus[Stage 2 Plus]
        Embodied[Embodied AI]
    end

    P0 --> P1 --> P2 --> P3 --> P4 --> P5 --> P6
    P6 --> Portfolio --> Job
    Portfolio -.-> VLA
    Job --> BEV --> Isaac --> Multi
    Blender -.-> Isaac
    Multi --> Embodied
    VLA --> Embodied
```

---

## 🗓️ 타임라인 요약

| 시기 | Stage | 내용 | 목표 |
|------|-------|------|------|
| 2026.01-02 | Stage 1 | Phase 0-1 (완료) | 환경 세팅, 수학 핵심 |
| 2026.03-04 | Stage 1 | Phase 2: CV 기초 | 카메라 모델 이해 |
| 2026.05 | Stage 1 | Phase 3: VO & BA (4주) | VO/BA 개념 이해 |
| 2026.06 전반 | Stage 1 | Phase 4: VIO 개념 (3주) | IMU-Vision 상호보완 이해 |
| 2026.06-08 | Stage 1 | **Phase 5: Detection + Depth** | 2D Perception + Jetson 배포 |
| 2026.09-10 | Stage 1 | **Phase 6: 3D Perception + BEV** | KITTI/nuScenes 3D Detection |
| 2026.11 | Stage 1 | 포트폴리오 | GitHub + 블로그 + 데모 영상 |
| 2026.12~ | Career | **이직 활동 + VLA 입문 병행** | Perception Engineer |
| 2027 중반~ | Stage 2 | BEV, Blender, Isaac Sim | 이직 후 심화 |
| 2028~ | Stage 2+ | **VLA 심화, Embodied AI** | 미래 역량 |

---

## 💻 언어 사용 전략

| Phase | 내용 | 언어 | 이유 |
|-------|------|------|------|
| Phase 0-1 | 환경 세팅, 수학 | Python | 빠른 프로토타이핑 |
| **Phase 2** | **컴퓨터 비전 기초** | **C++** | Jetson 실습, OpenCV C++ |
| **Phase 3** | **VO & BA** | **C++** | g2o, Ceres (C++ 전용) |
| **Phase 4** | **VIO 개념** | **C++** | Sophus, 개념 이해 중심 |
| **Phase 5** | **Detection + Depth** | **Python** (학습) + **C++/TensorRT** (배포) | PyTorch → Jetson 최적화 |
| **Phase 6** | **3D Perception** | **Python** | MMDetection3D, nuScenes |

### 핵심 원칙

✅ **SLAM 알고리즘 (Phase 2-4)**: **C++**
- 이유: VINS, ORB-SLAM 모두 C++, 개념 이해 수준

✅ **딥러닝 학습 (Phase 5-6)**: **Python** (PyTorch)
- 이유: 딥러닝 생태계 표준

✅ **딥러닝 배포**: **C++ + TensorRT** (Jetson)
- 이유: 실시간 추론 성능 (30+ FPS 목표)

---

## 🔷 Stage 1: 이직 전 (2026년)

### 실습 가이드 위치

**모든 실습 가이드는 `Studies/Phase X/PRACTICE.md`에 있습니다:**

| Phase | 가이드 위치 | 언어 |
|-------|------------|------|
| Phase 2 | [`Studies/Phase 2/week2/PRACTICE.md`](./Studies/Phase%202/week2/PRACTICE.md) | C++ |
| Phase 3 | 각 week별 PRACTICE.md (예: [`week8/PRACTICE.md`](./Studies/Phase%203/week8/PRACTICE.md)) | C++ |
| Phase 4 | [`Studies/Phase 4/PRACTICE.md`](./Studies/Phase%204/PRACTICE.md) | C++ |
| Phase 5 | [`Studies/Phase 5/PRACTICE.md`](./Studies/Phase%205/PRACTICE.md) | Python + TensorRT |
| Phase 6 | [`Studies/Phase 6/PRACTICE.md`](./Studies/Phase%206/PRACTICE.md) | Python |

---

### Phase 0-4: SLAM 기초
> Phase 0-1 완료, Phase 2 진행 중, Phase 3-4 개념 이해 중심

| Phase | 내용 | 기간 |
|-------|------|------|
| 0 | 환경 세팅, VINS 실행 | 2주 |
| 1 | 수학 핵심 (선형대수, 3D 기하) | 2개월 |
| 2 | 컴퓨터 비전 기초 | 2개월 |
| 3 | VO & BA (개념 + 코드 분석) | 4주 |
| 4 | VIO 개념 (직관적 이해 중심) | 3주 |

### Phase 5: Detection + Depth (3개월) ⭐
> **핵심 Phase** - Detection + Depth 필수, Instance Seg 선택

| 주차 | 내용 | 핵심 모델 | 우선순위 |
|------|------|----------|----------|
| 1-2 | PyTorch 복습 | - | 필수 |
| 3-4 | **YOLO 실습** | YOLOv8, RT-DETR | 필수 |
| 5-6 | **Depth Estimation** | DPT, Depth Anything | 필수 |
| 7-8 | Segmentation | SegFormer, SAM | 필수 |
| 9-10 | **Jetson 배포** | TensorRT | 필수 |
| 11-12 | Instance Segmentation | Mask R-CNN | ⚡선택 |

> ⚠️ **TensorRT 배포는 삽질 시간이 예상보다 길어질 수 있음**
> Instance Seg은 시간 여유 있을 때 진행

**산출물**: Jetson에서 실시간 Detection + Depth 데모

### Phase 6: 3D Perception (약 3개월) ⭐
> **이직 준비 핵심** - KITTI 3D → nuScenes 순서로 진행

| 주차 | 내용 | 핵심 |
|------|------|------|
| 1-2 | 3D Object Detection 개념 | 카메라 → 3D |
| 3-4 | **KITTI 3D 실습** | 가벼운 데이터셋으로 먼저 |
| 5-6 | Monocular 3D Detection | FCOS3D |
| 7-8 | **nuScenes + BEV 입문** | BEVFormer 개념 |

> 💡 **nuScenes (~400GB)는 셋업에만 며칠 걸릴 수 있음**
> KITTI 3D로 먼저 연습 후 nuScenes 진입 권장

**산출물**: 카메라 기반 3D 객체 검출 데모

### 포트폴리오 (2026.12)
| 항목 | 내용 |
|------|------|
| GitHub | 학습 정리 + 데모 코드 |
| 블로그 | 학습 여정 시리즈 |
| 데모 영상 | Jetson 실시간 Perception |

---

## 🔶 Stage 2: 이직 후 (2027년 중반~)

> 새 회사 적응 기간 (3-6개월) 후 심화 학습 시작
> ⚠️ 이직 직후 바로 시작하지 말고 적응 기간 버퍼 확보

### BEV & Occupancy (3개월)
| 주제 | 내용 |
|------|------|
| BEVFormer 심화 | Multi-camera → BEV |
| Occupancy Network | 3D 공간 점유 예측 |
| nuScenes 벤치마크 | 성능 평가 |

### 🎨 Blender for Simulation
> **목적**: Isaac Sim/Gazebo용 시뮬레이션 에셋 제작

| 단계 | 내용 | 기간 | 산출물 |
|------|------|------|--------|
| 1 | 기초 UI, 모델링, 텍스처링 | 4주 | 장애물/박스 모델 |
| 2 | Python API (Scripting) | 2주 | 자동화 스크립트 |
| 3 | **Isaac Sim/Gazebo Export** | 2주 | USD/URDF 변환 |
| 4 | Procedural Generation | 2주 (선택) | 다양한 에셋 자동 생성 |

> 💡 **3D 좌표계/투영 개념이 Blender 모델링에 바로 적용됨**

**산출물**: Isaac Sim에서 사용 가능한 커스텀 로봇 환경 에셋

### 🎮 Isaac Sim 연동 (2개월)
> Blender 에셋을 활용한 시뮬레이션 환경 구축

| 주제 | 내용 |
|------|------|
| Isaac Sim 기초 | 환경 세팅, USD 이해 |
| Blender → Isaac | 커스텀 에셋 Import |
| **Synthetic Data 생성** | Domain Randomization |
| Perception 파이프라인 | 시뮬레이션 → 실제 전이 |

> 💡 **Synthetic Data로 학습 데이터 무한 생성 가능!**

### Multi-modal Perception (3개월)
| 주제 | 내용 |
|------|------|
| Camera + LiDAR 융합 | 센서 퓨전 |
| Vision-Language | CLIP, BLIP |
| Open-vocabulary Detection | 텍스트 기반 검출 |

---

## 🤖 VLA 입문 (이직 활동과 병행, 2026.12~)

> 포트폴리오 완성 후, 이직 활동과 병행하며 VLA 입문
> 로봇이 **보고 → 이해하고 → 행동**하는 End-to-End 시스템

| 주제 | 내용 | 대표 모델 |
|------|------|----------|
| VLA 기초 | Vision-Language-Action 구조 | RT-2, OpenVLA |
| Policy Learning | 행동 정책 학습 | Diffusion Policy |
| Simulation | 시뮬레이션 환경 | Isaac Sim, MuJoCo |

> 💡 Phase 5-6의 Perception 지식 + AMR ROS 경험이 VLA 학습의 토대가 됨

---

## 🚀 Stage 2+: 장기 확장 (2028년~)

### Embodied AI
| 주제 | 내용 |
|------|------|
| Navigation + Manipulation | 이동 + 조작 통합 |
| World Models | 환경 이해/예측 |
| Foundation Models for Robotics | 대규모 로봇 모델 |

### 당신의 강점이 빛나는 부분
| Stage 2+ 요소 | AMR 경험 연결 |
|---------------|---------------|
| Action 실행 | ROS 5년 경험 |
| 실제 배포 | 제품 레벨 경험 |
| 하드웨어 이해 | 펌웨어 2년 |
| 로봇 도메인 | AMR 도메인 지식 |

> 💡 **VLA는 "Vision+Language 연구자" + "로봇 실무자"가 만나야 가능한 영역**
> 당신은 후자를 이미 갖추고 있음!

---

## 🔧 실습 환경

| 장비 | 용도 |
|------|------|
| **Jetson Orin Nano** | 실시간 추론, 배포 |
| **ELP 800P Stereo Monochrome** | 스테레오 비전 |

### 외부 소스코드

Phase 3-4에서 코드 분석 대상인 VINS-Fusion이 git submodule로 포함되어 있습니다.

```bash
# 처음 클론할 때
git clone --recurse-submodules <repo-url>

# 이미 클론한 경우
git submodule update --init
```

| 경로 | 내용 |
|------|------|
| `Assets/VINS-Fusion/` | VINS-Fusion 소스코드 (submodule) |

---

## 🏆 커리어 경로

```
현재: AMR ROS Application 개발자 (7년차)
      ↓
2026.12~: 이직 활동 + VLA 입문 병행
      ↓
2027: Perception Engineer (로봇/자율주행)
      ↓
2028+: Embodied AI / VLA Engineer
```

### 최종 포지셔닝
> "SLAM 기초를 이해하고, 딥러닝 Perception을 할 수 있으며,
> 실제 로봇 제품에 배포해본 경험이 있는 **Perception Engineer**"

---

## ✅ 마일스톤 체크리스트

### Stage 1 (2026~2027년)
- [x] VINS-Fusion 실행 성공
- [x] 수학 기초 이해
- [ ] Phase 2 완료 (컴퓨터 비전)
- [ ] Phase 3 완료 (VO & BA 개념)
- [ ] Phase 4 완료 (VIO 개념)
- [ ] Phase 5 완료 (Detection + Depth, Jetson 배포)
- [ ] Phase 6 완료 (3D Perception + BEV)
- [ ] 포트폴리오 완성
- [ ] **이직 성공! 🎉**

### Stage 2 (2027년)
- [ ] BEV Perception 심화
- [ ] 🎨 Blender 기초 완료
- [ ] 🎮 Isaac Sim 연동
- [ ] Multi-modal 학습
- [ ] 시니어 성장

### VLA (이직 활동 병행)
- [ ] VLA 논문 읽기 (RT-2, OpenVLA)
- [ ] 간단한 VLA 실험

### Stage 2+ (2028년~)
- [ ] VLA 심화
- [ ] Embodied AI 역량
- [ ] 미래 리더십 🚀

---

## 💡 핵심 원칙

1. **역순 학습**: 먼저 돌려보고, 모르는 것을 채운다
2. **개념은 설명할 수 있게, 구현은 찾을 수 있게**: "이게 뭐고 왜 쓰나요?" → 답할 수 있어야 함 (면접 대비). "코드로 어떻게 짜나요?" → 뭘 찾아봐야 하는지 알면 OK (실무 대비). 수학적 유도는 스킵. 막힌 부분은 표시해두고 다음으로.
3. **실무 연결**: 항상 "이게 로봇에 어떻게 쓰이나?" 생각
4. **기록 습관**: 배운 것을 짧게라도 기록
5. **가족 우선**: 학습은 마라톤, 번아웃 방지
6. **SLAM 기초는 무기, Perception이 본체**: SLAM 이해가 차별점, Perception이 메인 스킬

---

> 📝 **경로**: SLAM 기초 → **AI Perception** → **이직** → BEV → **VLA**
> AMR 실무 경험을 살려 **Perception Engineer**로 성장합니다.

---

## 🔥 이직 시나리오 분석 — 흔들릴 때 다시 읽기

> 아이도 생겼고, 가족도 있고, 책임질 게 많아졌다.
> 그래도 지금 하고 있는 도메인을 벗어나고 싶다.
> 기술은 빠르게 발전하고 경쟁은 치열하다. 내 자리를 만들기 위해 어떤 노력이든 할 마음이 되어있다.
>
> 원래 인생은 최상과 최악을 검토하고 살아가다 보면, 그 중간 어딘가에 도달하기 마련이다.

### 희망적인 경우 (Best Case)

**2027년 상반기, Perception Engineer로 이직 성공**

- 7년차 로봇 실무 경험은 대체 불가능한 자산. Perception을 논문으로만 아는 사람은 많지만, 실제 로봇 제품에 배포해본 사람은 드묾
- AMR에서 ROS, 센서, 실시간 시스템을 다뤄본 경험은 면접에서 "이 사람은 현장을 안다"는 신뢰를 줌
- Phase 5-6까지 완주하고 **Jetson 실시간 데모 영상**이 있으면 포트폴리오로 충분히 설득력 있음
- 자율주행/로봇 업계는 "SLAM도 알고 Perception도 되는 사람"을 원하는데, 정확히 그 포지셔닝
- C++과 Python 양쪽을 다루는 엔지니어는 수요 대비 공급이 적음

**필요한 것**: Phase 5-6 완주 + Jetson 데모 1개 + 블로그/GitHub 정리

### 현실적인 경우 (Realistic Case)

**2027년 하반기~2028년 초, 약간의 우회를 거쳐 이직**

- 아이가 있는 직장인의 학습 시간은 하루 1-2시간이 현실. 일정은 밀릴 수 있음
- TensorRT 배포는 삽질 시간이 예상의 2-3배. 여기서 일정이 밀림
- 첫 이직 시도에서 바로 합격하지 못할 수 있음. 면접 2-3회 경험 후 부족한 점을 보완하고 재도전

**그래도 되는 이유**:
- Perception Engineer 채용은 꾸준히 늘고 있음
- "완벽한 준비 후 이직"보다 "70% 준비 + 면접 경험"이 더 효율적
- Phase 5까지만 해도 (Detection + Depth + Jetson 배포) 지원 가능한 포지션이 있음
- 7년차 경력직은 신입과 다르게 평가됨. 도메인 전환이라도 엔지니어링 역량 자체는 인정받음

**전략**: Phase 5 완료 시점(2026.08경)부터 **이직 시장 탐색을 병행**. Phase 6를 기다리지 말고 먼저 시장 반응을 확인

### 최악의 경우 (Worst Case)

**학습이 지연되고, 이직 시장이 생각보다 어려운 경우**

- 가족 이벤트(아이 아픔, 육아 부담 증가)로 3-6개월 공백 발생
- 현 직장 업무 강도 증가로 학습 시간 확보 어려움
- Perception 포지션 지원했으나 "딥러닝 실무 경험 부족"으로 탈락 반복

**그런데 최악이어도 잃는 것은 없음**:
- 현 직장(AMR ROS 개발)은 유지됨. 이직 실패 = 현 상태 유지이지 추락이 아님
- 학습한 CV/DL 지식은 현 직장에서도 활용 가능. AMR에 Perception을 붙이는 역할로 현 회사에서 포지션 확장도 가능
- 아이가 좀 더 크면(2-3세) 학습 시간이 다시 생김
- 로봇/자율주행 시장은 축소되는 게 아니라 확장 중. 늦어져도 기회는 있음
- 최악의 최악: 2028년까지 이직 못 함 → 8-9년차 ROS 개발자 + Perception 지식을 가진 사람으로 더 강한 포지션에서 재도전

### 시나리오 요약

| 시나리오 | 시기 | 핵심 변수 |
|---------|------|----------|
| Best | 2027 상반기 | Phase 5-6 완주 + 데모 |
| Realistic | 2027 하반기~2028 초 | Phase 5 완료 후 시장 탐색 병행 |
| Worst | 2028+ 지연 | 현 직장 유지하며 재도전 |

> **세 경우 모두 공통점**: 지금 하고 있는 공부가 헛되는 시나리오는 없다. 속도가 다를 뿐이지 방향은 맞다.
>
> AMR → Perception은 완전한 도메인 이탈이 아니다. **같은 로봇 도메인 안에서 레이어를 옮기는 것**이다.
> 면접관 입장에서 "리스크가 낮은 경력직 전환"으로 보인다. 완전히 다른 분야에서 오는 사람보다 훨씬 유리한 위치에 있다.
