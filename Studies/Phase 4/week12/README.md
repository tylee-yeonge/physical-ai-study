# Week 12: Rerun 시각화 + 1분 데모 영상 마감


> **이번 주 목표**: Rerun.io 로 inference 시각화 (image / instruction / action / latency) + 1분 영상 제작. Phase 4 의 산출물 #2 의 마지막 1/3 (ROS2 minimal demo + 1분 영상) 완성.
> **예상 시간**: 10시간 (Rerun 통합 4h + 영상 녹화 3h + 편집 + 자막 3h)
> **핵심 질문**: "면접관이 1분 영상만 봐도 'OpenVLA inference + ROS2 통합' 의 핵심 흐름을 이해할 수 있는가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | Rerun 설치 + 기본 사용 | `PRACTICE.md` 1 | rerun-sdk |
| 2 | rerun_logger 노드 | `PRACTICE.md` 2 | ROS topic 들 -> Rerun |
| 3 | 영상 시나리오 작성 | `PRACTICE.md` 3 | 1분 절차 1-3 단계 |
| 4 | 녹화 + 편집 | `PRACTICE.md` 4 | OBS / Kazam |
| 5 | 자막 + 패키징 | `PRACTICE.md` 5 | 한글 자막 |
| 6 | 퀴즈 | quiz_easy / quiz_medium | 시각화 / 영상 |


---


## 시작하기 전에 — Phase 4 의 결말


이번 주가 끝나면 **산출물 #2 완성**:
- 블로그 2 편 (RT-2 + OpenVLA, week 3 + week 7)
- OpenVLA -> ROS2 토픽 minimal demo (week 8-11)
- **1분 데모 영상** (이번 주)
- 발행: `physical-ai-study` 레포의 `Portfolio/02_VLA_demo/`


---


## 핵심 개념


### 1. Rerun.io 의 큰 그림


Rerun 은 시각화 도구로 다음을 지원:
- 시간 동기화된 multi-modal data (image, point cloud, scalar, etc.)
- ROS topic 직접 연동 가능
- Python SDK 로 임의 data logging
- 웹 / desktop 양쪽


### 2. Rerun 기본 사용 패턴


```python
import rerun as rr
import numpy as np


# 초기화
rr.init("vla_demo", spawn=True)


# Image logging
img = np.random.rand(480, 640, 3) * 255
rr.log("camera/image", rr.Image(img.astype(np.uint8)))


# Scalar (latency)
rr.log("vla/latency_ms", rr.Scalar(165.0))


# Text (instruction)
rr.log("vla/instruction", rr.TextLog("pick up the can"))


# Action (vector visualization)
action = np.array([0.05, -0.03, 0.02])
rr.log("vla/action_xyz", rr.Arrows3D(vectors=[action]))
```


### 3. ROS topic -> Rerun


방법 A: rerun_logger ROS 노드 작성 (rclpy + rerun_sdk):


```python
import rclpy
from rclpy.node import Node
import rerun as rr


class RerunLogger(Node):
    def __init__(self):
        super().__init__('rerun_logger')
        rr.init('vla_demo', spawn=True)


        self.image_sub = self.create_subscription(
            Image, '/camera/image_raw', self.on_image, 1)
        self.action_sub = self.create_subscription(
            Twist, '/vla/action', self.on_action, 10)
        self.lat_sub = self.create_subscription(
            Float64, '/vla/latency_ms', self.on_latency, 10)


    def on_image(self, msg):
        img = cv_bridge.imgmsg_to_cv2(msg, 'rgb8')
        rr.log("camera/image", rr.Image(img))


    def on_action(self, msg):
        rr.log("vla/action", rr.Scalar(msg.linear.x), entity_path_suffix="dx")
        ...


    def on_latency(self, msg):
        rr.log("vla/latency_ms", rr.Scalar(msg.data))
```


방법 B: rerun-ros2 패키지 (커뮤니티):
- https://github.com/rerun-io/rerun-loader-python-example


### 4. 1분 영상의 권장 스토리


```
0:00 ~ 0:10 Intro
  - "Phase 4 산출물 #2: OpenVLA + ROS2 minimal demo"
  - 본인 정보 / 로드맵 한 줄


0:10 ~ 0:25 System 구조 (Rerun + ROS graph)
  - ros2 node 다이어그램
  - rqt_graph 화면 캡처


0:25 ~ 0:45 실시간 동작
  - 카메라 image
  - instruction 발행
  - action 발행 (Rerun 의 line chart)
  - latency 통계 (real-time)


0:45 ~ 0:55 결과 + 한계
  - mean latency 165ms / 6Hz
  - "양산에서 의미하는 것" (hierarchical 구조 필요)


0:55 ~ 1:00 Next
  - Phase 7 산출물 #4 의 예고편
```


