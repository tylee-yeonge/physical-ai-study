# Week 9 실습: VLA I/O spec + cv_bridge + msg 변환


> **실습 목표**: vla_node 의 입출력 spec 1 페이지 + msg 변환 함수 검증.
> **예상 시간**: 5-7시간


---


## 환경 설정


```bash
sudo apt install ros-humble-cv-bridge ros-humble-image-transport
pip install -r requirements.txt
```


ROS2 환경:
```bash
source /opt/ros/humble/setup.bash
```


---


## 실습 1: I/O Spec 1 페이지 작성


**파일명**: `~/phase4_notes/week9/vla_io_spec.md`


```markdown
# vla_node I/O Spec v1.0


## Subscribe


### /camera/image_raw
- Type: sensor_msgs/Image
- Encoding: bgr8
- Frame: 640x480 (또는 224x224)
- frame_id: camera_link
- QoS: best_effort, depth=1
- Rate: 30 Hz (실제 사용은 매 inference 마다 latest 1장)


### /vla/instruction
- Type: std_msgs/String
- Content: 자연어 명령 (예: "pick up the can")
- QoS: transient_local, depth=1 (latch)
- Rate: 사용자 입력 시


## Publish


### /vla/action
- Type: geometry_msgs/Twist
- Frame: ee_link (또는 base_link)
- Content:
  - linear.x = dx (m, end-effector delta translation)
  - linear.y = dy
  - linear.z = dz
  - angular.x = rx (rad, rotation)
  - angular.y = ry
  - angular.z = rz
- QoS: reliable, depth=10
- Rate: ~5 Hz (inference 완료 시마다)


### /vla/gripper
- Type: std_msgs/Float64
- Content: gripper position 0.0 (open) ~ 1.0 (closed)
- QoS: reliable, depth=10


### /vla/latency_ms
- Type: std_msgs/Float64
- Content: 한 inference 의 latency (ms)
- 용도: monitoring / 산출물 #4 의 latency 측정


## Diagnostic


### /vla/status
- Type: std_msgs/String
- Content: "running" / "warming_up" / "error: <msg>"


## 좌표계
- action 은 ee_link 좌표계의 delta
- downstream joint controller 가 base_link 로 변환 후 IK


## Timestamps
- 모든 msg 의 header.stamp 사용
- inference latency = action.header.stamp - image.header.stamp


## 실패 모드
- OOM: action 발행 안 함, /vla/status = "error: oom"
- VLAInputError: input 검증 실패, 무시 + log
- VLAOutputError: zero action 발행 + warning
```


---


## 실습 2: msg <-> Python 변환


**파일명**: `practice_msg_conversion.py`


