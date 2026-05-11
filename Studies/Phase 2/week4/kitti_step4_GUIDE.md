# kitti_step4.py 구현 가이드

목표: KITTI label_2 의 3D 박스를 calib P2 로 image_2 위에 재투영하고,
label_2 의 2D bbox 와 정합도(IoU)로 평가한다.

> TODO 주석에 이미 정답에 가까운 코드가 적혀 있다. 이 가이드는
> "왜 그렇게 쓰는지" + "쉽게 틀리는 지점"을 짚어주는 보조 자료다.
> 한 함수씩 채워가며 print 로 중간값을 찍어 확인하는 흐름을 권장.

---

## 0. 전체 데이터 흐름

```
calib/000000.txt  --parse_calib_p2-->  P2 (3,4)
                                              \
label_2/000000.txt --parse_label_line--> KittiLabel
                                              |
                                       compute_3d_corners
                                              |
                                      corners_cam (3,8) ──┐
                                              |           |
                                       project_corners(P2)|
                                              |           |
                                          uv (2,8)        |
                                         visible (8,)     |
                                              |           |
                            bbox_from_corners |           |
                                              v           v
                                         proj_bbox    draw_box_3d / Rerun
                                              |
                                  bbox_iou ←──┤
                                              ↓
                                     label.bbox_2d (정답)
```

각 함수의 입출력 shape 를 머릿속에 그려두면 디버깅이 빠르다.

---

## 1. KITTI 좌표계 컨벤션 (꼭 한 번에 외울 것)

가장 자주 틀리는 부분이라 먼저 정리한다.

### cam coord (rectified left color, cam2 기준)

```
        +z  (앞 - 차 진행 방향)
       /
      /
     +-------> +x  (오른쪽)
     |
     |
     v
    +y  (아래)
```

- OpenCV 와 동일. **y 가 아래** 인 점이 중요.
- `label.location = (x, y, z)` 는 **3D 박스의 바닥 중심** (지면).
  → 천장은 `y - h` (y 가 아래라서 위로 가려면 빼야 함).
- `rotation_y` 는 y 축(아래) 기준 회전. 일반적인 +y up 좌표계의 R_y 와
  부호가 뒤집힌다 (`compute_3d_corners` 노트 참조).

### dim 의 순서: (h, w, l) — height 가 첫 번째

| 기호 | 축 | 의미 |
|------|----|------|
| h    | y  | height (높이) |
| w    | z  | width (앞뒤 폭, 차로 치면 좌우 폭) |
| l    | x  | length (좌우 폭, 차로 치면 앞뒤 길이) |

> "차의 길이 l 은 x축, 폭 w 는 z축" 이라는 매핑이 헷갈릴 수 있다.
> KITTI 컨벤션이 그렇게 정의되어 있다고 외우는 게 빠르다.

---

## 2. 함수별 구현 가이드

### 2-1. `parse_label_line` (L48)

**한 줄 포맷** (공백 구분, 최소 15 컬럼):

```
type  trunc occl alpha  x1 y1 x2 y2   h w l   x y z   ry  [score]
 [0]   [1]  [2]  [3]    [4][5][6][7] [8][9][10] [11][12][13] [14]
```

- `tokens = line.split()` 이면 위 인덱스 그대로 쓸 수 있다.
- 숫자는 전부 `float(...)` 캐스팅. 좌표 -1 인 DontCare 행은
  `load_labels` 에서 이미 거른다 (구현 안 해도 됨).

**검증**: `print(parse_label_line(first_line))` 으로
`type_='Pedestrian'`, `bbox_2d=(712.40, 143.00, 810.73, 307.92)`,
`dim=(1.89, 0.48, 1.20)` 가 나오면 정상.

---

### 2-2. `parse_calib_p2` (L73, **여기서 에러 발생**)

**파일 예시** (한 줄):

```
P2: 7.07e+02 0 6.04e+02 4.57e+01  0 7.07e+02 1.81e+02 -3.45e-01  0 0 1 4.98e-03
```

콜론 뒤로 숫자 12 개 = 3x4 행렬.

```python
for line in calib_path.read_text().splitlines():
    if line.startswith("P2:"):
        vals = line.split()[1:]              # ['7.07e+02', ..., '4.98e-03']  (12개)
        return np.array(vals, dtype=float).reshape(3, 4)
raise ValueError("P2 not found")
```

**왜 P2 인가?**:
- P0=좌 grayscale, P1=우 grayscale, P2=**좌 color (image_2 가 이걸로 찍힘)**, P3=우 color.
- P2 는 이미 `K2 @ [R2 | t2]` 가 합쳐진 형태 + rectified 좌표계라
  R0_rect 추가 곱이 필요 없다. 행렬곱 한 번으로 cam → pixel.

