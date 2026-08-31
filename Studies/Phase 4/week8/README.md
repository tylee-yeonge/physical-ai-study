# Week 8: HuggingFace inference 셋업 안정화 + 실제 image 처리


> **이번 주 목표**: week 6 의 단발 inference 코드를 **안정적인 inference 서버** 형태로 재구성. 실제 이미지 (mock 이 아닌) 로 정확한 input format 검증. ROS2 통합 (week 10~) 의 사전 준비.
> **예상 시간**: 10시간 (안정화 4h + image pipeline 3h + 에러 처리 3h)
> **핵심 질문**: "OpenVLA inference 가 100회 연속으로 한 번도 fail 없이 동작하는가? OOM / shape mismatch / NaN 등 어떤 edge case 가 있는가?"


---


## 학습 순서


| 순서 | 단계 | 파일/자료 | 설명 |
|:----:|------|----------|------|
| 1 | 환경 | `requirements.txt` | OpenCV, ELP Stereo 드라이버 |
| 2 | inference wrapper class 작성 | `PRACTICE.md` 1 | model load + predict 분리 |
| 3 | image preprocess pipeline | `PRACTICE.md` 2 | OpenCV BGR -> PIL RGB -> tensor |
| 4 | edge case 핸들링 | `PRACTICE.md` 3 | OOM / shape / NaN |
| 5 | 100회 stress test | `PRACTICE.md` 4 | 안정성 검증 |
| 6 | 퀴즈 | quiz_easy / quiz_medium | inference 흐름 / 에러 처리 |


---


## 시작하기 전에 — 본 주의 결과물


본 주가 끝나면 다음 4개 파일이 생긴다 (week 10 ROS2 노드 코드의 직접 입력):


```
ros2_pkg/vla_inference/
  __init__.py
  inference.py # VLAInference class
  preprocess.py # image preprocess
  exceptions.py # custom exception class
  config.py # 환경 변수 / 모델 경로
```


이 4 개 파일이 ROS2 노드 (week 10) 의 `predict_action` 호출의 모든 것.


---


## 핵심 개념


### 1. inference wrapper class 의 설계 원칙


```python
class VLAInference:
    def __init__(self, model_id, quant_config, device):
        # 모델 로드 (한 번만)
        ...
    def predict(self, image, instruction) -> np.ndarray:
        # 한 번의 inference
        ...
    def warm_up(self, n_iter=5):
        # 초기 latency outlier 제거
        ...
    def close(self):
        # 자원 해제
        ...
```


설계 원칙:
- **stateless predict**: 매 호출마다 같은 결과 (deterministic)
- **resource ownership**: 모델 / processor 는 인스턴스가 소유
- **clear exceptions**: 모든 에러를 구분 가능한 형태로
- **logging**: 모든 inference 의 latency / GPU memory 기록


### 2. image preprocess pipeline 의 정확성


ROS2 / OpenCV 환경에서 자주 발생하는 함정:


| 함정 | 증상 | 해결 |
|---|---|---|
| BGR vs RGB | inference 결과가 이상함 | OpenCV BGR -> PIL RGB 명시적 변환 |
| Image size mismatch | shape error | resize 224x224 (model spec) |
| Pixel range | NaN 또는 saturated | uint8 [0,255] 유지, normalize 는 processor 가 |
| dtype | TypeError | uint8 이미지 그대로, fp16 변환은 processor 이후 |


표준 pipeline:


```python
import cv2
from PIL import Image


# 1. OpenCV BGR -> RGB
img_rgb = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB)


# 2. PIL Image 로 변환 (HuggingFace processor 입력 형식)
img_pil = Image.fromarray(img_rgb)


# 3. resize (선택, processor 가 알아서 함)
img_pil = img_pil.resize((224, 224))


# 4. processor 가 normalize + tensor 변환 담당
inputs = processor(prompt, img_pil).to("cuda:0", dtype=torch.float16)
```


### 3. edge case 카테고리


| 카테고리 | 예시 | 처리 |
|---|---|---|
| Memory | OOM 발생 | `torch.cuda.empty_cache()` + retry |
| Shape | image shape 가 비정상 | early validate + raise custom exception |
| Numerical | action 에 NaN | 검출 후 zero action 으로 fallback |
| External | model file corrupted | 재다운로드 안내 |
| Network | HuggingFace download fail | local cache 우선 |


### 4. Custom exceptions 의 의의


ROS2 환경에서 inference 에러가 발생하면 그게 어떤 종류인지 빠르게 판단해야 한다:


```python
class VLAInferenceError(Exception):
    """Base"""


class VLAOOMError(VLAInferenceError):
    """GPU memory 부족"""


class VLAInputError(VLAInferenceError):
    """입력 형식 잘못"""


class VLAOutputError(VLAInferenceError):
    """출력 NaN / shape 이상"""


class VLAModelError(VLAInferenceError):
    """모델 파일 손상"""
```


