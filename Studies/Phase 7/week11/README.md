# Week 11: ROS2 패키지 정리 + README


> **이번 주 목표**: Phase 7 의 모든 코드를 면접관이 따라할 수 있는 ROS2 패키지로 정리.
> **예상 시간**: 6시간


## 학습 순서
1. 패키지 구조 정리
2. README 작성
3. Quickstart 스크립트
4. Clean 환경 검증
5. 퀴즈


## 핵심 개념


### 패키지 구조


```
Portfolio/04_R2S2R/
  README.md
  r2s2r_demo.mp4
  demo.gif
  thumbnail.png
  src/
    vla_node/ # Phase 4 + 7 확장
    safety_node/ # C++ 안전 인터록
    digital_twin_node/ # Sim 통합
    rerun_logger/
  data/
    teleop_demos.h5 # OpenX format
    latency_data.csv
    sim_real_gap_report.md
  scripts/
    setup.sh
    train_lora.sh
    run_demo.sh
  notebooks/
    01_data_analysis.ipynb
    02_latency_analysis.ipynb
```


### README 구조 (Phase 4 week 14 패턴)


10 section:
1. Title + One-liner
2. Demo (영상 / GIF)
3. Quick Start
4. Architecture
5. Installation
6. Usage
7. Results (정량 표)
8. Troubleshooting
9. Limitations
10. License + Citation


### Quickstart


```bash
git clone ...
cd Portfolio/04_R2S2R
./scripts/setup.sh
./scripts/run_demo.sh
```


## 자체 점검
Q1. 패키지 구조? > Phase 4 패턴 + 자작 팔 통합.
Q2. README section 수? > 10.
Q3. Quickstart 의 가치? > 면접관의 진입점.
Q4. Clean 환경 검증? > Docker 또는 새 conda.
Q5. 가장 중요한 파일? > README + 영상.


## 요약
1. 패키지 구조 정리
2. README 10 section
3. Quickstart 스크립트
4. Clean 환경 검증
5. 다음: 공개


- [Week 10](../week10/README.md) | [Week 12](../week12/README.md)
