"""Phase 2 Week 4 - Step 5: nuScenes 두 카메라 삼각측량 (정답 코드).

스켈레톤 (nuscenes_step5.py) 의 reference 구현이다.
구조/시그니처/docstring 은 스켈레톤과 동일하며, TODO 본문만 채워졌다.
막힐 때 함수 단위로 비교하는 용도로 사용한다.

좌표계 메모 (헷갈리기 쉬움):
    nuScenes 의 calibrated_sensor / ego_pose 는 모두
    "sensor -> ego" / "ego -> world" 변환을 의미한다 (forward).
    annotation.translation 은 world(global) 좌표.
    카메라 coord 는 OpenCV 컨벤션 (+x 오른쪽, +y 아래, +z 앞).
"""

from dataclasses import dataclass
from pathlib import Path
from typing import Dict
from typing import List
from typing import Optional
from typing import Tuple

import cv2
import numpy as np
import rerun as rr
from nuscenes.nuscenes import NuScenes
from pyquaternion import Quaternion


# 경로 / 상수
NUSC_ROOT = Path("/datasets/nuscenes_mini")
NUSC_VERSION = "v1.0-mini"

# 인접 카메라 쌍 (시야가 겹쳐서 같은 객체가 동시에 보일 확률이 높다).
CAM_PAIR = ("CAM_FRONT", "CAM_FRONT_LEFT")

# 합격 기준
MAX_TRIANGULATION_ERROR_M = 0.5  # 50 cm 이내면 합격


@dataclass
class CameraView:
    """한 sample 의 한 카메라에 대한 모든 정보.

    Attributes:
        channel: 'CAM_FRONT' 등 nuScenes 채널 이름.
        image_path: 절대 경로.
        K: (3, 3) intrinsic.
        cam_from_world: (4, 4) world 좌표를 카메라 좌표로 보내는 변환.
        width: 이미지 폭 (px).
        height: 이미지 높이 (px).
    """

    channel: str
    image_path: Path
    K: np.ndarray
    cam_from_world: np.ndarray
    width: int
    height: int


# Step 5-1: nuScenes 로딩
def load_nuscenes() -> NuScenes:
    """nuScenes mini 데이터셋을 로드한다.

    Returns:
        NuScenes 인스턴스. verbose=False 로 콘솔을 비워둘 것.

    Note:
        v1.0-mini 는 10 scene, 약 3GB. 첫 로드 시 수 초 걸린다.
    """
    return NuScenes(version=NUSC_VERSION, dataroot=str(NUSC_ROOT), verbose=False)


# Step 5-2: sample + 카메라 채널 → CameraView 변환
def build_camera_view(nusc: NuScenes, sample: dict, channel: str) -> CameraView:
    """sample 의 지정 카메라 채널에 대한 CameraView 를 만든다.

    nuScenes 의 좌표 변환은 두 단계:
        sensor (cam) -> ego  (calibrated_sensor)
        ego -> world         (ego_pose)
    따라서 cam_from_world = inv(world_from_ego @ ego_from_sensor)

    Args:
        nusc: NuScenes 인스턴스.
        sample: nusc.sample[k] 로 얻은 dict.
        channel: 'CAM_FRONT' 등.

    Returns:
        CameraView.

    Note:
        Quaternion 은 nuScenes JSON 에 [w, x, y, z] 순서로 들어있다.
        pyquaternion.Quaternion(*[w, x, y, z]) 또는
        Quaternion(quat_list) 둘 다 OK.
    """
    sd = nusc.get("sample_data", sample["data"][channel])
    calib = nusc.get("calibrated_sensor", sd["calibrated_sensor_token"])
    ego = nusc.get("ego_pose", sd["ego_pose_token"])

    K = np.array(calib["camera_intrinsic"])

    ego_from_sensor = transform_matrix(
        calib["translation"], Quaternion(calib["rotation"])
    )
    world_from_ego = transform_matrix(
        ego["translation"], Quaternion(ego["rotation"])
    )
    world_from_sensor = world_from_ego @ ego_from_sensor
    cam_from_world = np.linalg.inv(world_from_sensor)

    image_path = NUSC_ROOT / sd["filename"]
    width, height = sd["width"], sd["height"]

    return CameraView(
        channel=channel,
        image_path=image_path,
        K=K,
        cam_from_world=cam_from_world,
        width=width,
        height=height,
    )