```python
"""
실습 2: ROS2 msg 와 Python (numpy / PIL) 간 변환 검증
"""
import numpy as np
import cv2
from PIL import Image


# ROS2 imports (Humble)
try:
    import rclpy
    from sensor_msgs.msg import Image as ImageMsg
    from geometry_msgs.msg import Twist
    from std_msgs.msg import Float64, String, Header
    from cv_bridge import CvBridge
    HAS_ROS = True
except ImportError:
    HAS_ROS = False
    print("[warn] ROS2 not available, mock 으로 진행")




print("=" * 60)
print("실습 2: msg <-> Python 변환")
print("=" * 60)


# -- 2-1. Image msg <-> OpenCV --
if HAS_ROS:
    rclpy.init()
    bridge = CvBridge()


    # 가짜 OpenCV image 생성
    img_bgr = (np.random.rand(480, 640, 3) * 255).astype(np.uint8)


    # OpenCV -> ROS Image
    img_msg = bridge.cv2_to_imgmsg(img_bgr, encoding='bgr8')
    img_msg.header.frame_id = 'camera_link'
    img_msg.header.stamp = rclpy.clock.Clock().now().to_msg()
    print(f"\n[2-1] OpenCV -> ROS Image")
    print(f"shape: {img_bgr.shape}, encoding: {img_msg.encoding}")
    print(f"frame_id: {img_msg.header.frame_id}")


    # ROS Image -> OpenCV
    img_recovered = bridge.imgmsg_to_cv2(img_msg, desired_encoding='bgr8')
    print(f"\n[2-2] ROS Image -> OpenCV")
    print(f"shape: {img_recovered.shape}, dtype: {img_recovered.dtype}")
    assert np.allclose(img_bgr, img_recovered), "변환 round-trip 실패"


    # OpenCV BGR -> PIL RGB (week 8 의 preprocess)
    img_rgb = cv2.cvtColor(img_recovered, cv2.COLOR_BGR2RGB)
    img_pil = Image.fromarray(img_rgb)
    print(f"\n[2-3] BGR -> RGB -> PIL")
    print(f"PIL mode: {img_pil.mode}, size: {img_pil.size}")




# -- 2-2. action ndarray -> Twist + gripper --
def action_to_twist(action: np.ndarray):
    """7-DoF action -> Twist + gripper float"""
    if HAS_ROS:
        twist = Twist()
        twist.linear.x = float(action[0])
        twist.linear.y = float(action[1])
        twist.linear.z = float(action[2])
        twist.angular.x = float(action[3])
        twist.angular.y = float(action[4])
        twist.angular.z = float(action[5])
        gripper = Float64()
        gripper.data = float(action[6])
        return twist, gripper
    else:
        return {'linear': action[:3].tolist(),
                'angular': action[3:6].tolist(),
                'gripper': float(action[6])}




print("\n[2-4] action ndarray -> Twist + gripper")
action = np.array([0.05, -0.03, 0.02, 0.5, -1.2, 0.0, 0.9], dtype=np.float32)
twist, grip = action_to_twist(action), None
print(f"action: {action}")
print(f"twist : {twist}")




# -- 2-3. timestamp 측정 --
if HAS_ROS:
    print("\n[2-5] Timestamp 측정 시뮬레이션")
    import time
    t_image_msg = rclpy.clock.Clock().now()
    time.sleep(0.15) # 150ms inference 시뮬레이션
    t_action_msg = rclpy.clock.Clock().now()
    latency_ms = (t_action_msg - t_image_msg).nanoseconds / 1e6
    print(f"image stamp -> action stamp = {latency_ms:.1f} ms (inference latency)")


    rclpy.shutdown()


print("\n 실습 2 완료!")
```


---


## 실습 3: BGR -> RGB 변환 검증


**파일명**: `practice_bgr_rgb_check.py`


```python
"""
실습 3: BGR/RGB 변환 누락의 영향을 시각적으로 확인
"""
import numpy as np
import cv2
from PIL import Image
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


print("=" * 60)
print("실습 3: BGR vs RGB 변환 영향")
print("=" * 60)


# RGB 표준 이미지 (PIL 로 만든다고 가정 - red can)
red_can = np.zeros((100, 100, 3), dtype=np.uint8)
red_can[20:80, 30:70] = [255, 0, 0] # R 채널만 255 (RGB)


# 잘못 받는 BGR 해석
wrong = red_can[..., ::-1] # B/R swap -> 파란 can 으로 보임


fig, ax = plt.subplots(1, 3, figsize=(12, 4))
ax[0].imshow(red_can); ax[0].set_title("Original RGB (red)"); ax[0].axis('off')
ax[1].imshow(wrong); ax[1].set_title("Wrong (BGR interpretation)"); ax[1].axis('off')
ax[2].imshow(cv2.cvtColor(wrong, cv2.COLOR_BGR2RGB)); ax[2].set_title("After cv2.COLOR_BGR2RGB"); ax[2].axis('off')
plt.tight_layout()
plt.savefig("bgr_rgb_check.png", dpi=100)
print("\n 저장: bgr_rgb_check.png")
print("-> OpenVLA 에 wrong (BGR) 그대로 주면 red can 을 blue can 으로 인식")
print("'pick up the red can' 명령이 실패할 수 있음")


print("\n 실습 3 완료!")
```


---


## 실습 체크리스트


- [ ] `vla_io_spec.md` 산출 (1 페이지)
- [ ] `practice_msg_conversion.py` 실행
  - [ ] OpenCV <-> ROS Image round-trip 성공
  - [ ] action -> Twist + gripper 변환
- [ ] `practice_bgr_rgb_check.py` 실행, 시각화 확인
- [ ] quiz_easy / quiz_medium 풀기


---


## 참고 자료


- [ROS2 sensor_msgs/Image](https://docs.ros.org/en/humble/Concepts/About-Quality-of-Service-Settings.html)
- [cv_bridge tutorial](https://github.com/ros-perception/vision_opencv)
- [QoS in ROS2](https://docs.ros.org/en/humble/Concepts/About-Quality-of-Service-Settings.html)
