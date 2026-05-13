# Week 12 실습: Rerun + rerun_logger + 1분 영상 + 패키징


> **실습 목표**: Rerun 시각화 + 1분 영상 + Portfolio/02_VLA_demo/ 완성.
> **예상 시간**: 8~10시간


---


## 환경 설정


```bash
# Rerun SDK 설치
pip install rerun-sdk>=0.15.0


# 녹화 도구 (Ubuntu)
sudo apt install kazam # 또는 obs-studio


# 편집 도구
sudo apt install shotcut # 또는 openshot-qt
```


---


## 실습 1: Rerun 기본 사용


**파일명**: `practice_rerun_basic.py`


```python
"""
실습 1: Rerun 으로 image + scalar + text 동시 시각화
"""
import rerun as rr
import numpy as np
import time


print("=" * 60)
print("실습 1: Rerun 기본")
print("=" * 60)


# 초기화 (UI 자동 spawn)
rr.init("phase4_week12_basic", spawn=True)


# 100 step 의 가짜 stream
for t in range(100):
    rr.set_time_sequence("step", t)


    # 1. Image
    img = (np.random.rand(224, 224, 3) * 255).astype(np.uint8)
    rr.log("camera/image", rr.Image(img))


    # 2. Scalar (latency)
    rr.log("vla/latency_ms", rr.Scalar(150 + np.random.randn() * 20))


    # 3. Scalar (gripper)
    rr.log("vla/gripper", rr.Scalar(np.sin(t * 0.1) * 0.5 + 0.5))


    # 4. Text (instruction)
    if t % 10 == 0:
        rr.log("vla/instruction", rr.TextLog(f"step {t}: pick up the can"))


    # 5. Action vector (3D arrow)
    action_xyz = np.random.randn(3) * 0.05
    rr.log("vla/action_xyz", rr.Arrows3D(
        vectors=[action_xyz],
        origins=[[0, 0, 0]],
    ))


    time.sleep(0.05) # 20 Hz


print("\n Rerun UI 에서 시각화 확인.")
print("Recording 저장: rr.save('/path/to/file.rrd')")
print("\n 실습 1 완료!")
```


**실행**:
```bash
python practice_rerun_basic.py
```


브라우저에서 http://localhost:9876 자동 열림.


---


## 실습 2: rerun_logger ROS 노드


**파일명**: `~/ros2_ws/src/vla_node/vla_node/rerun_logger.py`


```python
"""
ROS topics -> Rerun 시각화
"""
import rclpy
from rclpy.node import Node
import rerun as rr
from cv_bridge import CvBridge


from sensor_msgs.msg import Image
from geometry_msgs.msg import Twist
from std_msgs.msg import Float64, String




class RerunLogger(Node):
    def __init__(self):
        super().__init__('rerun_logger')


        rr.init('vla_demo', spawn=True)
        self.bridge = CvBridge()


        self.image_sub = self.create_subscription(
            Image, '/camera/image_raw', self.on_image, 1)
        self.action_sub = self.create_subscription(
            Twist, '/vla/action', self.on_action, 10)
        self.gripper_sub = self.create_subscription(
            Float64, '/vla/gripper', self.on_gripper, 10)
        self.lat_sub = self.create_subscription(
            Float64, '/vla/latency_ms', self.on_latency, 10)
        self.instr_sub = self.create_subscription(
            String, '/vla/instruction', self.on_instruction, 1)


        self.get_logger().info('RerunLogger started')


    def on_image(self, msg):
        try:
            img = self.bridge.imgmsg_to_cv2(msg, desired_encoding='rgb8')
            rr.log("camera/image", rr.Image(img))
        except Exception as e:
            self.get_logger().error(f'on_image: {e}')


    def on_action(self, msg):
        rr.log("vla/action/linear_x", rr.Scalar(msg.linear.x))
        rr.log("vla/action/linear_y", rr.Scalar(msg.linear.y))
        rr.log("vla/action/linear_z", rr.Scalar(msg.linear.z))
        rr.log("vla/action/angular_x", rr.Scalar(msg.angular.x))
        rr.log("vla/action/angular_y", rr.Scalar(msg.angular.y))
        rr.log("vla/action/angular_z", rr.Scalar(msg.angular.z))


    def on_gripper(self, msg):
        rr.log("vla/gripper", rr.Scalar(msg.data))


    def on_latency(self, msg):
        rr.log("vla/latency_ms", rr.Scalar(msg.data))


    def on_instruction(self, msg):
        rr.log("vla/instruction", rr.TextLog(msg.data))




def main():
    rclpy.init()
    node = RerunLogger()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()




if __name__ == '__main__':
    main()
```


**setup.py 에 entry_point 추가**:
```python
'rerun_logger = vla_node.rerun_logger:main',
```


**빌드 + 실행**:
```bash
cd ~/ros2_ws
colcon build --packages-select vla_node --symlink-install
source install/setup.bash


# 두 노드 동시 실행
ros2 run vla_node vla_inference_node &
ros2 run vla_node rerun_logger
```


---


## 실습 3: 1분 영상 시나리오 + script


**파일명**: `~/phase4_notes/week12/video_script.md`


