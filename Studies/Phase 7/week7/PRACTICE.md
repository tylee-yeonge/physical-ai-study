# Week 7 실습: Latency 종합 측정


```python
# latency_logger.py - 각 노드의 header.stamp 기록
# Image timestamp -> inference timestamp -> safety timestamp -> motor write timestamp


# 분석
import numpy as np
inference_lat = (t_inference_out - t_image).mean()
total_lat = (t_motor - t_image).mean()
print(f"Total: {total_lat:.1f} ms")
print(f"Inference: {inference_lat:.1f} ms ({inference_lat/total_lat*100:.1f}%)")
```


체크리스트:
- [ ] 모든 단계 timestamp 기록
- [ ] 1000 sample 측정
- [ ] Histogram 시각화
- [ ] 보고서 작성 (Phase 7 v3 의 input)
