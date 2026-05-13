# Week 4 실습: OpenVLA 논문 정독 + Hybrid Vision Encoder 분석


> **실습 목표**: OpenVLA 의 hybrid vision encoder 의 효과를 코드로 확인하고, RT-2 와의 비교 표를 완성한다.
> **예상 시간**: 6-8시간


---


## 환경 설정


```bash
conda activate phase4
pip install -r requirements.txt
# 추가: transformers (이미 설치됨), Pillow
```


```bash
# OpenVLA 논문 다운로드
wget -O ~/phase4_notes/papers/openvla.pdf "https://arxiv.org/pdf/2406.09246.pdf"
```


---


## 실습 1: OpenVLA 논문 reading note 템플릿


**파일명**: `~/phase4_notes/week4/openvla_reading_note.md`


RT-2 reading note 템플릿 (week 1) 와 동일 형식. 단 다음 항목 추가:


```markdown
# OpenVLA Reading Note


## 0. Meta
- 제목: OpenVLA: An Open-Source Vision-Language-Action Model
- 저자: Stanford / Princeton / UC Berkeley (2024)
- 1회독 소요: __h __m


## 1. One-liner
## 2. 핵심 contribution 4가지
## 3. Architecture diagram (손그림)
## 4. Hybrid vision encoder
   - DINOv2 의 역할: _____
   - SigLIP 의 역할: _____
   - 두 encoder 의 fusion 방식: _____
## 5. OpenX-Embodiment 데이터
   - 총 episode 수: ___
   - Embodiment 수: ___
## 6. Fine-tuning / LoRA
   - LoRA rank, alpha 설정: _____
   - 새 robot 적응 시간: ___ GPU hr
## 7. RT-2 와의 비교 (본 README 7장 표)
## 8. Limitations 5가지
## 9. 본 로드맵 관점
   - Phase 7 산출물 #4 에서 어떻게 쓸 것인가: _____
## 10. 다음 (week 5) 시작 질문
```


---


## 실습 2: DINOv2 vs SigLIP feature 시각화


**파일명**: `practice_dinov2_siglip.py`


```python
"""
실습 2: DINOv2 와 SigLIP 의 patch feature 를 시각화하여 차이를 본다.
"""
import torch
import numpy as np
from PIL import Image
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


from transformers import AutoModel, AutoImageProcessor


print("=" * 60)
print("실습 2: DINOv2 vs SigLIP feature 비교")
print("=" * 60)


device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')


# -- 2-1. DINOv2 로드 --
print("\n[2-1] DINOv2 로드")
dino_proc = AutoImageProcessor.from_pretrained("facebook/dinov2-base")
dino_model = AutoModel.from_pretrained("facebook/dinov2-base").to(device).eval()
print(f"DINOv2 hidden size: {dino_model.config.hidden_size}")


# -- 2-2. SigLIP 로드 --
print("\n[2-2] SigLIP 로드")
sig_proc = AutoImageProcessor.from_pretrained("google/siglip-base-patch16-224")
sig_model = AutoModel.from_pretrained("google/siglip-base-patch16-224").to(device).eval()
print(f"SigLIP hidden size: {sig_model.config.vision_config.hidden_size}")


# -- 2-3. 테스트 이미지 (랜덤 ImageNet 1장 또는 본인 이미지) --
img = Image.open("/path/to/your/test_image.jpg").convert("RGB") if False else Image.fromarray(
    (np.random.rand(224, 224, 3) * 255).astype(np.uint8)
)
print(f"\n[2-3] Test image size: {img.size}")


# -- 2-4. DINOv2 forward --
with torch.no_grad():
    dino_inputs = dino_proc(images=img, return_tensors="pt").to(device)
    dino_out = dino_model(**dino_inputs)
    dino_feat = dino_out.last_hidden_state # [1, n_patches+1, hidden]
print(f"\n[2-4] DINOv2 출력 shape: {dino_feat.shape}")


# -- 2-5. SigLIP vision encoder forward --
with torch.no_grad():
    sig_inputs = sig_proc(images=img, return_tensors="pt").to(device)
    sig_out = sig_model.vision_model(**sig_inputs)
    sig_feat = sig_out.last_hidden_state # [1, n_patches, hidden]
print(f"[2-5] SigLIP 출력 shape: {sig_feat.shape}")


# -- 2-6. patch feature 의 PCA 시각화 --
def pca_2d(feat):
    f = feat.squeeze(0).cpu().numpy() # [N, D]
    # CLS / pooling token 제거 (DINOv2 의 경우 첫 token)
    if f.shape[0] % 2 == 1:
        f = f[1:]
    n = int(np.sqrt(f.shape[0]))
    f = f[:n*n]
    # 가장 분산 큰 3 방향 (RGB 채널처럼 사용)
    mean = f.mean(axis=0, keepdims=True)
    centered = f - mean
    U, S, Vt = np.linalg.svd(centered, full_matrices=False)
    proj = centered @ Vt[:3].T # [N, 3]
    proj = (proj - proj.min(axis=0)) / (proj.max(axis=0) - proj.min(axis=0) + 1e-9)
    return proj.reshape(n, n, 3)


dino_pca = pca_2d(dino_feat)
sig_pca = pca_2d(sig_feat)


fig, ax = plt.subplots(1, 3, figsize=(12, 4))
ax[0].imshow(img); ax[0].set_title("RGB"); ax[0].axis('off')
ax[1].imshow(dino_pca); ax[1].set_title("DINOv2 PCA"); ax[1].axis('off')
ax[2].imshow(sig_pca); ax[2].set_title("SigLIP PCA"); ax[2].axis('off')
plt.tight_layout()
plt.savefig("dino_vs_siglip.png", dpi=100)
print("\n[2-6] 시각화 저장: dino_vs_siglip.png")


print("\n 실습 2 완료!")
print("-> 직접 비교: DINOv2 는 공간 경계가 선명, SigLIP 은 semantic 영역이 묶임")
```


