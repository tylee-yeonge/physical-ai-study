# Week 8 실습: VLAInference class + image pipeline + 100회 stress test


> **실습 목표**: ROS2 통합 직전의 안정적 inference 모듈 완성.
> **예상 시간**: 6-8시간


---


## 환경 설정


이번 주부터 OpenVLA 양자화 추론에 진입한다 — 무거운 의존성 (torch + transformers + bitsandbytes + peft) 이 본격 사용된다.

**진입 전 필수 점검**: [`../SETUP.md`](../SETUP.md) §5, §6, §7 (Colab/로컬 환경 + 버전 매칭). 특히 §7 의 버전 매트릭스는 LoRA 가중치를 Colab 에서 가져올 경우 반드시 일치해야 한다.

**공용 venv 사용**: week 8-12 는 의존성이 거의 동일하므로 `Studies/Phase 4/.venv-vla` 공용 venv 를 만들어 공유한다 (week 별 .venv 가 각 8-10GB 씩 누적되는 것을 막기 위함).


```bash
# Phase 4 공용 venv (week 8 진입 시 1회 생성)
cd "/workspace/study/physical-ai-study/Studies/Phase 4"
python3 -m venv .venv-vla
source .venv-vla/bin/activate
pip install --upgrade pip
pip install -r week8/requirements.txt


# 작업 디렉토리
mkdir -p ~/phase4_notes/week8/vla_inference
cd ~/phase4_notes/week8/vla_inference
```


> VRAM 점검: 4bit 양자화 후 OpenVLA 가 약 6GB 를 차지하므로 12GB 4070 에 안착해야 정상. `nvidia-smi` 로 inference 직후 사용량을 측정해 두면 week 11 에서 ROS2 오버헤드를 더했을 때의 여유 판단에 사용 가능 (SETUP.md §9.3 리스크 4).


---


## 실습 1: VLAInference class


**파일명**: `vla_inference/inference.py`


```python
"""
VLAInference: OpenVLA 의 inference 를 wrap. ROS2 노드의 직접 입력.
"""
import logging
import time


import numpy as np
import torch
from PIL import Image
from transformers import (
    AutoModelForVision2Seq, AutoProcessor, BitsAndBytesConfig,
)


from .exceptions import (
    VLAInferenceError, VLAOOMError, VLAInputError, VLAOutputError, VLAModelError,
)
from .config import MODEL_ID, QUANT_TYPE, DEVICE, UNNORM_KEY


logger = logging.getLogger(__name__)




class VLAInference:
    """OpenVLA inference wrapper for ROS2 integration."""


    def __init__(
        self,
        model_id: str = MODEL_ID,
        quant_type: str = QUANT_TYPE,
        device: str = DEVICE,
        unnorm_key: str = UNNORM_KEY,
    ):
        self.model_id = model_id
        self.quant_type = quant_type
        self.device = device
        self.unnorm_key = unnorm_key


        logger.info(f"Loading VLA: model={model_id}, quant={quant_type}, device={device}")
        self._load_model()
        self._warmed_up = False


    def _load_model(self):
        """모델 + processor 로드"""
        try:
            self.processor = AutoProcessor.from_pretrained(
                self.model_id, trust_remote_code=True,
            )


            bnb_config = None
            if self.quant_type == '4bit':
                bnb_config = BitsAndBytesConfig(
                    load_in_4bit=True,
                    bnb_4bit_quant_type='nf4',
                    bnb_4bit_use_double_quant=True,
                    bnb_4bit_compute_dtype=torch.float16,
                )
            elif self.quant_type == '8bit':
                bnb_config = BitsAndBytesConfig(load_in_8bit=True)
            # fp16 는 quantization_config 없음


            self.model = AutoModelForVision2Seq.from_pretrained(
                self.model_id,
                attn_implementation='eager',
                torch_dtype=torch.float16,
                low_cpu_mem_usage=True,
                trust_remote_code=True,
                quantization_config=bnb_config,
            )


            vram_gb = torch.cuda.memory_allocated(self.device) / 1e9
            logger.info(f"VLA loaded: vram={vram_gb:.2f} GB")
        except Exception as e:
            raise VLAModelError(f"Model load failed: {e}") from e


    def warm_up(self, n_iter: int = 5):
        """warm-up inference (latency outlier 제거)"""
        dummy = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
        for i in range(n_iter):
            self._predict_internal(dummy, "warm up")
        self._warmed_up = True
        logger.info(f"VLA warmed up ({n_iter} iter)")


    def predict(self, image_pil: Image.Image, instruction: str) -> np.ndarray:
        """한 번의 inference 호출.


        Args:
            image_pil: PIL Image (RGB)
            instruction: 자연어 명령


        Returns:
            7-DoF action [dx, dy, dz, rx, ry, rz, gripper]


        Raises:
            VLAInputError: 입력 형식 잘못
            VLAOOMError: GPU memory 부족
            VLAOutputError: 출력에 NaN
        """
        # 1. validate input
        if not isinstance(image_pil, Image.Image):
            raise VLAInputError(f"image must be PIL.Image, got {type(image_pil)}")
        if image_pil.mode != 'RGB':
            raise VLAInputError(f"image must be RGB, got {image_pil.mode}")
        if not isinstance(instruction, str) or len(instruction) == 0:
            raise VLAInputError(f"instruction must be non-empty str")


        if not self._warmed_up:
            logger.warning("Predict before warm_up — first latency may be high")


        # 2. predict
        try:
            action = self._predict_internal(image_pil, instruction)
        except torch.cuda.OutOfMemoryError as e:
            torch.cuda.empty_cache()
            raise VLAOOMError(f"OOM during inference: {e}") from e
        except Exception as e:
            raise VLAInferenceError(f"Inference failed: {e}") from e


        # 3. validate output
        if action.shape != (7,):
            raise VLAOutputError(f"action shape must be (7,), got {action.shape}")
        if np.isnan(action).any():
            raise VLAOutputError(f"action contains NaN: {action}")


        return action


    def _predict_internal(self, image_pil, instruction) -> np.ndarray:
        prompt = f"In: What action should the robot take to {instruction}?\nOut:"
        inputs = self.processor(prompt, image_pil).to(self.device, dtype=torch.float16)
        with torch.no_grad():
            action = self.model.predict_action(
                **inputs, unnormalize_key=self.unnorm_key, do_sample=False,
            )
        return np.asarray(action).astype(np.float32)


    def close(self):
        """자원 해제"""
        del self.model
        del self.processor
        torch.cuda.empty_cache()
        logger.info("VLA closed")
```


