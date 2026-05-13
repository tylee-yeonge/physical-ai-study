"""Phase 2 Week 4 - Step 4: KITTI 3D Object Detection 재투영 (정답 코드).

스켈레톤 (kitti_step4.py) 의 reference 구현이다.
구조/시그니처/docstring 은 스켈레톤과 동일하며, TODO 본문만 채워졌다.
막힐 때 함수 단위로 비교하는 용도로 사용한다.
"""

from dataclasses import dataclass
from pathlib import Path
from typing import List
from typing import Tuple

import cv2
import numpy as np
import rerun as rr


# ─── 경로 ───
KITTI_ROOT = Path("/datasets/kitti_object/training")
SAMPLE_ID = "000000"


@dataclass
class KittiLabel:
    """KITTI label_2 한 줄을 파싱한 결과.

    Attributes:
        type_: 객체 종류 (Car, Pedestrian, ...).
        bbox_2d: (x1, y1, x2, y2) 2D 박스 (검증용 정답).
        dim: (h, w, l) 3D 박스 크기 [m]. 순서 주의 — height 가 첫 번째.
        location: (x, y, z) 3D 박스의 *바닥 중심* [m, cam coord].
        rotation_y: cam coord y축 (아래쪽) 기준 회전각 [rad].
    """

    type_: str
    bbox_2d: Tuple[float, float, float, float]
    dim: Tuple[float, float, float]
    location: Tuple[float, float, float]
    rotation_y: float


# ─── Step 4-1: 라벨 파싱 ──────────────────────────────────────────────
def parse_label_line(line: str) -> KittiLabel:
    """label_2 의 한 줄을 KittiLabel 로 파싱한다.

    한 줄 포맷 (공백 구분, 15 컬럼; 평가 결과는 16번째에 score 가 붙기도 함):
        type trunc occl alpha x1 y1 x2 y2 h w l x y z ry [score]
        [0]  [1]   [2]  [3]  [4][5][6][7][8][9][10][11][12][13][14]

    Args:
        line: label_2/{frame}.txt 의 한 줄.

    Returns:
        KittiLabel 인스턴스.

    Note:
        DontCare 행도 같은 포맷이지만 좌표 일부가 -1. 호출 측에서 type 으로 거른다.
    """
    tokens = line.split()
    return KittiLabel(
        type_=tokens[0],
        bbox_2d=tuple(float(t) for t in tokens[4:8]),
        dim=tuple(float(t) for t in tokens[8:11]),
        location=tuple(float(t) for t in tokens[11:14]),
        rotation_y=float(tokens[14]),
    )


# ─── Step 4-2: calib P2 파싱 ──────────────────────────────────────────
def parse_calib_p2(calib_path: Path) -> np.ndarray:
    """calib/{frame}.txt 의 P2 행렬을 (3, 4) ndarray 로 반환한다.

    파일 한 줄 예:
        P2: 7.07e+02 0 6.04e+02 4.57e+01 0 7.07e+02 1.81e+02 -3.45e-01 0 0 1 4.98e-03

    Args:
        calib_path: calib 텍스트 파일 경로.

    Returns:
        (3, 4) numpy 배열.

    Raises:
        ValueError: 파일에서 P2 라인을 찾지 못한 경우.

    Note:
        P2 = K2 @ [R2 | t2] 가 이미 곱해진 cam2 (left color) 투영행렬.
        rectified 좌표계라 R0_rect 추가 곱셈 불필요.
    """
    for line in calib_path.read_text().splitlines():
        if line.startswith("P2:"):
            values = line.split()[1:]
            return np.array(values, dtype=float).reshape(3, 4)
    raise ValueError(f"P2 line not found in {calib_path}")


# ─── Step 4-3: 3D 박스 → 8 코너 (cam coord) ───────────────────────────
def compute_3d_corners(label: KittiLabel) -> np.ndarray:
    """라벨의 3D 박스를 cam coord 의 8 코너로 변환한다.

    KITTI 컨벤션 (외우면 디버깅이 쉬움):
        - cam coord 는 +x 오른쪽, +y 아래, +z 앞 (OpenCV 와 동일).
        - location 은 박스의 *바닥 중심*. 따라서 천장은 location.y - h.
        - rotation_y 는 y 축 (아래) 기준.

    Args:
        label: KittiLabel.

    Returns:
        (3, 8) cam coord 코너. 인덱스 순서:
            0..3 : 바닥 4 (앞좌, 앞우, 뒤우, 뒤좌)
            4..7 : 천장 4 (앞좌, 앞우, 뒤우, 뒤좌)
        EDGES 와 일관성 유지를 위해 이 순서를 바꾸지 말 것.

    Note:
        ry 의 회전 행렬은 일반적인 R_y 와 부호가 다르다.
        cam coord 의 y 가 *아래* 이기 때문 (오른손 좌표계 + y 가 아래 = 시계방향이 뒤집힘).
        다음 형태가 KITTI 컨벤션:
            R_y = [[ cos(ry), 0, sin(ry)],
                   [       0, 1,       0],
                   [-sin(ry), 0, cos(ry)]]
    """
    h, w, l = label.dim
    ry = label.rotation_y

    # 로컬 박스 (회전/이동 적용 전). 바닥 = y=0, 천장 = y=-h
    x_corners = np.array([ l / 2,  l / 2, -l / 2, -l / 2,  l / 2,  l / 2, -l / 2, -l / 2])
    y_corners = np.array([    0,      0,      0,      0,    -h,     -h,     -h,     -h])
    z_corners = np.array([ w / 2, -w / 2, -w / 2,  w / 2,  w / 2, -w / 2, -w / 2,  w / 2])

    c, s = np.cos(ry), np.sin(ry)
    rot_y = np.array([[ c, 0, s],
                      [ 0, 1, 0],
                      [-s, 0, c]])

    corners = rot_y @ np.stack([x_corners, y_corners, z_corners], axis=0)  # (3, 8)
    corners += np.array(label.location).reshape(3, 1)
    return corners


