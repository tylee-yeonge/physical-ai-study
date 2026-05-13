# Week 2: HuggingFace ViT inference + ImageNet 분류 + latency 측정


> **이번 주 목표**: HuggingFace transformers 의 ViT 모델을 다양한 입력으로 inference 해보고, latency / VRAM / 정확도를 측정한다.
> **예상 시간**: 8시간 (모델 inference 4h + benchmark 2h + 노트 2h)
> **핵심 질문**: "ViT-B / ViT-L 의 latency 와 VRAM 이 OpenVLA 의 latency 에 미치는 영향은?"


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 | `requirements.txt` | week1 그대로 |
| 2 | HF ViT 모델 zoo 정독 | `README.md` 2장 | 모델 종류 |
| 3 | ViT-B inference | `PRACTICE.md` 1 | ImageNet 분류 |
| 4 | ViT-L benchmark | `PRACTICE.md` 2 | latency / VRAM |
| 5 | 다양한 image 입력 | `PRACTICE.md` 3 | natural / synthetic / adversarial |
| 6 | 퀴즈 | quiz_easy / quiz_medium | inference / benchmark |


---


## 핵심 개념


### 1. HuggingFace transformers 의 ViT 모델 카탈로그


```
google/vit-base-patch16-224 # 표준
google/vit-base-patch16-224-in21k # ImageNet-21k pretrain
google/vit-large-patch16-224 # 큰 모델
google/vit-base-patch32-384 # 큰 patch + 큰 image
facebook/dinov2-base # DINOv2 (self-supervised, Phase 5 week 7)
facebook/dinov2-large # OpenVLA backbone
google/siglip-base-patch16-224 # SigLIP (Phase 5 week 10)
```


각 모델의 차이:
- `patch16`: 16x16 patch (표준)
- `patch32`: 32x32 patch (더 큰 patch, sequence 짧음, 빠르지만 정확도 낮음)
- `-in21k`: ImageNet-21k (21000 classes) 로 학습
- `-224 / -384`: input image 크기


### 2. ViT 모델별 latency / VRAM (예상)


RTX 4070 fp16 기준:


| 모델 | Params | latency | VRAM |
|---|---|---|---|
| ViT-B/16 | 86M | ~ 10 ms | 0.5 GB |
| ViT-L/16 | 300M | ~ 30 ms | 1.2 GB |
| ViT-H/14 | 632M | ~ 60 ms | 2.5 GB |
| ViT-22B | 22B | OOM (4070) | - |


OpenVLA 의 DINOv2 + SigLIP (각 ViT-L) 의 latency ~ 60ms 가 본 phase 의 핵심 측정 대상. Phase 4 week 6 의 OpenVLA 전체 latency 165ms 중 vision encoder 가 약 60ms 차지.


### 3. ImageNet-1k vs ImageNet-21k


| 항목 | ImageNet-1k | ImageNet-21k |
|---|---|---|
| Classes | 1000 | 21841 |
| Images | 1.28M | 14.2M |
| 용도 | downstream / inference | pretrain |
| ViT 학습 | fine-tune 표준 | 사전학습 표준 |


### 4. inference 시 주의점 (Phase 4 의 경험)


```python
# 1. 모델 eval mode
model.eval()


# 2. no_grad
with torch.no_grad():
    out = model(...)


# 3. CUDA synchronize (latency 측정 시)
torch.cuda.synchronize()


# 4. warm-up (첫 5회 측정 제외)
# 5. batch_size 1 (실 deployment 와 같이)
```


### 5. 다양한 image 입력의 영향


| Image 종류 | ViT 의 행동 |
|---|---|
| natural (photo) | 정상 분류 (학습 분포) |
| synthetic (graphic) | 분류 가능, 단 confidence 낮음 |
| 흑백 이미지 | 색 정보 없음, 형태 만으로 |
| adversarial | 의도적 misclassification 발생 |
| 회전 / scale | translation invariance 없음, 성능 저하 |


### 6. ViT 의 robustness 한계


- Out-of-distribution (학습 분포 밖) 에 약함
- Adversarial example 에 취약
- Image augmentation 없이는 회전 / scale 에 약함


본 phase 권장: natural image + adversarial 한 번씩 테스트.


### 7. patch_size 의 영향


```
patch_size 16: seq_len = 197 (표준)
patch_size 32: seq_len = 50 (짧음, 빠름, 정확도 낮음)
patch_size 8: seq_len = 785 (길음, 느림, 정확도 약간 향상)
patch_size 14: seq_len = 257 (DINOv2 사용)
```


OpenVLA 의 patch_size 14 선택 이유: spatial 정보 보존 + computation 적정 균형.


### 8. 본 주의 산출물


```
practice_vit_inference.py 의 결과:
- ViT-B/16 ImageNet 분류 결과
- ViT-L/16 ImageNet 분류 결과
- 두 모델의 latency 비교 표
- 두 모델의 VRAM 비교
```


---


## 자체 점검


**Q1. ViT-B 와 ViT-L 의 inference latency 차이는?**
> RTX 4070 fp16 기준 ViT-B ~ 10ms, ViT-L ~ 30ms. 약 3배 차이.


**Q2. patch_size 의 trade-off 는?**
> 작은 patch (8): sequence 길음, 메모리 많음, 정확도 약간 향상. 큰 patch (32): sequence 짧음, 빠름, 정확도 낮음. 표준 16 이 균형.


**Q3. OpenVLA 가 patch_size 14 를 선택한 이유?**
> spatial 정보 보존 + computation 적정 균형. DINOv2 의 표준이 patch 14.


**Q4. ViT 의 robustness 한계 3 가지?**
> 1) Out-of-distribution 에 약함, 2) Adversarial 에 취약, 3) 회전 / scale invariance 없음 (augmentation 필요).


**Q5. ViT-L 의 VRAM 약 1.2 GB. OpenVLA 7B 의 5 GB 와의 차이?**
> ViT-L = 300M params, OpenVLA = 7B params. fp16 기준 약 2배 크기. OpenVLA 의 vision encoder 만 따지면 ViT-L * 2 = ~ 0.6 GB (~ 12% of OpenVLA total).


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. ViT-B / ViT-L inference + 결과 확인
2. latency / VRAM benchmark
3. 다양한 image 입력 테스트 (natural / synthetic / 회전)
4. one-pager 보강
5. quiz_easy / quiz_medium


### 다음 주 (week 3) 준비
- OpenVLA backbone 의 ViT-L 가 그대로 vision encoder 임을 확인
- Phase 4 의 latency 데이터 (week 6 의 .npy) 다시 분석


---


## 이번 주 핵심 요약


1. **HuggingFace ViT 모델 zoo**: ViT-B, ViT-L 의 사용법.
2. **ViT-L latency ~ 30 ms** (RTX 4070, fp16). OpenVLA vision encoder 의 핵심.
3. **patch_size 16 표준**, OpenVLA 는 14 (DINOv2).
4. **Robustness 한계**: OOD / adversarial / scale invariance.
5. **OpenVLA 전체 latency 165ms 중 vision ~ 60ms**.


---


- 이전: [Week 1 - ViT 정독](../week1/README.md)


다음: [Week 3 - OpenVLA backbone 과의 관계](../week3/README.md)
