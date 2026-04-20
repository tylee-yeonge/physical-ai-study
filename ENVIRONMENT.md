# 학습 환경 & 워크플로우 가이드

> Perception 학습 전반에서 공용으로 참조하는 장비/원격 접속/시각화/데이터셋 가이드.
> 각 Phase 문서에서 이 파일을 링크로 참조한다.

---

## 1. 장비 개요

- **Ubuntu PC (RTX 4070, 12GB VRAM)** — 메인 학습/실험 장비
- **ELP Stereo Camera** — Ubuntu PC 에 USB 연결된 주변기기, 실카메라 입력용
- (보류) **Jetson Orin Nano** — 하드웨어 실습 시간 확보 시점에 재도입

---

## 2. 역할 분담 원칙

- 기본: **Ubuntu PC 중심** (ELP 는 USB 주변기기)
- 출장지: 원격 접속으로 동일 PC 사용. PC 에 ELP 가 연결되어 있으면 실카메라 데이터도 원격으로 처리 가능
- Jetson 작업은 현재 비활성 — Phase 3 후반 TensorRT 배포 파트에서 하드웨어 확보 시 재평가

---

## 3. 원격 접속 셋업

| 도구 | 용도 | 설치/시작 |
|---|---|---|
| Tailscale | PC + 노트북 메시 (포트 포워딩 불필요) | `curl -fsSL https://tailscale.com/install.sh \| sh` |
| VS Code Tunnel | 브라우저/데스크탑에서 원격 접속 | PC 에서 `code tunnel` 실행 → vscode.dev 로 접속 |
| Docker | CUDA + OpenCV + PyTorch 개발 환경 격리 | 예: `nvidia/cuda:12.1.0-cudnn8-devel-ubuntu22.04` |
| tmux | 학습 중단 방지 | `tmux new -s study` |

**포트 포워딩 (SSH tunnel 시)**: Rerun 9090, Jupyter 8888 등

> 이미 다른 원격 접속 도구(예: SSH + VPN)를 쓰고 있다면 대체 가능. 본 가이드는 권장 조합을 제시할 뿐 필수 아님.

---

## 4. 시각화 도구

| 도구 | 용도 | 설치/시작 |
|---|---|---|
| **Rerun.io** | perception 주력 (이미지, depth, point cloud, 3D box) | `pip install rerun-sdk` / https://rerun.io/docs/getting-started |
| Jupyter inline | 정적 이미지 빠른 확인 (matplotlib, PIL) | `pip install jupyterlab` |
| Foxglove Studio | ROS 2 토픽 실시간 모니터링 (ROS 2 쓰는 경우 한정) | https://foxglove.dev/ |
| VNC (TigerVNC) | rviz2 불가피 시 최후 수단 | `apt install tigervnc-standalone-server` |

기본 원칙: **Rerun.io + Jupyter inline** 으로 대부분 해결. VNC 는 최후의 수단.

---

## 5. 데이터셋 저장 전략

### 권장 저장 경로 (Ubuntu PC 디스크)

```
~/datasets/
├── kitti/              # raw, stereo
├── kitti_object/       # 3D detection (~12GB)
├── nuscenes_mini/      # 입문용 (~3GB)
└── middlebury/         # stereo matching 벤치마크
```

### 용량/시간 주의

- **KITTI Object Detection (~12GB)**: 홈 네트워크 기준 다운로드 30분~1시간
- **nuScenes mini-split (~3GB)**: 10-20분
- **nuScenes full (~400GB)**: Phase 4 진입 시 별도 검토, 디스크 확보 후

**다운로드 시간은 실습 시간에 미포함**. 출장 전 선행 다운로드 권장.

---

## 6. ELP Stereo Camera 실습 팁

- **좌/우 스트림 동시 캡처**: `cv::VideoCapture` 두 채널 (device index 확인: `v4l2-ctl --list-devices`)
- **기계적 고정**: 삼각대 또는 클램프로 baseline 유지 (움직이면 캘리브 무효화)
- **캘리브 타겟**: ChArUco 보드 A3 권장 (두꺼운 판지에 부착해 평탄도 확보)
- **원격 작업 시**: PC 에 ELP 가 항시 연결 + 전원 on 상태 유지 필요

---

## 7. Phase 별 권장 장비 매트릭스

| Phase | 주 장비 | 출장지 가능 여부 |
|---|---|---|
| Phase 2 Week 3 | Ubuntu PC (원격) | O |
| Phase 2 Week 3 (선택) | Ubuntu PC + ELP | 조건부 O (PC-ELP 연결 유지 시) |
| Phase 2 Week 4 | Ubuntu PC (원격) | O |
| Phase 3 학습 | Ubuntu PC (원격) | O |
| Phase 3 Jetson 배포 | (보류) | 하드웨어 확보 시 |
| Phase 4 | Ubuntu PC (원격) | O |

---

## 8. 문제 해결 (FAQ)

### 원격에서 ELP 카메라가 인식 안 될 때
- `udevadm monitor` 로 이벤트 확인
- `/dev/video*` 권한 확인 (필요 시 사용자를 `video` 그룹에 추가)
- USB 리셋: `usbreset <bus>:<device>` 또는 `echo 0 > /sys/bus/usb/devices/.../authorized` 후 다시 1
- Docker 사용 시 `--device=/dev/video0 --device=/dev/video1` 플래그 필요

### Rerun 네트워크 포트가 방화벽에 막힐 때
- Tailscale ACL / firewall 규칙 확인
- `rr.serve(web_port=9090, ws_port=9877)` 로 포트 명시적 지정

### Tailscale 경로가 느릴 때
- `tailscale status` 로 direct 연결 여부 확인 — `relay` 표시면 DERP 경유 중
- UDP 포트(41641) 허용 / NAT 타입 확인

### Docker GPU 패스스루가 안 될 때
- `nvidia-container-toolkit` 설치 확인
- 실행 시 `--gpus all` 플래그
- 호스트 NVIDIA 드라이버 버전과 컨테이너 CUDA 요구 버전 일치 확인
- `nvidia-smi` 가 컨테이너 내부에서 동작하는지 확인

---

## 참고

본 가이드는 **실습 환경의 원칙/진입점**만 다룬다. 상세 드라이버 셋업이나 udev 규칙 스크립트는 범위 밖 (별도 문서 또는 필요 시 추가 작성).
