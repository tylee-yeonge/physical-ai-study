# Week 7 실습: 카메라 확인 + FOV 측정 + Side-by-side


> **예상 시간**: 4시간


---


## 실습 1: 전체 뷰 카메라 확인 + FOV 측정


카메라는 이미 팔에 고정돼 있다 (Stage 1 W3). 여기서는 컨테이너에서 열리는지 보고, Sim 에 넣을 값 두 가지 (카메라 위치, 화각) 를 잰다.


```bash
# 컨테이너의 so101-attach 가 고정 노드를 만든다 (카메라를 다시 꽂았거나 컨테이너를 재시작했으면 재실행)
so101-attach
ls -la /dev/so101_cam_overview
# 기대: crw-r--r-- ... 81, N /dev/so101_cam_overview  (전체 뷰 카메라 1대 = 노드 1개)
```


```bash
# OpenCV 로 한 장 읽기 (녹화와 같은 설정: MJPG 1280x720)
/workspace/venvs/lerobot/bin/python -c "
import cv2
cap = cv2.VideoCapture('/dev/so101_cam_overview', cv2.CAP_V4L2)
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG')); cap.set(3, 1280); cap.set(4, 720)
ret, img = cap.read()
print(img.shape if ret else 'fail')   # 기대: (720, 1280, 3)
cap.release()
"
```


카메라를 여는 코드는 어디서나 같다. 이 설정이 아니면 열리지 않거나 fps 가 떨어진다:


```python
import cv2
cap = cv2.VideoCapture("/dev/so101_cam_overview", cv2.CAP_V4L2)  # so101-attach 가 만든 고정 경로. /dev/video* 번호는 바뀌므로 쓰지 않는다
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*"MJPG"))  # 압축 전송. YUYV 무압축은 1280x720 에서 10 fps 가 한계
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)  # 녹화 (week2_guide §1.4) 와 같은 해상도
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
cap.set(cv2.CAP_PROP_FPS, 30)  # 이 카메라가 지원하는 fps 는 30 하나뿐
ret, frame = cap.read()  # frame: BGR uint8 (720, 1280, 3) — 자를 것 없이 그대로 쓴다
```


**카메라 위치 재기** (Sim 의 `position` 에 들어간다)


팔로워 베이스 (바닥판 중심) 를 원점으로 두고 줄자로 센다. 단위는 m.


| 값 | 재는 법 | 메모 |
|---|---|---|
| x | 베이스에서 팔이 뻗는 방향으로 카메라 렌즈까지 | |
| y | 베이스에서 옆 방향으로 렌즈까지 (팔 기준 왼쪽이 +) | |
| z | 책상 면에서 렌즈까지 높이 | |
| 내려다보는 각도 | 렌즈가 향하는 방향과 수평면 사이 각 (약 45도) | |


**HFOV 재기** (Sim 의 `focal` 에 들어간다)


1. 뷰어를 켠다: `python /workspace/study/physical-ai-study/Studies/Hardware-Arm/spike/week2/scripts/live_view.py` (브라우저 보는 법은 week2_guide §1.2).
2. 렌즈 앞 거리 D (예: 0.60 m) 에 줄자를 가로로 놓고, 화면 왼쪽 끝과 오른쪽 끝에 보이는 눈금의 차이 L 을 읽는다.
3. 뷰어를 끈다 (`Ctrl+C`). 켜 둔 채로는 다른 프로그램이 카메라를 열지 못한다.


```python
import math
D = 0.60  # 렌즈에서 줄자까지 거리 (m) — 잰 값으로 바꾼다
L = 0.92  # 화면 가로에 들어온 폭 (m) — 잰 값으로 바꾼다
hfov = 2 * math.atan(L / (2 * D))  # 가로 화각 (rad)
print(math.degrees(hfov))  # 도 단위로 확인. 웹캠은 보통 60-90도
```


---


## 실습 2: Sim Camera


```python
"""
practice_sim_camera.py
"""
# Sim setup
from omni.isaac.sensor import Camera
import math
import numpy as np


hfov = math.radians(78.0)  # 실습 1 에서 잰 HFOV — 잰 값으로 바꾼다


# 실습 1 에서 잰 Real 카메라 위치 (팔로워 베이스 기준, m) — 잰 값으로 바꾼다
camera = Camera(
    prim_path="/World/ExternalCamera",
    position=np.array([0.45, -0.35, 0.50]),
    # 자작 팔 향함 (orientation 은 자작 팔의 setup 따라 조정 — 약 45도로 내려다보는 자세)
    orientation=np.array([0.7071, -0.7071, 0.0, 0.0]),
    resolution=(1280, 720),  # Real 과 같은 해상도
)
camera.initialize()


aperture = camera.get_horizontal_aperture()  # 센서 가로 폭 (mm). 버전마다 기본값이 달라 읽어서 쓴다
focal = aperture / (2 * math.tan(hfov / 2))  # pinhole 공식: 같은 화각이 되는 초점거리 (mm)
camera.set_focal_length(focal)
print(f"aperture {aperture:.2f} mm -> focal {focal:.2f} mm")
```


---


## 실습 3: Side-by-side 영상 capture


```python
"""
practice_side_by_side.py
"""
import cv2
import numpy as np


cap = cv2.VideoCapture("/dev/so101_cam_overview", cv2.CAP_V4L2)  # so101-attach 가 만든 고정 경로
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*"MJPG"))  # 무압축 YUYV 로는 30 fps 가 안 나온다
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)  # 실습 1 과 같은 설정
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
cap.set(cv2.CAP_PROP_FPS, 30)
# Sim setup ... (실습 2 의 camera 부착)


import imageio
out_frames = []


for i in range(300): # 10초 @ 30 FPS
    # Real
    ret, real = cap.read()  # (720, 1280, 3) 그대로. 자르지 않는다
    real = cv2.cvtColor(real, cv2.COLOR_BGR2RGB)  # OpenCV 는 BGR, imageio 는 RGB


    # Sim
    world.step(render=True)
    sim = camera.get_rgba()[:, :, :3].astype('uint8')  # (720, 1280, 3) — resolution 을 Real 과 맞췄으므로 크기가 같다


    # Concat
    side = np.hstack([real, sim])  # 가로로 붙인다: (720, 2560, 3)
    out_frames.append(side)


imageio.mimsave('real_vs_sim.mp4', out_frames, fps=30)
cap.release()  # 카메라를 놓아 lerobot 이 다시 열 수 있게 한다
```


---


## 체크리스트
- [ ] 전체 뷰 카메라가 컨테이너에서 1280x720 으로 열린다
- [ ] 카메라 위치 (x, y, z, 각도) 와 HFOV 를 쟀다
- [ ] Sim Camera 위치 · 해상도 · focal 매칭
- [ ] Side-by-side 영상 capture
- [ ] quiz