ROS2 노드에서:
- OOM → safety policy 로 fallback
- Input/Output error → log + safe action publish
- Model error → 노드 종료 + alarm


### 5. Stress test 의 의미


100 회 inference 가 **한 번도 fail 없이** 통과해야 한다. ROS2 환경에서 한 시간 동안 (5Hz × 3600초 = 18000 frame) 운영 시 fail 율 < 0.01% 가 양산 기준.


본 주는 100 회 시도 + 그 안의 fail 모두 분석.


### 6. ELP Stereo 카메라 사용 (선택)


본 로드맵 보유 hardware. 실제 카메라 입력으로 테스트하는 게 mock 보다 가치.


```python
import cv2


cap = cv2.VideoCapture(0) # ELP Stereo 의 left
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)


ret, frame = cap.read()
# frame: BGR uint8 (480, 640, 3)
```


> ELP Stereo 는 left + right 가 한 frame 에 합쳐져서 나옴 (1280x480). 한쪽만 잘라 사용.


### 7. Logging 표준


```python
import logging
logger = logging.getLogger(__name__)


logger.info(f"VLA loaded: model_id={model_id}, vram={vram_gb:.2f}GB")
logger.debug(f"Predict: image shape={image.shape}, instruction='{instruction}'")
logger.warning(f"Slow inference: {latency_ms:.1f}ms (p95 threshold)")
logger.error(f"OOM: torch.cuda.empty_cache() 시도")
```


ROS2 통합 시 표준 logging 으로 자동 호환.


### 8. config 관리


환경 변수로 모델 경로 / quantization 설정 분리:


```python
# config.py
import os


MODEL_ID = os.environ.get('VLA_MODEL_ID', 'openvla/openvla-7b')
QUANT_TYPE = os.environ.get('VLA_QUANT', '4bit') # '4bit', '8bit', 'fp16'
DEVICE = os.environ.get('VLA_DEVICE', 'cuda:0')
UNNORM_KEY = os.environ.get('VLA_UNNORM_KEY', 'bridge_orig')
```


ROS2 launch file 에서 환경 변수 또는 ROS parameter 로 주입 가능.


---


## 자체 점검


**Q1. inference wrapper class 가 단일 함수보다 좋은 이유는?**
> 모델 로드 (오래 걸림) 와 predict (자주 호출) 의 lifetime 분리. ROS2 node 환경에서 한 번 로드 후 반복 호출 패턴에 적합.


**Q2. OpenCV BGR 을 PIL RGB 로 변환 안 하면?**
> Inference 결과가 약간 이상한 action (R/B 채널이 바뀐 image 로 학습된 적 없음). 무작위에 가까운 noise.


**Q3. OOM 발생 시 자동 복구의 표준 패턴은?**
> `torch.cuda.empty_cache()` → retry 1회 → 또 실패 시 safety fallback action publish + log error.


**Q4. Stress test 의 통과 기준은?**
> 100 회 중 0 fail. 1 fail 이라도 발생하면 원인 분석 후 처리 코드 추가.


**Q5. config 를 환경 변수로 빼는 이유는?**
> ROS2 launch 시 다른 모델 / quantization 으로 빠르게 전환 가능. 코드 수정 없이 deployment 변경.


---


## 이번 주 실습 & 다음 주 준비


### 이번 주 실습 과제
1. `vla_inference/inference.py` - VLAInference class 구현
2. `vla_inference/preprocess.py` - image preprocess
3. `vla_inference/exceptions.py` - custom exceptions
4. `vla_inference/config.py` - 환경 변수
5. `practice_stress_test.py` - 100회 stress test 통과
6. (선택) ELP Stereo 로 실제 이미지 테스트
7. quiz_easy / quiz_medium


### 다음 주 (week 9) 준비
- 입출력 인터페이스 spec 1 페이지 작성 (week 9 의 시작점)
- ROS2 msg 타입 후보 검토 (sensor_msgs/Image, geometry_msgs/Twist 등)


---


## 이번 주 핵심 요약


1. **inference wrapper class** 가 ROS2 통합의 진입점.
2. **OpenCV BGR -> PIL RGB** 명시적 변환 필수.
3. **Custom exceptions** 로 ROS2 에서 에러 종류별 처리.
4. **100회 stress test** 통과가 양산 기준의 진입점.
5. **Config 환경 변수** 로 deployment flexibility.


---


- 이전: [Week 7 - OpenVLA vla-lab 문서 1편](../week7/README.md)


다음: [Week 9 - inference 입출력 인터페이스 정리](../week9/README.md)
