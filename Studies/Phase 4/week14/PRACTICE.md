# Week 14 실습: README 작성 + 환경 세팅 가이드 + 검증

> [goal] **실습 목표**: `Portfolio/02_VLA_demo/README.md` 완성 + clean 환경에서 재현 검증.
> [time] **예상 시간**: 5시간

---

## [note] 실습 1: README 작성 (전체 골격)

**파일명**: `Portfolio/02_VLA_demo/README.md`

```markdown
# OpenVLA -> ROS2 Minimal Demo

> **Phase 4 산출물 #2** of [physical-ai-study](https://github.com/...)
> Vision-Language-Action 모델을 ROS2 환경에 통합한 minimal demo.

[![demo](demo.gif)](vla_demo.mp4)

## One-liner
OpenVLA 7B (4-bit nf4) inference 를 ROS2 토픽으로 받는 minimal demo. RTX 4070 12GB 에서 mean 165ms latency / 6Hz throughput.

## Demo
- [1분 영상](vla_demo.mp4)
- [RT-2 정독 블로그](https://velog.io/...)
- [OpenVLA 정독 + 실측 블로그](https://velog.io/...)

## Architecture

\`\`\`mermaid
flowchart LR
    cam[/camera/image_raw/] --> node[vla_node]
    instr[/vla/instruction/] --> node
    node --> act[/vla/action/]
    node --> grip[/vla/gripper/]
    node --> lat[/vla/latency_ms/]
\`\`\`

## Quick Start

\`\`\`bash
git clone https://github.com/<you>/physical-ai-study
cd physical-ai-study/Portfolio/02_VLA_demo

# 환경 세팅 (~ 10분)
./scripts/setup.sh

# 실행
ros2 launch vla_node demo.launch.py
\`\`\`

## Installation

### Hardware Requirements
- GPU: NVIDIA RTX 4070 (12GB) 이상
- RAM: 16 GB 이상
- Disk: 30 GB (OpenVLA model 포함)

### Software
- Ubuntu 22.04
- CUDA 11.8 또는 12.x
- Python 3.10
- ROS2 Humble

### Step-by-step

\`\`\`bash
# 1. CUDA + ROS2
nvidia-smi  # 검증
source /opt/ros/humble/setup.bash
ros2 --version  # 검증

# 2. Conda env
conda create -n vla python=3.10 -y
conda activate vla

# 3. Python deps
pip install -r requirements.txt

# 4. vla_inference 패키지
pip install -e ./vla_inference

# 5. ROS2 workspace 빌드
cd vla_node
colcon build --symlink-install
source install/setup.bash

# 6. 검증
python -c "import torch; print(torch.cuda.is_available())"  # True
ros2 pkg list | grep vla_node                                # vla_node
\`\`\`

## Usage

\`\`\`bash
# Terminal 1: vla_inference_node
ros2 run vla_node vla_inference_node

# Terminal 2: rerun_logger (선택)
ros2 run vla_node rerun_logger

# Terminal 3: instruction 발행
ros2 topic pub --once /vla/instruction std_msgs/String \\
  "data: 'pick up the red can'"

# Terminal 4: image 입력 (bag 재생 또는 카메라)
ros2 bag play sample_bag/ --loop
\`\`\`

## Results (RTX 4070, 4-bit nf4)

| Metric | Value |
|---|---|
| Mean latency | 165 ms |
| p95 latency | 220 ms |
| p99 latency | 250 ms |
| Throughput | 6.0 Hz |
| GPU memory | 5.3 GB (model) + 1.0 GB (inference) |

Raw data: [latency_data.csv](latency_data.csv)

## Troubleshooting

| 증상 | 원인 | 해결 |
|---|---|---|
| CUDA OOM | 다른 GPU process | nvidia-smi 후 kill |
| flash_attn ImportError | flash-attn 미설치 | `attn_implementation="eager"` |
| inference 가 0 action | unnormalize_key 잘못 | parameter 확인 |
| colcon build fail | ROS2 source 누락 | source /opt/ros/humble/setup.bash |
| Image topic 없음 | bag 재생 안 함 | ros2 bag play |

## Limitations
- 6Hz throughput - 30Hz 실시간 제어 불가
- Zero-shot 정확도 낮음 (LoRA fine-tune 권장)
- 4-bit 정확도 손실 ~ 1~2%p

## License
- Code: MIT
- OpenVLA weights: Llama 2 license (commercial use 시 확인)

## Citation
\`\`\`
@misc{phase4_vla_demo,
  author = {Your Name},
  title  = {Phase 4 Vla Demo - OpenVLA + ROS2},
  year   = {2026},
  url    = {https://github.com/<you>/physical-ai-study/tree/main/Portfolio/02_VLA_demo},
}
\`\`\`
```

