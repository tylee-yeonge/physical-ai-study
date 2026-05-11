# nuscenes_step5.py 구현 가이드

목표: nuScenes 한 sample 에서 두 카메라에 모두 보이는 annotation 의 3D 중심을
두 픽셀로 투영한 뒤, `cv2.triangulatePoints` 로 다시 3D 를 복원해
annotation 의 원본 위치(`translation`)와의 오차를 측정한다.

> KITTI Step 4 와 가장 다른 점은 **좌표 변환이 2 단계** 라는 것.
> 이 부분만 정확히 잡으면 나머지는 OpenCV API 조립이라 어렵지 않다.

---

## 0. 전체 데이터 흐름

```
NuScenes(mini)              ← load_nuscenes()
       |
       v
sample = nusc.sample[0]
       |
       +--build_camera_view(sample, CAM_FRONT)      --> view_a (K_a, cam_from_world_a, ...)
       +--build_camera_view(sample, CAM_FRONT_LEFT) --> view_b (K_b, cam_from_world_b, ...)
                       |
                       v
              find_common_annotation
              (각 annotation 의 3D 중심을
               두 카메라에 project_world_point)
                       |
                       v
              (ann, uv_a, uv_b)
                       |
       +---------------+---------------+
       v                               v
build_projection_matrix         build_projection_matrix
       (view_a) -> P_a                 (view_b) -> P_b
       |                               |
       +---------------+---------------+
                       v
                  triangulate
                 (P_a, P_b, uv_a, uv_b)
                       |
                       v
                  est_world (3,)
                       |
                       v
        gt_world = ann['translation']
                       |
                       v
            np.linalg.norm(est - gt)   <-- 오차 (m)
```

---

## 1. nuScenes 좌표계 컨벤션 (꼭 한 번에 정리)

nuScenes 는 3 개의 좌표계가 등장한다. 각각의 변환을 머릿속에 그려두면 디버깅이 쉽다.

### 1-1. 세 좌표계

| 좌표계 | 정의 | 특징 |
|--------|------|------|
| **sensor** (cam) | 각 카메라 기준 | OpenCV 컨벤션 (+x 오른쪽, +y 아래, +z 앞) |
| **ego** (vehicle) | 차량 후축 중심 | +x 전진, +y 좌측, +z 위 |
| **world** (global) | 각 scene 별 map 기준 | UTM 같은 글로벌 좌표 |

### 1-2. JSON 의 forward 변환 의미

nuScenes JSON 에서 (`translation`, `rotation`) 은 **forward (source -> destination)** 변환이다:

| JSON 객체 | (translation, rotation) 의 의미 |
|-----------|--------------------------------|
| `calibrated_sensor` | **sensor -> ego** (카메라가 ego 좌표계에서 어디 있는지) |
| `ego_pose`          | **ego -> world**   (ego 가 world 좌표계에서 어디 있는지) |
| `sample_annotation.translation` | **world** 좌표 그 자체 (3D 박스의 중심) |

### 1-3. 우리가 필요한 변환: `cam_from_world`

world 점을 카메라 좌표계로 보내려면:

```
world_from_sensor = world_from_ego @ ego_from_sensor   (forward 두 단계 연결)
cam_from_world    = inv(world_from_sensor)
```

즉 forward 두 개를 곱해서 합친 후 *역행렬*. 4x4 동차변환이라 `np.linalg.inv` 한 번이면 충분.

### 1-4. Quaternion 순서

- nuScenes JSON: **`[w, x, y, z]`** 순서
- `pyquaternion.Quaternion([w, x, y, z])` 또는 `Quaternion(w, x, y, z)` 둘 다 OK
- scipy 의 `Rotation.from_quat` 은 `[x, y, z, w]` 라 순서를 바꿔야 한다 (혼동 주의)

`Quaternion.rotation_matrix` 가 (3, 3) 회전 행렬을 돌려준다.

---

## 2. 함수별 구현 가이드

### 2-1. `load_nuscenes` (L70)

