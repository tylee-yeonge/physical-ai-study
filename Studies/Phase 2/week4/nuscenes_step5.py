"""Phase 2 Week 4 - Step 5: nuScenes 두 카메라 삼각측량 (스켈레톤).

목표:
    한 sample 에서 인접한 두 카메라(예: CAM_FRONT, CAM_FRONT_LEFT)에
    동시에 보이는 annotation 의 3D 중심을 각각 픽셀로 투영한 뒤,
    cv2.triangulatePoints 로 다시 3D 를 복원해 원본 annotation 위치와의
    오차를 측정한다. Rerun 으로 6 카메라 + 3D 박스 + 복원점 시각화.

선행 조건:
    /datasets/nuscenes_mini/v1.0-mini/ (devkit json + samples/sweeps/maps)
    pip install nuscenes-devkit rerun-sdk opencv-python  (이미 설치됨)

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
    # TODO:
    #   - 위에 정의된 NUSC_VERSION / NUSC_ROOT 상수를 그대로 사용
    #   - dataroot 는 문자열 경로를 받으므로 Path 를 str() 로 변환
    #   - 디버깅이 끝났다면 verbose 를 꺼서 로그 노이즈 줄이기
    raise NotImplementedError


# Step 5-2: sample + 카메라 채널 → CameraView 변환
def build_camera_view(nusc: NuScenes, sample: dict, channel: str) -> CameraView:
    """sample 의 지정 카메라 채널에 대한 CameraView 를 만든다.

    nuScenes 의 좌표 변환은 두 단계:
        sensor (cam) -> ego  (calibrated_sensor)
        ego -> world         (ego_pose)
    따라서 cam_from_world = inv(ego_from_world) @ inv(sensor_from_ego)
                           = world_from_ego.T   @ ego_from_sensor.T (회전 부분만 보면)
    실제로는 4x4 행렬 곱으로 처리한다.

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
    # TODO:
    #   1) sample['data'][channel] 토큰을 nusc.get('sample_data', ...) 로 dict 조회
    #      → 여기에 calibrated_sensor_token / ego_pose_token / filename / width / height 가 들어있다
    #   2) 두 token 으로 calib (sensor->ego) 와 ego_pose (ego->world) dict 를 추가 조회
    #   3) 카메라 내부행렬 K 는 calib['camera_intrinsic'] (이미 (3,3) 리스트)
    #   4) 두 변환을 합성해 world->cam 만들기
    #      - 각 (translation, rotation) → 아래 transform_matrix 로 4x4 변환
    #      - rotation 은 [w,x,y,z] 순서 quaternion → pyquaternion.Quaternion 로 감싸기
    #      - world_from_sensor = world_from_ego @ ego_from_sensor (순서 주의)
    #      - cam_from_world 는 그 *역행렬*
    #   5) image_path 는 NUSC_ROOT / sd['filename'] 로 절대경로 만들기
    #   6) CameraView 의 필드 순서대로 채워 반환
    raise NotImplementedError


