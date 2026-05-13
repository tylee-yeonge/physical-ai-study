# Week 1 실습: Isaac Sim 설치 + hello-world


> **예상 시간**: 6시간


---


## 실습 1: 시스템 점검


```bash
# GPU
nvidia-smi
# 기대: RTX 4070 12GB


# CUDA
nvcc --version
# 기대: 11.8 또는 12.x


# Disk
df -h
# 기대: 100GB+ 여유


# RAM
free -h
# 기대: 32GB+
```


---


## 실습 2: Isaac Sim 설치


```bash
# Conda 환경
conda create -n phase6 python=3.10 -y
conda activate phase6


# pip 으로 Isaac Sim 4.x
pip install --extra-index-url https://pypi.nvidia.com \
    isaacsim==4.5.0.0 \
    isaacsim-extscache-physics==4.5.0.0 \
    isaacsim-extscache-kit==4.5.0.0


# 동작 확인 (Python REPL)
python -c "import isaacsim; print(isaacsim.__version__)"
```


trouble:
- pip install fail -> CUDA / NVIDIA driver 확인
- import fail -> conda env activate 확인


---


## 실습 3: 첫 SimulationApp


```python
"""
practice_hello_isaac.py
"""
from isaacsim import SimulationApp


# Headless 또는 GUI
HEADLESS = True
sim_app = SimulationApp({"headless": HEADLESS})


# Isaac Sim 4.x API
from omni.isaac.core import World
from omni.isaac.core.objects import DynamicCuboid


world = World()
world.scene.add_default_ground_plane()


cube = world.scene.add(
    DynamicCuboid(
        prim_path="/World/cube",
        name="my_cube",
        position=[0, 0, 1.0],
        scale=[0.5, 0.5, 0.5],
        color=[1.0, 0, 0],
    )
)


world.reset()


# Step 100 frames
for i in range(100):
    world.step(render=True)
    pos = cube.get_world_pose()[0]
    if i % 10 == 0:
        print(f"step {i}: cube at {pos}")


sim_app.close()
```


실행:
```bash
python practice_hello_isaac.py
```


기대 출력:
```
step 0: cube at [0, 0, 1.0]
step 10: cube at [..., dropping...]
...
step 90: cube at [0, 0, 0.25] # 떨어져 ground 위
```


---


## 실습 4: USD 파일로 저장


```python
"""
practice_save_usd.py
"""
from isaacsim import SimulationApp
sim_app = SimulationApp({"headless": True})


from omni.isaac.core.utils.stage import save_stage
from omni.isaac.core import World
from omni.isaac.core.objects import DynamicCuboid


world = World()
world.scene.add_default_ground_plane()
world.scene.add(DynamicCuboid(prim_path="/World/cube", scale=[0.5]*3))


# Stage 저장
save_stage("my_first.usd")
print("saved to my_first.usd")


sim_app.close()
```


이후 GUI Isaac Sim 에서 `my_first.usd` 열면 cube 확인 가능.


---


## 실습 5: trouble 기록


`~/phase6_notes/week1/errors_log.md`:


```markdown
# Isaac Sim 설치 에러 기록


## 에러 1: pip install fail
- 증상: CUDA error
- 해결: ___


## 에러 2: import fail
- 증상: ___
- 해결: ___


## 안정 동작 확인
- [ ] SimulationApp 생성 성공
- [ ] hello cube 동작
- [ ] USD 저장
```


---


## 체크리스트
- [ ] 시스템 요구사항 통과
- [ ] Isaac Sim 4.x 설치 성공
- [ ] hello-world cube 동작
- [ ] USD 저장
- [ ] errors_log
- [ ] quiz
