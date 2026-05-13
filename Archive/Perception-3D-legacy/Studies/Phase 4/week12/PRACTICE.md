# Week 12 실습: 블로그 포스팅 & Demo 영상 제작


> **목표**: KITTI 3D Detection 블로그 포스팅을 작성하고, Demo 영상 스크립트를 준비하며, LinkedIn 포스팅 초안을 작성한다
> **언어**: Python (Markdown 생성, 시각화)
> **예상 시간**: 8시간


---


## 실습 개요


| Step | 내용 | 난이도 | 시간 |
|------|------|--------|------|
| 1 | 블로그 포스팅 초안 작성 | 필수 | 3시간 |
| 2 | Demo 영상 스크립트 작성 | 필수 | 2시간 |
| 3 | LinkedIn 포스팅 작성 | 필수 | 1시간 |
| 4 | 면접 대비 Q&A 정리 | 필수 | 2시간 |


---


## 환경 설정


```bash
pip install -r requirements.txt
```


---


## Step 1: 블로그 포스팅 초안 작성


```python
"""
블로그 포스팅 Markdown 생성기
KITTI 3D Detection 입문기 블로그 초안을 자동으로 생성한다.
"""




def generate_blog_post_1() -> str:
    """
    1편: KITTI 3D Detection 입문기 블로그 초안 생성


    Returns:
        Markdown 형식의 블로그 포스팅 문자열
    """
    blog = """# 카메라 한 대로 3D 물체를 잡는다? — KITTI Monocular 3D Detection 도전기


## 들어가며


자율주행에서 주변 물체의 3D 위치를 아는 것은 생존의 문제입니다.
LiDAR를 사용하면 정확한 거리를 측정할 수 있지만, 센서 가격이 수백만 원에 달합니다.
**"카메라 한 대만으로 3D 물체를 검출할 수는 없을까?"** — 이것이 Monocular 3D Detection의 출발점입니다.


이 글에서는:
- KITTI 3D Object Detection 데이터셋을 소개하고
- Monocular 3D Detection의 핵심 어려움을 설명하며
- FCOS3D 모델로 실제 학습 및 평가한 경험을 공유합니다.


---


## 1. 왜 3D Detection인가?


### 2D Detection의 한계


2D Detection은 이미지 내에서 물체의 위치를 (x, y, w, h)로 표현합니다.
하지만 자율주행에서는 이 정보만으로는 부족합니다.


```
2D Detection: "앞에 차가 있다" (O)
              "앞에 차가 얼마나 멀리 있다" (X)
              "앞에 차의 크기가 어느 정도이다" (X)


3D Detection: "앞에 차가 15m 거리에, 4.5m x 1.8m x 1.5m 크기로,
              30도 회전된 상태로 있다" (O)
```


### 3D Bounding Box


```
3D bbox = [x, y, z, l, w, h, θ]
           중심 좌표 크기 회전각
```


7개의 파라미터를 정확하게 예측해야 합니다.


---


## 2. KITTI 데이터셋


### 데이터 구조


KITTI는 자율주행 3D Detection의 대표적인 벤치마크입니다.


```
KITTI/
+-- training/
| +-- image_2/ # 왼쪽 카메라 이미지 (7,481장)
| +-- calib/ # 카메라 캘리브레이션
| +-- label_2/ # 3D 어노테이션
+-- testing/ # 테스트 세트
```


### 레이블 형식


```
Car 0.0 0 1.85 387 173 614 200 1.48 1.65 3.69 1.84 1.47 8.41 0.01
```


각 필드: 클래스, truncated, occluded, alpha, 2D bbox, 3D 크기(h,w,l), 3D 위치(x,y,z), 회전각(ry)


[결과 이미지: KITTI 3D bbox 시각화]


---


## 3. FCOS3D로 학습하기


### 모델 선택 이유


FCOS3D는 Anchor-free 방식의 Monocular 3D Detection 모델입니다.
- 2D Detection (FCOS)의 확장
- 각 위치에서 3D 파라미터를 직접 회귀
- MMDetection3D 프레임워크에서 쉽게 사용 가능


### 핵심 도전


**Depth 추정의 모호성**이 가장 큰 어려움이었습니다.


```python
# 먼 곳의 작은 차 vs 가까운 곳의 큰 차
# → 이미지에서는 동일하게 보일 수 있음!
# → Depth 추정 오차가 3D 위치 오차에 직결
```


### 학습 결과


| 난이도 | AP3D (IoU=0.7) |
|--------|----------------|
| Easy | 18.52% |
| Moderate | 13.87% |
| Hard | 11.23% |


**참고**: LiDAR 기반 모델은 AP3D > 80%. Monocular의 한계가 명확합니다.


---


## 4. 배운 점


1. **Depth 추정이 핵심**: 카메라만으로는 거리 정보가 모호
2. **좌표계 이해 필수**: KITTI의 Camera/LiDAR/World 좌표계
3. **평가 지표 이해**: AP3D는 3D IoU 기반으로 2D AP보다 훨씬 엄격


다음 글에서는 nuScenes와 BEV 기반 3D Detection을 다루겠습니다.


---


## 참고 자료


- [FCOS3D Paper](https://arxiv.org/abs/2104.10956)
- [KITTI Benchmark](http://www.cvlibs.net/datasets/kitti/)
- [GitHub Repository](https://github.com/YOUR_USERNAME/camera-3d-perception)
"""
    return blog




# 실행
blog_content = generate_blog_post_1()
print("=" * 50)
print("블로그 포스팅 1편 초안 생성 완료!")
print("=" * 50)
print(blog_content[:500])
print("...\n")
print(f"총 {len(blog_content)}자")
print("\n이 내용을 블로그 플랫폼(Velog, Tistory 등)에 붙여넣으세요.")
print("결과 이미지를 추가하면 완성도가 높아집니다.")
```


