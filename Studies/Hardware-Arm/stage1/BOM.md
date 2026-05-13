# Hardware-Arm Stage 1 - BOM (Bill of Materials)


> 구매 시점: 2026.10
> 약 30~50만원


---


## 모터 (Dynamixel XL330)


| 항목 | 수량 | 단가 | 합계 |
|---|---|---|---|
| Dynamixel XL330-M288-T | 2~3 | ~10만 | 20~30만 |
| (또는 XL430 으로 upgrade) | | | |


XL330 spec:
- Stall torque: 0.16 Nm (~16 kgf-cm)
- Speed: 480 rpm (no load)
- Communication: TTL (3-pin)
- Position resolution: ~0.088 deg
- Voltage: 5V


---


## 컨트롤러


| 항목 | 수량 | 단가 | 합계 |
|---|---|---|---|
| U2D2 (USB-DXL 컨버터) | 1 | ~5만 | 5만 |
| 5V SMPS 전원 | 1 | ~2만 | 2만 |
| 5-pin / 3-pin 케이블 (Dynamixel 표준) | 5 | ~2K | 1만 |
| USB-A cable | 1 | ~5K | 5K |


---


## 그리퍼


| 항목 | 비용 |
|---|---|
| 단순 2-finger (3D 프린트) | 0 (filament 만) |
| 또는 RobotIs OpenManipulator gripper | ~10~15만 |


---


## 3D 프린트 부품 (보유 프린터)


- Base plate
- Link 1, Link 2 (필요 시 Link 3)
- Joint mount (Dynamixel 호환)
- Gripper (2-finger)


오픈소스 reference:
- OpenManipulator-X (ROBOTIS, 일부 fork 가능)
- BCN3D MOVEO


PLA filament: 1 kg ~ 3만원.


---


## 카메라


- ELP Stereo (보유) - 0
- External 또는 ee-mount (Stage 2 에서 추가)


---


## Stage 1 BOM 합계


```
모터 (XL330 x 3) : 30만
U2D2 + 전원 + 케이블 : 10만
3D 프린트 filament : 3만
그리퍼 : 0 (직접 프린트)
카메라 : 0 (보유)
------------------------
합계 : 약 43만원
```


---


## 구매처


- ROBOTIS 공식: https://www.robotis.com/
- 한국: https://www.robotisplaza.com/
- 미국: https://www.trossenrobotics.com/


---


## Stage 2 추가 BOM (참고, 2027.02~)


- Dynamixel XM430-W350-T x 3~4 (출력 모터): ~80~100만
- Teleop 입력 (PS4 패드 또는 leader-follower 부품): ~5~30만
- 카메라 ee-mount 부품 (3D 프린트): 0


Stage 2 추가 합계: 약 100~150만.


---


## 합계 (Stage 1 + Stage 2)


약 150~200만 (모든 옵션 포함).