한 줄짜리. devkit 로드.

```python
return NuScenes(version=NUSC_VERSION, dataroot=str(NUSC_ROOT), verbose=False)
```

> `verbose=True` 면 콘솔이 메타데이터 출력으로 도배된다. False 권장.

---

### 2-2. `transform_matrix` (L116)

(translation, Quaternion) -> 4x4 동차변환.

```python
T = np.eye(4)
T[:3, :3] = rotation.rotation_matrix
T[:3, 3]  = translation
return T
```

> 이건 다음 단계에서 두 번 호출되니 먼저 짜두는 게 편하다.

---

### 2-3. `build_camera_view` (L86)

가장 신경 써야 할 함수. 4 단계로 나눠 짜면 헷갈리지 않는다.

```python
# 1) 해당 카메라의 sample_data 가져오기
sd = nusc.get('sample_data', sample['data'][channel])

# 2) calibrated_sensor / ego_pose 가져오기
calib = nusc.get('calibrated_sensor', sd['calibrated_sensor_token'])
ego   = nusc.get('ego_pose',          sd['ego_pose_token'])

# 3) intrinsic
K = np.array(calib['camera_intrinsic'])              # (3, 3)

# 4) forward 두 단계 합치고 역행렬
ego_from_sensor   = transform_matrix(calib['translation'], Quaternion(calib['rotation']))
world_from_ego    = transform_matrix(ego['translation'],   Quaternion(ego['rotation']))
world_from_sensor = world_from_ego @ ego_from_sensor
cam_from_world    = np.linalg.inv(world_from_sensor)

return CameraView(
    channel=channel,
    image_path=NUSC_ROOT / sd['filename'],
    K=K, cam_from_world=cam_from_world,
    width=sd['width'], height=sd['height'],
)
```

**검증**:
- `K[0, 0]` (fx) 가 ~1200 부근이면 정상 (CAM_FRONT 기준 보통 1266 근처).
- `cam_from_world.shape == (4, 4)`, 마지막 행이 `[0, 0, 0, 1]`.

---

### 2-4. `project_world_point` (L134)

world 점 한 개를 카메라 픽셀로 투영. 가시성 판정이 포함되어 있다는 점이 다름.

```python
p_h    = np.append(point_world, 1.0)                # (4,)
p_cam  = view.cam_from_world @ p_h                   # (4,)
depth  = p_cam[2]
if depth <= 0:                                       # 카메라 뒤
    return None, depth

uvw  = view.K @ p_cam[:3]                            # (3,)
u, v = uvw[0] / uvw[2], uvw[1] / uvw[2]

if 0 <= u < view.width and 0 <= v < view.height:
    return (float(u), float(v)), float(depth)
return None, float(depth)
```

> `uvw[2] == p_cam[2]` (K 의 마지막 행이 [0, 0, 1] 이라서). 일관성 확인용.

**검증**: CAM_FRONT 정면 ~10m 거리에 가짜 점 `[ego_x + 10, ego_y, ego_z]` 를
넣어보고 픽셀이 이미지 중앙(~830, ~470) 근처에 찍히면 변환이 맞다.

---

### 2-5. `find_common_annotation` (L171)

sample 의 annotation 들을 순회하며 두 카메라 모두에 가시인 것을 찾는다.

```python
for ann_token in sample['anns']:
    ann = nusc.get('sample_annotation', ann_token)
    p_world = np.array(ann['translation'])

    uv_a, _ = project_world_point(p_world, view_a)
    if uv_a is None:
        continue
    uv_b, _ = project_world_point(p_world, view_b)
    if uv_b is None:
        continue

    return ann, uv_a, uv_b
return None
```

**왜 CAM_FRONT + CAM_FRONT_LEFT 인가?**:
- 서로 인접해서 시야가 일부 겹친다 → 같은 객체가 동시에 보일 확률이 높다.
- 너무 멀리 떨어진 쌍 (예: CAM_FRONT + CAM_BACK) 은 절대 안 겹친다.

