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
    #   1) sample → sample_data dict 가져오기
    #      - sample["data"][channel] 은 토큰 문자열 (예: "e3d495d4...")
    #      - nusc.get("sample_data", token) 으로 한 번 풀면 dict 가 된다
    #      - 그 dict 안에 다음 필드가 들어있다 (이후 단계에서 모두 쓴다):
    #          calibrated_sensor_token : sensor->ego 변환 테이블 키
    #          ego_pose_token          : ego->world 변환 테이블 키
    #          filename                : 'samples/CAM_FRONT/xxx.jpg' 같은 상대경로
    #          width, height           : 이미지 해상도 (px)
    #      - 변수 이름은 짧게 'sd' 로 두는 편이 devkit 관용
    #
    #   2) calib (sensor->ego) 와 ego_pose (ego->world) dict 를 추가 조회
    #      - nuScenes JSON 테이블 간 join 은 항상 nusc.get(table_name, token) 한 패턴
    #        (sd 자체는 그냥 파이썬 dict 이므로 sd.get(...) 으로는 join 못 한다)
    #      - 두 dict 모두 'translation' (길이 3) 과 'rotation' (길이 4) 를 갖는다
    #      - rotation 은 [w, x, y, z] 순서 quaternion
    #
    #   3) 카메라 내부행렬 K
    #      - calib['camera_intrinsic'] 은 이미 (3, 3) list-of-list
    #      - np.array() 로 감싸기만 하면 끝 (재계산/스케일링 불필요)
    #
    #   4) 두 변환을 합성해 cam_from_world 만들기
    #      - 헬퍼 transform_matrix(translation, Quaternion) 가 (4, 4) 동차변환 반환
    #      - rotation list 는 pyquaternion.Quaternion(...) 으로 감싸서 헬퍼에 전달
    #        (Quaternion(list) / Quaternion(*list) 둘 다 OK, 일관성만 유지)
    #      - 변수명 컨벤션: 'A_from_B' = B 좌표를 A 좌표로 옮기는 변환
    #        → 곱하기 순서는 chain rule 처럼 가운데가 약분되는 방향으로 외우기:
    #            world_from_[ego] @ [ego]_from_sensor  =  world_from_sensor
    #      - 필요한 건 cam_from_world. forward (world_from_sensor) 를 먼저 만들고
    #        마지막에 한 번만 np.linalg.inv 하는 편이 부호/순서 헷갈림이 적다
    #
    #   5) image_path 만들기
    #      - sd['filename'] 은 dataroot 기준 상대경로
    #      - NUSC_ROOT / sd['filename'] 으로 합치면 절대 Path 가 된다 (Path 라 / 동작)
    #      - str 변환은 cv2.imread 단계에서만 필요
    #
    #   6) CameraView 채워 반환
    #      - dataclass 이므로 키워드 인자로 채우는 편이 가독성 좋다
    #      - 필드: channel, image_path, K, cam_from_world, width, height
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
    #   동차변환 (homogeneous transform) 의 구조 복습:
    #       T = | R  t |    R: (3, 3) 회전,  t: (3,) 평행이동
    #           | 0  1 |    마지막 행은 [0, 0, 0, 1] 로 고정
    #       T @ [X, Y, Z, 1].T  =  [X', Y', Z', 1].T
    #     이 한 행렬로 회전 + 평행이동을 동시에 표현, 곱하기로 변환 합성이 가능해진다.
    #
    #   구현 절차:
    #   - np.eye(4) 로 시작 → 마지막 행 [0, 0, 0, 1] 자동 충족, R 부분도 항등으로 초기화됨
    #   - 좌상단 (3, 3) 블록은 슬라이싱 [:3, :3] 으로 잡고 rotation.rotation_matrix 대입
    #     (pyquaternion 의 rotation_matrix 는 이미 (3, 3) ndarray)
    #   - 마지막 열 (3,) 블록은 [:3, 3] 으로 잡고 translation 대입
    #     (translation 이 list 여도 numpy 가 알아서 변환)
    #
    #   주의:
    #   - 슬라이스 [:3, 3] 와 [:3, 3:4] 는 shape 이 다르다 ((3,) vs (3, 1)).
    #     단순 대입에는 (3,) 이 편하다.
    #   - 헬퍼 자체는 forward 변환 (예: world_from_ego) 을 만든다. 역방향이 필요하면
    #     호출 측에서 np.linalg.inv 를 한 번 더 해라 (이 헬퍼 안에서 invert 하지 않는다).
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
    #   파이프라인 한눈에:
    #       world (3,)  --동차좌표화-->  (4,)
    #                   --cam_from_world @ -->  cam (4,)
    #                   --앞 3 성분 X--> cam_xyz (3,)
    #                   --K @ -->  pixel_h (3,)
    #                   --perspective divide (앞 두 성분 / 세 번째)--> (u, v)
    #
    #   1) 동차좌표 만들기
    #      - point_world 는 (3,). 끝에 1 을 붙여 (4,) 로 확장
    #      - 방법 예: np.append(point_world, 1.0) 또는 np.concatenate([p, [1.0]])
    #      - 4x4 @ (4,) → (4,) 가 그대로 동작 (ndim 1 끼리도 numpy 가 알아서 처리)
    #
    #   2) 카메라 좌표로 옮기고 depth 추출
    #      - view.cam_from_world @ point_h 로 cam 좌표 (4,) 획득
    #      - depth = cam_z 성분 (앞 3 성분 중 마지막 즉 index 2)
    #      - depth 가 0 이면 광선이 이미지 평면과 만나지 않는다 (분모 0)
    #        → depth <= 0 일 때 (None, depth) 로 일찍 종료. depth 값 자체는 그대로 반환
    #          (호출 측이 음수/원거리 분류를 할 수 있게 보존)
    #
    #   3) 내부행렬로 픽셀 좌표 만들기
    #      - K (3, 3) @ cam_xyz (3,) = pixel_h (3,)
    #      - 핀홀 모델은 *비선형* (나눗셈) 이므로 K 곱 후 반드시 perspective divide:
    #            u = pixel_h[0] / pixel_h[2]
    #            v = pixel_h[1] / pixel_h[2]
    #        cam 좌표 시점에서는 pixel_h[2] == depth 와 같다 (K 가 마지막 행 [0,0,1])
    #      - 결과 (u, v) 는 float. 정수 캐스팅은 그리기 단계에서만 한다 (정밀도 손실 방지)
    #
    #   4) 시야 안인지 판정
    #      - 0 <= u < width 그리고 0 <= v < height 모두 참일 때만 (u, v) 반환
    #      - 등호 위치 주의: 오른쪽은 strict (< width). u == width 는 인덱스 범위 밖.
    #      - 한 조건이라도 어긋나면 None 반환 (단, depth 는 여전히 반환)
    #
    #   디버깅 팁:
    #   - 화면이 아예 안 찍히면: cam_from_world 부호가 뒤집혀 depth 가 항상 음수일 가능성
    #   - 픽셀이 화면 밖으로만 나가면: K 와 카메라 채널 불일치 (CAM_FRONT 의 K 로
    #     CAM_FRONT_LEFT 의 사진을 투영하지 않았는지 확인)
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
    #   목적:
    #     sample 에 달린 모든 3D annotation 중, 두 카메라 view_a / view_b 에 *동시에*
    #     찍히는 첫 번째 것을 찾는다. 삼각측량은 두 시점 관측이 모두 있어야 가능.
    #
    #   구현 절차:
    #   1) annotation 목록 순회
    #      - sample['anns'] 는 토큰 문자열 list (sample 마다 보통 수십 개)
    #      - 각 토큰을 nusc.get('sample_annotation', token) 으로 dict 변환
    #      - dict 의 필드 중 이번 단계에 필요한 건 'translation' (length-3 list, world 좌표)
    #        그 외 'category_name', 'size', 'rotation' 등은 시각화/디버깅용
    #
    #   2) 두 view 각각에 3D 중심을 투영
    #      - center = np.array(ann['translation']) 형태로 ndarray 화 (project_world_point 기대 shape)
    #      - project_world_point(center, view_a) → (uv_a, depth_a)
    #        uv_a 가 None 이면 view_a 에 안 보임. 이 경우 view_b 까지 호출할 필요 없음
    #        (early-skip: continue 로 다음 annotation 으로 넘어가기 → 불필요한 행렬곱 절약)
    #      - 그리고 project_world_point(center, view_b) → (uv_b, depth_b)
    #
    #   3) 첫 hit 즉시 반환
    #      - 두 uv 모두 not None 이면 (ann, uv_a, uv_b) 즉시 return
    #      - 굳이 best 후보를 찾을 필요 없음 (시각화 1 점만 있으면 충분)
    #
    #   4) 끝까지 못 찾으면 None 반환 (caller 가 다른 카메라 쌍/sample 로 재시도)
    #
    #   주의:
    #   - annotation 의 'translation' 은 *world (global) 좌표* 다 (ego 가 아니다).
    #     ego 좌표로 잘못 해석하면 합성 변환 차이만큼 오차가 누적된다.
    #   - 3D 중심이 카메라 시야에 들어와도 객체 전체가 잘려 나갈 수 있다.
    #     이번 단계에서는 중심 한 점만 보는 걸로 충분 (정밀 가시성은 box corner 까지 봐야 함).
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
    #   배경: P 행렬의 의미
    #     투영의 전체 수식은
    #         pixel_h (3,) = K (3,3) @ [R | t] (3,4) @ X_world_h (4,)
    #     여기서 P = K @ [R | t] 로 묶어 두면, world 점을 한 번의 행렬곱으로 픽셀까지 보낸다.
    #     이 P 가 바로 cv2.triangulatePoints 가 받는 입력 (각 카메라 당 하나).
    #
    #   구현 절차:
    #   1) cam_from_world (4, 4) 에서 앞 3 행만 추출
    #      - 슬라이스 [:3, :] 또는 [:3] 으로 (3, 4) 획득
    #      - 잘려나간 마지막 행 [0, 0, 0, 1] 은 동차좌표 유지용이라 P 에선 불필요
    #      - 추출된 (3, 4) 의 의미: 좌상단 (3, 3) 이 R, 마지막 열 (3,) 이 t. 곧 [R | t]
    #
    #   2) K @ [R | t] 로 P 만들기
    #      - K (3, 3) @ [R|t] (3, 4) = P (3, 4)
    #      - 좌측 곱 순서 (K 먼저) 잊지 말 것: pixel = K @ (R X + t) 의 결합법칙으로
    #        K(RX+t) = (KR)X + Kt 로 펼쳐도 동일
    #
    #   3) 두 카메라의 P 좌표계 일치 확인
    #      - cv2.triangulatePoints 는 두 P 가 *같은 world 좌표계* 기준이라고 가정
    #      - 본 단계에서는 cam_from_world 를 썼으니 자동으로 world 기준
    #      - (만약 cam_a_from_cam_b 같이 한 카메라 기준으로 묶었다면, 다른 카메라도 같은 기준
    #        으로 환산해야 한다)
    #
    #   주의:
    #   - dtype: cv2 함수는 내부적으로 float64 를 기대. cam_from_world 가 float64 면
    #     P 도 자동으로 float64 가 되어 별도 캐스팅 불필요.
    #   - cv2.triangulatePoints 는 *normalized* coord 가 아닌 raw pixel coord 를 기대.
    #     따라서 K 를 빼지 말고 그대로 곱해 두는 게 맞다.
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
    #   배경: 삼각측량 (DLT) 의 직관
    #     한 카메라가 보는 픽셀 (u, v) 는 *광선 하나* 만 결정한다 (depth 자유도 남음).
    #     두 카메라가 각자 본 광선이 3D 에서 교차하는 점을 푸는 것이 삼각측량.
    #     실제 측정 노이즈로 두 광선이 정확히 만나지 않으므로 최소제곱(SVD)으로 푼다.
    #     이 모든 과정을 cv2.triangulatePoints 가 한 번에 처리해 준다.
    #
    #   cv2.triangulatePoints API 형태:
    #       projPoints = triangulatePoints(projMatr1, projMatr2, projPoints1, projPoints2)
    #         projMatr1, projMatr2 : 각각 (3, 4) 투영행렬
    #         projPoints1, projPoints2 : 각각 (2, N) 픽셀 좌표
    #           - 첫 행이 u, 둘째 행이 v
    #           - 한 점만 삼각측량할 거면 N=1, 즉 (2, 1) shape
    #         반환 projPoints : (4, N) 동차좌표 (X, Y, Z, W)
    #
    #   구현 절차:
    #   1) 픽셀 좌표를 (2, 1) 배열로 정리
    #      - uv_a = (u, v) 튜플 → np.array([[u], [v]], dtype=np.float64)
    #      - dtype 을 float64 로 명시하면 OpenCV 내부에서 dtype 캐스팅 비용을 아낀다
    #      - 두 카메라 모두 같은 방식으로 정리
    #
    #   2) cv2.triangulatePoints 호출
    #      - 인자 순서 주의: (P_a, P_b, pts_a, pts_b). P 와 pts 의 a/b 짝이 맞아야 한다
    #      - 반환 X_h shape 은 (4, 1). 4D homogeneous (X, Y, Z, W)
    #
    #   3) homogeneous → Euclidean 변환
    #      - 앞 3 성분을 마지막 성분(W) 으로 나눠야 실제 (X, Y, Z) 가 나온다:
    #            X_h[:3] / X_h[3]
    #        scale 이 임의이므로 W 정규화는 필수. 빠뜨리면 좌표 자체가 잘못된다.
    #      - 결과 shape 을 (3,) 로 squeeze 해 반환 (호출 측이 단순 ndarray 연산을 가정)
    #
    #   주의 / 디버깅:
    #   - 결과가 명백히 엉뚱한 곳이면 (예: 수십 미터 오차):
    #       (a) uv_a / uv_b 가 한 점이 아닌 다른 객체의 픽셀일 수 있다
    #       (b) P_a 와 uv_a 의 카메라 짝이 어긋났을 수 있다 (인자 순서 점검)
    #       (c) cam_from_world 의 부호/회전이 뒤집혀 있을 수 있다 (P 단계 점검)
    #   - 정확도는 두 카메라 baseline 이 클수록 좋아진다 (CAM_FRONT + CAM_FRONT_LEFT
    #     은 baseline 이 짧아 5 m 객체에서 수 cm 오차는 정상 범위).
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
    #   cv2.circle API 형태:
    #       cv2.circle(img, center, radius, color, thickness=1, lineType=8, shift=0)
    #         img       : 그릴 대상 이미지 (in-place 수정)
    #         center    : (cx, cy) *정수* tuple. float 넘기면 TypeError
    #         radius    : 반지름 (px, 정수)
    #         color     : (B, G, R) BGR 순서 — RGB 가 아님에 주의
    #         thickness : 양수면 외곽선 두께, *음수* 면 채워진 원 (예: -1)
    #
    #   구현 절차:
    #   - uv 는 (float, float) 이므로 round 후 int 캐스팅
    #         center = (int(round(uv[0])), int(round(uv[1])))
    #     단순 int() 캐스팅은 truncate (소수점 버림) 라 반올림과 1 px 차이날 수 있다.
    #     관측 정확도가 중요한 시각화에서는 round 가 정직.
    #   - cv2.circle 호출 한 번이면 끝. 호출 후 img_bgr 가 in-place 로 바뀌어 있다
    #   - return 은 동일한 img_bgr 참조 (함수형 체이닝 편의)
    #
    #   주의:
    #   - 이미지를 BGR 로 읽고 BGR 로 그린다 (cv2.imread → cv2.imwrite 흐름).
    #     Rerun 으로 보낼 때만 cv2.cvtColor(..., COLOR_BGR2RGB) 로 변환.
    #   - center 가 이미지 범위 밖이어도 cv2.circle 은 에러 없이 잘려 그려진다.
    #     (가시성 판정은 이미 project_world_point 가 했으므로 추가 체크 불필요)
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
    #   Rerun 데이터 모델 복습:
    #     - 엔티티는 경로 문자열로 식별 (예: "world/cam/CAM_FRONT/image")
    #     - 자식 엔티티 좌표는 부모의 frame 에 *상대적*. 부모에 Transform3D 를 박으면
    #       그 아래 모든 자식이 해당 frame 으로 이동/회전 한다.
    #     - 한 엔티티에 Transform3D 와 Pinhole 을 동시에 박으면:
    #         Transform3D : world 좌표계에서 카메라가 어디 놓였는지
    #         Pinhole     : 그 카메라의 내부 파라미터 / 이미지 해상도
    #       → 자식에 Image / Points2D 가 들어오면 자동으로 핀홀 frustum 안에 그려진다.
    #
    #   엔티티 트리 (docstring 와 동일):
    #     world/                           : root, world 좌표계
    #     world/cam/{channel}              : 채널별 카메라 노드 (Transform3D + Pinhole)
    #     world/cam/{channel}/image        : 그 카메라 이미지 (Pinhole frustum 안에 자동 배치)
    #     world/cam/{channel}/observation  : 관측 픽셀 (Points2D)
    #     world/gt_point                   : annotation 3D 위치 (적색, Points3D)
    #     world/triangulated_point         : 복원 3D 위치 (녹색, Points3D)
    #
    #   각 view 처리 절차:
    #   1) prefix 만들기
    #      - f"world/cam/{v.channel}" 형태. 채널 이름 그대로 (예: 'CAM_FRONT')
    #
    #   2) world_from_cam 변환 계산
    #      - Rerun 의 Transform3D 는 부모 -> 자식이 아니라 *부모에서 본 자식의 pose*.
    #        즉 카메라가 world 어디에 놓였나 = world_from_cam.
    #      - 보유한 건 cam_from_world 이므로 한 번 invert: np.linalg.inv(view.cam_from_world)
    #      - translation = world_from_cam[:3, 3] (마지막 열 3 성분, shape (3,))
    #      - mat3x3     = world_from_cam[:3, :3] (좌상단 3x3 블록)
    #      - rr.Transform3D(translation=..., mat3x3=...) 로 prefix 에 log
    #
    #   3) Pinhole 등록
    #      - rr.Pinhole(image_from_camera=view.K, resolution=[view.width, view.height])
    #        를 같은 prefix 에 log (덮어쓰지 않고 같은 엔티티에 component 추가)
    #      - resolution 의 순서는 [width, height] 임에 주의 (HxW 가 아님)
    #
    #   4) 이미지 / 관측 픽셀 child 에 log
    #      - prefix + "/image" 에 rr.Image(images_rgb[v.channel])
    #        ※ 이미지는 RGB 로 들어와야 한다 (cv2 의 BGR 을 미리 cvtColor 해 둘 것)
    #      - obs 에 channel 키가 있을 때만 prefix + "/observation" 에 rr.Points2D([[u, v]])
    #        (단일 점도 (N, 2) shape, 즉 이중 리스트로 감싸기. radius/color 도 옵션으로 줄 수 있음)
    #
    #   3D 점 두 개 (정답 / 복원):
    #   - rr.log("world/gt_point", rr.Points3D([[x, y, z]], colors=[[255, 0, 0]]))
    #     처럼 (N, 3) shape 으로 위치, (N, 3) shape 으로 RGB color 를 넘긴다 (0..255 정수)
    #   - 적색은 정답, 녹색은 복원 — 두 점이 가까이 붙어 보여야 삼각측량이 잘 된 것
    #   - 색 표기 주의: Rerun 은 RGB. OpenCV BGR 과 헷갈리지 말 것.
    #
    #   주의 / 디버깅:
    #   - 카메라가 지구 반대편에 박혀 있으면: world_from_cam 부호 오류
    #     (cam_from_world 를 invert 하지 않고 그대로 넘긴 경우가 가장 흔함)
    #   - 이미지가 엉뚱한 방향으로 보이면: Pinhole 의 resolution 가 (h, w) 로 뒤집혔는지 점검
    #   - 관측 점이 이미지와 같이 안 움직이면: Points2D 를 prefix 가 아닌 root 에 박았을 가능성
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

    # Rerun: RRD 파일로 저장. 로컬에서 `rerun nuscenes_step5_<channel>.rrd` 로 열어볼 것
    rr.init("nuscenes_step5", spawn=False)
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
