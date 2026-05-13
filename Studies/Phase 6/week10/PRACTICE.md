# Week 10 실습: Force / Torque 비교

> [time] **예상 시간**: 4시간

---

## [note] 실습 1: Dynamixel torque read

```python
from dynamixel_sdk import *
PORT = "/dev/ttyUSB0"
BAUDRATE = 1000000

portHandler = PortHandler(PORT)
packetHandler = PacketHandler(2.0)
portHandler.openPort()
portHandler.setBaudRate(BAUDRATE)

ADDR_PRESENT_CURRENT = 126  # XM430
for dxl_id in range(1, 7):
    cur, _, _ = packetHandler.read2ByteTxRx(portHandler, dxl_id, ADDR_PRESENT_CURRENT)
    # Signed conversion
    if cur > 32767:
        cur -= 65536
    torque_mNm = cur * 4.1  # Approx
    print(f"DXL {dxl_id}: {torque_mNm/1000:.3f} Nm")
```

---

## [note] 실습 2: Sim effort

```python
sim_effort = arm.get_applied_joint_efforts()
print(f"Sim effort (Nm): {sim_effort}")
```

매 step 마다 기록.

---

## [note] 실습 3: 시각화

```python
import matplotlib.pyplot as plt
import numpy as np

t = np.arange(len(real_torques))
fig, axes = plt.subplots(6, 1, figsize=(8, 12), sharex=True)
for i in range(6):
    ax = axes[i]
    ax.plot(t, real_torques[:, i], label='Real', alpha=0.8)
    ax.plot(t, sim_efforts[:, i], label='Sim', alpha=0.8)
    ax.set_title(f"Joint {i+1}")
    ax.set_ylabel("Nm")
    ax.legend()
plt.tight_layout()
plt.savefig("torque_compare.png")
```

---

## [O] 체크리스트
- [ ] Dynamixel torque read
- [ ] Sim effort 측정
- [ ] 시계열 비교
- [ ] quiz