```markdown
# OpenVLA + ROS2 minimal demo - 1분 영상 script


## 0:00 ~ 0:10 (인트로)


[화면: Title 카드]
"Phase 4 산출물 #2"
"OpenVLA -> ROS2 minimal demo"
"AMR ROS 양산 SW + Physical AI 통합 - 9년 SW 엔지니어"


[narration]
"OpenVLA 의 inference 를 ROS2 토픽으로 받는 minimal demo 입니다.
1분 안에 핵심 흐름을 보여드립니다."


## 0:10 ~ 0:25 (System 구조)


[화면: rqt_graph 또는 Mermaid 다이어그램]
- /camera/image_raw -> vla_node -> /vla/action
- + /vla/instruction
- + /vla/latency_ms


[narration]
"ROS2 노드 구조: 카메라 이미지를 받아 OpenVLA 추론 후 7-DoF action 발행.
또한 inference latency 를 실시간 publish."


## 0:25 ~ 0:45 (실시간 동작)


[화면: Rerun UI - 4 패널 동시]
1) camera image
2) action linear (line chart)
3) latency_ms (line chart)
4) instruction (text)


[narration]
"실시간 동작: instruction 발행 후 매 frame 추론.
RTX 4070 4-bit nf4 에서 mean latency 165ms, 6Hz."


[액션: 카메라 앞에 컵 놓기, instruction 발행]


## 0:45 ~ 0:55 (결과 + 한계)


[화면: latency 통계 표]
| Metric | Value |
| mean | 165ms |
| p95 | 220ms |
| Throughput | 6 Hz |


[narration]
"6Hz 는 cm 단위 manipulation 에 충분하지만, 30Hz 실시간 제어는 불가.
양산에서는 안전 인터록과 함께 hierarchical 구조 필수."


## 0:55 ~ 1:00 (Next)


[화면: Phase 7 다이어그램 미리보기]
"다음 산출물: Real-to-Sim-to-Real (Phase 7)
자작 6DOF 팔 + Isaac Sim 디지털 트윈 + OpenVLA fork
2027년 7월 공개"


[narration]
"감사합니다. Phase 7 의 결정타 산출물 #4 에서 다시 뵙겠습니다."
```


---


## 실습 4: 녹화 + 편집


```bash
# Kazam 녹화 (간단)
kazam &
# 화면 영역 선택 -> Record -> 1분 녹화


# 또는 OBS Studio
obs &


# 편집 (Shotcut)
shotcut my_recording.mkv
# 자막 추가 -> Filters -> Text: Simple
# 자막 timing 맞추기
# 컷 편집 (불필요한 부분 자르기)
# Export -> H.264 / mp4 / 30fps
```


---


## 실습 5: Portfolio 패키징


**디렉토리**: `physical-ai-study/Portfolio/02_VLA_demo/`


```bash
cd ~/code/physical-ai-study
mkdir -p Portfolio/02_VLA_demo
cd Portfolio/02_VLA_demo


# 영상
cp /path/to/vla_demo.mp4 .


# README
cat > README.md << EOF
# 산출물 #2: OpenVLA -> ROS2 minimal demo


## 영상
[![demo](vla_demo.mp4)](vla_demo.mp4)


## 블로그
- [RT-2 정독 노트](https://velog.io/...)
- [OpenVLA + RTX 4070 실측](https://velog.io/...)


## 구성
- vla_inference/: OpenVLA Python wrapper
- vla_node/: ROS2 ament_python 패키지
- bag/: 1분 dry-run bag (압축)
- latency_data.csv: 측정 데이터


## 측정 결과 (RTX 4070, 4-bit nf4)
| Metric | Value |
|---|---|
| Mean latency | 165 ms |
| p95 latency | 220 ms |
| Throughput | 6 Hz |
| GPU Memory | 5.3 GB (model) + 1.0 GB (inference) |


## 의의
"VLA inference 가 실 robot SW 와 통합 가능함을 검증.
6Hz throughput 의 양산 의미: hierarchical (slow VLA + fast safety) 구조 필요."
EOF


# git
git add Portfolio/02_VLA_demo
git commit -m "phase4: vla_demo (산출물 #2)"
git push
```


---


## 실습 체크리스트


- [ ] Rerun 기본 사용 (`practice_rerun_basic.py`)
- [ ] rerun_logger ROS 노드 작성 + 빌드
- [ ] 영상 시나리오 작성 (`video_script.md`)
- [ ] 1분 녹화 + 편집 + 자막
- [ ] `Portfolio/02_VLA_demo/` 디렉토리 생성
- [ ] README + mp4 + 데이터 commit
- [ ] git push
- [ ] LinkedIn / Twitter 공유 (선택)
- [ ] quiz_easy / quiz_medium


---


## 참고 자료


- [Rerun.io 공식 문서](https://www.rerun.io/docs)
- [Rerun ROS examples](https://github.com/rerun-io/rerun-loader-python-example)
- [Shotcut tutorial](https://shotcut.org/howtos/)
- [OBS Studio (Ubuntu)](https://obsproject.com/wiki/install-instructions#linux)
