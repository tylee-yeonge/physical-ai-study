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
    #   포맷 다시 한번 (총 15 컬럼, score 가 붙으면 16):
    #       type  trunc  occl  alpha  x1 y1 x2 y2  h w l  x y z  ry  [score]
    #       [0]   [1]    [2]   [3]    [4][5][6][7] [8][9][10][11][12][13][14]
    #
    #   구현 절차:
    #   1) 한 줄을 토큰 list 로 분해
    #      - line.split() (공백 여러 개도 자동 처리) 또는 line.strip().split(" ")
    #      - 결과는 모두 str → 숫자 필드는 float() 캐스팅 필요
    #
    #   2) 필드별 인덱스 매핑
    #      - type_      : tokens[0]  (문자열 그대로 유지)
    #      - bbox_2d    : tokens[4:8]  (x1, y1, x2, y2) — 4 개
    #      - dim        : tokens[8:11] (h, w, l)         — 3 개, *높이가 첫 번째* 임에 주의
    #                     (대부분의 다른 데이터셋은 l, w, h 순서라 헷갈리기 쉽다)
    #      - location   : tokens[11:14] (x, y, z)        — 3 개, cam coord 의 *바닥 중심*
    #      - rotation_y : tokens[14]                     — 1 개, radian
    #      - tokens[1:4] (trunc / occl / alpha) 는 이번 단계에선 안 씀 (필터링용)
    #
    #   3) 슬라이스를 적절한 자료형으로 묶기
    #      - bbox/dim/location 은 list-of-str → list-of-float → tuple(...) 로 굳히기
    #        tuple 로 굳혀두면 KittiLabel 의 hash 안정성 + 실수 방지
    #      - 길이 검증 (len == 4 / 3 / 3) 을 assert 한 줄로 두면 파싱 실수 즉시 발견
    #
    #   4) KittiLabel 생성
    #      - dataclass 라 위치 인자 (type_, bbox_2d, dim, location, rotation_y) 순서대로
    #        한 번에 만들 수 있다. 가독성 위해 키워드 인자도 OK.
    #
    #   주의:
    #   - 라인이 비어있거나 'DontCare' 인 경우는 호출 측 load_labels 에서 이미 걸러진다
    #     → 이 함수는 항상 유효한 한 줄을 받는다고 가정해도 안전
    #   - score 필드 (tokens[15]) 가 있는 detection 결과 파일도 같은 파서로 읽고 싶다면
    #     tokens 길이 체크 후 무시하는 식으로 확장 가능 (이번 task 에선 ground truth 만 처리)
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
    #   calib 파일 구조 (한 파일에 여러 행렬이 라벨 prefix 와 함께):
    #       P0: ... (12 개 float)
    #       P1: ... (12 개)
    #       P2: ... (12 개)  ← 이번에 필요한 것 (cam2, left color)
    #       P3: ... (12 개)
    #       R0_rect: ... (9 개)
    #       Tr_velo_to_cam: ... (12 개)
    #       Tr_imu_to_velo: ... (12 개)
    #
    #   구현 절차:
    #   1) 파일 읽기
    #      - calib_path.read_text().splitlines() 로 줄 단위 list 획득
    #      - 또는 .read_text().split("\n") (마지막 빈 줄 처리 차이만 있다)
    #
    #   2) "P2:" 로 시작하는 줄만 골라내기
    #      - line.startswith("P2:") 조건이 가장 안전 (P20, P21 같은 변종도 없으니)
    #      - 못 찾으면 raise (예: ValueError(f"P2 not found in {calib_path}"))
    #        조용히 None 반환하면 호출 측에서 다음 단계까지 가서야 터져 디버깅이 어렵다
    #
    #   3) 콜론 뒤 숫자 12 개를 ndarray 로 변환
    #      - line.split(":", 1)[1] 로 prefix 제거 후 다시 split() 으로 토큰화
    #      - np.fromstring 도 가능하지만 deprecate 됐으므로 np.array([float(t) for t in tokens])
    #        또는 np.asarray(tokens, dtype=np.float64) 가 무난
    #      - 길이 검증: len(tokens) == 12 인지 assert (P2 가 아닌 다른 줄을 잘못 잡았는지 점검)
    #
    #   4) (3, 4) 로 reshape
    #      - row-major 가 KITTI 컨벤션 (P2 의 12 개 값이 [p00 p01 p02 p03 p10 p11 ... p23] 순서)
    #      - numpy 기본 reshape 가 곧 row-major (C order) 이므로 .reshape(3, 4) 한 번이면 끝
    #
    #   주의:
    #   - rectified KITTI 의 P2 는 K2 @ [R | t] 가 이미 곱해진 (3, 4) — K 만 따로 분리할 필요 없음
    #   - dtype 은 float64 가 안전. cv2 와 자연스럽게 호환
    #   - 같은 패턴으로 P0, P3, R0_rect 도 파싱 가능 → 나중에 다른 캠 확장 시 함수 시그니처를
    #     parse_calib(calib_path, key="P2") 식으로 일반화하기 쉬움 (이번 task 는 P2 만)
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
    #   변환 흐름:
    #       local box (객체 중심 좌표) --R_y@--> rotated box --+location--> cam coord box
    #     로컬 박스는 객체의 길이/너비/높이 축에 정렬된 표준 박스 (회전·이동 X).
    #     여기에 ry 회전을 먹이고 위치를 더하면 cam coord 의 실제 박스가 된다.
    #
    #   1) 로컬 박스 8 점 만들기
    #      - 박스 크기는 (h, w, l). KITTI 의 축 매핑:
    #            x 축 (오른쪽)  ← 길이 l (전후)
    #            y 축 (아래)    ← 높이 h
    #            z 축 (앞)      ← 너비 w (좌우)
    #        헷갈리기 쉬운 포인트: dim 의 첫 번째가 h. l, w 순서는 그 뒤.
    #
    #      - 바닥 4 점 (y = 0, *바닥 중심이 원점* 이므로 바닥은 0):
    #            앞좌 ( +l/2, 0, +w/2 )
    #            앞우 ( +l/2, 0, -w/2 )
    #            뒤우 ( -l/2, 0, -w/2 )
    #            뒤좌 ( -l/2, 0, +w/2 )
    #      - 천장 4 점 (y = -h, KITTI cam coord 의 y 가 *아래* 라 천장이 음수):
    #            바닥 4 점에서 y 만 0 → -h 로 바꾼 동일한 (x, z) 순서
    #      - 인덱스 0..7 순서 (앞좌, 앞우, 뒤우, 뒤좌, 그리고 천장도 같은 순서)는
    #        절대 바꾸지 말 것. EDGES = [(0,1),(1,2),...] 가 이 순서를 가정하므로
    #        어긋나면 모서리가 대각선/X 자로 꼬여 그려진다.
    #      - 만들기 편한 방법: np.array([[x0,y0,z0], ..., [x7,y7,z7]]).T 로 (3, 8) 구성
    #
    #   2) ry 회전 적용
    #      - docstring 에 명시된 KITTI R_y (y 가 아래라 부호가 일반 R_y 와 다름):
    #            R_y = [[ cos,  0, sin],
    #                   [   0,  1,   0],
    #                   [-sin,  0, cos]]
    #      - 회전 행렬은 (3, 3), 로컬 박스가 (3, 8) → R_y @ local_box = (3, 8)
    #      - np.cos / np.sin 은 radian 입력. label.rotation_y 가 이미 radian 이라 변환 불필요
    #
    #   3) location 평행이동
    #      - location 은 (x, y, z) tuple → np.array(location).reshape(3, 1) 로 (3, 1) 화
    #      - (3, 8) + (3, 1) = (3, 8) broadcasting 으로 모든 코너에 동시에 더해진다
    #      - (3,) 그대로 더하면 (3, 8) 과 broadcasting rule 이 안 맞아 ValueError
    #
    #   디버깅 / 시각화 팁:
    #   - 천장이 *위로 솟지 않고* 아래로 가는 게 정상 (KITTI cam coord 는 y 가 아래)
    #   - 결과 점들의 평균이 location 과 *동일하지 않다* (location 은 바닥 중심,
    #     8 코너 평균은 박스 *부피 중심* 으로 y 방향만 h/2 위쪽으로 차이)
    #   - ry=0 이면 박스의 길이 방향이 정확히 +x 축에 정렬 (앞 = +x). ry 가 90도면
    #     길이 방향이 +z 로 회전 → 차량이 카메라 정면을 향함
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

        P2 = K · [R | t] 가 이미 합쳐진 형태:
            KITTI 의 P2 는 단순한 K 가 아니라 K 와 외부 파라미터 [R | t] 가
            곱해진 결과. 그래서 fx, fy, cx, cy 가 행렬 안에 섞여서 들어있다.

                K = [[fx,  0, cx],
                     [ 0, fy, cy],
                     [ 0,  0,  1]]
                [R | t] = (3, 4)
                P2 = K @ [R | t]  → (3, 4)

            rectified 좌표계에서는 cam0, cam2 가 동일 평면 + 광축 평행이라
            R = I (단위행렬) 이고 baseline 평행이동만 t 에 들어간다. 따라서
                P2 = K @ [I | t] = [K | K·t]
            즉 P2 의 좌상단 3x3 은 *수학적으로 정확히* K 자체이고,
            마지막 컬럼이 K · t (cam0 → cam2 baseline 보정) 이다.

            실제 KITTI 000000 의 P2:

                P2 = [[707.0,     0, 604.0,  45.7],   # 좌3x3 = K  → fx=707, cx=604
                      [    0, 707.0, 181.0,  -0.3],   #            fy=707, cy=181
                      [    0,     0,   1.0, 0.005]]   # 마지막 컬럼 = K·t

            검산: cam2 의 baseline tx ≈ -0.065 m 라면
                fx·tx ≈ 707 · 0.065 ≈ 45.96  →  P2[0, 3] = 45.7 과 일치.
            이게 우연이 아니라 P2 = K @ [I | t] 분해의 직접적인 결과다.

        왜 P2 @ corners_h 한 번이면 핀홀 공식이 다 풀리는가:
            P2 = K · [R | t] 이고, 동차좌표 (X, Y, Z, 1) 을 곱하면
                P2 @ [X, Y, Z, 1]^T
                  = K @ [R @ (X,Y,Z) + t]
                  = K @ (X', Y', Z')           # cam2 좌표로 미세 보정된 점
                  = [fx·X' + cx·Z',
                     fy·Y' + cy·Z',
                            Z'     ]
            여기서 세 번째 성분 Z' 로 나누면 (perspective divide)
                u = fx·X'/Z' + cx
                v = fy·Y'/Z' + cy
            즉 핀홀 공식 u = fx·X/Z + cx, v = fy·Y/Z + cy 가 자동 등장.
            fx/fy/cx/cy 를 직접 꺼내지 않아도 P2 안에 다 포함되어 있다.

        rectified KITTI 의 [R | t] 는 cam0 → cam2 (left color) 미세 보정이고,
        baseline 으로 인한 평행이동 (~수십 cm) 을 자동 반영해준다.
        그래서 K 를 직접 빼서 쓰는 것보다 P2 전체를 곱하는 쪽이 더 정확.
    """
    # TODO:
    #   구현 절차:
    #   1) 동차좌표화: (3, 8) → (4, 8)
    #      - 코너 행렬 아래에 길이 8 의 ones 행을 한 줄 붙인다
    #      - np.vstack([corners_cam, np.ones((1, 8))])
    #        또는 np.concatenate([corners_cam, np.ones((1, corners_cam.shape[1]))], axis=0)
    #      - shape 끝 차원 8 을 하드코딩하기보다 corners_cam.shape[1] 로 잡아두면
    #        나중에 코너 수가 바뀌어도 동작 (조심: 본 task 는 항상 8)
    #
    #   2) P2 곱하기: pixel_h = P2 (3, 4) @ corners_h (4, 8) → (3, 8)
    #      - row 0 = fx·X' + cx·Z'   = u 후보 (아직 Z' 로 안 나눔)
    #      - row 1 = fy·Y' + cy·Z'   = v 후보
    #      - row 2 = Z'              = depth (cam2 보정 후 z)
    #
    #   3) perspective divide
    #      - depth = pixel_h[2]            (shape (8,))
    #      - uv (2, 8) = pixel_h[:2] / depth
    #          → numpy broadcasting 규칙: (2, 8) / (8,) 은 행마다 자동 나눔
    #      - 결과는 float. 정수 캐스팅은 그리기 단계에서만 한다
    #
    #   4) 가시성 판정
    #      - visible = depth > 0  (shape (8,) bool)
    #      - 카메라 *뒤* 의 코너는 픽셀이 무한대로 튀거나 부호 반전이 일어나
    #        이미지 평면 어디에 그려도 잘못된 결과. depth <= 0 코너는 제외.
    #      - 화면 밖 (u, v 가 [0, W) x [0, H) 밖) 은 본 함수에선 검사하지 않음
    #        (호출자가 필요시 추가). 박스의 일부가 잘려도 모서리는 시각화하고 싶을 때가 있다.
    #
    #   주의 / 함정:
    #   - depth 가 0 인 케이스: 코너가 정확히 카메라 광심 위면 나눗셈 폭주.
    #     KITTI 데이터셋에서 실제로 발생하긴 어렵지만, 코드 안정성을 위해 depth > 0 만
    #     uv 를 신뢰. 0 이거나 음수면 visible=False 로 마스킹되므로 화면에 안 그려진다.
    #   - dtype 불일치 (P2: float64, corners: float32) → 자동 promotion 으로 동작은 하지만
    #     성능 이슈가 신경 쓰이면 corners_cam 도 float64 로 통일
    #   - 행/열 헷갈림 디버깅: uv.shape 출력해 (2, 8) 인지 확인. (8, 2) 면 어딘가 .T 누락
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
    #   목적:
    #     8 코너의 픽셀을 감싸는 *축정렬* 박스 (axis-aligned bounding box) 를 구한다.
    #     이게 label_2 의 2D bbox (gt) 와 비교 가능한 형태.
    #     주의: 3D 박스의 진짜 2D 투영은 회전된 다각형 (convex hull) 이지만,
    #     KITTI 의 2D bbox 정답은 그 hull 의 축정렬 외접박스로 정의된다.
    #
    #   구현 절차:
    #   1) 가시 코너 0 개 케이스 처리
    #      - if not visible.any(): raise ValueError(...)
    #      - 0 개일 때 min/max 호출하면 numpy 가 빈 배열 에러를 던지지만, 메시지가 불친절해
    #        명시적으로 ValueError 를 던지는 편이 디버깅이 쉽다
    #
    #   2) boolean 인덱싱으로 visible 코너만 추출
    #      - uv shape 은 (2, 8). u 행 = uv[0], v 행 = uv[1]
    #      - u_vis = uv[0, visible]  → shape (n_visible,)
    #      - v_vis = uv[1, visible]
    #      - 슬라이스 순서 주의: uv[visible] 이라고 쓰면 (2,) 가 아닌 행을 마스킹해 잘못된 결과
    #
    #   3) min / max 추출
    #      - x1 = u_vis.min(), x2 = u_vis.max()
    #        y1 = v_vis.min(), y2 = v_vis.max()
    #      - (x1, y1, x2, y2) 가 곧 KITTI 의 2D bbox 포맷 (XYXY, 좌상단/우하단)
    #        XYWH (좌상단 + 너비/높이) 가 아님에 유의 (다른 데이터셋과 헷갈리기 쉬움)
    #
    #   4) 반환 형태 정리
    #      - tuple(float(x1), float(y1), float(x2), float(y2)) 처럼 plain float 으로 캐스팅
    #        numpy 스칼라 (np.float64) 가 섞이면 bbox_iou 안에서 max/min 호출 시
    #        반환 dtype 이 의도와 달라져 디버깅이 꼬일 수 있다
    #
    #   주의:
    #   - 코너 한두 개가 카메라 뒤로 가 잘려도 (visible.sum() < 8) 박스는 만들 수 있다
    #     → 나머지 가시 코너만으로 외접박스를 구함 (느슨해지지만 합리적인 근사)
    #   - 이 박스가 gt bbox 보다 *살짝 크게* 나오는 경우가 많다 (3D 박스의 모든 코너를
    #     감싸야 하므로). IoU 가 0.95 가 아니라 0.80 정도면 정상 범위.
    raise NotImplementedError


def bbox_iou(
    a: Tuple[float, float, float, float], b: Tuple[float, float, float, float]
) -> float:
    """두 축정렬 박스의 IoU (0 ~ 1).

    합격 기준 (PRACTICE.md):
        IoU > 0.7 또는 max corner diff < 5 px 면 정합도 양호.
    """
    # TODO:
    #   IoU 직관:
    #     두 박스 A, B 의 *교집합 넓이* 를 *합집합 넓이* 로 나눈 비율.
    #     완전히 겹치면 1, 안 겹치면 0. 객체 검출 평가의 표준 지표.
    #
    #     ┌──A──┐
    #     │  ┌──┼──B──┐    교집합 = A∩B 박스의 넓이
    #     │  │##│     │    합집합 = areaA + areaB - 교집합 (이중 카운트 보정)
    #     └──┼──┘     │
    #        └────────┘
    #
    #   구현 절차:
    #   1) 교집합 박스의 좌상단 / 우하단
    #      - x1_inter = max(a[0], b[0])    (왼쪽 두 변 중 더 *오른쪽* 인 것)
    #        y1_inter = max(a[1], b[1])    (위쪽 두 변 중 더 *아래쪽* 인 것)
    #        x2_inter = min(a[2], b[2])    (오른쪽 두 변 중 더 *왼쪽* 인 것)
    #        y2_inter = min(a[3], b[3])
    #      - 이 조합이 두 박스가 동시에 차지하는 영역의 외곽
    #
    #   2) 폭 / 높이 계산 (음수면 안 겹친 것)
    #      - w_inter = max(0.0, x2_inter - x1_inter)
    #        h_inter = max(0.0, y2_inter - y1_inter)
    #      - 한쪽이라도 0 이면 inter_area = 0 → IoU = 0
    #      - max(0, ...) 처리 빼먹으면 안 겹친 케이스에서 *음수 넓이* 가 나와 잘못된 IoU
    #
    #   3) 면적 / IoU
    #      - inter = w_inter * h_inter
    #      - area_a = (a[2] - a[0]) * (a[3] - a[1])
    #      - area_b = (b[2] - b[0]) * (b[3] - b[1])
    #      - union = area_a + area_b - inter
    #      - return inter / union  (float)
    #
    #   주의:
    #   - 박스 좌표가 XYXY 인지 XYWH 인지 헷갈리지 말 것 (이 task 는 XYXY).
    #     XYWH 면 (x+w-x) = w 라 면적 계산 식이 달라진다
    #   - union 이 0 인 케이스 (두 박스 모두 면적 0) 는 IoU 정의 자체가 모호.
    #     이 task 의 입력 (P2 투영 박스 + label gt) 에서는 발생하지 않으므로 분모 0 가드 생략 가능
    #   - 합격 기준 (PRACTICE.md): IoU > 0.7 또는 max corner diff < 5 px
    #     IoU 만 보면 박스 크기 차이가 작아도 위치가 살짝 어긋날 때 점수가 떡락하므로
    #     보조 지표로 corner-wise pixel 차이도 같이 본다
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
    #   배경: 3D 박스 = 8 코너 + 12 모서리
    #     8 코너만 점으로 찍으면 박스 모양이 안 보인다. 코너끼리 어떻게 이어지는지를
    #     EDGES list 가 정의한다 (12 쌍). 이 함수는 각 쌍을 cv2.line 으로 잇기만 한다.
    #
    #   EDGES 구조 복습 (이미 모듈 상단에 정의):
    #       바닥 사각형  : (0,1) (1,2) (2,3) (3,0)   → 4 개
    #       천장 사각형  : (4,5) (5,6) (6,7) (7,4)   → 4 개
    #       수직 4 모서리: (0,4) (1,5) (2,6) (3,7)   → 4 개
    #     합계 12 모서리. compute_3d_corners 의 코너 순서와 1:1 매칭 (어긋나면 X 자 꼬임).
    #
    #   cv2.line API:
    #       cv2.line(img, pt1, pt2, color, thickness=1, lineType=8)
    #         pt1, pt2 : (x, y) *정수* tuple. float 넘기면 TypeError
    #         color    : (B, G, R) BGR 순서
    #
    #   구현 절차:
    #   1) EDGES 12 쌍 순회
    #      - for i, j in EDGES: ...
    #      - i, j 는 0..7 의 코너 인덱스
    #
    #   2) 양 끝 코너의 가시성 확인
    #      - visible[i] 와 visible[j] 가 *모두* True 일 때만 그린다
    #      - 한 쪽만 보이는 모서리를 그리면:
    #         - 카메라 뒤 코너의 uv 값이 부호 반전 / 폭주 → 화면 가로지르는 잘못된 선
    #         - 시각적으로 명백한 버그처럼 보이므로 반드시 가드
    #      - 시야각 안인지 (uv 가 이미지 안인지) 는 검사하지 않는다 — 박스 일부가 잘려도
    #        모서리 일부는 보여주는 게 사용자에게 더 유용
    #
    #   3) 좌표 캐스팅 후 cv2.line
    #      - pt1 = (int(round(uv[0, i])), int(round(uv[1, i])))
    #        pt2 = (int(round(uv[0, j])), int(round(uv[1, j])))
    #      - uv shape (2, 8) 에서 열-단위 인덱싱: uv[:, i] 가 i 번째 코너의 (u, v)
    #      - int 캐스팅은 round 후 (Step 5 draw_observation 의 동일한 이유)
    #
    #   4) img_bgr 그대로 반환
    #      - cv2.line 은 in-place. img_bgr 가 이미 수정되어 있으므로 그대로 return
    #      - 호출자가 chaining 하려면 참조 반환이 편함 (out = draw_box_3d(img, ...))
    #
    #   주의:
    #   - 색 선택: 이 함수 기본값은 녹 (B=0, G=255, R=0). gt 박스도 같은 이미지에 그릴 거면
    #     색을 다르게 줘서 구분 (예: gt 는 적, projected 는 녹). 호출자가 color 인자로 제어
    #   - thickness 가 너무 작으면 (1px) anti-aliasing 없이 끊겨 보일 수 있음. 기본 2 가 무난
    #   - 이미지 외곽 너머로 선을 그어도 cv2 가 알아서 clipping 해주므로 별도 처리 불필요
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
    #   Rerun 의 두 가지 시각화 패러다임:
    #     A) 2D 트리 — image 엔티티 평면 위에 점/박스를 직접 찍기 (Pinhole 없이도 OK)
    #     B) 3D 트리 — world 좌표계의 Pinhole 카메라 frustum 안에 이미지 + 3D 점 배치
    #   본 함수는 둘 다 로깅해서 좌우로 동기화 비교 가능하게 만든다.
    #
    #   엔티티 트리 (docstring 동일):
    #     image                 : 원본 RGB 이미지 (2D 트리 root)
    #     image/proj_corners    : 8 코너 재투영 (Points2D, 녹)
    #     image/gt_bbox         : 2D bbox (Boxes2D, 적)
    #     world/cam             : Pinhole 카메라 (K, W, H)
    #     world/box_corners     : 8 코너 3D 위치 (Points3D)
    #
    #   2D 트리 (image/...) 구현:
    #   1) 원본 이미지 로깅
    #      - rr.log("image", rr.Image(img_rgb))
    #      - 반드시 *RGB* 가 들어와야 색이 정상 (main() 에서 cv2.cvtColor 로 변환된 상태)
    #
    #   2) 8 코너 점 찍기
    #      - rr.Points2D 는 (N, 2) shape 을 요구. uv 는 (2, 8) 이므로 .T 로 (8, 2)
    #      - color 는 (R, G, B) 0..255. 예: colors=[0, 255, 0] 으로 녹
    #      - rr.log("image/proj_corners", rr.Points2D(uv.T, colors=[0, 255, 0]))
    #      - 자식 엔티티 (image/proj_corners) 라 image 평면 위에 자동 정렬
    #
    #   3) gt bbox
    #      - rr.Boxes2D 는 array_format 으로 어떤 포맷의 좌표인지 명시해야 한다
    #      - gt_bbox = (x1, y1, x2, y2) → rr.Boxes2D(array=[gt_bbox],
    #                                                  array_format=rr.Box2DFormat.XYXY,
    #                                                  colors=[255, 0, 0])
    #        (XYXY vs XYWH 헷갈리지 말 것)
    #      - array 가 list 한 겹 (단일 박스도 [[...]] 의 (N, 4) shape) 임에 주의
    #
    #   3D 트리 (world/...) 구현:
    #   4) Pinhole 카메라 (world/cam)
    #      - rectified KITTI 의 P2 는 K @ [I | t] 형태 → 앞 3 열이 *수학적으로 정확히* K
    #        K = P2[:, :3]
    #      - 해상도는 img_rgb.shape = (H, W, C). Pinhole 의 resolution 인자는 [W, H] 순서:
    #            resolution = [img_rgb.shape[1], img_rgb.shape[0]]
    #        H/W 거꾸로 넣으면 frustum 종횡비가 뒤집힌다
    #      - rr.log("world/cam", rr.Pinhole(image_from_camera=K, resolution=resolution))
    #      - Transform3D 는 *생략 가능*. KITTI 단일 카메라이므로 world == cam 으로 두면
    #        cam 자체가 원점에 놓이고 corners_cam 좌표가 그대로 world 좌표가 된다.
    #
    #   5) 3D 코너 점 찍기 (world/box_corners)
    #      - rr.Points3D 는 (N, 3) shape. corners_cam 은 (3, 8) 이므로 .T 로 (8, 3)
    #      - colors 옵션으로 녹 (또는 다른 색) 지정
    #      - rr.log("world/box_corners", rr.Points3D(corners_cam.T, colors=[0, 255, 0]))
    #
    #   주의 / 디버깅:
    #   - 이미지가 흑백으로 나오거나 색이 이상하면: BGR 을 그대로 넘긴 것. cv2.cvtColor 점검
    #   - Pinhole frustum 안에 이미지가 안 나타나면: world/cam 과 image 가 *분리된 트리*.
    #     image 도 frustum 안에 보고 싶다면 'world/cam/image' 같은 자식 경로에 별도 로깅하거나
    #     2D 와 3D 를 그냥 분리해서 보는 게 단순 (본 함수는 후자)
    #   - Points2D / Points3D 의 shape 가 (N, 2) / (N, 3) 인지 transpose 확인. (2, N) 으로
    #     넘기면 점 개수가 2 개로 잘못 해석돼 사실상 안 보임
    #   - rr.Image, rr.Pinhole 등의 인자 명세는 rerun 버전마다 살짝 바뀌므로 import 한 버전의
    #     docstring (help(rr.Pinhole) 등) 으로 확인하는 게 가장 안전
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

    # Rerun: RRD 파일로 저장. 로컬에서 `rerun kitti_step4.rrd` 로 열어볼 것
    rr.init("kitti_step4", spawn=False)
    log_to_rerun(
        cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB),
        P2,
        corners_cam,
        uv,
        label.bbox_2d,
    )
    rrd_path = Path("output") / f"kitti_step4_{SAMPLE_ID}.rrd"
    rr.save(str(rrd_path))
    print(f"저장: {rrd_path}  (로컬에서 `rerun {rrd_path.name}` 로 열기)")

if __name__ == "__main__":
    main()
