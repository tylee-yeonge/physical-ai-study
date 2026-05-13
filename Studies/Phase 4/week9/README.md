# Week 9: Inference 입출력 인터페이스 정리 + ROS2 msg 매핑


> **이번 주 목표**: OpenVLA inference 의 입출력을 ROS2 msg 와 명확히 매핑한 spec 1 페이지를 산출. week 10 의 ROS2 패키지 골격 작성의 진입 spec.
> **예상 시간**: 8시간 (spec 설계 3h + msg 변환 코드 3h + 검증 2h)
> **핵심 질문**: "내 VLA 노드는 어떤 topic 을 subscribe 하고 어떤 topic 을 publish 하는가? 각 msg 의 frame_id / timestamp / 좌표계는?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | spec 1 페이지 작성 | `PRACTICE.md` 1 | input topic / output topic / 좌표계 |
| 2 | msg 타입 결정 | `PRACTICE.md` 2 | sensor_msgs vs custom |
| 3 | image -> PIL 변환 | `PRACTICE.md` 3 | cv_bridge 사용 |
| 4 | action -> Twist 변환 | `PRACTICE.md` 4 | 7-DoF -> geometry_msgs/Twist + gripper |
| 5 | 퀴즈 | quiz_easy / quiz_medium | msg 변환 / 좌표계 |


---


## 시작하기 전에 — ROS2 토픽 설계의 원칙


본 phase 의 ROS2 minimal demo 의 토픽 구조 (Roadmap Phase 4.md):


```
Input topics:
  /camera/image_raw (sensor_msgs/Image)
  /vla/instruction (std_msgs/String)


Output topics:
  /vla/action (geometry_msgs/Twist 또는 custom)
```


이게 본 phase 의 spec. 본 주가 이를 정밀하게 다듬는다.


---


## 핵심 개념


### 1. ROS2 msg 타입 결정 trade-off


| 옵션 | 장점 | 단점 |
|---|---|---|
| `sensor_msgs/Image` | 표준, 모든 ROS 도구 호환 | image 변환 (cv_bridge) 필요 |
| `geometry_msgs/Twist` | 표준, 6-DoF velocity | gripper 별도 처리 필요 |
| custom msg `vla_msgs/Action` | 7-DoF + metadata 한 번에 | custom package 빌드 필요 |


본 phase 권장: minimal demo 단계에서는 **표준 msg**:
- `sensor_msgs/Image`
- `geometry_msgs/Twist` (linear, angular) + `std_msgs/Float64` (gripper)
- 또는 `geometry_msgs/PoseStamped` 로 absolute pose 출력


Phase 7 의 산출물 #4 에서 custom msg 로 전환 권장:


```
# vla_msgs/msg/Action.msg
std_msgs/Header header
float64[7] action # [dx, dy, dz, rx, ry, rz, gripper]
string instruction
float64 latency_ms # 측정 latency
```


### 2. 좌표계 (frame_id) 설계


| 좌표계 | 의미 | 본 phase 에서 |
|---|---|---|
| `base_link` | robot base | action 의 origin |
| `tool0` 또는 `ee_link` | end-effector | OpenVLA action 이 측정되는 곳 |
| `camera_link` | 카메라 | image 의 frame_id |


OpenVLA action 은 **end-effector 좌표계의 delta** 가 표준. tf 로 변환:


```
action (ee frame) -> 자신의 좌표계 (base_link) -> joint command
```


### 3. cv_bridge 사용


ROS2 의 `sensor_msgs/Image` <-> OpenCV `np.ndarray` 변환:


```python
from cv_bridge import CvBridge


bridge = CvBridge()


# 수신: ROS msg -> OpenCV BGR
img_bgr = bridge.imgmsg_to_cv2(image_msg, desired_encoding='bgr8')


# 송신: OpenCV BGR -> ROS msg
image_msg = bridge.cv2_to_imgmsg(img_bgr, encoding='bgr8')
```


> OpenVLA 는 RGB 입력이므로 추가로 BGR -> RGB 변환 필요 (week 8 의 preprocess).


### 4. Timestamp / Synchronization


`header.stamp` 의 정확성이 양산에서 매우 중요:


- **subscribe 한 image** 의 timestamp = inference 시작 시점
- **publish 한 action** 의 timestamp = inference 완료 시점
- 둘의 차이 = inference latency


```python
# subscribe callback
def image_callback(self, msg):
    t_recv = self.get_clock().now()
    t_image = rclpy.time.Time.from_msg(msg.header.stamp)
    age_ms = (t_recv - t_image).nanoseconds / 1e6
    self.get_logger().info(f"Image age: {age_ms:.1f} ms")
    # inference -> publish
```


