# Week 4: OpenVLA 1회독 + Architecture Diagram 정독


> **이번 주 목표**: OpenVLA 논문을 1회독하고, Llama 7B + DINOv2 + SigLIP 라는 hybrid vision encoder 구조를 한 페이지로 설명할 수 있는 수준에 도달한다.
> **예상 시간**: 10-12시간
> **핵심 질문**: "OpenVLA 가 RT-2 의 open-source 재현이라면, 그저 작은 모델을 쓴 것 외에 어떤 본질적 개선이 있는가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | 환경 (week2 그대로) | `requirements.txt` | 추가 의존성 없음 |
| 2 | 사전 지식 점검 | `README.md` 2장 | DINOv2 / SigLIP / Llama 의 큰 그림 |
| 3 | 논문 1회독 (Sec 1-3) | OpenVLA PDF | Introduction + Background + Approach |
| 4 | 논문 1회독 (Sec 4-6) | OpenVLA PDF | Experiments + Open-source release + Discussion |
| 5 | Architecture 분해 | `PRACTICE.md` 1-2 | hybrid vision encoder 의 의도 분석 |
| 6 | 퀴즈 | quiz_easy / quiz_medium | OpenVLA 의 핵심 결정 / 수치 |
| 7 | 노트 정리 | `PRACTICE.md` 3 | "한 페이지 OpenVLA" 노트 |


---


## 시작하기 전에 — OpenVLA 가 본 로드맵에 가지는 의미


OpenVLA 는 본 로드맵 **산출물 #2 (2026.12), #4 결정타 (2027.07) 모두에서 직접 사용** 하는 모델이다:


| 산출물 | OpenVLA 의 역할 |
|---|---|
| #2 (2026.12) | OpenVLA HuggingFace inference → ROS2 토픽 minimal demo (week 8-12) |
| #4 (2027.07) | OpenVLA fork + ROS2 노드 + 자작 6DOF 팔 통합 (Real-to-Sim-to-Real) |


즉 이번 주의 정독 깊이 = 산출물 #4 의 깊이. RT-2 (week 1-3) 보다 더 정밀하게 읽어야 한다.


> 2026.11 분기 재평가에서 OpenVLA 가 한 세대 뒤 (π0 / Helix / GR00T 등 등장) 라면 모델 갱신 가능. 단, 본 마스터는 OpenVLA 표준 가정.


---


## 핵심 개념 자세히 알아보기


### 1. OpenVLA 의 한 줄 요약


> *"7B Llama LM + 두 종류 vision encoder (DINOv2 + SigLIP) 의 결합을 OpenX-Embodiment 970K episode 로 fine-tune 한 open-source VLA."*


이 한 줄 안에 OpenVLA 의 4 가지 핵심 결정이 있다:


1. **Backbone LM**: Llama 2 7B (RT-2 의 PaLI-X 5B/55B 대비 더 작고 open)
2. **Vision encoder**: DINOv2 + SigLIP 의 hybrid (RT-2 의 ViT 단일 대비)
3. **데이터**: OpenX-Embodiment 970K episodes (RT-1 의 130K 대비 7배)
4. **개방성**: weight / training code / data 모두 공개


### 2. 사전 지식: DINOv2 (Meta, 2023)


DINOv2 는 **self-supervised** 방식의 vision foundation model. CLIP / SigLIP 과 달리 텍스트 없이 학습.


```
입력 : RGB image
학습 : self-distillation (student-teacher) + masked image modeling
출력 : patch-level features (geometric / spatial 정보 강함)
```


특징:
- **spatial / geometric 정보가 강함** (자세, 공간 관계, 깊이 등)
- semantic 정보 (label, 의미) 는 상대적으로 약함
- ViT-L / ViT-G (1B parameter) 가 가장 일반적
- robotics 에 자주 사용되는 vision encoder


### 3. 사전 지식: SigLIP (Google, 2023)


SigLIP 은 CLIP 의 개선판. **sigmoid loss** 를 사용한 image-text contrastive 학습.


```
입력 : RGB image + text caption
학습 : image-text matching (sigmoid pairwise loss, softmax 안 씀)
출력 : image embedding (semantic 정보 강함)
```


특징:
- **semantic 정보가 강함** (object identity, category, attribute)
- spatial 정보는 상대적으로 약함
- CLIP 보다 더 효율적 (batch 의존성 약함)


### 4. OpenVLA 의 hybrid vision encoder 의 의도