---


## 실습 2: image preprocess


**파일명**: `vla_inference/preprocess.py`


```python
"""
OpenCV (BGR) <-> PIL (RGB) 변환 + 검증.
"""
import logging


import cv2
import numpy as np
from PIL import Image


from .exceptions import VLAInputError


logger = logging.getLogger(__name__)




def opencv_to_pil(img_bgr: np.ndarray) -> Image.Image:
    """OpenCV BGR uint8 -> PIL RGB Image.


    Raises:
        VLAInputError: 입력 shape / dtype 가 잘못
    """
    if not isinstance(img_bgr, np.ndarray):
        raise VLAInputError(f"img must be np.ndarray, got {type(img_bgr)}")
    if img_bgr.dtype != np.uint8:
        raise VLAInputError(f"img dtype must be uint8, got {img_bgr.dtype}")
    if img_bgr.ndim != 3 or img_bgr.shape[2] != 3:
        raise VLAInputError(f"img shape must be (H,W,3), got {img_bgr.shape}")


    img_rgb = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB)
    return Image.fromarray(img_rgb)




def resize_for_vla(img_pil: Image.Image, target: int = 224) -> Image.Image:
    """OpenVLA 의 표준 입력 크기로 resize. processor 가 알아서 하지만 명시적."""
    return img_pil.resize((target, target), Image.BILINEAR)




def validate_image(img_pil: Image.Image):
    """sanity check"""
    if img_pil.mode != 'RGB':
        raise VLAInputError(f"image mode must be RGB, got {img_pil.mode}")
    w, h = img_pil.size
    if w < 100 or h < 100:
        raise VLAInputError(f"image too small: {w}x{h}")
```


---


## 실습 3: exceptions + config


**파일명**: `vla_inference/exceptions.py`


```python
class VLAInferenceError(Exception):
    """Base VLA inference error"""




class VLAModelError(VLAInferenceError):
    """모델 로드 / 파일 손상"""




class VLAInputError(VLAInferenceError):
    """입력 검증 실패"""




class VLAOutputError(VLAInferenceError):
    """출력 검증 실패 (NaN, shape 등)"""




class VLAOOMError(VLAInferenceError):
    """GPU memory 부족"""
```


**파일명**: `vla_inference/config.py`


```python
import os


MODEL_ID = os.environ.get('VLA_MODEL_ID', 'openvla/openvla-7b')
QUANT_TYPE = os.environ.get('VLA_QUANT', '4bit')
DEVICE = os.environ.get('VLA_DEVICE', 'cuda:0')
UNNORM_KEY = os.environ.get('VLA_UNNORM_KEY', 'bridge_orig')
```


---


## 실습 4: 100회 Stress Test


**파일명**: `practice_stress_test.py`


```python
"""
실습 4: 100 회 연속 inference. 0 fail 통과가 목표.
"""
import logging
import numpy as np
from PIL import Image
import time


from vla_inference.inference import VLAInference
from vla_inference.exceptions import VLAInferenceError


logging.basicConfig(level=logging.INFO)


print("=" * 60)
print("실습 4: 100회 stress test")
print("=" * 60)


vla = VLAInference()
vla.warm_up(n_iter=5)


n_test = 100
latencies = []
fails = []


for i in range(n_test):
    img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
    inst = "pick up the can"


    try:
        t0 = time.time()
        action = vla.predict(img, inst)
        elapsed = (time.time() - t0) * 1000
        latencies.append(elapsed)
    except VLAInferenceError as e:
        fails.append((i, str(e)))
        print(f"[{i}] FAIL: {e}")


print(f"\n결과:")
print(f"Success: {len(latencies)}/{n_test}")
print(f"Failures: {len(fails)}")
if latencies:
    arr = np.array(latencies)
    print(f"Mean latency: {arr.mean():.1f} ms")
    print(f"p95 latency : {np.percentile(arr, 95):.1f} ms")


if len(fails) == 0:
    print("\n PASS: 100 회 연속 inference 성공!")
else:
    print(f"\n FAIL: {len(fails)} fails - 원인 분석 필요")
    for i, msg in fails[:5]:
        print(f"{i}: {msg}")


vla.close()
```


---


## 실습 체크리스트


- [ ] `vla_inference/` 패키지 4 파일 작성
- [ ] `practice_stress_test.py` 실행
  - [ ] 100/100 success
  - [ ] mean latency < 200 ms
- [ ] (선택) ELP Stereo 로 실제 이미지 테스트
- [ ] git commit
- [ ] quiz_easy / quiz_medium


---


## 참고 자료


- [HuggingFace transformers logging](https://huggingface.co/docs/transformers/main_classes/logging)
- [PyTorch CUDA memory management](https://pytorch.org/docs/stable/notes/cuda.html)