### 5. 입출력 spec 1 페이지 (산출물 #2 의 일부)


```
+------------------------+
| /camera/image_raw | (sensor_msgs/Image, BGR8, 224x224 or 640x480)
| /vla/instruction | (std_msgs/String)
+------------------------+
            v
     vla_node (subscribe)
     - image age check
     - preprocess (BGR->RGB)
     - VLAInference.predict()
     - validate output
            |
            v
+------------------------+
| /vla/action | (geometry_msgs/Twist + gripper float)
| linear x,y,z (m) | end-effector delta translation
| angular x,y,z (rad) | end-effector delta rotation
| /vla/gripper | (std_msgs/Float64, 0~1)
| /vla/latency_ms | (std_msgs/Float64)
+------------------------+
            v
     downstream:
     - joint controller
     - safety policy
     - rerun visualization
```


### 6. QoS 설정


ROS2 의 QoS (Quality of Service) 가 중요:


| Topic | QoS | 이유 |
|---|---|---|
| /camera/image_raw | best_effort, depth 1 | drop 허용, 최신 image 만 |
| /vla/action | reliable, depth 10 | 모든 action 보존, 순서 중요 |
| /vla/instruction | transient_local, depth 1 | 한 번 보내면 latch (새 노드도 받음) |


### 7. 노드 lifecycle


ROS2 의 lifecycle node (선택):


```
configure() -> activate() -> running -> deactivate() -> cleanup()
       | | | |
   모델 로드 warm-up topic 닫음 모델 unload
```


본 phase 의 minimal demo 는 lifecycle 안 써도 OK. Phase 7 의 결정타에서는 lifecycle 권장.


### 8. 디버깅 toolchain


| Tool | 용도 |
|---|---|
| `ros2 topic echo /vla/action` | action 직접 확인 |
| `ros2 topic hz /camera/image_raw` | image rate |
| `rqt_image_view` | image 시각화 |
| `rerun.io` | 양쪽 + action 통합 시각화 (week 12) |
| `ros2 bag record` | 시연 영상 데이터 보존 |


---


## 자체 점검


**Q1. OpenVLA action 의 7-DoF 를 ROS2 msg 로 어떻게 publish 하는 게 표준?**
> minimal demo: geometry_msgs/Twist (6-DoF, 좌표축 + rotation) + std_msgs/Float64 (gripper). 결정타 (Phase 7): custom vla_msgs/Action 로 한 번에.


**Q2. image age 측정의 의미는?**
> subscribe 받은 시점 - image header timestamp = image 가 카메라에서 노드까지 도달한 latency. 양산에서 30ms+ 면 통신 병목.


**Q3. `cv_bridge` 의 BGR8 vs RGB8 차이는?**
> ROS 의 image msg encoding 옵션. OpenCV 표준은 BGR8. RGB8 은 OpenVLA 입력 호환. 변환 안 하면 색 정보 swap.


**Q4. QoS depth=1 best_effort 의 의미는?**
> 메시지 큐 1개만 유지, 손실 허용. 최신 image 만 보고 처리할 때 적합 (image 가 자주 새로 옴, 옛날 image 는 무의미).


**Q5. inference latency 를 measure 하는 가장 정확한 방법은?**
> image header.stamp - action header.stamp. ROS2 의 모든 timestamp 가 ROS time 으로 일치. clock skew 영향 없음.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. spec 1 페이지 작성 (`vla_io_spec.md`)
2. `practice_msg_conversion.py` - msg <-> Python 변환 코드
3. cv_bridge 사용법 익히기
4. timestamp 측정 시뮬레이션
5. quiz_easy / quiz_medium


### 다음 주 (week 10) 준비
- ROS2 Humble 또는 Iron 환경 점검 (`ros2 doctor`)
- `vla_node` 패키지 디렉토리 구조 미리 계획


---


## 이번 주 핵심 요약


1. **표준 msg 우선**, custom msg 는 Phase 7 에서.
2. **cv_bridge** 로 ROS Image <-> OpenCV 변환, 추가로 BGR -> RGB.
3. **header.stamp** 가 모든 latency 측정의 근간.
4. **QoS** image=best_effort, action=reliable.
5. **spec 1 페이지** 가 week 10 의 진입 input.


---


- 이전: [Week 8 - inference 안정화](../week8/README.md)


다음: [Week 10 - ROS2 패키지 골격](../week10/README.md)
