# Phase 6: 3D Perception

> ⏰ **기간**: 약 3개월 (12주 분량)  
> 🎯 **목표**: 카메라 기반 3D 객체 검출 마스터  
> 💻 **언어**: **Python** (PyTorch, MMDetection3D)  
> 🛠️ **환경**: PC (GPU 필수), Jetson (선택)  
> 💼 **이직 준비 핵심 Phase**  
> ⏱️ **주간 시간**: 약 12-15시간

---

## 👉 **실습 가이드**: [`Studies/Phase 6/PRACTICE.md`](../Studies/Phase%206/PRACTICE.md)

**핵심 산출물**:
- KITTI 3D Detection 구현
- nuScenes Multi-view 3D Detection
- BEV (Bird's Eye View) 개념 이해
- **포트폴리오 완성** (이직 준비)

> ⚠️ **목표**: 자율주행/로봇 면접에서 3D Perception 경험 어필

---

## 📋 Section 6.1: 3D Detection 기초 (2주)

### Week 1: 3D Detection 개념

#### 왜 3D Detection인가?
- [ ] **2D Detection의 한계**
  - 거리 정보 부족
  - 가려진 부분 (Occlusion) 처리 어려움
  - 3D 공간 이해 불가
  
- [ ] **3D Detection이 필요한 이유**
  - AMR: 장애물의 정확한 위치/크기
  - 자율주행: 차량 간 거리, 경로 계획
  - 로봇 조작: 물체 Grasping

#### 3D Bounding Box
```
2D bbox: [x, y, w, h]
3D bbox: [x, y, z, l, w, h, θ]
         └─ 중심 ─┘ └ 크기 ┘ └회전┘
```
- [ ] 중심 좌표 (x, y, z): 월드 또는 카메라 좌표계
- [ ] 크기 (length, width, height)
- [ ] 회전 각도 (yaw θ, pitch, roll)
- [ ] Bird's Eye View에서 주로 yaw만 사용

#### 3D Detection 방법론 분류

| 방법 | 입력 | 장점 | 단점 |
|------|------|------|------|
| **LiDAR 기반** | Point cloud | 정확한 거리 | 비싼 센서, 날씨 영향 |
| **Camera 기반** | RGB 이미지 | 저렴, 텍스처 정보 | Depth 모호성 |
| **Fusion** | Camera + LiDAR | 두 장점 결합 | 캘리브레이션 복잡 |

**이 Phase 초점**: **Camera 기반** (Monocular, Stereo, Multi-view)

### Week 2: 좌표계 이해

#### 다양한 좌표계
- [ ] **Camera 좌표계**: 카메라 중심, Z축 = 전방
- [ ] **World 좌표계**: 절대 좌표 (예: 지도 기준)
- [ ] **LiDAR 좌표계**: LiDAR 센서 중심
- [ ] **Bird's Eye View (BEV)**: 위에서 본 시점

#### KITTI 좌표계
```python
# KITTI 3D bbox 레이블
# Camera 좌표계 기준
[h, w, l, x, y, z, ry]
```
- [ ] x: 좌우 (왼쪽 +)
- [ ] y: 상하 (아래 +)
- [ ] z: 전후 (앞 +)
- [ ] ry: Y축 회전 (yaw)

#### 좌표 변환 실습
- [ ] Camera → World 변환 (Phase 2 복습)
- [ ] 3D bbox corners 계산
- [ ] 2D 이미지에 3D bbox 투영

### 🔍 Section 6.1 자체 점검
1. 2D Detection과 3D Detection의 가장 큰 차이는?
2. KITTI에서 ry (yaw)는 어느 축 회전인가?
3. BEV 표현의 장점은?

---

## 📋 Section 6.2: KITTI 3D Detection (4주)

### Week 3: KITTI 데이터셋

#### 데이터셋 다운로드 (~20GB)
- [ ] http://www.cvlibs.net/datasets/kitti/eval_object.php?obj_benchmark=3d
- [ ] Left color images
- [ ] Calibration files
- [ ] Training labels

#### 데이터 구조 이해
```
KITTI/
├── training/
│   ├── image_2/      # 왼쪽 카메라
│   ├── calib/        # 내부/외부 파라미터
│   └── label_2/      # 3D annotations
└── testing/
```

#### 레이블 파싱
```python
# label_2/000000.txt
Car 0.0 0 1.85 387 173 614 200 1.48 1.65 3.69 1.84 1.47 8.41 0.01
└┬┘                   └─ 2D bbox ─┘ └─ 3D info ─┘
 class                               (h,w,l,x,y,z,ry)
```

#### 시각화 실습
- [ ] 2D bbox 그리기
- [ ] 3D bbox corners 계산
- [ ] 3D bbox를 이미지에 투영
- [ ] BEV 시각화

### Week 4: Monocular 3D Detection 모델

#### SMOKE (Simple Monocular 3D Object Detection)
- [ ] 원리: 2D Detection + 3D 파라미터 회귀
- [ ] Keypoint 기반 (중심점 검출)
- [ ] Depth 추정이 핵심

#### FCOS3D (Fully Convolutional One-Stage)
- [ ] Anchor-free 방식
- [ ] Multi-task learning (2D + 3D)
- [ ] 더 나은 성능

#### 핵심 개념
- [ ] **Depth 추정 방법**
  - Direct regression
  - Keypoint 기반 기하학적 계산
- [ ] **3D Box 인코딩**
  - Center offset
  - Dimension (l, w, h)
  - Rotation (sin/cos 표현)

### Week 5: MMDetection3D 실습

#### 환경 세팅
```bash
pip install openmim
mim install mmcv-full
mim install mmdet
pip install mmdet3d
```

#### KITTI로 학습
```bash
# Config 파일 수정
vim configs/fcos3d/fcos3d_r101_kitti.py

# 학습
python tools/train.py \
    configs/fcos3d/fcos3d_r101_kitti.py \
    --work-dir work_dirs/fcos3d_kitti
```

#### Inference & 평가
- [ ] 학습된 모델로 추론
- [ ] 3D bbox 시각화
- [ ] **AP3D** (Average Precision 3D) 계산
  - Easy, Moderate, Hard 난이도별
  - IoU threshold: 0.7 (Car), 0.5 (Pedestrian)

### Week 6: 성능 분석 및 개선

#### 오류 분석
- [ ] **Depth 오차**: 멀리 있는 객체
- [ ] **Orientation 오차**: 회전 각도 추정
- [ ] **Occlusion**: 가려진 객체
- [ ] **Truncation**: 이미지 경계 밖

#### 개선 전략
- [ ] Multi-scale training
- [ ] Data augmentation (3D 회전, 깊이 변화)
- [ ] Post-processing (NMS 3D)

#### 목표 성능 (KITTI val set)
- [ ] Car (Moderate): **AP3D > 15%** (Monocular는 어려움)
- [ ] 참고: SOTA는 ~25% (Monocular)

### 🔍 Section 6.2 자체 점검
1. KITTI에서 "Moderate" 난이도의 기준은?
2. Monocular 3D Detection의 가장 큰 어려움은?
3. AP3D와 AP2D의 차이는?

---

## 📋 Section 6.3: nuScenes & BEV (4주)

### Week 7: nuScenes 데이터셋

#### nuScenes 특징
- [ ] **360도 Multi-camera** (6대 카메라)
- [ ] LiDAR + Radar 포함
- [ ] 1000 scene, 40,000 keyframe
- [ ] 23개 클래스 (KITTI의 3개 vs)

#### 데이터셋 다운로드 (~400GB)
⚠️ **주의**: 다운로드/압축 해제에 며칠 소요
- [ ] Full dataset (v1.0-trainval)
- [ ] Mini dataset (v1.0-mini, 10GB) 먼저 실험 권장

#### 데이터 구조
```
nuScenes/
├── maps/             # HD 맵
├── samples/          # 키프레임 (2Hz)
│   ├── CAM_FRONT/
│   ├── CAM_FRONT_LEFT/
│   ├── CAM_BACK/
│   └── LIDAR_TOP/
└── v1.0-trainval/    # Annotations
```

#### nuScenes Devkit
```python
from nuscenes.nuscenes import NuScenes
nusc = NuScenes(version='v1.0-mini', dataroot='./data/nuscenes')

# Scene 탐색
scene = nusc.scene[0]
sample = nusc.get('sample', scene['first_sample_token'])

# 시각화
nusc.render_sample(sample['token'])
```

### Week 8: BEV 개념 이해

#### Bird's Eye View란?
```
Side View (Camera):       BEV (Top-down):
    │  🚗 │                  ┌─────────┐
    │     │                  │    ▲    │
    ├─────┤   ⇒              │    │    │
  Road                       │  🚗    │
                             └─────────┘
```
- [ ] 위에서 본 시점
- [ ] X-Y 평면 (높이 정보 압축)
- [ ] 주행 경로 계획에 유용

#### 왜 BEV인가?
- [ ] **Occlusion 해결**: 가려진 부분도 표현 가능
- [ ] **Multi-camera 융합** 자연스러움
- [ ] **경로 계획** 직관적
- [ ] **Temporal 정보** 통합 쉬움

#### BEV 생성 방법
1. **IPM (Inverse Perspective Mapping)**: 단순 기하 변환
2. **MLP 기반**: 학습으로 Camera → BEV
3. **Transformer 기반**: BEVFormer (SOTA)

### Week 9: BEVFormer 이해

#### BEVFormer 구조
```
Multi-view Images 
    ↓
Feature Extraction (ResNet/ViT)
    ↓
BEV Queries (Learnable)
    ↓
Spatial Cross-Attention (Image features → BEV)
    ↓
Temporal Self-Attention (이전 프레임 BEV)
    ↓
Detection Head (3D bbox + Velocity)
```

#### 핵심 개념
- [ ] **BEV Queries**: Grid 형태의 learnable embedding
  - 예: 200×200 그리드, 각 셀 = 0.5m
- [ ] **Spatial Cross-Attention**: Image feature를 BEV로 변환
- [ ] **Temporal Attention**: 이전 프레임 정보 활용

#### 논문 읽기
- [ ] BEVFormer: "BEVFormer: Learning Bird's-Eye-View Representation from Multi-Camera Images via Spatiotemporal Transformers" (ECCV 2022)
- [ ] Figure 2, 3 중점 이해
- [ ] Ablation study 결과 분석

### Week 10: BEVFormer 실습 (선택)

⚠️ **주의**: BEVFormer 학습은 GPU 메모리 많이 필요 (A100 권장)

#### 추론만 실습
```bash
# Pretrained model 다운로드
wget https://github.com/fundamentalvision/BEVFormer/releases/download/.../bevformer_r101_dcn.pth

# Inference
python tools/test.py \
    projects/configs/bevformer/bevformer_base.py \
    bevformer_r101_dcn.pth \
    --eval bbox
```

#### BEV 시각화
- [ ] BEV 맵에 3D bbox 그리기
- [ ] Multi-frame temporal 시각화
- [ ] 예측 궤적 (Velocity) 표시

#### 성능 지표
- [ ] **NDS** (nuScenes Detection Score)
- [ ] **mAP**: mean Average Precision
- [ ] Pretrained model: NDS ~0.52 (SOTA: ~0.56)

### 🔍 Section 6.3 자체 점검
1. BEV 표현이 Camera view보다 나은 점은?
2. BEVFormer의 Spatial Cross-Attention 역할은?
3. Temporal Attention이 왜 중요한가?

---

## 📋 Section 6.4: 포트폴리오 마무리 (2주)

### Week 11: 코드 및 문서 정리

#### GitHub Repository 구성
```
3d-perception-portfolio/
├── README.md                  # 전체 개요
├── kitti_3d/
│   ├── train.py
│   ├── inference.py
│   ├── configs/
│   └── README.md              # KITTI 결과
├── nuscenes_bev/
│   ├── visualize_bev.py
│   ├── results/
│   └── README.md              # nuScenes 결과
└── docs/
    ├── setup.md               # 환경 세팅
    └── evaluation.md          # 평가 지표
```

#### README.md 작성 요소
- [ ] **프로젝트 개요**: 무엇을 했는지
- [ ] **환경 세팅**: 재현 가능하도록
- [ ] **데이터셋 준비**: 다운로드 링크
- [ ] **학습 방법**: 커맨드 예시
- [ ] **결과**: AP3D, NDS 표 + 시각화 이미지
- [ ] **Demo 영상**: GIF 또는 YouTube 링크

### Week 12: 블로그 & 영상

#### 블로그 포스팅 (3-4개)
1. **KITTI 3D Detection 입문**
   - 데이터셋 소개
   - Monocular 3D의 어려움
   - 내 구현 과정

2. **nuScenes와 BEV의 이해**
   - 왜 BEV인가?
   - BEVFormer 개념 정리
   - Multi-camera fusion

3. **3D Perception 면접 준비**
   - 자주 나오는 질문 정리
   - 내 경험담

#### Demo 영상 제작 (5-7분)
- [ ] **인트로** (30초)
  - 프로젝트 소개
  
- [ ] **KITTI 3D Detection** (2분)
  - 학습 과정
  - Inference 결과 시각화
  - 성능 지표

- [ ] **nuScenes BEV** (2분)
  - BEV 시각화
  - Multi-camera fusion 시연
  
- [ ] **마무리** (30초)
  - 배운 점, 어려웠던 점
  - GitHub 링크

#### LinkedIn 포스팅
- [ ] 프로젝트 완성 공유
- [ ] Demo 영상 링크
- [ ] 해시태그: #3DDetection #AutonomousDriving #ComputerVision

### 🔍 Section 6.4 자체 점검
1. 포트폴리오에서 가장 어필할 점은?
2. 면접관이 물어볼 만한 질문 5개는?
3. 다음 단계로 배우고 싶은 것은?

---

## ✅ Phase 6 완료 체크리스트

### KITTI 3D Detection
- [ ] 데이터셋 준비 및 이해
- [ ] FCOS3D (또는 유사 모델) 학습
- [ ] AP3D (Moderate) > 15% 달성
- [ ] 3D bbox 시각화

### nuScenes & BEV
- [ ] nuScenes 데이터셋 탐색
- [ ] BEV 개념 이해
- [ ] BEVFormer 논문 읽기
- [ ] Pretrained model로 추론

### 포트폴리오
- [ ] GitHub 코드 정리 + README
- [ ] 블로그 포스팅 3개
- [ ] Demo 영상 제작
- [ ] LinkedIn 공유

---

## 🎯 Phase 6 완료 기준

> "KITTI와 nuScenes에서 3D Detection을 실습하고, BEV 개념을 이해하며, 이를 증명하는 포트폴리오 (GitHub + 블로그 + 영상)를 완성"

---

## 📚 참고 자료

### 논문

| 논문 | 핵심 내용 |
|------|----------|
| FCOS3D (ICCVW 2021) | Monocular 3D Detection |
| BEVFormer (ECCV 2022) | BEV Transformer |
| DETR3D (CoRL 2021) | Query-based 3D Detection |

### 데이터셋

| 데이터셋 | 링크 |
|---------|------|
| KITTI 3D | http://www.cvlibs.net/datasets/kitti/ |
| nuScenes | https://www.nuscenes.org/ |

### 라이브러리

| 라이브러리 | 용도 |
|-----------|------|
| MMDetection3D | 3D Detection 프레임워크 |
| nuScenes-devkit | nuScenes 데이터 처리 |

---

## 💡 팁

1. **KITTI 먼저**: nuScenes는 크고 복잡함, KITTI로 개념 잡기
2. **Mini dataset 활용**: nuScenes mini로 빠르게 실험
3. **시각화 중요**: 3D는 눈으로 봐야 이해됨
4. **면접 준비**: "왜 BEV인가?"는 단골 질문
5. **포트폴리오 품질**: README + 영상이 면접 통과의 열쇠

---

## 💼 이직 준비 최종 점검

### 필수 역량 체크
- [ ] SLAM 기초 (Phase 2-4) 이해
- [ ] 2D Detection (Phase 5) 실무 경험
- [ ] 3D Detection (Phase 6) 프로젝트 경험
- [ ] Jetson 배포 경험
- [ ] 포트폴리오 3종 세트

### 면접 대비 질문 (예상)
1. "3D Detection 프로젝트 설명해주세요"
2. "BEV의 장점은 무엇인가요?"
3. "Monocular vs LiDAR 3D Detection 차이?"
4. "Depth Estimation과 3D Detection 관계는?"
5. "실제 로봇에 어떻게 적용하시겠습니까?"

---

## ❓ 다음 단계

Phase 6 완료 후:
- **이직 활동 시작** (2027.03)
- 병행: **Blender 시뮬레이션** 학습
- 이직 후: **Stage 2** (BEV Segmentation, Isaac Sim)
