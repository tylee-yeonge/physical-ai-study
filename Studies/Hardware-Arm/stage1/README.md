# Hardware-Arm Stage 1 - 가이드


> **기간**: 2027.01-02 (2개월, 스파이크로 디리스크된 본 빌드)
> **목표**: Dynamixel 2-3DOF + URDF + ROS2 + Isaac Sim 임포트
> **30-50만원**


---


## Stage 1 일정


```
2027.01 전반: Dynamixel + U2D2 구매 (스파이크 부품 재활용) + URDF + ROS2 드라이버
2027.01 후반: pick-and-place (programmed motion)
2027.02: Isaac Sim 임포트 + 1분 영상
        -> v2 선행 하드웨어
```


---


## 학습 파일


| 파일 | 내용 |
|---|---|
| [BOM.md](BOM.md) | 부품 구매 + 비용 |
| [URDF_guide.md](URDF_guide.md) | URDF 작성 |
| [ros2_driver_setup.md](ros2_driver_setup.md) | dynamixel_hardware |
| [isaac_sim_import.md](isaac_sim_import.md) | Isaac Sim 임포트 |


---


## 단계별 진행


### 2027.01 전반: 부품 + 조립
- XL330 2-3 + U2D2 구매
- 3D 프린트 부품 (open-source fork)
- Dynamixel Wizard 동작 확인


### 2027.01 후반: URDF + ROS2
- URDF/XACRO 작성
- RViz 검증
- dynamixel_hardware 패키지


### 2027.02: 동작 + Sim
- pick-and-place (programmed motion)
- Isaac Sim URDF 임포트
- 1분 영상 -> v2 선행 하드웨어


---


## 안전 메커니즘 (Stage 1 basic)
- Position limit (URDF joint limit)
- Velocity limit (50% of max)
- 수동 e-stop (전원 차단)


Stage 2 에서 자동화 (C++ 노드).


---


## 완료 체크리스트
- [ ] 2-3DOF 동작
- [ ] URDF + RViz
- [ ] ROS2 드라이버
- [ ] pick-and-place
- [ ] Isaac Sim 임포트
- [ ] 1분 영상


---


## 참고
- [Roadmap/Hardware-Arm.md](../../../Roadmap/Hardware-Arm.md)
- Dynamixel: https://emanual.robotis.com/
- dynamixel_hardware: https://github.com/dynamixel-community/dynamixel_hardware
