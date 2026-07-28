# 학습 환경 & 워크플로우 가이드


> Perception 학습 전반에서 공용으로 참조하는 장비/원격 접속/시각화/데이터셋 가이드.
> 각 Phase 문서에서 이 파일을 링크로 참조한다.


---


## 1. 장비 개요


- **Ubuntu PC** — 메인 학습/실험 장비. RTX 4070 12GB VRAM / RAM 31GB / 28 코어 / 드라이버 580.173.02
  - **하드웨어 변경 불가** (RAM·GPU 증설 포함). 로컬 사양이 모자라는 작업은 증설이 아니라 **RunPod 이관**으로 대응한다
  - 단 **GUI 스트리밍이 필요한 작업은 RunPod 로 옮길 수 없다** — RunPod 은 UDP 미지원이고 Isaac Sim WebRTC 는 UDP 47998 이 필수다 (2026-07-28 확인, 상세는 [Hardware-Arm.md](Roadmap/Hardware-Arm.md) Stage 1 실행 머신 절). headless 스크립트 워크로드만 이관 가능
  - 자택 이전 조율 중 (2026-07-28) — 성사되면 휴직 중 물리 접근 제약이 해소된다. Isaac Sim GUI 작업이 로컬 전용이라 Stage 1 (2027.01-02) 의 선행 조건이기도 하다 (remediation plan 결정 #5)
- **MacBook Pro 14 (M4 Pro)** — 원격 접속·문서 작업. LeRobot 은 macOS Apple Silicon 을 공식 지원해 teleop·record 백업 경로로 쓸 수 있으나, **ROS2 는 불가** (macOS 바이너리 패키지 없음 + Docker Desktop for Mac 의 USB 패스스루 미지원)
- **Jetson Orin Nano (8GB)** — **256GB SSD + Ubuntu 22.04 설치 완료**. Ubuntu 22.04 이므로 **ROS 2 Humble 을 apt 로 설치**할 수 있고 (Phase 3 배포판과 동일 절차) LeRobot 이 요구하는 Python 3.10 도 기본이다. SO-101 + LeRobot 구동 사례가 공개돼 있고 NVIDIA 공식 SO-101 코스의 기준 구성이기도 하다. **4070 PC 자택 이전이 무산될 경우 Stage 1 의 팔 옆 기계 역할** — 상세는 [Hardware-Arm.md](Roadmap/Hardware-Arm.md) Stage 1 실행 머신 절
  - 한계: ACT 학습은 소규모 데이터셋에서 보고된 사례가 있으나 정석은 4070/RunPod. **Isaac Sim 은 불가** (x86 전용), OpenVLA 7B 도 비현실적
  - 확인 필요: JetPack / L4T 버전과 **CUDA·cuDNN 동반 설치 여부** (Ubuntu 22.04 만 올린 경우 CUDA 스택이 없으면 PyTorch GPU 가 안 붙는다) — `cat /etc/nv_tegra_release`, `dpkg -l nvidia-jetpack`
- **ELP Stereo Camera** — USB 연결 주변기기, 실카메라 입력용
- (예정) **손목 카메라** — 소형 UVC USB 웹캠 1대. SO-101 팔로워 그리퍼용, Stage 1 (2027.01-02) 시점 구매


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


### 4-1. Rerun 사용 시나리오 (원격 PC + 로컬 viewer)


Ubuntu PC (원격) 에서 코드 실행 -> 맥북/노트북 (로컬) 에서 viewer 보기 가 표준 흐름.
세 가지 방법이 있고 상황에 맞게 골라쓴다.


| 방법 | 원격 코드 | 로컬 필요 | 포트포워딩 | 언제 좋은가 |
|---|---|---|---|---|
| **A. RRD 파일** | `rr.save("out.rrd")` | rerun viewer | 불필요 | 결과 한 번 확인. 가장 안정 |
| **A-2. RRD + hosted 뷰어** | `rr.save("out.rrd")` | 브라우저만 | 불필요 | 로컬 설치 없이 .rrd 확인 |
| **B. serve_web** | `rr.serve_web(...)` | 브라우저만 | 9090 + 9876 | 빠른 반복 확인 |
| **C. gRPC connect** | `rr.serve_grpc(...)` | rerun viewer | 9876 | 큰 데이터 / 부드러운 3D 인터랙션 |


**로컬 (맥북) viewer 설치 — conda 권장**:


```bash
# conda 가 없다면 한 번만
brew install --cask miniforge
conda init "$(basename "${SHELL}")"   # zsh / bash

# rerun 전용 env
conda create -n rerun python=3.12 -c conda-forge -y
conda activate rerun
# conda-forge 채널에는 최신 버전이 없을 수 있어 pip 로 설치 (원격과 버전 일치)
pip install "rerun-sdk==0.33.0"
rerun --version
```


> **버전 고정 중요**: RRD 포맷이 버전 간 비호환적일 수 있어, 원격 컨테이너의 `rerun-sdk` 와 같은 버전으로 맞춘다. `python3 -c "import rerun; print(rerun.__version__)"` 로 원격 버전 확인.


**방법 A 표준 패턴** (가장 추천):


```python
# 원격 코드
rr.init("my_app", spawn=False)
log_to_rerun(...)
rr.save("output/result.rrd")
```


VSCode 파일트리에서 RRD 우클릭 -> Download -> 로컬에서 `rerun result.rrd`.


**방법 A-2: 로컬 설치 없이 hosted 웹 뷰어로 열기** (가장 간단):


로컬에 `rerun-sdk` 를 안 깔고도 .rrd 를 볼 수 있다. 위와 동일하게 .rrd 를 Download 한 뒤:


1. 브라우저로 https://rerun.io/viewer 접속
2. .rrd 파일을 화면에 드래그앤드롭 (또는 메뉴에서 Open)


뷰어는 WASM 으로 브라우저 안에서 전부 처리되므로 파일이 서버로 업로드되지 않는다 (로컬에서만 읽힘).


> **버전 일치 필수**: hosted 뷰어도 .rrd 를 만든 SDK 버전과 맞아야 한다. https://rerun.io/viewer 는 최신 안정 버전을 띄우므로, 원격 `rerun-sdk` 가 최신이면 그대로 열린다. version mismatch 에러가 나면 버전을 박은 URL 을 쓴다: `https://app.rerun.io/version/<원격버전>/` (예: `https://app.rerun.io/version/0.33.0/`). 원격 버전은 `python3 -c "import rerun; print(rerun.__version__)"` 로 확인.


**VSCode Tunnel + serve_web (방법 B)**:


PORTS 패널에서 `9090` (web), `9876` (gRPC) 두 포트 모두 forward. 9090 의 forwarded URL 을 브라우저로 열면 viewer 가 자동으로 gRPC 에 연결.


---


## 5. 데이터셋 저장 전략


### 권장 저장 경로 (Ubuntu PC 디스크)


```
~/datasets/
├── kitti/ # raw, stereo
├── kitti_object/ # 3D detection (~12GB)
├── nuscenes_mini/ # 입문용 (~3GB)
└── middlebury/ # stereo matching 벤치마크
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
- `rr.serve_web(web_port=9090, grpc_port=9876)` 로 포트 명시적 지정
- rerun 0.23+ 에서 `ws_port` 인자는 제거됨 (`grpc_port` 로 통합)


### Rerun viewer 가 빈 화면으로 멈출 때 (serve_web 사용 시)
- web (9090) 만 forward 되고 gRPC (9876) 가 누락된 경우. 두 포트 모두 forward 필요
- 원격/로컬 `rerun-sdk` 버전 불일치 가능. `rerun --version` 과 원격 `python3 -c "import rerun; print(rerun.__version__)"` 비교


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