def transform_matrix(translation: List[float], rotation: Quaternion) -> np.ndarray:
    """(translation, quaternion) 을 (4, 4) 동차변환 행렬로 만든다.

    Args:
        translation: [tx, ty, tz].
        rotation: pyquaternion.Quaternion 인스턴스.

    Returns:
        (4, 4) numpy 배열. 마지막 행은 [0, 0, 0, 1].
    """
    T = np.eye(4)
    T[:3, :3] = rotation.rotation_matrix
    T[:3, 3] = translation
    return T


# Step 5-3: world 점 → 카메라 픽셀 투영 (가시성 판정 포함)
def project_world_point(
    point_world: np.ndarray, view: CameraView
) -> Tuple[Optional[Tuple[float, float]], float]:
    """world 좌표의 3D 점을 카메라 픽셀로 투영한다.

    Args:
        point_world: shape (3,) world 좌표.
        view: 카메라 정보.

    Returns:
        uv: (u, v) 픽셀 좌표. 가시성 실패면 None.
        depth: 카메라 +z 성분 (m). 음수면 카메라 뒤.

    Note:
        가시성 조건 3개 모두 만족해야 uv 반환:
            (1) depth (cam_z) > 0          : 카메라 앞
            (2) 0 <= u < width              : 가로 시야 내
            (3) 0 <= v < height             : 세로 시야 내
    """
    p_h = np.append(point_world, 1.0)         # (4,)
    p_cam = view.cam_from_world @ p_h          # (4,)
    depth = float(p_cam[2])

    if depth <= 0:
        return None, depth

    uvw = view.K @ p_cam[:3]                   # (3,)
    u = uvw[0] / uvw[2]
    v = uvw[1] / uvw[2]

    if 0 <= u < view.width and 0 <= v < view.height:
        return (float(u), float(v)), depth
    return None, depth


# Step 5-4: 두 카메라에 동시에 보이는 annotation 찾기
def find_common_annotation(
    nusc: NuScenes, sample: dict, view_a: CameraView, view_b: CameraView
) -> Optional[Tuple[dict, Tuple[float, float], Tuple[float, float]]]:
    """sample 의 annotation 중 두 카메라에 모두 보이는 첫 번째 것을 반환.

    각 annotation 의 3D 중심(translation, world 좌표)을 두 카메라에 투영해
    가시성을 판정한다. 두 카메라 모두 가시면 채택.

    Args:
        nusc: NuScenes 인스턴스.
        sample: 현재 sample dict.
        view_a: 첫 번째 카메라.
        view_b: 두 번째 카메라.

    Returns:
        (annotation, uv_a, uv_b) 또는 None.
    """
    for ann_token in sample["anns"]:
        ann = nusc.get("sample_annotation", ann_token)
        p_world = np.array(ann["translation"])

        uv_a, _ = project_world_point(p_world, view_a)
        if uv_a is None:
            continue

        uv_b, _ = project_world_point(p_world, view_b)
        if uv_b is None:
            continue

        return ann, uv_a, uv_b
    return None


