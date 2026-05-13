# Week 8 실습: Latency 측정


> **예상 시간**: 5시간


---


## 실습 1: latency_monitor 노드


```python
"""
latency_monitor.py - Sim 과 Real 의 latency 모니터링
"""
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
from std_msgs.msg import Float64
import numpy as np




class LatencyMonitor(Node):
    def __init__(self):
        super().__init__('latency_monitor')
        self.sim_lats = []
        self.real_lats = []
        self.sub = self.create_subscription(
            JointState, '/joint_states', self.on_joint, 30)
        self.timer = self.create_timer(10.0, self.summary)


    def on_joint(self, msg):
        t_msg = rclpy.time.Time.from_msg(msg.header.stamp)
        t_now = self.get_clock().now()
        latency = (t_now - t_msg).nanoseconds / 1e6
        # 분류 (msg source 에 따라)
        self.real_lats.append(latency)


    def summary(self):
        if self.real_lats:
            arr = np.array(self.real_lats[-1000:])
            self.get_logger().info(
                f"Real latency: mean={arr.mean():.1f}ms, p95={np.percentile(arr, 95):.1f}ms")




def main():
    rclpy.init()
    rclpy.spin(LatencyMonitor())
```


---


## 실습 2: Sim step time 측정


```python
"""
sim_step_bench.py
"""
import time
import numpy as np


step_times = []
for i in range(1000):
    t0 = time.time()
    world.step(render=True)
    step_times.append((time.time() - t0) * 1000)


arr = np.array(step_times[100:]) # warm-up 제외
print(f"Sim step mean: {arr.mean():.2f} ms")
print(f"Sim step p95 : {np.percentile(arr, 95):.2f} ms")
```


---


## 실습 3: Real actuator response


```python
"""
real_response_bench.py - Dynamixel 의 response time
"""
# 자작 팔의 motor 에 command -> joint_state 받기까지의 시간
import time
import numpy as np


response_times = []
for i in range(100):
    cmd_time = time.time()
    # send command (dynamixel_sdk 사용)
    # 다음 read 까지의 시간
    response_time = ... # 측정
    response_times.append((time.time() - cmd_time) * 1000)


print(f"Real actuator mean: {np.mean(response_times):.2f} ms")
```


---


## 실습 4: Histogram + 보고서


```python
import matplotlib.pyplot as plt
import numpy as np


sim_lats = np.array(sim_step_times)
real_lats = np.array(real_response_times)


fig, ax = plt.subplots(1, 2, figsize=(12, 4))
ax[0].hist(sim_lats, bins=50, alpha=0.7)
ax[0].set_title(f"Sim Latency (mean={sim_lats.mean():.1f}ms)")
ax[0].set_xlabel("Latency (ms)")
ax[1].hist(real_lats, bins=50, alpha=0.7, color='orange')
ax[1].set_title(f"Real Latency (mean={real_lats.mean():.1f}ms)")
plt.tight_layout()
plt.savefig("latency_compare.png")


# Phase 7 산출물 #4 의 보고서에 인용 가능
```


---


## 체크리스트
- [ ] latency_monitor 노드
- [ ] sim step 1000 sample
- [ ] real actuator 100 sample
- [ ] histogram 시각화
- [ ] quiz