# ─── Step 4-4: 재투영 ──────────────────────────────────────────────────
def project_corners(
    corners_cam: np.ndarray, P2: np.ndarray
) -> Tuple[np.ndarray, np.ndarray]:
    """8 코너를 P2 로 재투영해 픽셀 좌표 + 가시성 마스크 반환.

    Args:
        corners_cam: (3, 8) cam coord 코너.
        P2: (3, 4) 투영행렬.

    Returns:
        uv: (2, 8) 픽셀 좌표.
        visible: (8,) bool. cam_z > 0 인 코너만 True.

    Note:
        OpenCV solvePnP / projectPoints 안 써도 됨.
        rectified P2 는 이미 K · [R|t] 가 합쳐져 있어 한 번의 행렬곱으로 충분.
    """
    ones = np.ones((1, corners_cam.shape[1]))
    corners_h = np.vstack([corners_cam, ones])  # (4, 8)
    proj = P2 @ corners_h                        # (3, 8)
    cam_z = proj[2]                              # (8,) 카메라 +z = 깊이
    uv = proj[:2] / cam_z                        # perspective divide → (2, 8)
    visible = cam_z > 0
    return uv, visible


# ─── Step 4-5: 정합도 평가 ────────────────────────────────────────────
def bbox_from_corners(
    uv: np.ndarray, visible: np.ndarray
) -> Tuple[float, float, float, float]:
    """가시 코너의 축정렬 박스 (umin, vmin, umax, vmax).

    Args:
        uv: (2, 8).
        visible: (8,).

    Returns:
        (x1, y1, x2, y2).

    Raises:
        ValueError: 가시 코너가 0 개일 때.
    """
    if not visible.any():
        raise ValueError("no visible corner")
    uv_v = uv[:, visible]
    return (
        float(uv_v[0].min()),
        float(uv_v[1].min()),
        float(uv_v[0].max()),
        float(uv_v[1].max()),
    )


def bbox_iou(
    a: Tuple[float, float, float, float], b: Tuple[float, float, float, float]
) -> float:
    """두 축정렬 박스의 IoU (0 ~ 1).

    합격 기준 (PRACTICE.md):
        IoU > 0.7 또는 max corner diff < 5 px 면 정합도 양호.
    """
    ix1, iy1 = max(a[0], b[0]), max(a[1], b[1])
    ix2, iy2 = min(a[2], b[2]), min(a[3], b[3])
    iw, ih = max(0.0, ix2 - ix1), max(0.0, iy2 - iy1)
    inter = iw * ih
    area_a = (a[2] - a[0]) * (a[3] - a[1])
    area_b = (b[2] - b[0]) * (b[3] - b[1])
    return inter / (area_a + area_b - inter)


# ─── Step 4-6: 이미지에 12 모서리 그리기 ──────────────────────────────
EDGES = [
    (0, 1), (1, 2), (2, 3), (3, 0),  # 바닥 사각형
    (4, 5), (5, 6), (6, 7), (7, 4),  # 천장 사각형
    (0, 4), (1, 5), (2, 6), (3, 7),  # 수직 4 모서리
]


def draw_box_3d(
    img_bgr: np.ndarray,
    uv: np.ndarray,
    visible: np.ndarray,
    color: Tuple[int, int, int] = (0, 255, 0),
    thickness: int = 2,
) -> np.ndarray:
    """이미지 위에 12 모서리를 그려 반환.

    Args:
        img_bgr: BGR 이미지 (in-place 변경).
        uv: (2, 8).
        visible: (8,).
        color: BGR.
        thickness: 선 두께.

    Returns:
        그려진 이미지 (참조).
    """
    for i, j in EDGES:
        # 양 끝 코너가 모두 보일 때만 그려야 화면 밖으로 뻗는 잘못된 선이 생기지 않는다
        if visible[i] and visible[j]:
            p1 = (int(uv[0, i]), int(uv[1, i]))
            p2 = (int(uv[0, j]), int(uv[1, j]))
            cv2.line(img_bgr, p1, p2, color, thickness)
    return img_bgr