**검증**: `print(P2)` 했을 때 `P2[:, :3]` 의 좌상단이 ~707 (초점거리 fx) 이어야 정상.

---

### 2-3. `compute_3d_corners` (L98)

박스의 8 코너를 **로컬 좌표(회전/이동 전)** 로 만들고 → 회전 → 평행이동 순서.

**로컬 코너 정의** (바닥 중심이 원점, y 가 아래):

```
인덱스 |  x   |  y  |  z
  0    | +l/2 |  0  | +w/2     ← 바닥 앞좌
  1    | +l/2 |  0  | -w/2     ← 바닥 앞우
  2    | -l/2 |  0  | -w/2     ← 바닥 뒤우
  3    | -l/2 |  0  | +w/2     ← 바닥 뒤좌
  4    | +l/2 | -h  | +w/2     ← 천장 앞좌
  5    | +l/2 | -h  | -w/2     ← 천장 앞우
  6    | -l/2 | -h  | -w/2     ← 천장 뒤우
  7    | -l/2 | -h  | +w/2     ← 천장 뒤좌
```

> **천장이 -h** 인 이유: y 가 아래라서 위로 가려면 음수.
> **0→1→2→3 순서가 EDGES 의 바닥 사각형과 짝**. 임의로 바꾸지 말 것.

**회전 행렬** (KITTI 의 ry 컨벤션):

```
        [ cos(ry)   0   sin(ry) ]
R_y =   [    0      1      0    ]
        [-sin(ry)   0   cos(ry) ]
```

일반 +y up 좌표계의 R_y 와 sin 부호가 반대다. y 가 아래라 시계방향이
뒤집힌 결과인데, 그냥 KITTI 표준으로 외우면 된다.

```python
corners_local = np.stack([x_corners, y_corners, z_corners], axis=0)  # (3,8)
corners_cam   = R @ corners_local + np.array(location).reshape(3, 1) # (3,8)
```

**검증**:
- `corners_cam.shape == (3, 8)`
- 8 코너의 z 평균이 `label.location[2]` 근처여야 함 (회전·축대칭이라).
- 천장 4 점의 y < 바닥 4 점의 y (y 가 아래라 천장이 더 작음).

---

### 2-4. `project_corners` (L144)

동차좌표 트릭. `P2 (3x4) @ [X; Y; Z; 1] = [u·z; v·z; z]` 이므로
앞 두 성분을 z 로 나누면 `(u, v)`.

```python
corners_h = np.vstack([corners_cam, np.ones((1, 8))])   # (4, 8)
proj      = P2 @ corners_h                              # (3, 8)
cam_z     = proj[2]                                     # (8,)  - 깊이
uv        = proj[:2] / cam_z                            # (2, 8)
visible   = cam_z > 0                                   # 카메라 뒤면 False
```

**왜 visible 마스크?**:
- `cam_z <= 0` 이면 카메라 뒤에 있는 점. 그냥 나누면 좌표가 뒤집혀
  엉뚱한 위치에 점이 찍힌다. 가시 코너만 모서리 그리기 / bbox 계산에 사용.

**검증**: 8개 모두 가시이고, `uv` 값이 이미지 크기(보통 ~1242 x 375) 안이면 정상.

---

### 2-5. `bbox_from_corners` (L173)

가시 코너의 (u, v) 의 min/max 가 그대로 박스.

```python
if not visible.any():
    raise ValueError("no visible corner")
uv_v = uv[:, visible]
return (float(uv_v[0].min()),  # x1
        float(uv_v[1].min()),  # y1
        float(uv_v[0].max()),  # x2
        float(uv_v[1].max()))  # y2
```

---

### 2-6. `bbox_iou` (L196)

표준 IoU. 교집합 면적 / 합집합 면적.

```python
ix1, iy1 = max(a[0], b[0]), max(a[1], b[1])
ix2, iy2 = min(a[2], b[2]), min(a[3], b[3])
iw, ih   = max(0.0, ix2 - ix1), max(0.0, iy2 - iy1)
inter    = iw * ih
area_a   = (a[2] - a[0]) * (a[3] - a[1])
area_b   = (b[2] - b[0]) * (b[3] - b[1])
return inter / (area_a + area_b - inter + 1e-9)
```

`max(0, ...)` 가 핵심 — 박스가 안 겹치면 음수 폭이 나오는데 0 으로 클램프.

**합격 기준** (PRACTICE.md):
- IoU > 0.7, 또는
- 모서리 별 max 픽셀 차이 < 5 px

