# Week 2 실습


```python
# 1. LoRA 학습 실행
# (OpenVLA repo 의 vla-scripts/finetune.py 사용)


# 2. vla_node 확장
class VLAArmNode(Node):
    def __init__(self):
        self.vla = load_finetuned_openvla()
        self.ik = IKSolver(urdf_path)


    def image_callback(self, msg):
        action = self.vla.predict(image, instruction)
        ee_delta = action[:6]
        gripper = action[6]
        joint_delta = self.ik.compute(ee_delta)
        # publish to /joint_command
```


체크리스트:
- [ ] LoRA 학습 완료
- [ ] vla_node 확장 동작
- [ ] IK 통합
- [ ] quiz
