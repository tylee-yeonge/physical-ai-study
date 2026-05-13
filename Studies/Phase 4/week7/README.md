# Week 7: OpenVLA 블로그 1편 작성


> **이번 주 목표**: week 4-6 의 reading note + 한 페이지 요약 + 실측 latency 데이터를 합쳐서 OpenVLA 블로그 1편 (산출물 #2 의 2/3) 마감.
> **예상 시간**: 10시간 (outline 1h + 본문 6h + 다이어그램 1h + 퇴고 2h)
> **핵심 질문**: "OpenVLA 블로그가 RT-2 블로그 (week 3) 와 어떻게 차별화되는가? '내가 직접 돌려본 OpenVLA' 만의 가치가 있는가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | week 3 블로그 패턴 복습 | week3/README.md | 8-section 표준 구조 |
| 2 | OpenVLA outline 작성 | `PRACTICE.md` 1 | RT-2 와의 차별점 강조 |
| 3 | 다이어그램 작성 | `PRACTICE.md` 2 | hybrid vision encoder 다이어그램 |
| 4 | latency 데이터 인용 | week 6 의 `.npy` | "내가 직접 측정" 의 증거 |
| 5 | 본문 작성 | 블로그 플랫폼 | 약 3500-4500 자 한국어 |
| 6 | 퀴즈 | quiz_easy / quiz_medium | OpenVLA 블로그의 차별점 |
| 7 | 퇴고 + 발행 | Velog + 본 레포 사본 | 산출물 #2 의 2/3 완성 |


---


## 시작하기 전에 — RT-2 블로그와의 차별점


OpenVLA 블로그는 week 3 의 RT-2 블로그와 동일한 8-section 구조지만, 핵심 차별점:


| RT-2 블로그 | OpenVLA 블로그 |
|---|---|
| 논문만 정독 | 논문 + **직접 inference 실행** |
| Latency 는 추정 | **본인 RTX 4070 에서 직접 측정한 수치** |
| closed-source 의 한계 | open-source 의 가능성 |
| 일반론 | "내 자작 팔 (Phase 7) 에 어떻게 적용할지" 의 구체 계획 |


> OpenVLA 블로그의 가장 큰 강점: "내가 직접 돌려본 데이터" 가 면접관에게 더 신뢰감을 준다.


---


## OpenVLA 블로그의 8-section 권장 내용


### Section 1: 한 줄 요약 (예시)
> "open-source 7B VLA + DINOv2/SigLIP hybrid vision + OpenX-Embodiment 970K 의 결합. RT-2 의 학술적 의미를 실제 RTX 4070 에서 검증 가능한 형태로 가져왔다."


### Section 2: 배경
- RT-2 의 한계 5가지 복습 (특히 closed-source / 큰 모델)
- OpenVLA 의 등장 배경 (Stanford, 2024)
- 본 로드맵에서 OpenVLA 를 선택한 이유


### Section 3: 한 페이지 요약
- Architecture diagram (hybrid vision encoder 강조)
- 4 가지 핵심 결정 (week 4 README 참고)
- "내가 RTX 4070 4-bit 에서 inference 했다" 의 증명 (latency 통계 표)


### Section 4: 자세한 동작 (1000-1500 자)
- 4-1. Hybrid vision encoder 의 의도
- 4-2. Llama 2 7B 의 의의
- 4-3. OpenX-Embodiment 의 22 embodiments
- 4-4. LoRA fine-tuning 흐름
- 4-5. 4-bit quantization 의 trade-off (~ 1-2%p 정확도 손실, 메모리 1/8)


### Section 5: 결과 + 실측 데이터
- OpenVLA 논문의 평가 결과 요약
- **본인이 측정한 latency 통계** (week 6 의 `.npy` 데이터)
- p95 / p99 / throughput 표


예시 표:


```
| Metric | RTX 4070, 4-bit nf4 |
|---|---|
| mean latency | 165 ms |
| p95 latency | 220 ms |
| Throughput | 6.0 Hz |
| GPU memory | 5.3 GB (model) + 1.0 GB (inference) |
```


### Section 6: 한계 5가지
- 데이터 분포 의존 (OpenX 외 환경)
- VRAM (4-bit 필수)
- Latency (실시간 30Hz 불가)
- Single-arm
- No teleop policy


### Section 7: 양산 SW 엔지니어의 관점 + 본 로드맵 연결
- 4-bit quantization 의 trade-off 를 양산 시점에 어떻게 평가할 것인가
- 5Hz 가 가능한 robot 작업 vs 불가능한 작업
- 본 로드맵의 자작 팔 + LoRA fine-tune 계획 미리 소개


### Section 8: 다음
- Phase 4 의 다음 단계 (week 8-12 의 ROS2 demo)
- Phase 6-7 의 자작 팔 통합
- π0 / Helix 등 후속 모델


---


## RT-2 블로그와 한 줄로 차별화


**RT-2 블로그의 메시지**: "VLA 분야의 시작점인 RT-2 의 본질과 한계."


**OpenVLA 블로그의 메시지**: "그 한계를 실측으로 검증할 수 있는 open-source 모델. 내 RTX 4070 에서도 inference 가능. 실시간 30Hz 는 여전히 불가능."


두 블로그가 함께 면접관에게 던지는 메시지:
> "이 사람은 VLA 의 학술적 흐름 (RT-2 -> OpenVLA) 을 알고, 그 한계를 본인 hardware 에서 직접 측정해서 양산 관점으로 해석할 수 있다."


---


## 자체 점검


**Q1. RT-2 블로그와 OpenVLA 블로그의 가장 큰 차별점은?**
> OpenVLA 블로그는 본인이 직접 돌려본 실측 latency 데이터를 포함. "직접 검증" 의 신뢰감이 가장 큰 차이.


**Q2. OpenVLA 블로그 Section 5 의 핵심 표는?**
> latency 통계 표: mean / p95 / p99 / throughput / GPU memory. 양산 SW 엔지니어의 차별화 메시지.


**Q3. 두 블로그 (RT-2 + OpenVLA) 를 함께 면접관에게 던지는 메시지는?**
> "VLA 의 학술적 흐름 (RT-2 -> OpenVLA) 의 이해 + 본인 hardware 에서 직접 검증 + 양산 관점 해석."


**Q4. 4-bit quantization 의 trade-off 를 블로그에서 어떻게 다루는가?**
> "정확도 1-2%p 손실 vs 메모리 1/8 / 속도 1.5-2x" 같은 정량 trade-off. 양산 시점에는 어느 쪽이 우선인지 본인 의견.


**Q5. 양산 SW 엔지니어의 차별화 메시지는 Section 7 에 어떻게 적나?**
> "OpenVLA 5Hz 는 cm 단위 pick-and-place 에는 충분하지만, 자동차 부품 조립 (mm 단위 + 60Hz 제어) 에는 부적합. 양산 vehicle 에 통합하려면 hierarchical (slow VLA + fast safety) 구조가 필수." 같은 인과 관계.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. week 4-6 의 reading note + 한 페이지 노트 + latency 데이터 통합
2. OpenVLA 블로그 outline (8-section)
3. 다이어그램 2 개 작성 (Mermaid)
4. 본문 작성 (3500-4500 자 한국어)
5. self-review + 퇴고
6. Velog 발행 + 본 레포 사본
7. quiz_easy / quiz_medium 풀기


### 다음 주 (week 8) 준비
- ROS2 환경 점검 (Humble 또는 Iron)
- OpenVLA inference 스크립트 stabilize (week 6 의 코드)
- (선택) Rerun.io 설치 (week 12 시각화용)


---


## 이번 주 핵심 요약


1. **RT-2 블로그와 차별화**: 직접 측정한 latency 데이터.
2. **8-section 구조 동일**: week 3 의 패턴 그대로.
3. **분량 3500-4500 자**: RT-2 블로그보다 약간 더 (실측 데이터 표 추가).
4. **양산 SW 엔지니어 메시지**: 5Hz 의 양산 의미.
5. **산출물 #2 의 2/3 완료**: RT-2 블로그 (1/3) + OpenVLA 블로그 (2/3) + ROS2 demo (3/3).


---


- 이전: [Week 6 - HuggingFace + Inference](../week6/README.md)


다음: [Week 8 - HuggingFace inference 셋업 (안정화)](../week8/README.md)
