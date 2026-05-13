# Week 6: Open-vocabulary classification mini-demo


> **이번 주 목표**: CLIP 으로 자작 팔 환경 객체를 open-vocabulary classification 하는 mini-demo. Phase 4 ROS2 demo 보강에 활용 가능.
> **예상 시간**: 6시간


---


## 학습 순서


| 순서 | 단계 | 파일 | 설명 |
|:----:|------|------|------|
| 1 | 환경 | `requirements.txt` | gradio 옵션 |
| 2 | mini-demo | `PRACTICE.md` 1 | image + dynamic prompts |
| 3 | (선택) ROS2 통합 | `PRACTICE.md` 2 | CLIP 노드 |
| 4 | (선택) Gradio UI | `PRACTICE.md` 3 | 시연용 |
| 5 | 노트 + 퀴즈 | | |


---


## 핵심 개념


### 1. Mini-demo 의 목표


```
User: image + 자유 텍스트 prompt list
  -> CLIP -> top-k similarity
  -> Phase 4 ROS2 demo 의 보강용
```


### 2. 표준 구조


```python
class OpenVocabClassifier:
    def __init__(self):
        self.model = CLIPModel.from_pretrained(...)
        self.processor = CLIPProcessor.from_pretrained(...)
    def classify(self, image, candidate_labels):
        prompts = [f"a photo of a {l}" for l in candidate_labels]
        ...
```


### 3. ROS2 통합 (Phase 4 보강)


```
/camera/image_raw -> clip_node -> /clip/top_class (String)
                                  /clip/probs (Float64MultiArray)
```


OpenVLA action 발행 시 CLIP top class 도 함께 -> Rerun 에 동시 시각화.


### 4. Gradio UI (시연용)


```python
import gradio as gr
gr.Interface(
    classify,
    inputs=[gr.Image(type='pil'), gr.Textbox(label='labels')],
    outputs=gr.Label(num_top_classes=5),
).launch()
```


### 5. CLIP 의 robotics 한계


| 한계 | Robotics 영향 |
|---|---|
| Spatial 약함 | "leftmost cup" 명령 약함 |
| Instance 구분 어려움 | "same cup as before" 약함 |
| OCR 약함 | 글자 객체 약함 |
| Static image | dynamic scene 약함 |


OpenVLA 같은 통합 모델 필요 (Vision + LM 결합으로 보강).


### 6. Phase 4 보강 시나리오


영상의 0:25 ~ 0:30 추가:
- "환경 이해 (CLIP zero-shot)"
- "a photo of a red cup" -> 95%
- 그리고 OpenVLA action 발행


---


## 자체 점검


**Q1. classify 의 signature?**
> `classify(image, candidate_labels: List[str]) -> List[Tuple[str, float]]`


**Q2. ROS2 topic 설계?**
> `/clip/top_class` + `/clip/probs`.


**Q3. CLIP robotics 가장 큰 한계?**
> Spatial reasoning. VLA 통합 모델로 보강.


**Q4. Phase 4 영상 보강 시점?**
> 0:25 ~ 0:30 의 환경 이해 section.


**Q5. Mini-demo 가 Phase 7 에서 활용되는 부분?**
> CLIP 의 환경 인식이 OpenVLA 의 input 보조로 사용 가능 (이중 검증).


---


## 실습


- mini-demo 구현
- (선택) ROS2 / Gradio
- Phase 4 보강용 코드 commit


---


## 핵심 요약


1. **CLIP open-vocab mini-demo**
2. **ROS2 통합 옵션**
3. **Gradio UI 시연용**
4. **CLIP robotics 한계 4가지**
5. **Phase 4 보강 가치**


- 이전: [Week 5](../week5/README.md) | 다음: [Week 7 - DINOv2](../week7/README.md)