---


## Step 2: Demo 영상 스크립트 작성


```python
"""
Demo 영상 스크립트 생성기
시간별 대본과 화면 설명을 포함한 스크립트를 생성한다.
"""




def generate_video_script() -> str:
    """
    Demo 영상 스크립트 생성


    Returns:
        스크립트 문자열
    """
    script = """
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Demo 영상 스크립트 (총 6분)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


[00:00 ~ 00:30] 인트로
━━━━━━━━━━━━━━━━━━━━━━
화면: 프로젝트 제목 + 결과 이미지 몽타주
대본: "안녕하세요. 이 영상에서는 카메라 기반 3D Object Detection
       프로젝트를 소개합니다.
       KITTI에서 Monocular 3D Detection을,
       nuScenes에서 BEV 기반 Multi-view Detection을 실습했습니다."


[00:30 ~ 01:00] 문제 정의
━━━━━━━━━━━━━━━━━━━━━━━
화면: 2D Detection vs 3D Detection 비교 이미지
대본: "2D Detection은 물체의 위치만 알려줍니다.
       하지만 자율주행에서는 물체까지의 거리,
       물체의 실제 크기, 방향을 알아야 합니다.
       이것이 3D Detection이 필요한 이유입니다."


[01:00 ~ 02:30] KITTI 3D Detection
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
화면: KITTI 데이터 예시 → 학습 과정 (터미널) → 결과 시각화
대본: "KITTI 데이터셋은 7,481장의 학습 이미지와
       3D 어노테이션을 제공합니다.
       FCOS3D 모델을 사용하여 학습했고,
       AP3D Moderate 기준 13.87%를 달성했습니다."


       [터미널 화면]
       "학습 과정을 보시면, Loss가 점차 감소하는 것을 확인할 수 있습니다."


       [결과 시각화]
       "3D 바운딩 박스가 이미지에 정확히 투영된 것을 볼 수 있습니다.
        가까운 차량은 정확하게 검출되지만,
        먼 차량은 Depth 추정 오차로 인해 부정확합니다."


[02:30 ~ 04:00] nuScenes & BEV
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
화면: nuScenes 6대 카메라 이미지 → BEV 시각화 → BEV Feature
대본: "nuScenes는 6대 카메라로 360도를 커버합니다.
       BEVFormer는 이 이미지들을 BEV 표현으로 변환하여
       3D Detection을 수행합니다."


       [BEV 시각화]
       "위에서 내려다본 시점으로, 주변 차량의 위치와
        방향을 한눈에 파악할 수 있습니다."


       "BEVFormer는 NDS 0.517을 달성했습니다."


[04:00 ~ 05:00] 기술적 인사이트
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
화면: 요약 슬라이드 (핵심 포인트 3가지)
대본: "이 프로젝트에서 가장 어려웠던 점은
       Depth 추정의 모호성이었습니다.
       카메라만으로는 깊이 정보가 본질적으로 부족하고,
       이것이 Monocular 3D Detection의 가장 큰 한계입니다."


       "이를 해결하기 위해 BEV 표현이 등장했고,
        Multi-camera fusion이 중요한 발전 방향임을 배웠습니다."


       "개선 방향으로는 Depth supervision 추가,
        Temporal 정보 활용, Stereo 카메라 도입 등이 있습니다."


[05:00 ~ 05:30] 마무리
━━━━━━━━━━━━━━━━━━━━━━
화면: GitHub 링크 + QR 코드
대본: "전체 코드는 GitHub에 공개되어 있습니다.
       감사합니다. 피드백은 언제든 환영합니다."
"""
    return script




# 실행
script = generate_video_script()
print(script)
print("\n이 스크립트를 기반으로 OBS Studio에서 녹화하세요.")
print("각 섹션별로 화면을 미리 준비해두면 효율적입니다.")
```