### 5. 자막 작성 가이드


- 한국어 (1순위 회사 한국 기반)
- 한 줄 ~ 15자 이내, 2초 이상 유지
- 핵심 수치 (165ms, 6Hz, OpenVLA-7B 등) 강조
- 화면 하단 가운데


### 6. 영상 편집 도구


| 도구 | 가격 | 특징 |
|---|---|---|
| OBS (녹화) | 무료 | Ubuntu / Windows 호환 |
| Kazam (녹화) | 무료 | Ubuntu 전용, 간단 |
| DaVinci Resolve (편집) | 무료 | 강력 |
| Shotcut (편집) | 무료 | 가벼움 |
| OpenShot (편집) | 무료 | 초보 친화 |


### 7. 영상 spec


- 해상도: 1920x1080 (or 1280x720 가벼움)
- 프레임률: 30 fps (60 도 OK)
- 형식: mp4 (H.264 codec)
- 길이: 60 ~ 80 초 (60 권장)
- 용량: < 50 MB (GitHub 직접 호스팅 가능)


### 8. GitHub Portfolio 패키징


`physical-ai-study` 레포의 구조:


```
physical-ai-study/
  README.md # 전체 진입점
  Portfolio/
    01_Detection_Depth/ # 산출물 #1 (Phase 3)
    02_VLA_demo/ # 산출물 #2 (Phase 4) <- 본 주의 산출물
      README.md
      vla_demo.mp4
      blog_links.md
      vla_node/ # ROS2 패키지
      vla_inference/ # Python 패키지
      bag/ # 1분 dry-run bag 의 압축
      latency_data.csv
```


---


## 자체 점검


**Q1. Rerun 이 단순 rqt_image_view 보다 좋은 이유?**
> 시간 동기화된 multi-modal (image / action / latency) 한 화면에. ROS topic 별로 흩어진 monitor 를 통합.


**Q2. 1분 영상의 권장 구조는?**
> Intro (10s) + System 구조 (15s) + 실시간 동작 (20s) + 결과/한계 (10s) + Next (5s).


**Q3. 자막을 한국어로 하는 이유는?**
> 본 로드맵의 1순위 회사 (마음AI WoRV / 카카오모빌리티 VLA / 신생 휴머노이드 스타트업) 가 한국 기반. 한국어 면접관에게 직접 어필.


**Q4. 영상 용량 < 50 MB 의 이유는?**
> GitHub Portfolio 에 직접 호스팅 가능 (file size limit 100MB, 권장 50MB 이하). 별도 YouTube / Vimeo 없이 link 만으로 공유.


**Q5. 영상 마무리 (0:55 ~ 1:00) 에 무엇을 넣나?**
> Phase 7 산출물 #4 의 예고편. "다음은 자작 6DOF 팔과 통합" 같은 한 줄. 면접관에게 다음 산출물의 기대 만들기.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. Rerun 설치 + 기본 사용
2. rerun_logger ROS 노드 작성
3. 1분 영상 시나리오 작성
4. OBS / Kazam 으로 녹화
5. 편집 + 자막
6. `Portfolio/02_VLA_demo/` 에 packaging
7. quiz_easy / quiz_medium


### 다음 주 (week 13) 준비
- 블로그 2편 다시 읽고 부족한 부분 메모
- Portfolio 의 다른 산출물 (#1, #2.5) 정리 상태 점검


---


## 이번 주 핵심 요약


1. **Rerun.io 로 multi-modal 시각화** — image / action / latency 한 화면.
2. **1분 영상 = Phase 4 산출물 #2 의 결정타** — 면접관의 가장 짧은 진입점.
3. **한국어 자막** + 핵심 수치 강조.
4. **GitHub Portfolio 직접 호스팅** (< 50 MB).
5. **Next 한 줄** 로 Phase 7 산출물 #4 예고편.


---


- 이전: [Week 11 - 실 inference 통합](../week11/README.md)


다음: [Week 13 - 블로그 2편 퇴고 + 다이어그램](../week13/README.md)