```
                    +---------------+
        +-------> | DINOv2 (ViT-L) | -> patch token (spatial)
        | +---------------+ \
RGB --+ concat -> 결합 patch token
        | +---------------+ /
        +-------> | SigLIP (ViT-L) | -> patch token (semantic)
                    +---------------+
```


왜 두 개를 합치는가:
- **DINOv2**: "객체가 **어디** 있는가" (위치 / 자세 / 깊이)
- **SigLIP**: "객체가 **무엇** 인가" (정체성 / 의미 / category)
- Robot manipulation 에는 **둘 다 필요**.


논문에 따르면 DINOv2 only 또는 SigLIP only 모델 대비 hybrid 가 일관되게 우수.


### 5. OpenVLA Architecture Diagram (논문 Figure 1)


```
        +-----------+
        | RGB image |
        +-----------+
            / \
           v v
   +-----------+ +-----------+
   | DINOv2 | | SigLIP |
   | (ViT-L/14)| | (ViT-L/14)|
   +-----------+ +-----------+
        | |
        v v
   +-----------+ +-----------+
   | patch tok | | patch tok |
   +-----------+ +-----------+
            \ /
             v v
        +-----------+
        | concat | ~ 512 tokens
        +-----------+
              |
              v
        +-----------+ +------------------+
        | projector | | text instruction |
        | (MLP) | | "pick up the can"|
        +-----------+ +------------------+
              | |
              +---------+-----------+
                        v
              +-------------------+
              | Llama 2 7B |
              | (LM decoder) |
              +-------------------+
                        |
                        v
              +-------------------+
              | action tokens | (7 dim, same vocab trick as RT-2)
              +-------------------+
                        |
                        v (de-tokenize)
              +-------------------+
              | 7-DoF action |
              +-------------------+
```


### 6. OpenX-Embodiment 데이터 (week 5 의 핵심)


OpenX-Embodiment 는 Google + DeepMind + 21 개 institution 협력의 **로봇 trajectory aggregation 데이터셋**:


- 약 1M trajectory (970K episodes)
- 22 개 robot embodiments
- 60 개 dataset (RT-1, Bridge, Berkeley Cable, etc.)


이게 RT-1 데이터셋 (130K, 단일 embodiment) 대비 가장 큰 차이.


| 항목 | RT-1 (RT-2 사용) | OpenX-Embodiment (OpenVLA 사용) |
|---|---|---|
| 총 episode | 130K | 970K (7배) |
| Embodiment 종류 | 1 (Google robot) | 22 |
| 공개 여부 | 부분 공개 | 완전 공개 |
| 작업 다양성 | 700+ | 수만+ |


> week 5 에서 OpenX-Embodiment 의 자세한 구조와 fine-tuning 흐름을 다룬다.


### 7. RT-2 와 OpenVLA 의 구체 비교 표


| 항목 | RT-2 | OpenVLA |
|---|---|---|
| LM backbone | PaLI-X 5B / 55B | Llama 2 7B |
| Vision encoder | ViT (PaLI-X 의 일부) | DINOv2 + SigLIP hybrid |
| 학습 데이터 (robot) | RT-1 dataset (130K) | OpenX-Embodiment (970K) |
| 학습 방식 | Co-fine-tune (web + robot) | Robot-only fine-tune (LM 은 pre-trained 그대로) |
| 출처 | Google DeepMind (closed) | Stanford + collaborators (open) |
| Weight 공개 | X | O (HuggingFace) |
| Training code | X | O (GitHub) |
| VRAM (4-bit) | ~ 14 GB+ (55B) | ~ 8 GB (7B) |
| Inference latency | ~ 200ms | ~ 100-150ms |


### 8. OpenVLA 의 핵심 contribution 4 가지


논문 contribution section 에서:


1. **Open-source large-scale VLA** (7B parameter)
2. **Hybrid vision encoder** 의 효과 검증
3. **OpenX-Embodiment 활용**: 22 embodiments 의 generalization
4. **LoRA fine-tuning 지원**: 새 robot 에 빠르게 적응


LoRA fine-tuning 은 본 로드맵 Phase 7 의 산출물 #4 에서 자작 6DOF 팔에 OpenVLA 를 적응시킬 때 핵심 기법.


### 9. 한계 (OpenVLA 의 정직한 결점)


블로그 (week 7) 에서 인용할 5 가지:


1. **데이터 분포 의존**: OpenX-Embodiment 에 없는 robot / 환경 / 작업은 zero-shot 실패율 큼
2. **VRAM**: 7B fp16 = ~14GB. RTX 4070 12GB 에서는 4-bit quantization 필수
3. **Latency**: ~100-150ms (5-7Hz). 여전히 실시간 30Hz 불가
4. **Single-arm**: bimanual / mobile manipulation 약함
5. **No teleoperation policy**: behavioral cloning only, RL 또는 teleop 보강 따로 필요