---


## Step 3: LinkedIn 포스팅 작성


```python
"""
LinkedIn 포스팅 초안 생성
"""




def generate_linkedin_post() -> str:
    """
    LinkedIn 포스팅 초안 생성


    Returns:
        포스팅 문자열
    """
    post = """
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  LinkedIn 포스팅 초안
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


[한국어 버전]


카메라 한 대로 3D 물체 검출이 가능할까요?


6개월간 Camera-based 3D Object Detection 프로젝트를 진행했습니다.


주요 내용:
- KITTI에서 FCOS3D로 Monocular 3D Detection (AP3D 13.87%)
- nuScenes에서 BEVFormer로 Multi-view 3D Detection (NDS 0.517)
- 좌표계, BEV 표현, Depth 추정의 핵심 개념 학습


가장 큰 배움:
Depth 추정의 모호성이 Monocular 3D Detection의 핵심 병목이며,
BEV 기반 Multi-view 접근이 이를 효과적으로 해결한다는 것.


GitHub: [링크]
블로그: [링크]
Demo: [링크]


피드백 환영합니다!


#3DDetection #ComputerVision #AutonomousDriving #BEV #DeepLearning
#KITTI #nuScenes #BEVFormer #Robotics #Portfolio


━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


[English Version]


Can a single camera detect 3D objects?


I spent 6 months working on Camera-based 3D Object Detection:


Key Results:
- Monocular 3D Detection on KITTI with FCOS3D (AP3D: 13.87%)
- Multi-view BEV Detection on nuScenes with BEVFormer (NDS: 0.517)


Key Insight:
Depth ambiguity is the fundamental challenge in monocular 3D detection.
BEV-based multi-view approaches effectively address this limitation.


GitHub: Blog: 

Feedback welcome!


#3DDetection #ComputerVision #AutonomousDriving #BEV #DeepLearning
"""
    return post




# 실행
post = generate_linkedin_post()
print(post)
print("\n위 초안을 기반으로 LinkedIn에 포스팅하세요.")
print("결과 이미지 1~2장을 첨부하면 노출이 증가합니다.")
```


---


## Step 4: 면접 대비 Q&A 정리