**가시 객체가 없으면**: 다른 인접 쌍 (CAM_FRONT_RIGHT + CAM_FRONT, CAM_BACK_LEFT + CAM_BACK 등) 으로 바꾸거나 다른 sample 시도. `nusc.sample[1]`, `nusc.sample[5]` 등.

---

### 2-6. `build_projection_matrix` (L209)

```python
Rt = view.cam_from_world[:3, :]       # (3, 4)   = [R | t]
P  = view.K @ Rt                      # (3, 4)
return P
```

> 이 P 는 **world -> pixel** 변환이다. cv2.triangulatePoints 에 넣을 두 P 는
> 반드시 **같은 좌표계** 기준이어야 한다. 여기서는 둘 다 world 기준.

---

### 2-7. `triangulate` (L228)

OpenCV API 호출 + homogeneous 정규화.

```python
pts_a = np.array([[uv_a[0]], [uv_a[1]]], dtype=np.float64)   # (2, 1)
pts_b = np.array([[uv_b[0]], [uv_b[1]]], dtype=np.float64)   # (2, 1)

X_h = cv2.triangulatePoints(P_a, P_b, pts_a, pts_b)          # (4, 1)
X   = (X_h[:3] / X_h[3]).flatten()                           # (3,)
return X
```

> `cv2.triangulatePoints` 의 입력 shape 은 `(2, N)` 이라 컬럼 벡터로 만들어야 한다.
> 행/열 순서 헷갈리면 그냥 reshape 해서 (2, 1) 만들면 됨.

**알고리즘**: DLT (Direct Linear Transform). 두 식 `uv = P @ X` 을 묶어
4 방정식 -> SVD 의 최소 특이벡터로 풀어준다. 학습 목적이면
`Studies/Phase 2/week4/basic.cpp` 의 manual 구현을 한 번 읽어볼 것.

---

### 2-8. `draw_observation` (L260)

```python
center = (int(round(uv[0])), int(round(uv[1])))
cv2.circle(img_bgr, center, radius, color, thickness)
return img_bgr
```

---

### 2-9. `log_to_rerun` (L281)

Rerun 에서 핀홀 카메라 + 이미지 + 점들을 한 world 좌표계에 배치한다.

```python
for v in views:
    prefix = f"world/cam/{v.channel}"
    world_from_cam = np.linalg.inv(v.cam_from_world)
    rr.log(prefix,
           rr.Transform3D(translation=world_from_cam[:3, 3],
                          mat3x3=world_from_cam[:3, :3]))
    rr.log(prefix,
           rr.Pinhole(image_from_camera=v.K,
                      width=v.width, height=v.height))
    rr.log(f"{prefix}/image", rr.Image(images_rgb[v.channel]))
    if v.channel in obs:
        u, v_ = obs[v.channel]
        rr.log(f"{prefix}/observation",
               rr.Points2D([[u, v_]], radii=5, colors=[(0, 255, 0)]))

rr.log("world/gt_point",
       rr.Points3D([gt_world],  radii=0.2, colors=[(255, 0, 0)]))
rr.log("world/triangulated_point",
       rr.Points3D([est_world], radii=0.2, colors=[(0, 255, 0)]))
```

> Rerun 의 자식 엔티티는 **부모 frame 상대** 다. 그래서 `Pinhole` 에는 K 만 주고,
> 카메라가 world 어디에 있는지는 부모 `world/cam/{channel}` 의 `Transform3D` 로 표현.
> `Transform3D` 는 **부모 -> 자식** 변환이므로 `world_from_cam` (= inv(cam_from_world)) 을 줘야 한다.

---

## 3. 구현 순서 추천

`main()` 호출 순서대로 + 의존성 고려:

