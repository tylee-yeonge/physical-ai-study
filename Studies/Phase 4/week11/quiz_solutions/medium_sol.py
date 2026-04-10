"""
Phase 6 Week 11 - 코드 및 문서 정리 중급 퀴즈 정답 및 해설
"""


def problem1_solution():
    print("\n" + "━" * 36)
    print("문제 1 정답: 코드 리팩토링")
    print("━" * 36 + "\n")

    print("  나쁜 코드의 문제점:")
    print()
    print("  1. 함수 이름 불명확: 'process'는 무엇을 처리하는지 알 수 없음")
    print("     → 개선: 'filter_objects_by_distance'")
    print()
    print("  2. 변수 이름 불명확: 'd', 't', 'r'은 의미를 알 수 없음")
    print("     → 개선: 'data', 'verbose', 'filtered_objects'")
    print()
    print("  3. 매직 넘버 사용: 50이 무엇인지 알 수 없음")
    print("     → 개선: MAX_DETECTION_RANGE = 50  # meters")
    print()
    print("  4. docstring 없음: 함수의 목적, 인자, 반환값 설명 없음")
    print("     → 개선: Google 스타일 docstring 추가")
    print()
    print("  5. type hint 없음: 인자와 반환값의 타입을 알 수 없음")
    print("     → 개선: def filter_objects(data: List[List], ...) -> List[Dict]")
    print()
    print("  6. 단일 책임 위반: 필터링과 출력을 한 함수에서 수행")
    print("     → 개선: filter_objects()와 print_objects()로 분리")
    print()
    print("  7. 하드코딩된 클래스 목록: ['Car', 'Pedestrian', 'Cyclist']")
    print("     → 개선: 인자로 받거나 상수로 정의")
    print()
    print("  8. 인덱스 하드코딩: item[11], item[13]의 의미가 불명확")
    print("     → 개선: 명시적 키나 상수 사용 (IDX_X = 11, IDX_Z = 13)")

    print()
    print("  개선된 코드:")
    improved = '''
from typing import List, Dict, Optional

VALID_CLASSES = ['Car', 'Pedestrian', 'Cyclist']
MAX_DETECTION_RANGE = 50.0  # meters

def filter_objects_by_distance(
    labels: List[List[str]],
    max_distance: float = MAX_DETECTION_RANGE,
    target_classes: Optional[List[str]] = None,
) -> List[Dict]:
    """
    KITTI 레이블에서 지정 거리 내의 객체만 필터링한다.

    Args:
        labels: KITTI 레이블 데이터 (각 행은 공백으로 분리된 문자열 리스트).
        max_distance: 최대 검출 거리 (m).
        target_classes: 필터링할 클래스 목록.

    Returns:
        필터링된 객체 정보 딕셔너리의 리스트.
    """
    if target_classes is None:
        target_classes = VALID_CLASSES

    filtered = []
    for label in labels:
        obj_class = label[0]
        if obj_class not in target_classes:
            continue

        x = float(label[11])
        z = float(label[13])
        distance = (x**2 + z**2)**0.5

        if distance < max_distance:
            filtered.append({
                'type': obj_class,
                'distance': distance,
            })

    return filtered
'''
    print(improved)