# Step 5-5: P 행렬 만들기 (cv2.triangulatePoints 입력용)
def build_projection_matrix(view: CameraView) -> np.ndarray:
    """world -> pixel 의 (3, 4) 투영행렬 P = K @ [R | t] 를 만든다.

    cv2.triangulatePoints 는 P 의 좌표계가 두 카메라 사이에서 *동일* 해야 한다.
    여기서는 world 좌표계 기준으로 통일.

    Args:
        view: CameraView.

    Returns:
        (3, 4) numpy 배열.

    Note:
        cam_from_world 의 앞 3 행 4 열이 곧 [R | t]. 마지막 행 [0,0,0,1] 은 버린다.
    """
    Rt = view.cam_from_world[:3, :]
    return view.K @ Rt


# Step 5-6: 삼각측량 (DLT, OpenCV API)
def triangulate(
    P_a: np.ndarray,
    P_b: np.ndarray,
    uv_a: Tuple[float, float],
    uv_b: Tuple[float, float],
) -> np.ndarray:
    """두 픽셀 관측을 합쳐 world 좌표 3D 점을 복원한다.

    Args:
        P_a: (3, 4) view_a 의 world->pixel 투영행렬.
        P_b: (3, 4) view_b 의 world->pixel 투영행렬.
        uv_a: view_a 에서의 픽셀 (u, v).
        uv_b: view_b 에서의 픽셀 (u, v).

    Returns:
        (3,) world 좌표.

    Note:
        cv2.triangulatePoints 는 4D homogeneous 좌표 (4, N) 를 돌려준다.
        앞 3 성분을 마지막 w 로 나눠야 실제 (X, Y, Z).
    """
    pts_a = np.array([[uv_a[0]], [uv_a[1]]], dtype=np.float64)  # (2, 1)
    pts_b = np.array([[uv_b[0]], [uv_b[1]]], dtype=np.float64)  # (2, 1)

    X_h = cv2.triangulatePoints(P_a, P_b, pts_a, pts_b)         # (4, 1)
    X = (X_h[:3] / X_h[3]).flatten()                            # (3,)
    return X


# Step 5-7: 시각화 (이미지에 점 찍기 + Rerun)
def draw_observation(
    img_bgr: np.ndarray,
    uv: Tuple[float, float],
    color: Tuple[int, int, int] = (0, 255, 0),
    radius: int = 6,
    thickness: int = 2,
) -> np.ndarray:
    """이미지 위에 관측 픽셀을 원으로 그린다.

    Args:
        img_bgr: BGR 이미지 (in-place 변경).
        uv: 관측 (u, v).
        color: BGR.
        radius: 픽셀.
        thickness: 선 두께.

    Returns:
        그려진 이미지 (참조).
    """
    center = (int(round(uv[0])), int(round(uv[1])))
    cv2.circle(img_bgr, center, radius, color, thickness)
    return img_bgr


def log_to_rerun(
    views: List[CameraView],
    images_rgb: Dict[str, np.ndarray],
    obs: Dict[str, Tuple[float, float]],
    gt_world: np.ndarray,
    est_world: np.ndarray,
) -> None:
    """Rerun 으로 카메라 핀홀 + 관측 + 3D 점들을 로깅한다.

    엔티티 트리:
        world/cam/{channel}              : 각 카메라 Pinhole + 이미지
        world/cam/{channel}/observation  : 관측 픽셀 Points2D
        world/gt_point                   : annotation 3D 위치 (적)
        world/triangulated_point         : 복원 3D 위치 (녹)

    Note:
        Pinhole 의 transform 은 world->cam 의 역, 즉 world_from_cam 으로 줘야
        world 좌표계에 카메라가 올바른 위치에 놓인다. Rerun 은 자식 엔티티가
        부모 frame 에 상대적이라는 점에 주의.
    """
    for view in views:
        prefix = f"world/cam/{view.channel}"
        world_from_cam = np.linalg.inv(view.cam_from_world)
        rr.log(
            prefix,
            rr.Transform3D(
                translation=world_from_cam[:3, 3],
                mat3x3=world_from_cam[:3, :3],
            ),
        )
        rr.log(
            prefix,
            rr.Pinhole(
                image_from_camera=view.K,
                width=view.width,
                height=view.height,
            ),
        )
        rr.log(f"{prefix}/image", rr.Image(images_rgb[view.channel]))

        if view.channel in obs:
            u, v = obs[view.channel]
            rr.log(
                f"{prefix}/observation",
                rr.Points2D([[u, v]], radii=5, colors=[(0, 255, 0)]),
            )

    rr.log(
        "world/gt_point",
        rr.Points3D([gt_world], radii=0.2, colors=[(255, 0, 0)]),
    )
    rr.log(
        "world/triangulated_point",
        rr.Points3D([est_world], radii=0.2, colors=[(0, 255, 0)]),
    )


