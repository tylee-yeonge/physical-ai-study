# Week 6 실습: Open-vocab mini-demo + (선택) ROS2 / Gradio


> **예상 시간**: 4시간


---


## 실습 1: OpenVocabClassifier class


```python
"""
practice_clip_classifier.py
"""
import torch
from transformers import CLIPProcessor, CLIPModel
from PIL import Image




class OpenVocabClassifier:
    def __init__(self, model_id='openai/clip-vit-large-patch14'):
        self.proc = CLIPProcessor.from_pretrained(model_id)
        self.model = CLIPModel.from_pretrained(model_id).eval()


    def classify(self, image: Image.Image, labels: list):
        prompts = [f"a photo of a {l}" for l in labels]
        inputs = self.proc(text=prompts, images=image, return_tensors='pt', padding=True)
        with torch.no_grad():
            out = self.model(**inputs)
        probs = out.logits_per_image.softmax(dim=-1).squeeze().tolist()
        results = sorted(zip(labels, probs), key=lambda x: -x[1])
        return results




if __name__ == '__main__':
    import numpy as np
    clf = OpenVocabClassifier()
    img = Image.fromarray((np.random.rand(224, 224, 3) * 255).astype(np.uint8))
    labels = ['cat', 'dog', 'cup', 'mouse']
    for label, p in clf.classify(img, labels):
        print(f"{label}: {p:.3f}")
```


---


## 실습 2 (선택): ROS2 노드


```python
"""
~/ros2_ws/src/vla_node/vla_node/clip_node.py
"""
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from std_msgs.msg import String, Float64MultiArray
from cv_bridge import CvBridge


from clip_classifier import OpenVocabClassifier




class ClipNode(Node):
    def __init__(self):
        super().__init__('clip_node')
        self.declare_parameter('labels', 'cup,mouse,keyboard,robot arm')
        labels_str = self.get_parameter('labels').get_parameter_value().string_value
        self.labels = [l.strip() for l in labels_str.split(',')]


        self.bridge = CvBridge()
        self.clf = OpenVocabClassifier()


        self.image_sub = self.create_subscription(
            Image, '/camera/image_raw', self.on_image, 1)
        self.top_pub = self.create_publisher(String, '/clip/top_class', 10)
        self.probs_pub = self.create_publisher(Float64MultiArray, '/clip/probs', 10)


    def on_image(self, msg):
        from PIL import Image as PILImage
        import cv2
        img_bgr = self.bridge.imgmsg_to_cv2(msg, 'bgr8')
        img_rgb = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB)
        pil = PILImage.fromarray(img_rgb)


        results = self.clf.classify(pil, self.labels)
        top_label, top_prob = results[0]


        m = String(); m.data = f"{top_label} ({top_prob:.2f})"
        self.top_pub.publish(m)


        pm = Float64MultiArray()
        pm.data = [p for _, p in results]
        self.probs_pub.publish(pm)




def main():
    rclpy.init()
    node = ClipNode()
    rclpy.spin(node)




if __name__ == '__main__':
    main()
```


---


## 실습 3 (선택): Gradio UI


```python
"""
practice_gradio_clip.py
"""
import gradio as gr
from clip_classifier import OpenVocabClassifier


clf = OpenVocabClassifier()




def classify_fn(image, labels_str):
    labels = [l.strip() for l in labels_str.split(',')]
    results = clf.classify(image, labels)
    return {label: prob for label, prob in results}




gr.Interface(
    classify_fn,
    inputs=[
        gr.Image(type='pil'),
        gr.Textbox(label='labels (comma-separated)',
                   value='cup, mouse, keyboard, robot arm'),
    ],
    outputs=gr.Label(num_top_classes=5),
    title="CLIP Open-Vocabulary Classification",
).launch()
```


---


## 체크리스트
- [ ] OpenVocabClassifier 동작
- [ ] (선택) ROS2 통합
- [ ] (선택) Gradio UI
- [ ] quiz
