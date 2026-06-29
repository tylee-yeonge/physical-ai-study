# Week 11: 실제 inference 연결 + dry-run


> **이번 주 목표**: week 10 의 dummy 노드를 **week 8 의 VLAInference 와 연결**. ros2 bag 으로 녹화한 video 또는 ELP Stereo 실시간 input 으로 dry-run 성공.
> **예상 시간**: 10시간
> **핵심 질문**: "내 노드가 1분 동안 실시간 image input 으로 한 번도 fail 없이 action publish 하는가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | VLAInference 통합 | `PRACTICE.md` 1 | week 10 의 dummy 를 실 inference 로 |
| 2 | preprocess pipeline | `PRACTICE.md` 2 | cv_bridge -> BGR -> RGB -> PIL |
| 3 | image age check | `PRACTICE.md` 3 | header.stamp 기반 |
| 4 | error handling | `PRACTICE.md` 4 | week 8 의 exception 들 |
| 5 | 1분 dry-run | `PRACTICE.md` 5 | bag play 또는 ELP 실시간 |
| 6 | 퀴즈 | quiz_easy / quiz_medium | 통합 / 디버깅 |


---


## 시작하기 전에 — 본 주의 결과물


본 주가 끝나면 다음이 완성:


1. **vla_node 가 실제 OpenVLA inference** 를 호출
2. **image -> action** 의 full pipeline 동작
3. **1분 dry-run** (bag 또는 ELP 실시간) 에서 0 fail
4. **latency 측정 데이터** 가 ROS topic 으로 publish


다음 주 (week 12) 에 Rerun 시각화 (레포 결과 기록용 선택). 1분 영상 제작은 v1 에서 제외하고 v2(Phase 6)로 이관했다.


---


## 핵심 개념


### 1. VLAInference 를 ROS2 패키지에서 import


week 8 의 `vla_inference` 패키지를 ROS2 노드에서 그대로 import 하려면, 그 패키지를 ROS2 workspace 안으로 가져와 노드 실행 환경 (공용 venv) 에 설치해야 한다.


`/workspace/phase4_workspace/vla_inference` (week 8 결과물) 을 ROS2 workspace 의 `src/vla_inference` 로 복사한 뒤, 그 위치에서 공용 venv 에 pip editable 설치 (`pip install -e`) 한다. 이렇게 하면 import 경로 (`from vla_inference...`) 가 그대로 유지되고 week 8 라이브러리와 ROS 노드의 분리도 보존된다.


구체적인 복사·설치 절차는 [`PRACTICE.md`](PRACTICE.md) 의 "환경 설정" (Step 0) 에서 수행한다.


### 2. 노드 코드 통합 (week 10 -> week 11)


```python
# week 10 (dummy)
twist = Twist()
twist.linear.x = 0.0
...


# week 11 (실제 inference 통합)
from vla_inference.inference import VLAInference
from vla_inference.preprocess import opencv_to_pil
from vla_inference.exceptions import (
    VLAOOMError, VLAInputError, VLAOutputError,
)


class VLANode(Node):
    def __init__(self):
        ...
        self.vla = VLAInference(model_id=..., quant_type='4bit')
        self.vla.warm_up(n_iter=5)
        self.bridge = CvBridge()


    def image_callback(self, msg):
        if not self.current_instruction:
            return


        # 1. image age
        age_ns = (self.get_clock().now() - rclpy.time.Time.from_msg(msg.header.stamp)).nanoseconds
        age_ms = age_ns / 1e6
        if age_ms > 200:
            self.get_logger().warning(f'image too old: {age_ms:.1f}ms')
            return


        # 2. preprocess
        img_bgr = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
        img_pil = opencv_to_pil(img_bgr)


        # 3. inference (with error handling)
        t0 = self.get_clock().now()
        try:
            action = self.vla.predict(img_pil, self.current_instruction)
        except VLAOOMError as e:
            self.get_logger().error(f'OOM: {e}')
            return
        except VLAOutputError as e:
            self.get_logger().error(f'NaN: {e}')
            return


        # 4. publish
        twist = Twist()
        twist.linear.x = float(action[0])
        twist.linear.y = float(action[1])
        twist.linear.z = float(action[2])
        twist.angular.x = float(action[3])
        twist.angular.y = float(action[4])
        twist.angular.z = float(action[5])
        self.action_pub.publish(twist)


        gripper = Float64()
        gripper.data = float(action[6])
        self.gripper_pub.publish(gripper)


        # 5. latency
        latency_ms = (self.get_clock().now() - t0).nanoseconds / 1e6
        lat_msg = Float64()
        lat_msg.data = latency_ms
        self.latency_pub.publish(lat_msg)
```


### 3. ros2 bag 으로 video 입력 시뮬레이션