> 실제 robot 환경 사진으로 시도해보면 차이가 더 명확. 책상 위 물체들이 있는 사진을 권장.


---


## 실습 3: 한 페이지 OpenVLA 노트 산출


**파일명**: `~/phase4_notes/week4/openvla_one_page.md`


```markdown
# OpenVLA 한 페이지


## 1. One-liner
> _____________________________________________________________


## 2. 4가지 핵심 결정
1. Backbone LM : Llama 2 7B
2. Vision encoder : DINOv2 + SigLIP hybrid
3. 데이터 : OpenX-Embodiment 970K
4. Fine-tuning : LoRA 지원


## 3. Architecture (손그림)
[이미지 첨부]


## 4. Hybrid vision encoder 의 의도
- DINOv2: spatial / geometric
- SigLIP: semantic / category
- concat 후 projector (MLP) -> LM


## 5. RT-2 vs OpenVLA 비교 (한 페이지)
| 항목 | RT-2 | OpenVLA |
| ... | ... | ... |


## 6. Limitations 5가지
1. 데이터 분포 의존
2. VRAM (4-bit 필수)
3. Latency 100~150ms
4. Single-arm
5. No teleop policy


## 7. 본 로드맵 관점
- Phase 7 산출물 #4 에서 LoRA fine-tuning 으로 자작 6DOF 팔 적응
- 4-bit quantization 으로 RTX 4070 inference 가능


## 8. 다음 주 (week 5) 시작 질문
- OpenX-Embodiment 의 22 embodiments 중 자작 팔에 가장 가까운 것은?
```


---


## 실습 체크리스트


- [ ] OpenVLA 논문 1회독 (대략 15페이지)
- [ ] reading note 빈칸 채움
- [ ] `practice_dinov2_siglip.py` 실행, 시각화 비교
- [ ] RT-2 vs OpenVLA 비교 표 완성
- [ ] 한 페이지 노트 산출
- [ ] quiz_easy / quiz_medium 풀기
- [ ] git commit


---


## 참고 자료


- [OpenVLA paper](https://arxiv.org/abs/2406.09246)
- [OpenVLA project page](https://openvla.github.io/)
- [OpenVLA GitHub](https://github.com/openvla/openvla)
- [DINOv2 paper](https://arxiv.org/abs/2304.07193)
- [SigLIP paper](https://arxiv.org/abs/2303.15343)
- [Llama 2 paper](https://arxiv.org/abs/2307.09288)