---


## 한 페이지 OpenVLA 요약


### 핵심 4 가지 결정


```
1. Backbone LM : Llama 2 7B (open)
2. Vision encoder : DINOv2 + SigLIP hybrid (spatial + semantic)
3. 데이터 : OpenX-Embodiment 970K episodes
4. Fine-tuning : LoRA 지원 (새 robot 빠르게 적응)
```


### 입출력 인터페이스


```
입력 : RGB (224x224 / 384x384) + text instruction
출력 : 7-DoF action [dx, dy, dz, rx, ry, rz, gripper]
주기 : ~ 5~7Hz (RTX 4070 4-bit 기준)
```


### RT-2 와의 차이 한 문장


> "RT-2 의 closed-source 55B VLM 을 open-source 7B + hybrid vision encoder 로 축소하면서 OpenX-Embodiment 의 22 embodiments 로 generalization 까지 챙긴 모델."


---


## 자체 점검


**Q1. OpenVLA 가 RT-2 와 다른 본질적 차이 3 가지는?**
> (1) open-source (weight + code + data 공개), (2) hybrid vision encoder (DINOv2 + SigLIP), (3) OpenX-Embodiment 데이터 (22 embodiments, generalization 강화). 모델 크기 (7B vs 55B) 는 부수적.


**Q2. DINOv2 와 SigLIP 의 차이를 한 줄로?**
> DINOv2 는 self-supervised 로 학습된 vision encoder 로 spatial / geometric 정보가 강하다. SigLIP 은 image-text contrastive 로 학습되어 semantic 정보가 강하다.


**Q3. OpenVLA 가 두 encoder 를 합치는 이유는?**
> Robot manipulation 에는 "물체가 어디 있는가" (DINOv2 의 spatial) 와 "물체가 무엇인가" (SigLIP 의 semantic) 가 둘 다 필요. 두 encoder 의 강점이 보완적이라 hybrid 가 일관되게 single encoder 보다 우수.


**Q4. OpenX-Embodiment 의 970K 가 가지는 의미는?**
> RT-1 dataset 의 130K 의 약 7배. 더 중요한 건 22 embodiments 다양성으로 새 robot 으로의 generalization (LoRA fine-tuning) 가능성 확대. Phase 7 의 산출물 #4 에서 자작 6DOF 팔에 LoRA 로 적응 시 핵심 근거.


**Q5. RTX 4070 12GB 에서 OpenVLA 를 inference 하려면?**
> 7B model fp16 은 ~ 14GB → 12GB 메모리 부족. 4-bit quantization (bitsandbytes / GPTQ) 필요. 4-bit 시 약 4-5GB 로 inference 가능. 단 정확도 약간 떨어질 수 있음.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. OpenVLA 논문 1회독 (paper: https://arxiv.org/abs/2406.09246)
2. RT-2 와의 비교 표 본문 7장의 항목 모두 채우기
3. `practice_dinov2_siglip.py` - 두 encoder 의 출력 직접 시각화
4. `practice_openvla_arch.py` - architecture diagram 손으로 다시 그리기
5. "한 페이지 OpenVLA" 노트 산출
6. quiz_easy / quiz_medium 풀기


### 다음 주 (week 5) 준비
- OpenX-Embodiment 데이터셋 페이지 한 번 훑기: https://robotics-transformer-x.github.io/
- HuggingFace Datasets 라이브러리 한 번 설치 (week 5 에서 사용): `pip install datasets`
- (선택) DINOv2 의 self-supervised 학습 5분 영상


---


## 이번 주 핵심 요약


1. **OpenVLA 는 RT-2 의 open-source 재현 + 개선**: 모델 크기 + 데이터 + 개방성 모두 변경.
2. **Hybrid vision encoder**: DINOv2 (spatial) + SigLIP (semantic).
3. **OpenX-Embodiment 970K episodes**: 22 embodiments, generalization 강화.
4. **LoRA 지원**: 새 robot 빠르게 적응 (산출물 #4 의 핵심 기법).
5. **양산 SW 엔지니어 관점**: 4-bit quantization 으로 RTX 4070 에 inference 가능 → Phase 7 직접 사용 가능.


---


- 이전: [Week 3 - RT-2 블로그 1편 작성](../week3/README.md)


다음: [Week 5 - OpenX-Embodiment 데이터 + Fine-tuning 흐름](../week5/README.md)
