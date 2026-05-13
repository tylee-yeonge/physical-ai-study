# Week 11 실습: Image Gap + Domain Randomization

> [time] **예상 시간**: 5시간

---

## [note] 실습 1: 4 dim gap 측정

```python
"""
practice_image_gap.py
"""
import cv2
import numpy as np

real = cv2.imread("real.png")
sim = cv2.imread("sim.png")

# 1. RGB histogram
real_h = cv2.calcHist([real], [0, 1, 2], None, [16, 16, 16], [0, 256] * 3)
sim_h = cv2.calcHist([sim], [0, 1, 2], None, [16, 16, 16], [0, 256] * 3)
hist_dist = cv2.compareHist(real_h, sim_h, cv2.HISTCMP_CHISQR)
print(f"Hist distance: {hist_dist:.2f}")

# 2. Color mean
print(f"Real BGR mean: {real.mean(axis=(0,1))}")
print(f"Sim  BGR mean: {sim.mean(axis=(0,1))}")

# 3. Geometry (homography alignment 시도)
# (선택, 복잡)

# 4. Noise variance
real_g = cv2.cvtColor(real, cv2.COLOR_BGR2GRAY)
sim_g = cv2.cvtColor(sim, cv2.COLOR_BGR2GRAY)
real_noise = cv2.Laplacian(real_g, cv2.CV_64F).var()
sim_noise = cv2.Laplacian(sim_g, cv2.CV_64F).var()
print(f"Real noise: {real_noise:.2f}")
print(f"Sim noise : {sim_noise:.2f}")
```

---

## [note] 실습 2: Domain Randomization

```python
"""
practice_domain_random.py
"""
# Isaac Sim 안에서 매 episode 마다 randomize
import numpy as np

def randomize_scene():
    # Light intensity
    intensity = np.random.uniform(2000, 5000)
    # ... 적용

    # Camera focal length
    focal = np.random.uniform(12, 18)

    # Material color jitter
    # ...

for episode in range(10):
    randomize_scene()
    # episode 실행
```

---

## [note] 실습 3: image gap 보고서

`~/phase6_notes/week11/image_gap_report.md`:

```markdown
# Image Gap 측정 결과

## RGB Histogram distance
- Real vs Sim (default): 145.3
- Real vs Sim (DR): 38.2 (4x 가까워짐)

## Color mean difference (BGR)
- Default: [25, 18, 31]
- After DR: [8, 5, 9]

## Noise variance
- Real: 320 (high)
- Sim default: 12 (low)
- Sim + noise injection: 280 (close to Real)

## 결론
Domain Randomization 으로 Sim image 를 Real 에 가깝게.
Phase 7 산출물 #4 에서 Sim/Real 의 robust action 검증 가능.
```

---

## [O] 체크리스트
- [ ] 4 dim gap 측정
- [ ] DR 적용 후 다시 측정
- [ ] 보고서 작성
- [ ] quiz