# ─── Step 4-7: Rerun 시각화 ───────────────────────────────────────────
def log_to_rerun(
    img_rgb: np.ndarray,
    P2: np.ndarray,
    corners_cam: np.ndarray,
    uv: np.ndarray,
    gt_bbox: Tuple[float, float, float, float],
) -> None:
    """Rerun 으로 2D (이미지 평면) + 3D (cam coord) 동시 로깅.

    엔티티 트리:
        image                 : 원본 이미지
        image/proj_corners    : 8 코너 재투영 (Points2D, 녹)
        image/gt_bbox         : 2D bbox (정답, 적)
        world/cam             : Pinhole (K, W, H)
        world/box_corners     : 8 코너 3D (Points3D)

    Note:
        spawn=True 로 init 하면 viewer 가 자동 실행되지만
        원격 (Tunnel) 환경에서는 rr.serve_web() 또는 rr.save("xxx.rrd") 가 더 편함.
    """
    rr.log("image", rr.Image(img_rgb))
    rr.log(
        "image/proj_corners",
        rr.Points2D(uv.T, radii=3, colors=[(0, 255, 0)]),
    )
    rr.log(
        "image/gt_bbox",
        rr.Boxes2D(
            array=np.array([gt_bbox]),
            array_format=rr.Box2DFormat.XYXY,
            colors=[(255, 0, 0)],
        ),
    )

    # rectified P2 = K · [R|t] 이므로 앞 3 열이 곧 K
    K = P2[:, :3]
    height, width = img_rgb.shape[:2]
    rr.log(
        "world/cam",
        rr.Pinhole(image_from_camera=K, width=width, height=height),
    )
    rr.log("world/box_corners", rr.Points3D(corners_cam.T, radii=0.05))


# ─── 메인 파이프라인 ──────────────────────────────────────────────────
def load_labels(label_path: Path) -> List[KittiLabel]:
    """label_2 파일에서 DontCare 제외 라벨 리스트 반환."""
    labels = []
    for line in label_path.read_text().splitlines():
        if not line.strip() or line.startswith("DontCare"):
            continue
        labels.append(parse_label_line(line))
    return labels


def main() -> None:
    """000000 샘플 한 장으로 Step 4 파이프라인을 실행한다."""
    img_path = KITTI_ROOT / "image_2" / f"{SAMPLE_ID}.png"
    calib_path = KITTI_ROOT / "calib" / f"{SAMPLE_ID}.txt"
    label_path = KITTI_ROOT / "label_2" / f"{SAMPLE_ID}.txt"

    img_bgr = cv2.imread(str(img_path))
    if img_bgr is None:
        raise FileNotFoundError(f"image not found: {img_path}")

    P2 = parse_calib_p2(calib_path)
    labels = load_labels(label_path)
    if not labels:
        print(f"[{SAMPLE_ID}] 유효 라벨 없음")
        return

    label = labels[0]
    print(
        f"[{SAMPLE_ID}] {label.type_}  dim={label.dim}  "
        f"loc={label.location}  ry={label.rotation_y:.3f}"
    )

    corners_cam = compute_3d_corners(label)
    uv, visible = project_corners(corners_cam, P2)
    print(f"가시 코너: {visible.sum()} / 8")

    proj_bbox = bbox_from_corners(uv, visible)
    iou = bbox_iou(proj_bbox, label.bbox_2d)
    diffs = np.abs(np.array(proj_bbox) - np.array(label.bbox_2d))
    print(
        f"proj_bbox: ({proj_bbox[0]:.2f}, {proj_bbox[1]:.2f}, "
        f"{proj_bbox[2]:.2f}, {proj_bbox[3]:.2f})"
    )
    print(f"gt_bbox  : {label.bbox_2d}")
    print(f"IoU      : {iou:.4f}   (>0.7 합격)")
    print(f"max edge diff: {diffs.max():.2f} px   (<5 px 합격)")

    out_img = draw_box_3d(img_bgr.copy(), uv, visible)
    out_path = Path("output") / f"kitti_step4_{SAMPLE_ID}.png"
    out_path.parent.mkdir(exist_ok=True)
    cv2.imwrite(str(out_path), out_img)
    print(f"저장: {out_path}")

    # Rerun (옵션). spawn=False + save 로 RRD 파일만 만들고 싶으면
    # rr.init("kitti_step4", spawn=False); rr.save("kitti_step4.rrd")
    rr.init("kitti_step4", spawn=True)
    log_to_rerun(
        cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB),
        P2,
        corners_cam,
        uv,
        label.bbox_2d,
    )


if __name__ == "__main__":
    main()