---

## [note] 실습 2: setup.sh 스크립트

**파일명**: `Portfolio/02_VLA_demo/scripts/setup.sh`

```bash
#!/bin/bash
set -e

echo "=== VLA Demo Quick Setup ==="

# 1. ROS2
if ! command -v ros2 &> /dev/null; then
    echo "[!] ROS2 Humble 이 설치되어 있지 않습니다."
    echo "    https://docs.ros.org/en/humble/Installation.html 참고"
    exit 1
fi

source /opt/ros/humble/setup.bash

# 2. CUDA
if ! command -v nvidia-smi &> /dev/null; then
    echo "[!] CUDA 가 설치되어 있지 않습니다."
    exit 1
fi

# 3. Conda env
if ! conda env list | grep -q '^vla '; then
    conda create -n vla python=3.10 -y
fi
source activate vla

# 4. Python deps
pip install -r requirements.txt

# 5. vla_inference
pip install -e ./vla_inference

# 6. ROS2 build
cd vla_node
colcon build --symlink-install
source install/setup.bash

echo ""
echo "=== Setup 완료 ==="
echo "다음 단계:"
echo "  ros2 launch vla_node demo.launch.py"
```

---

## [note] 실습 3: 트러블슈팅 보강

자주 발생할 추가 항목:

```markdown
| 증상 | 원인 | 해결 |
|---|---|---|
| pip install bitsandbytes fail | 오래된 CUDA | CUDA 11.8+ 확인 |
| HuggingFace download 멈춤 | network 불안정 | `--resume-download` 추가 |
| RAM 사용량 폭증 | swap 부족 | swap 8GB+ 추가 |
| rerun_logger 가 GUI 안 띄움 | X server 없음 | export DISPLAY=:0 |
| 카메라 frame 안 옴 | USB 권한 | sudo chmod 666 /dev/video0 |
```

---

## [note] 실습 4: 본인이 README 따라하기 (검증)

```bash
# 새 Docker 또는 새 conda env 에서
# README 의 Installation section 만 보고 따라하기
# 막히는 step 이 있으면 그 step 의 명령 / 검증을 README 에 추가
```

검증 결과:

```markdown
- [ ] Step 1 (CUDA) 명확하게 검증 가능
- [ ] Step 2 (ROS2) 명확
- [ ] Step 3 (Conda) 명확
- [ ] Step 4 (pip install) 모든 의존성 명시
- [ ] Step 5 (vla_inference pip install -e) 동작
- [ ] Step 6 (colcon build) 동작
- [ ] Step 7 (검증 명령) 모두 expected output 명시
- [ ] Quick Start 가 5분 안에 종료
```

---

## [O] 실습 체크리스트

- [ ] README.md 작성 (10 section)
- [ ] scripts/setup.sh 작성
- [ ] 트러블슈팅 표 10개 이상
- [ ] (선택) Docker 또는 새 conda 검증
- [ ] quiz_easy / quiz_medium

---

## [link] 참고 자료

- [Best practices for README](https://www.makeareadme.com/)
- [Awesome README list](https://github.com/matiassingers/awesome-readme)
- [Shields.io for badges](https://shields.io/)