```python
"""
면접 대비 Q&A 정리
3D Perception 면접에서 자주 나오는 질문과 답변을 정리한다.
"""




def prepare_interview_qa():
    """면접 예상 질문과 STAR 프레임워크 기반 답변을 출력한다."""


    qa_list = [
        {
            "question": "3D Detection 프로젝트를 설명해주세요.",
            "answer": """
  S: KITTI와 nuScenes에서 카메라 기반 3D Object Detection 프로젝트를 진행했습니다.
  T: Monocular(FCOS3D)와 Multi-view(BEVFormer) 접근법을 비교하고,
     각각의 성능과 한계를 분석하는 것이 목표였습니다.
  A: KITTI에서 FCOS3D를 학습하여 AP3D 13.87%(Moderate)를 달성했고,
     nuScenes에서 BEVFormer Pretrained 모델로 NDS 0.517을 확인했습니다.
     오류 분석을 통해 원거리 Depth 추정이 핵심 병목임을 파악했습니다.
  R: Monocular의 근본적 한계(Depth 모호성)와 BEV 표현의 장점을 체감했고,
     이 경험을 블로그와 GitHub에 정리했습니다.""",
        },
        {
            "question": "BEV 표현의 장점은 무엇인가요?",
            "answer": """
  1. Occlusion 해결: 위에서 보므로 가려진 부분도 표현 가능
  2. Multi-camera 융합: 여러 카메라 영상을 하나의 공간에서 자연스럽게 합침
  3. 경로 계획 연결: BEV 공간은 주행 경로와 직접 연결
  4. Temporal 통합: 이전 프레임 BEV를 ego-motion으로 정렬하여 쉽게 합산
  5. 일관된 크기: 거리에 관계없이 물체 크기가 일관됨 (perspective 왜곡 없음)""",
        },
        {
            "question": "Monocular vs LiDAR 3D Detection의 장단점?",
            "answer": """
  Monocular:
    장점: 저렴한 센서 ($10~100), 풍부한 텍스처/색상 정보, 높은 해상도
    단점: Depth 모호성, 날씨/조명 민감, AP3D ~15% (KITTI)


  LiDAR:
    장점: 정확한 거리 정보, 날씨 강건, AP3D ~80% (KITTI)
    단점: 비싼 센서 ($1,000~10,000), 텍스처 정보 없음, 해상도 낮음


  실무 선택: 비용과 성능의 트레이드오프. AMR은 카메라 중심,
  자율주행은 LiDAR+Camera Fusion이 표준.""",
        },
        {
            "question": "Depth 추정의 어려움과 해결 방법은?",
            "answer": """
  어려움:
    - Scale ambiguity: 큰 물체(먼 곳) vs 작은 물체(가까운 곳) 구분 불가
    - 텍스처 없는 영역: 하늘, 벽면 등에서 Depth 추정 불가
    - 가림(Occlusion): 가려진 부분의 Depth 불확실


  해결 방법:
    1. Direct Regression: 네트워크가 Depth를 직접 예측 (FCOS3D)
    2. Depth Distribution: 이산 Depth 분포 예측 (Lift-Splat-Shoot)
    3. Depth Supervision: LiDAR GT로 Depth 학습 보조 (BEVDet)
    4. Multi-view: 여러 시점으로 기하학적 Depth 추론 (BEVFormer)
    5. Temporal: 이전 프레임으로 움직임 기반 Depth 추정""",
        },
        {
            "question": "실제 로봇에 3D Detection을 적용할 때 고려 사항은?",
            "answer": """
  1. 추론 속도: AMR은 실시간(>10 FPS) 필수, Jetson에서 TensorRT 최적화
  2. 정확도 vs 속도 트레이드오프: 용도에 따라 모델 크기 결정
  3. 좌표계 통일: 카메라/로봇/월드 좌표계 변환 정확히 구현
  4. 캘리브레이션: 카메라 내부/외부 파라미터 정확히 측정
  5. 엣지 케이스: 역광, 유리 반사, 투명 물체 등 대응
  6. 안전 마진: 검출 실패 시 안전한 행동 (멈춤, 서행)
  7. 센서 Fusion: 단일 센서 한계 보완 (Camera + Depth Sensor)""",
        },
    ]


    print("━" * 50)
    print("3D Perception 면접 대비 Q&A")
    print("━" * 50)


    for i, qa in enumerate(qa_list, 1):
        print(f"\n{'-' * 50}")
        print(f"Q{i}. {qa['question']}")
        print(f"{'-' * 50}")
        print(qa['answer'])


    print(f"\n{'━' * 50}")
    print("이 답변들을 자신의 경험에 맞게 수정하세요.")
    print("소리 내어 연습하면 면접에서 자연스럽게 답변할 수 있습니다.")
    print("━" * 50)




# 실행
prepare_interview_qa()
```


---


## 체크리스트


### 블로그
- [ ] 블로그 플랫폼 선택 (Velog, Tistory 등)
- [ ] 1편(KITTI 입문기) 초안 작성
- [ ] 결과 이미지 3장 이상 포함
- [ ] GitHub 링크 연결


### Demo 영상
- [ ] OBS Studio 설치 및 설정
- [ ] 스크립트 기반 녹화
- [ ] 자막 추가
- [ ] YouTube 업로드


### LinkedIn
- [ ] 프로필 업데이트 (헤드라인, 기술)
- [ ] 프로젝트 포스팅 작성
- [ ] 결과 이미지 첨부


### 면접 대비
- [ ] Q&A 5개 답변 정리
- [ ] STAR 프레임워크로 경험 구조화
- [ ] 소리 내어 연습 (각 답변 2분 이내)


---


## 추가 실험 아이디어


1. **블로그 시리즈**: 2편(nuScenes & BEV), 3편(면접 대비) 추가 작성
2. **영상 시리즈**: KITTI 편, nuScenes 편 별도 제작
3. **기술 발표**: 사내 또는 커뮤니티 발표 슬라이드 제작
4. **오픈소스 기여**: MMDetection3D에 시각화 유틸리티 PR 제출


---


이전: [Week 12 README](./README.md)


Phase 4 완료! [Phase 4 로드맵으로 돌아가기](../../../Roadmap/Phase%206.md)
