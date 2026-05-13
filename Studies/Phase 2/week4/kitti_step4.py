"""Phase 2 Week 4 - Step 4: KITTI 3D Object Detection 재투영 (스켈레톤).

목표:
    label_2 의 3D 박스를 calib P2 로 image_2 위에 다시 그리고,
    label_2 의 2D bbox 와 정합도 (IoU 또는 모서리 픽셀 거리) 로 평가.
    Rerun 으로 2D + 3D 동시 시각화.

선행 조건:
    /datasets/kitti_object/training/{image_2, calib, label_2}/000000.{png, txt, txt}
    pip install rerun-sdk opencv-python numpy   (이미 설치됨)
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
    # TODO:
    #   - docstring 의 컬럼 표를 보고 슬라이스 인덱스를 정할 것
    #   - split 결과는 모두 문자열 → 숫자 필드는 float 으로 캐스팅
    #   - bbox / dim / location 은 각각 4, 3, 3 개 값으로 묶이는지 길이 확인
    #   - KittiLabel 은 dataclass 이므로 위치 인자 순서대로 한 번에 생성 가능
    raise NotImplementedError


# ─── Step 4-2: calib P2 파싱 ──────────────────────────────────────────
def parse_calib_p2(calib_path: Path) -> np.ndarray:
    """calib/{frame}.txt 의 P2 행렬을 (3, 4) ndarray 로 반환한다.

    파일 한 줄 예:
        P2: 7.07e+02 0 6.04e+02 4.57e+01 0 7.07e+02 1.81e+02 -3.45e-01 0 0 1 4.98e-03

    Args:
        calib_path: calib 텍스트 파일 경로.

    Returns:
        (3, 4) numpy 배열.

    Note:
        P2 = K2 @ [R2 | t2] 가 이미 곱해진 cam2 (left color) 투영행렬.
        rectified 좌표계라 R0_rect 추가 곱셈 불필요.
    """
    # TODO:
    #   - 파일을 줄 단위로 훑어 "P2:" 라벨로 시작하는 줄만 골라낼 것
    #   - 콜론 뒤 토큰 12 개를 numpy 배열로 만들고 (3, 4) 로 reshape
    #   - 못 찾았을 때는 조용히 None 대신 예외를 던지는 편이 디버깅에 유리
    raise NotImplementedError


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
    # TODO:
    #   1) 회전/이동을 적용하기 전 *로컬 박스* 8 점을 만든다
    #      - 부피 축: x 는 +-l/2, z 는 +-w/2 부호 조합으로 4 가지
    #      - y 는 바닥 4 점 = 0, 천장 4 점 = -h (location 이 바닥 중심이므로)
    #      - 인덱스 순서는 docstring 의 '앞좌-앞우-뒤우-뒤좌' 를 지킬 것
    #        (EDGES 가 이 순서를 가정한다 → 어긋나면 모서리가 X 자로 꼬임)
    #   2) docstring 에 있는 KITTI 컨벤션 R_y 로 회전 적용
    #      - shape: (3,3) @ (3,8) = (3,8). np.stack 으로 점을 열-축에 쌓을 것
    #   3) location 을 더해 world 위치로 평행이동
    #      - (3,8) 에 (3,) 를 더하려면 (3,1) 로 reshape 해 broadcasting
    raise NotImplementedError


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
    # TODO:
    #   - (3, 8) 코너 아래에 1 행을 붙여 동차좌표 (4, 8) 로 확장
    #     (vstack / concatenate 등 어떤 방법이든 OK)
    #   - P2 @ corners_h = (3, 8). 세 번째 행이 cam 의 +z = 깊이
    #   - u, v 는 처음 두 행을 깊이로 나누는 perspective divide 로 구한다
    #     (broadcasting: (2,8) / (8,) 는 자동으로 행 단위로 나눠짐)
    #   - 가시성은 depth > 0 만 검사 (시야각 안인지는 호출자에서 추가 검증 가능)
    raise NotImplementedError


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
    # TODO:
    #   - 가시 코너가 0 개면 박스를 정의할 수 없으니 ValueError
    #   - boolean 인덱싱으로 visible 한 코너만 골라내고
    #     u 행과 v 행 각각의 min / max 를 추출하면 그대로 (x1, y1, x2, y2)
    #   - 반환은 plain float tuple. numpy 스칼라가 섞이면 IoU 계산 시 dtype 사고 잦음
    raise NotImplementedError


def bbox_iou(
    a: Tuple[float, float, float, float], b: Tuple[float, float, float, float]
) -> float:
    """두 축정렬 박스의 IoU (0 ~ 1).

    합격 기준 (PRACTICE.md):
        IoU > 0.7 또는 max corner diff < 5 px 면 정합도 양호.
    """
    # TODO:
    #   - 교집합 박스 좌상단 = 두 박스 좌상단의 *큰 쪽*
    #     교집합 박스 우하단 = 두 박스 우하단의 *작은 쪽*
    #   - 폭/높이가 음수면 두 박스가 안 겹친 것 → 0 으로 clip
    #   - IoU = inter / (areaA + areaB - inter)
    #     (분모가 0 인 케이스는 호출 측에서 막혔다고 가정)
    raise NotImplementedError


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
    # TODO:
    #   - 위에 정의된 EDGES (12 모서리의 코너 인덱스 쌍) 을 순회
    #   - 양 끝 코너가 *둘 다 visible* 일 때만 그린다
    #     (한 쪽만 보이는 모서리를 그리면 화면 밖으로 뻗는 잘못된 선이 나옴)
    #   - cv2.line 의 좌표 인자는 int tuple → uv 값을 캐스팅해 전달
    raise NotImplementedError


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
    # TODO:
    #   2D 트리 (image/...) - 위 docstring 의 엔티티 구조와 1:1 대응:
    #     - 원본 RGB 이미지를 rr.Image 로 로깅
    #     - uv 8 코너를 Points2D 로. rr 의 Points2D 는 (N, 2) 모양을 요구하므로
    #       (2, N) 인 uv 는 전치해서 넘긴다 (color 는 녹)
    #     - 정답 2D bbox 는 Boxes2D 로. XYXY 포맷을 명시할 것 (적)
    #
    #   3D 트리 (world/...):
    #     - rectified P2 는 K | [R0|t] 가 합쳐진 형태 → 앞 3 열이 곧 K
    #     - 이미지 해상도는 img_rgb.shape 의 (H, W). Pinhole 인자 순서 주의
    #     - corners_cam 도 Points3D 로 같이 찍어두면 2D 와 3D 정합을 눈으로 비교 가능
    raise NotImplementedError


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
    print(f"[{SAMPLE_ID}] {label.type_}  dim={label.dim}  "
          f"loc={label.location}  ry={label.rotation_y:.3f}")

    corners_cam = compute_3d_corners(label)
    uv, visible = project_corners(corners_cam, P2)
    print(f"가시 코너: {visible.sum()} / 8")

    proj_bbox = bbox_from_corners(uv, visible)
    iou = bbox_iou(proj_bbox, label.bbox_2d)
    diffs = np.abs(np.array(proj_bbox) - np.array(label.bbox_2d))
    print(f"proj_bbox: ({proj_bbox[0]:.2f}, {proj_bbox[1]:.2f}, "
          f"{proj_bbox[2]:.2f}, {proj_bbox[3]:.2f})")
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