def problem2_solution():
    print("\n" + "━" * 36)
    print("문제 2 정답: Docstring 작성")
    print("━" * 36 + "\n")

    docstring = '''
def project_to_2d(
    points_3d: np.ndarray,
    P2: np.ndarray,
) -> Tuple[np.ndarray, np.ndarray]:
    """
    3D 점을 KITTI P2 투영 행렬을 사용하여 2D 이미지 좌표로 투영한다.

    카메라 좌표계의 3D 점을 동차 좌표로 변환한 후,
    P2 투영 행렬을 곱하고 정규화하여 2D 픽셀 좌표를 얻는다.
    카메라 뒤에 있는 점(z <= 0)은 유효하지 않은 것으로 표시한다.

    Args:
        points_3d: (N, 3) 3D 좌표 배열. 카메라 좌표계 기준.
            각 행은 [x, y, z] (m).
        P2: (3, 4) 투영 행렬. KITTI 캘리브레이션에서 제공.

    Returns:
        points_2d: (N, 2) 투영된 2D 좌표 [u, v] (pixels).
            유효하지 않은 점도 포함되어 있으므로 valid 마스크와 함께 사용.
        valid: (N,) bool 배열. 카메라 앞에 있는 점이면 True.

    Examples:
        >>> import numpy as np
        >>> points = np.array([[2.0, 1.0, 15.0]])  # 전방 15m
        >>> P2 = np.array([[721.5, 0, 609.5, 44.8],
        ...                [0, 721.5, 172.8, 0.2],
        ...                [0, 0, 1, 0.003]])
        >>> pts_2d, valid = project_to_2d(points, P2)
        >>> print(pts_2d[valid])  # [[705.7, 220.8]]

    Note:
        P2 행렬은 이미 보정(rectification)이 적용된 좌표계 기준이다.
        Tr_velo_to_cam이나 R0_rect와 함께 사용할 때는 적용 순서에 주의하라.
    """
    N = points_3d.shape[0]
    pts_homo = np.hstack([points_3d, np.ones((N, 1))])
    projected = P2 @ pts_homo.T
    projected = projected.T
    valid = projected[:, 2] > 0
    pts_2d = projected[:, :2] / projected[:, 2:3]
    return pts_2d, valid
'''
    print("  완성된 Docstring:")
    print(docstring)

    print("  핵심 포인트:")
    print("  1. 첫 줄: 함수가 무엇을 하는지 한 문장으로")
    print("  2. 상세 설명: 동작 원리를 2-3문장으로")
    print("  3. Args: 각 인자의 shape, 타입, 의미")
    print("  4. Returns: 반환값의 shape, 타입, 의미")
    print("  5. Examples: 실제 실행 가능한 코드")
    print("  6. Note: 주의사항이나 관련 정보")


def problem3_solution():
    print("\n" + "━" * 36)
    print("문제 3 정답: 프로젝트 구조 설계")
    print("━" * 36 + "\n")

    print("  파일 배치 정답:")
    print()
    print("  project/")
    print("  ├── README.md              (루트)")
    print("  ├── requirements.txt       (루트)")
    print("  ├── src/")
    print("  │   ├── models/")
    print("  │   │   ├── fcos3d_model.py       (모델 정의)")
    print("  │   │   └── bevformer_model.py    (모델 정의)")
    print("  │   ├── datasets/")
    print("  │   │   ├── kitti_loader.py       (데이터 로드)")
    print("  │   │   └── nuscenes_loader.py    (데이터 로드)")
    print("  │   ├── evaluation/")
    print("  │   │   ├── compute_ap3d.py       (평가 메트릭)")
    print("  │   │   └── compute_nds.py        (평가 메트릭)")
    print("  │   └── visualization/")
    print("  │       ├── draw_3d_bbox.py       (시각화)")
    print("  │       └── bev_viz.py            (시각화)")
    print("  └── scripts/")
    print("      ├── train.py                  (실행 스크립트)")
    print("      └── test.py                   (실행 스크립트)")
    print()
    print("  배치 기준:")
    print("  - models/: 모델 정의, 래퍼, 설정")
    print("  - datasets/: 데이터 로딩, 전처리, 변환")
    print("  - evaluation/: 평가 메트릭 계산")
    print("  - visualization/: 시각화 유틸리티")
    print("  - scripts/: 학습/테스트 실행 스크립트")
    print("  - 루트: README, requirements, LICENSE")
    print()
    print("  원칙: '이 파일이 주로 무엇을 하는가?'에 따라 분류")


if __name__ == "__main__":
    print("━" * 40)
    print("  Week 11 Quiz - Medium 정답")
    print("━" * 40)
    problem1_solution()
    problem2_solution()
    problem3_solution()
    print("\n" + "━" * 40)
