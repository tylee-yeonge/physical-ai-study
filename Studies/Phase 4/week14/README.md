# Week 14: ROS2 demo README + 환경 세팅 가이드


> **[v1 범위 조정 — 2026-06]** 이 주차(공개용 Portfolio README + 재현 가이드)는 **v1 에서 제외하고 v2(Phase 6)로 이관**했다. v1 은 외부 공개를 하지 않으므로 면접관용 재현 가이드 수준의 README 는 v2 에서 만든다. v1 에서는 결과 기록 수준의 가벼운 README(성공률 표 + latency 수치)만 순서 5 에서 레포에 남긴다. 아래 공개용 README 표준 구조는 v2 참조용으로 보존한다. 결정 배경: `Roadmap/Phase 4.md` "산출물 v1" 절.


> **이번 주 목표**: `Portfolio/01_VLA_v1/README.md` 를 면접관이 따라할 수 있는 수준으로 다듬는다. 다른 사람도 환경 셋업 + 재현 가능.
> **예상 시간**: 6시간 (README 작성 4h + 검증 2h)
> **핵심 질문**: "내 README 만 따라가서 다른 사람이 30분 안에 inference 동작까지 갈 수 있는가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | README 구조 잡기 | `PRACTICE.md` 1 | section 6-8 개 |
| 2 | 환경 세팅 가이드 | `PRACTICE.md` 2 | venv + apt + pip 모두 |
| 3 | 실행 방법 | `PRACTICE.md` 3 | step-by-step |
| 4 | 트러블슈팅 | `PRACTICE.md` 4 | 자주 발생 + 해결 |
| 5 | 검증 (clean 환경에서) | `PRACTICE.md` 5 | Docker 또는 새 venv |
| 6 | 퀴즈 | quiz_easy / quiz_medium | README 패턴 |


---


## 핵심 개념


### 1. 좋은 README 의 표준 구조


```
1. Title + Badge (build status, license)
2. One-liner (한 줄 설명)
3. Demo 영상 (가장 위, 첫 인상)
4. Quick Start (5분 setup)
5. Architecture (Mermaid 다이어그램)
6. Installation (자세한 환경 세팅)
7. Usage (실행 방법)
8. Results / Performance (실측 데이터)
9. Troubleshooting
10. License / Contribution
```


### 2. 면접관의 README 사용 흐름


```
30초 : Title + One-liner + Demo 영상 -> "흥미로운지" 판단
2분 : Architecture + Results -> 깊이 / 가치 판단
10분 : Installation -> "실제 동작 코드인가" 검증
30분 : Quick Start 따라하기 -> "재현 가능한가"
```


각 단계에서 떨어지면 면접관이 떠남.


### 3. 환경 세팅 가이드의 단계


```
1. Hardware 요구사항: RTX 4070+ (12GB), Ubuntu 22.04
2. CUDA 설치: 11.8 또는 12.x
3. ROS2 설치 (apt)
4. Python venv 생성
5. Python 의존성 설치 (pip install -r requirements.txt)
6. OpenVLA 모델 다운로드 (~ 15GB)
7. 빌드: colcon build --packages-select vla_node
```


각 단계마다 검증 명령 추가:


```bash
# 검증
nvidia-smi # GPU 확인
ros2 --version # ROS2 확인
python -c "import torch; print(torch.cuda.is_available())"
ros2 pkg list | grep vla_node
```


### 4. Quick Start 의 가치


별도 "5 분 안에 실행" section 이 면접관에게 가장 어필:


```bash
git clone https://github.com/<you>/physical-ai-study
cd physical-ai-study/Portfolio/01_VLA_v1
docker-compose up # 또는 ./quickstart.sh
```


도커 / Quickstart 스크립트 권장. 단 본 phase minimal demo 는 venv 만으로 OK.


### 5. 트러블슈팅 section


자주 발생 + 해결 표:


```markdown
| 증상 | 원인 | 해결 |
|---|---|---|
| `CUDA out of memory` | 다른 GPU process | nvidia-smi 확인, kill |
| `flash_attn ImportError` | flash-attn 미설치 | `attn_implementation="eager"` |
| `Image topic 안 나옴` | bag 재생 안 함 | ros2 bag play |
| `inference 가 0 action` | unnormalize_key 잘못 | parameter 확인 |
| `colcon build fail` | source 안 함 | source /opt/ros/${ROS_DISTRO}/setup.bash |
```


### 6. Result 의 표시 방법


수치 + 그래프 + 영상 세 가지:


```markdown
## Results


### Latency (RTX 4070, 4-bit nf4, 100 회 inference)
| Metric | Value |
| mean | 165 ms |
| p95 | 220 ms |
| Throughput | 6 Hz |


### Demo
[![demo](demo.gif)](vla_demo.mp4)


### Detailed data
- [latency_data.csv](latency_data.csv) - 100 회 측정 raw data
- [latency_histogram.png](latency_histogram.png)
```


### 7. License / 한계 명시


```markdown
## License
MIT License (코드)
Llama 2 License (OpenVLA weights - commercial use 시 확인)


## Limitations
- Real-time 30Hz 제어 불가 (6Hz throughput)
- 자작 robot 의 LoRA fine-tune 필요 (zero-shot 정확도 낮음)
- 4-bit quantization 의 정확도 손실 ~ 1~2%p
```


---


## 자체 점검


**Q1. README 의 첫 30초에 무엇을 보여야 하나?**
> Title + One-liner + Demo 영상. 면접관의 첫 인상.


**Q2. 재현성을 강하게 하는 가장 좋은 방법은?**
> 1) Docker 또는 Quickstart 스크립트, 2) 모든 명령에 검증 명령 동반, 3) Clean 환경에서 본인이 따라해 보기.


**Q3. 좋은 트러블슈팅 표의 조건은?**
> 증상 / 원인 / 해결의 3 열, 최소 5 ~ 10 개 항목. 면접관이 흔히 겪을 문제 우선.


**Q4. License section 의 중요한 부분?**
> OpenVLA 의 Llama 2 license 명시. Commercial use 시 별도 확인 필요 가능. 면접관 (기업) 에게 중요한 정보.


**Q5. README 검증의 표준 방법은?**
> Clean 환경 (새 venv 또는 Docker) 에서 본인이 README 만 보고 처음부터 따라하기. 막히면 그 단계의 명령 추가.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. README 구조 잡기 + 작성
2. 트러블슈팅 표 (5-10개)
3. Clean 환경 검증 (선택, Docker 또는 새 venv)
4. quiz_easy / quiz_medium


### 다음 주 (week 15) 준비
- week 12 의 영상을 최종 마감 (자막 / 음악 / 인트로 / 아웃트로)


---


## 이번 주 핵심 요약


1. **README 표준 구조** 10 section.
2. **5 분 Quick Start** 가 면접관의 진입점.
3. **트러블슈팅 표** 자주 발생 + 해결.
4. **Clean 환경 검증** 으로 재현성 확보.
5. **Result + License** 명시.


---


- 이전: [Week 13 - vla-lab 문서 퇴고](../week13/README.md)


다음: [Week 15 - 1분 영상 최종 + 자막](../week15/README.md)