def transform_matrix(translation: List[float], rotation: Quaternion) -> np.ndarray:
    """(translation, quaternion) 을 (4, 4) 동차변환 행렬로 만든다.

    Args:
        translation: [tx, ty, tz].
        rotation: pyquaternion.Quaternion 인스턴스.

    Returns:
        (4, 4) numpy 배열. 마지막 행은 [0, 0, 0, 1].
    """
    # TODO:
    #   - 4x4 항등행렬에서 시작 (마지막 행 [0,0,0,1] 자동 충족)
    #   - 좌상단 3x3 블록에 pyquaternion 의 rotation_matrix 속성을 채운다
    #   - 마지막 열 3x1 블록에 translation 채우기
    raise NotImplementedError


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
    # TODO:
    #   1) point_world 에 동차좌표 1 을 붙여 (4,) 로 만든 뒤 cam_from_world 와 곱하기
    #   2) cam 좌표 z 성분이 곧 depth. <= 0 이면 카메라 뒤이므로 일찍 종료 (None, depth)
    #   3) K (3x3) 와 cam 좌표 앞 3 성분의 곱 후 perspective divide → (u, v)
    #      (직접 / 로 나누거나, 처음 두 성분을 세 번째로 나누는 방식 둘 다 OK)
    #   4) (u, v) 가 [0, width) x [0, height) 안일 때만 좌표 반환, 그 외엔 None
    #   - depth 는 가시성과 무관하게 항상 반환 (디버깅에서 음수/원거리 객체 분류용)
    raise NotImplementedError


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
    # TODO:
    #   - sample['anns'] 의 각 토큰을 'sample_annotation' 으로 조회하여 ann dict 획득
    #   - ann['translation'] 이 world 좌표 3D 중심 (numpy 배열로 감싸 두면 편하다)
    #   - project_world_point 를 두 view 모두에 호출해 *둘 다* uv 가 not None 일 때만 hit
    #     (한 쪽 view 에서 일찍 실패하면 두 번째 view 호출은 굳이 안 해도 됨)
    #   - 가장 먼저 발견된 hit 를 즉시 반환, 모두 실패하면 None
    raise NotImplementedError


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
    # TODO:
    #   - cam_from_world 의 마지막 행 [0,0,0,1] 은 P 행렬에 불필요 → 앞 3 행만 슬라이스
    #     (앞 3 행 4 열의 의미가 정확히 [R | t])
    #   - K (3x3) @ [R | t] (3x4) = P (3x4)
    #   - 두 카메라의 P 가 *같은 world 좌표계* 기준이어야 cv2.triangulatePoints 가 동작
    raise NotImplementedError


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
    # TODO:
    #   - cv2.triangulatePoints 는 점 좌표를 (2, N) 모양으로 받는다
    #     (한 점이면 (2, 1). u 가 첫 행, v 가 둘째 행이 되도록 배치)
    #   - 반환은 (4, N) 동차좌표 → 앞 3 성분을 마지막 성분(w)으로 나눠야 (X, Y, Z)
    #   - 결과는 (3,) 모양으로 정리해 반환 (호출 측은 ndarray 의 elementwise 연산을 가정)
    #   - dtype 은 float64 로 통일하는 편이 OpenCV 내부 변환 비용을 줄여준다
    raise NotImplementedError


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
    # TODO:
    #   - uv 는 float 이므로 round 후 int 캐스팅이 필요 (cv2.circle 의 center 는 int tuple)
    #   - cv2.circle 한 번 호출이면 끝. thickness 음수면 채워진 원
    raise NotImplementedError


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
    # TODO:
    #   각 view 별로 (위 docstring 의 엔티티 트리를 그대로 따라가면 된다):
    #     - 채널별 prefix 를 짓는다 (예: f"world/cam/{v.channel}")
    #     - cam_from_world 의 역행렬 = world_from_cam → Transform3D 로 카메라 pose 로깅
    #       (translation 은 마지막 열 3 성분, mat3x3 은 좌상단 3x3 블록)
    #     - 같은 prefix 에 Pinhole 로 K + 이미지 해상도 등록
    #     - prefix 아래 child 엔티티에 이미지 / 관측 픽셀 추가
    #       (Rerun 은 child 가 parent frame 에 *상대적* 으로 배치된다)
    #     - obs 에 그 채널 키가 있을 때만 Points2D 로 관측 픽셀 찍기
    #
    #   world 노드 자체에 정답/복원 3D 점도 Points3D 로 색 다르게 (적/녹) 로깅
    #   (단일 점이라도 [[x,y,z]] 형태의 (N, 3) shape 으로 넘긴다)
    raise NotImplementedError


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
        print(f"두 카메라({CAM_PAIR})에 동시 가시 annotation 없음. "
              f"다른 카메라 쌍이나 sample 로 시도하라.")
        return
    ann, uv_a, uv_b = hit
    gt_world = np.array(ann['translation'])

    print(f"annotation: {ann['category_name']}  "
          f"gt_world={gt_world.round(3).tolist()}")
    print(f"  {view_a.channel} uv = ({uv_a[0]:.2f}, {uv_a[1]:.2f})")
    print(f"  {view_b.channel} uv = ({uv_b[0]:.2f}, {uv_b[1]:.2f})")

    # 삼각측량
    P_a = build_projection_matrix(view_a)
    P_b = build_projection_matrix(view_b)
    est_world = triangulate(P_a, P_b, uv_a, uv_b)

    err = float(np.linalg.norm(est_world - gt_world))
    print(f"est_world: {est_world.round(3).tolist()}")
    print(f"오차      : {err * 100:.2f} cm   "
          f"({'합격' if err < MAX_TRIANGULATION_ERROR_M else '불합격'}, "
          f"기준 < {MAX_TRIANGULATION_ERROR_M*100:.0f} cm)")

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
