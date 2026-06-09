# Studies/Phase 4 — VLA 논문 reading + OpenVLA → ROS2 minimal demo


> **학습 기간**: 2026.06-09 (4개월, 16주)
> **마스터 가이드**: [`Roadmap/Phase 4.md`](../../Roadmap/Phase%204.md)
> **자료 미리 작성됨** — 진입 시 (2026.06) 다시 체크 후 학습 시작


---


## 0. 시작하기 전 — 필수 진입점

Phase 4 의 모든 week 를 시작하기 *전에* [`SETUP.md`](SETUP.md) 를 1회 수행한다.

**왜 별도 단계가 필요한가**: OpenVLA 7B 는 RTX 4070 12GB 로 학습이 불가능하다. 학습은 Colab A100/L4, 추론은 로컬 4070 + 4bit 양자화로 분업하는 것이 본 Phase 의 핵심 의사결정이다. 환경을 두 갈래로 구축해야 하므로 week 1 진입 전에 미리 짚어둔다. 컴퓨트 전략의 근거, 버전 매칭, 가중치 전송 워크플로우, 주차별 환경 요구도가 모두 SETUP.md 에 정리되어 있다.


---


## 디렉토리 구조 (예정)


```
Studies/Phase 4/
  week1/ ~ week3/ # RT-2 정독 + 블로그 1편
  week4/ ~ week7/ # OpenVLA 정독 + 블로그 1편
  week8/ ~ week12/ # OpenVLA HuggingFace → ROS2 minimal demo
  week13/ ~ week16/ # 블로그 마무리 + 산출물 v1 패키징
  blog/ # RT-2 / OpenVLA 블로그 초고
  ros2_pkg/ # OpenVLA → ROS2 토픽 패키지 (v1 demo)
```


## week 자료 형식 (Phase 3 패턴)


각 week 디렉토리:
- `README.md` — 이번 주 목표 + 학습 순서 + 핵심 개념
- `PRACTICE.md` — 실습 단계별 가이드 (논문 reading note 또는 코드 실습)
- `quiz_easy.py` / `quiz_medium.py` — 개념 / 코드 퀴즈
- `quiz_solutions/easy_sol.py` / `medium_sol.py` — 답
- `requirements.txt` — 의존성


## 참고
- 기존 Phase 4 (3D Perception/KITTI/BEV) 자료는 [`Archive/Perception-3D-legacy/Studies/Phase 4/`](../../Archive/Perception-3D-legacy/Studies/Phase%204/) 로 이동됨 (F안 방향 전환).