1. **`load_nuscenes`** → `print(len(nusc.sample))` 로 10 (mini) 이 나오는지
2. **`transform_matrix`** → 의존성 없음, 먼저 짠다
3. **`build_camera_view`** → `print(view_a.K, view_a.cam_from_world.shape)` 확인
4. **`project_world_point`** → 임의의 world 점 (ego_pose 의 translation + 앞쪽 10m) 으로 테스트
5. **`find_common_annotation`** → 두 카메라 동시 가시 객체가 있는지 확인. 없으면 카메라 쌍 변경
6. **`build_projection_matrix`** + **`triangulate`** → 오차 출력 확인
7. **`draw_observation`** → 저장된 jpg 열어서 점이 객체 위에 있는지
8. **`log_to_rerun`** → Rerun 시각화 (옵션)

특히 **5번에서 막힐 가능성** 이 높다. mini 의 첫 sample 은 보행자/차량이 많아 보통은 잡히지만, 안 잡히면 카메라 쌍을 `("CAM_FRONT_RIGHT", "CAM_FRONT")` 같은 식으로 바꿔보자.

---

## 4. 흔한 함정 체크리스트

- [ ] Quaternion 순서: nuScenes JSON 은 `[w, x, y, z]`. scipy 와 반대.
- [ ] JSON 의 (translation, rotation) 은 **forward** (source -> destination) 변환.
- [ ] `cam_from_world = inv(world_from_ego @ ego_from_sensor)` — 곱셈 순서 주의.
      world_from_ego 가 **왼쪽**, ego_from_sensor 가 **오른쪽**. (열벡터 컨벤션)
- [ ] `cv2.triangulatePoints` 의 두 P 는 **같은 좌표계** 기준이어야 한다. 여기선 둘 다 world.
- [ ] 결과 `X_h` 는 4D homogeneous. `X_h[:3] / X_h[3]` 정규화 필수.
- [ ] `cv2.triangulatePoints` 입력 pts shape 은 `(2, N)` (행이 [u, v]). 헷갈리면 reshape.
- [ ] Rerun `Transform3D` 는 **부모 -> 자식** 변환. 카메라는 `world_from_cam` 으로 줘야 함.

---

## 5. 합격 출력 예시 (참고)

```
sample token: ca9a282c..., anns: 69
annotation: vehicle.car  gt_world=[373.21, 1130.48, 1.08]
  CAM_FRONT      uv = (824.31, 482.95)
  CAM_FRONT_LEFT uv = (1247.84, 451.20)
est_world: [373.18, 1130.45, 1.09]
오차      : 4.27 cm   (합격, 기준 < 50 cm)
저장: output/nuscenes_step5_CAM_FRONT.jpg
저장: output/nuscenes_step5_CAM_FRONT_LEFT.jpg
```

수치는 정확히 같지 않더라도 **오차가 cm 단위**면 정상.
m 단위로 나오면 좌표 변환 어디선가 좌/우, forward/inverse 가 뒤집힌 것.

---

## 6. 디버깅 팁: 좌표 변환이 안 맞을 때

오차가 수 m 이상이면 거의 항상 변환 행렬 문제. 다음 순서로 확인:

1. **K 가 맞는가**: `view.K[0, 0]` ~1266 (CAM_FRONT 기준)
2. **forward 두 단계가 맞는가**: 가짜 점으로 sanity check
   ```python
   # ego 의 +10m 앞 (전진 방향) 점이 CAM_FRONT 이미지 중앙 근처에 찍혀야 함
   ego_pos    = np.array(ego['translation'])
   ego_q      = Quaternion(ego['rotation'])
   forward    = ego_q.rotate(np.array([10.0, 0.0, 0.0]))  # ego +x
   test_world = ego_pos + forward
   print(project_world_point(test_world, view_front))  # ~(800, 470) 부근?
   ```
3. **annotation 의 cam_z 가 양수인가**: 음수면 카메라 뒤. 곱셈 순서 뒤집혔을 가능성.
4. **두 P 가 같은 좌표계 기준인가**: 둘 다 world 기준이어야 함.

오차가 10~50cm 정도면 정상 범위 (특히 인접 카메라 쌍은 baseline 이 좁아 잘 안 좁혀짐).
