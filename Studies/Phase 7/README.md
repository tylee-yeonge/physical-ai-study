# Studies/Phase 7 — Real-to-Sim-to-Real (결정타 산출물 #4)


> **학습 기간**: 2027.05~07 (3개월, 12주)
> **마스터 가이드**: [`Roadmap/Phase 7.md`](../../Roadmap/Phase%207.md)
> **자료 미리 작성됨** — 진입 시 (2027.05) 다시 체크 후 학습 시작


---


## 디렉토리 구조 (예정)


```
Studies/Phase 7/
  week1/ ~ week3/ # OpenVLA fork + ROS2 통합
  week4/ ~ week6/ # 안전 인터록 (C++)
  week7/ ~ week9/ # latency 측정 + Sim/Real gap 영상
  week10/ ~ week12/ # 통합 영상 + 패키징
  vla_pkg/ # OpenVLA fork + ROS2 노드
  safety_pkg/ # 안전 인터록 (C++)
  latency_logs/ # latency 측정 raw data
```


## week 자료 형식 (기존 Phase 3 패턴)


- `README.md` / `PRACTICE.md` / `quiz_easy.py` / `quiz_medium.py` / `quiz_solutions/` / `requirements.txt`


## 핵심 의존성


| 의존 | 출처 |
|---|---|
| OpenVLA → ROS2 minimal demo | Phase 4 산출물 #2 |
| Isaac Sim 디지털 트윈 + Sim/Real gap 측정 인프라 | Phase 6 |
| 자작 6DOF 팔 + 안전 인터록 기초 | [`Studies/Hardware-Arm/`](../Hardware-Arm/) Stage 2 |


> 본 Phase 가 *Real-to-Sim-to-Real* 산출물 #4 (결정타) 의 마지막 통합 단계.
