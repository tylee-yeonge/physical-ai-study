# Week 7 실습: 카메라 부착 + Side-by-side


> **예상 시간**: 4시간


---


## 실습 1: ELP Stereo 부착


```bash
# ELP Stereo 의 USB 연결
ls /dev/video*
# 기대: /dev/video0, /dev/video2 (stereo 의 left/right)


# OpenCV 로 read
python -c "import cv2; cap=cv2.VideoCapture(0); ret, img = cap.read(); print(img.shape if ret else 'fail')"
```


ELP Stereo 의 양쪽 image 가 한 frame 에 결합 (1280x480).
한 쪽 (left) 만 사용:
```python
import cv2
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
ret, frame = cap.read()
left = frame[:, :640] # left half
```


---


## 실습 2: Sim Camera


```python
"""
practice_sim_camera.py
"""
# Sim setup
from omni.isaac.sensor import Camera
import numpy as np


# 자작 팔 위치 (0,0,0) 에서 0.5m 떨어진 곳 / 0.4m 높이
camera = Camera(
    prim_path="/World/ExternalCamera",
    position=np.array([0.5, 0.0, 0.4]),
    # 자작 팔 향함 (orientation 은 자작 팔의 setup 따라 조정)
    orientation=np.array([0.7071, -0.7071, 0.0, 0.0]),
    resolution=(640, 480),
)
camera.initialize()
camera.set_focal_length(15.0)
```


---


## 실습 3: Side-by-side 영상 capture


```python
"""
practice_side_by_side.py
"""
import cv2
import numpy as np


elp = cv2.VideoCapture(0)
# Sim setup ... (camera 부착)


import imageio
out_frames = []


for i in range(300): # 10초 @ 30 FPS
    # Real
    ret, real = elp.read()
    real = real[:, :640] # left
    real = cv2.cvtColor(real, cv2.COLOR_BGR2RGB)


    # Sim
    world.step(render=True)
    sim = camera.get_rgba()[:, :, :3].astype('uint8')


    # Concat
    side = np.hstack([real, sim])
    out_frames.append(side)


imageio.mimsave('real_vs_sim.mp4', out_frames, fps=30)
elp.release()
```


---


## 체크리스트
- [ ] ELP Stereo 동작
- [ ] Sim Camera 위치 매칭
- [ ] Side-by-side 영상 capture
- [ ] quiz
