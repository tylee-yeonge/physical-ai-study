# Hardware-Arm Stage 1 - 가이드 (SO-101 본 빌드)

> **기간**: 2026.10-11 (2개월, 스파이크로 디리스크된 본 빌드 — 2026-08-30 실기 전환)
> **선행**: 2026.09 스파이크 판정 통과 (2026-09-21, [실기 전환 plan](../../../docs/superpowers/plans/2026-08-30-realworld-transition-execution.md) §5.4). 팔은 이미 조립돼 LeRobot 으로 돈다 — 본 빌드의 초점은 **완성도 + ROS 2 층**
> **추가 지출 없음** (키트·카메라·자재는 2026.08 말-09 초 선집행 — [../BOM.md](../BOM.md))
> **역할 분리**: 데이터·학습 = LeRobot / 배포·통합 = ROS 2 (같은 포트 — 동시에 한 스택만)

---

## Stage 1 일정

```
2026.10 첫 주 : ROS2 드라이버 검증 (구 스파이크 항목 인수)
                — 모터 1개 위치 명령 → 데이지체인 → 최소 URDF + RViz
2026.10       : ROS2 래핑 (feetech_ros2_driver + ros2_control) + 조립 완성
                (케이블 정리·작업대 고정·손목 카메라 마운트)
2026.11       : 안전 기초 (소프트 리밋·토크 상한·물리 e-stop) + URDF 오프셋 반영
                + 이중 latency 측정 + 1분 영상
                -> v2 선행 하드웨어. v2.5 (teleop 데이터셋 + SmolVLA) 병행 개시
```

---

## must / nice ([실기 전환 plan] §6 과 동일)

| 항목 | must/nice | 내용 |
|---|---|---|
| 조립 완성 | must | 케이블 정리, 작업대 고정, 손목 카메라 마운트 |
| 안전 기초 | must | 소프트 리밋 (관절 범위) + 토크 상한 + 물리 e-stop (전원 차단 스위치) |
| ROS2 래핑 | must | `feetech_ros2_driver` + ros2_control 노드 — LeRobot 스택과 병행 운영 |
| URDF | must | SO-101 공개 URDF 재사용 + 캘리브레이션 오프셋 반영 |
| 이중 latency | must | (a) LeRobot 직결 / (b) ROS2 토픽 경유 — (b)-(a) = **통합 오버헤드** (셋째 층 증거) |
| 1분 영상 | must | teleop + 정책 실행 + e-stop 시연 |
| Isaac Sim 임포트 | nice | Phase 6 이월 허용 ([isaac_sim_import.md](isaac_sim_import.md)) |

---

## 학습 파일

| 파일 | 내용 |
|---|---|
| [../BOM.md](../BOM.md) | SO-101 키트 구매 + 확인 항목 (구매는 스파이크 선행) |
| [URDF_guide.md](URDF_guide.md) | SO-101 공개 URDF 재사용 + 검증 |
| [ros2_driver_setup.md](ros2_driver_setup.md) | feetech_ros2_driver + ros2_control |
| [isaac_sim_import.md](isaac_sim_import.md) | Isaac Sim 임포트 (nice) |

---

## 완료 체크리스트

- [ ] ROS2 드라이버 검증 (모터 위치 명령 → 데이지체인 → 최소 URDF+RViz)
- [ ] 조립 완성 (케이블·작업대·손목 카메라)
- [ ] 안전 기초 — 소프트 리밋 + 토크 상한 + 물리 e-stop
- [ ] ROS2 래핑 — joint state / command 노드
- [ ] URDF — 공개 URDF 재사용 + 오프셋 반영, RViz 검증
- [ ] 이중 latency 측정 — (a)/(b)/통합 오버헤드
- [ ] 1분 영상 — teleop + 정책 실행 + e-stop
- [ ] (nice) Isaac Sim 임포트

---

## 참고

- [Roadmap/Hardware-Arm.md](../../../Roadmap/Hardware-Arm.md) — 마스터 가이드 (하드웨어 확정·비채택 기록)
- `feetech_ros2_driver` (ros2_control): https://github.com/JafarAbdi/feetech_ros2_driver
- LeRobot SO-101 문서: https://huggingface.co/docs/lerobot (진입 시 명령어 재확인)
- SO-101 공개 설계/URDF: https://github.com/TheRobotStudio/SO-ARM100