```bash
# bag 녹화 (한 번만, ELP Stereo 직접 사용)
ros2 bag record /camera/image_raw -o my_test_bag


# bag 재생 (다른 터미널)
ros2 bag play my_test_bag --loop
```


본 phase 권장: 자작 팔 환경 사진 / 동영상 1분 짜리 bag 으로 dry-run.


### 4. 1분 dry-run 의 success criteria


| 지표 | 목표 |
|---|---|
| total inference 수 | 60s × 5Hz = 300 ~ 350 frame |
| Success rate | 100% (0 fail) |
| Mean latency | < 200 ms |
| p95 latency | < 300 ms |
| GPU memory | < 10 GB |
| 노드가 죽지 않음 | True |


### 5. 디버깅 toolchain


```bash
# topic 부하 모니터링
ros2 topic hz /vla/action
# 기대: ~ 5 Hz


# action 내용 확인
ros2 topic echo /vla/action --no-arr | head -50


# latency 통계
ros2 topic echo /vla/latency_ms
# > tee latency.log -> Python 분석


# 노드 상태
ros2 node info /vla_node
```


### 6. 자주 발생하는 통합 에러


| 증상 | 원인 | 해결 |
|---|---|---|
| `cv_bridge import error` | ros-${ROS_DISTRO}-cv-bridge 누락 | apt install |
| 노드 시작 시 OOM | GPU 메모리 부족 | 다른 process 종료, 4-bit 확인 |
| inference 가 0 action | unnormalize_key 잘못 | parameter 확인 |
| image_callback 호출 안 됨 | topic 이름 불일치 | ros2 topic list 확인 |
| GPU 100% 인데 inference 안 함 | 다른 모델이 점유 | nvidia-smi 확인 |


### 7. callback 의 thread-safety


ROS2 rclpy 의 default executor 는 single-threaded:
- 한 번에 하나의 callback 만 실행
- inference 중에 새 image 받으면 큐
- 큐 누적 방지: BEST_EFFORT QoS + depth=1


Multi-threaded executor 는 권장 안 함 (이번 phase):
- inference 가 GPU 점유라 동시 실행 무의미
- thread-safety 신경 써야 함


### 8. ros2 bag 녹화의 표준 패턴


```bash
# 모든 topic 한 번에
ros2 bag record -a -o full_dry_run


# 특정 topic 만 (디스크 절약)
ros2 bag record /camera/image_raw /vla/action /vla/latency_ms -o demo


# bag 정보 확인
ros2 bag info demo/


# bag 의 latency_ms 데이터 추출
ros2 bag play demo/ &
ros2 topic echo /vla/latency_ms > latency.log
```


---


## 자체 점검


**Q1. image_callback 에서 VLAInference 호출 시 첫 단계는?**
> image age check. 너무 오래된 image 면 skip. inference 비용 (~150ms) 이 크기 때문.


**Q2. ros2 bag 재생 시 시간 동기화는?**
> `--clock` 옵션으로 ROS time 을 bag time 으로 설정. 시간 정합 필수.


**Q3. inference OOM 발생 시 노드 동작은?**
> `torch.cuda.empty_cache()` 후 retry 1회. 또 OOM 시 action 발행 안 함 + error log. 노드는 계속 살아있음.


**Q4. 1분 dry-run 의 가장 중요한 기준은?**
> 0 fail. 1 fail 이라도 발생하면 stress test 회귀.


**Q5. callback 에 try/except 가 광범위해야 하는 이유는?**
> 한 callback 의 예외가 노드 전체를 멈출 수 있음. 모든 외부 호출 (inference / cv_bridge / publish) 은 보호.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. vla_inference 를 ros2 환경에서 import 가능하게 정리
2. vla_inference_node.py 에 실 inference 통합
3. ros2 bag 으로 1분 dry-run
4. latency 통계 확인 (ros2 topic echo /vla/latency_ms)
5. quiz_easy / quiz_medium


### 다음 주 (week 12) 준비
- Rerun.io 설치 (https://rerun.io) — 레포 결과 기록용 시각화(선택)
- (1분 영상 녹화 도구 OBS / Kazam 준비는 v2 로 이관)


---


## 이번 주 핵심 요약


1. **VLAInference 와 ROS2 노드 통합** 완료.
2. **image age check + preprocess + error handling** 의 표준 패턴.
3. **1분 dry-run** 에서 0 fail, < 200ms mean latency.
4. **ros2 bag** 으로 재현 가능한 테스트.
5. **다음 주 Rerun 시각화** (레포 기록용) 의 사전 준비. 1분 영상은 v2 로 이관.


---


- 이전: [Week 10 - ROS2 패키지 골격](../week10/README.md)


다음: [Week 12 - Rerun 시각화 + 1분 영상](../week12/README.md)