KITTI 000000 의 첫 라벨(Pedestrian)은 보통 IoU 0.8 이상 나온다.

---

### 2-7. `draw_box_3d` (L223)

`EDGES` 가 이미 정의되어 있으니 그대로 순회 + `cv2.line` 호출.

```python
for i, j in EDGES:
    if visible[i] and visible[j]:
        p1 = (int(uv[0, i]), int(uv[1, i]))
        p2 = (int(uv[0, j]), int(uv[1, j]))
        cv2.line(img_bgr, p1, p2, color, thickness)
return img_bgr
```

`visible[i] and visible[j]` 조건은 한쪽 끝이라도 카메라 뒤면 모서리 안 그림.

---

### 2-8. `log_to_rerun` (L253)

Rerun 은 "엔티티 경로" 트리로 데이터를 쌓는다. 같은 prefix(`image/...`)는
같은 뷰포트에 겹쳐 그려준다.

```python
rr.log("image", rr.Image(img_rgb))                              # 원본
rr.log("image/proj_corners",
       rr.Points2D(uv.T, radii=3, colors=[(0, 255, 0)]))         # 8 코너
rr.log("image/gt_bbox",
       rr.Boxes2D(array=np.array([gt_bbox]),
                  array_format=rr.Box2DFormat.XYXY,
                  colors=[(255, 0, 0)]))                          # 정답 박스

K = P2[:, :3]                          # P2 의 앞 3 컬럼이 K · R (rectified 라 R=I)
H, W = img_rgb.shape[:2]
rr.log("world/cam",
       rr.Pinhole(image_from_camera=K, width=W, height=H))
rr.log("world/box_corners",
       rr.Points3D(corners_cam.T, radii=0.05))
```

> `uv.T`, `corners_cam.T`: Rerun 은 점 N 개를 `(N, dim)` 으로 받는다.
> 우리는 `(dim, N)` 으로 가지고 있어 transpose 필요.

**원격 환경**: `spawn=True` 는 로컬 viewer 를 띄움. SSH/Tunnel 이면
```python
rr.init("kitti_step4", spawn=False)
rr.save("kitti_step4.rrd")   # 파일로 저장 → 로컬에서 열기
```
로 바꾸는 게 편하다.

---

## 3. 구현 순서 추천

`main()` 이 호출하는 순서대로 채우면 한 번에 하나씩 검증된다:

1. **`parse_calib_p2`** → `print(P2.shape, P2)` 로 확인
2. **`parse_label_line`** → `print(labels[0])` 로 필드 값 확인
3. **`compute_3d_corners`** → `print(corners_cam.shape)`, 8 코너의 z 평균이 location.z 근처인지
4. **`project_corners`** → `print(visible.sum(), uv.min(), uv.max())`, 이미지 안에 들어가는지
5. **`bbox_from_corners`** + **`bbox_iou`** → IoU 출력, 0.7 넘기는지 확인
6. **`draw_box_3d`** → `output/kitti_step4_000000.png` 열어서 박스 모양 확인
7. **`log_to_rerun`** → Rerun viewer/RRD 확인 (옵션)

**1, 2 까지만 짜고 한 번 돌려보면** `parse_calib_p2` 에러는 사라지고
`compute_3d_corners` 에서 다시 `NotImplementedError` 가 난다.
이런 식으로 한 단계씩 전진하면 디버깅이 쉽다.

---

## 4. 흔한 함정 체크리스트

- [ ] `dim` 순서는 `(h, w, l)`. `(l, w, h)` 아님.
- [ ] 천장 y 좌표가 `+h` 가 아니라 `-h` (y 가 아래).
- [ ] `R_y` 의 sin 부호가 일반 좌표계와 반대.
- [ ] `project_corners` 에서 z 가 음수인 점은 visible=False (나누면 좌표 뒤집힘).
- [ ] Rerun 에 점 넘길 때 `(N, dim)` 으로 transpose.
- [ ] IoU 계산 시 `max(0, ...)` 클램프 빠뜨리지 말기.

---

## 5. 합격 출력 예시 (참고)

```
[000000] Pedestrian  dim=(1.89, 0.48, 1.2)  loc=(1.84, 1.47, 8.41)  ry=-0.010
가시 코너: 8 / 8
proj_bbox: (712.xx, 142.xx, 810.xx, 308.xx)
gt_bbox  : (712.4, 143.0, 810.73, 307.92)
IoU      : 0.98xx   (>0.7 합격)
max edge diff: ~2 px (<5 px 합격)
저장: output/kitti_step4_000000.png
```

이 값이 안 나오면 위 함정 체크리스트로 돌아가 어디서 어긋났는지 확인.