# 메인 파이프라인
def main() -> None:
    """nuScenes mini 의 첫 sample 로 Step 5 파이프라인을 실행한다."""
    nusc = load_nuscenes()
    sample = nusc.sample[0]
    print(f"sample token: {sample['token']}, anns: {len(sample['anns'])}")

    # 두 카메라 정보 만들기
    view_a = build_camera_view(nusc, sample, CAM_PAIR[0])
    view_b = build_camera_view(nusc, sample, CAM_PAIR[1])

    # 두 카메라에 동시에 보이는 annotation 찾기
    hit = find_common_annotation(nusc, sample, view_a, view_b)
    if hit is None:
        print(
            f"두 카메라({CAM_PAIR})에 동시 가시 annotation 없음. "
            f"다른 카메라 쌍이나 sample 로 시도하라."
        )
        return
    ann, uv_a, uv_b = hit
    gt_world = np.array(ann["translation"])

    print(
        f"annotation: {ann['category_name']}  "
        f"gt_world={gt_world.round(3).tolist()}"
    )
    print(f"  {view_a.channel} uv = ({uv_a[0]:.2f}, {uv_a[1]:.2f})")
    print(f"  {view_b.channel} uv = ({uv_b[0]:.2f}, {uv_b[1]:.2f})")

    # 삼각측량
    P_a = build_projection_matrix(view_a)
    P_b = build_projection_matrix(view_b)
    est_world = triangulate(P_a, P_b, uv_a, uv_b)

    err = float(np.linalg.norm(est_world - gt_world))
    print(f"est_world: {est_world.round(3).tolist()}")
    print(
        f"오차      : {err * 100:.2f} cm   "
        f"({'합격' if err < MAX_TRIANGULATION_ERROR_M else '불합격'}, "
        f"기준 < {MAX_TRIANGULATION_ERROR_M*100:.0f} cm)"
    )

    # 이미지에 관측 그리기 + 저장
    out_dir = Path("output")
    out_dir.mkdir(exist_ok=True)
    images_rgb: Dict[str, np.ndarray] = {}
    for view, uv in [(view_a, uv_a), (view_b, uv_b)]:
        img_bgr = cv2.imread(str(view.image_path))
        if img_bgr is None:
            raise FileNotFoundError(f"image not found: {view.image_path}")
        draw_observation(img_bgr, uv)
        out_path = out_dir / f"nuscenes_step5_{view.channel}.jpg"
        cv2.imwrite(str(out_path), img_bgr)
        images_rgb[view.channel] = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB)
        print(f"저장: {out_path}")

    # Rerun: RRD 파일로 저장. 로컬에서 `rerun kitti_step4.rrd` 로 열어볼 것
    rr.init("nuscenes_step5", spawn=True)
    log_to_rerun(
        views=[view_a, view_b],
        images_rgb=images_rgb,
        obs={view_a.channel: uv_a, view_b.channel: uv_b},
        gt_world=gt_world,
        est_world=est_world,
    )

    rrd_path = Path("output") / f"nuscenes_step5_{view.channel}.rrd"
    rr.save(str(rrd_path))
    print(f"저장: {rrd_path}  (로컬에서 `rerun {rrd_path.name}` 로 열기)")

if __name__ == "__main__":
    main()
