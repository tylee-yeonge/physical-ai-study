"""측정한 영점 오프셋을 URDF joint 의 <origin rpy> 에 합성해 새 rpy 를 출력한다.

파일을 읽기만 한다. 출력된 새 rpy 를 so101.urdf.xacro 의 해당 joint 에 손으로 옮겨 적는다
(URDF_guide.md 5.5절). bringup 과 무관하게 언제든 실행할 수 있다.

    python3 /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/apply_joint_offset.py shoulder_lift 4.7
    python3 /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/apply_joint_offset.py elbow_flex 12.6
    python3 /workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/scripts/apply_joint_offset.py gripper 39.7

오프셋의 부호: "관절값이 q 일 때 실물이 URDF 보다 관절의 +방향으로 몇 도 더 돌아 있는가".
    수평계로 잰 실물 각도 - 그때의 관절값 = 오프셋 (예: 위팔 7.4도, 관절값 2.7도 -> 4.7도).

왜 yaw 에 그냥 더하면 안 되는가:
    URDF 의 rpy 는 회전 행렬 R = Rz(yaw) * Ry(pitch) * Rx(roll) 이고, 관절은 그 뒤에 자식 z 축
    둘레로 q 만큼 돈다 (자식 좌표계 = R * Rz(q)). 실물이 q 에서 URDF 보다 theta 만큼 더 돌아
    있으면 R_new = R * Rz(theta) 로 두어야 R_new * Rz(q) = R * Rz(q + theta) 가 된다.
    R 에 pitch 나 roll 이 있으면 (이 파일의 대부분) R * Rz(theta) 의 yaw 는 yaw + theta 가
    아니므로, 행렬을 곱한 뒤 rpy 세 값으로 다시 풀어야 한다. 이 스크립트가 그 일을 한다.
"""

import argparse
import math
import xml.etree.ElementTree as ET
from typing import List
from typing import Tuple

# 기본 대상: 이 레포의 URDF 원본 (install 폴더는 여기로 심링크돼 있다)
DEFAULT_XACRO = (
    "/workspace/study/physical-ai-study/Studies/Hardware-Arm/stage1/"
    "ros2_pkg/so101_description/urdf/so101.urdf.xacro"
)

Matrix = List[List[float]]


def rot_x(a: float) -> Matrix:
    """x 축 둘레 회전 행렬 (roll)."""
    c, s = math.cos(a), math.sin(a)
    return [[1, 0, 0], [0, c, -s], [0, s, c]]


def rot_y(a: float) -> Matrix:
    """y 축 둘레 회전 행렬 (pitch)."""
    c, s = math.cos(a), math.sin(a)
    return [[c, 0, s], [0, 1, 0], [-s, 0, c]]


def rot_z(a: float) -> Matrix:
    """z 축 둘레 회전 행렬 (yaw). 이 URDF 의 관절축은 전부 자식 z 축이다."""
    c, s = math.cos(a), math.sin(a)
    return [[c, -s, 0], [s, c, 0], [0, 0, 1]]


def matmul(a: Matrix, b: Matrix) -> Matrix:
    """3x3 행렬 곱 a * b."""
    return [[sum(a[i][k] * b[k][j] for k in range(3)) for j in range(3)] for i in range(3)]


def rpy_to_matrix(r: float, p: float, y: float) -> Matrix:
    """URDF 규약 (고정축 XYZ): R = Rz(y) * Ry(p) * Rx(r)."""
    return matmul(rot_z(y), matmul(rot_y(p), rot_x(r)))


def matrix_to_rpy(m: Matrix) -> Tuple[float, float, float]:
    """회전 행렬을 URDF rpy 로 되돌린다. pitch 가 +-90도인 경우 (gimbal lock) 는 yaw 를 0 으로 둔다."""
    if abs(m[2][0]) < 1.0 - 1e-9:
        pitch = -math.asin(m[2][0])
        roll = math.atan2(m[2][1], m[2][2])
        yaw = math.atan2(m[1][0], m[0][0])
    elif m[2][0] > 0:
        pitch, yaw = -math.pi / 2, 0.0
        roll = math.atan2(-m[0][1], m[1][1])
    else:
        pitch, yaw = math.pi / 2, 0.0
        roll = math.atan2(m[0][1], m[1][1])
    return roll, pitch, yaw


def main() -> None:
    """joint 이름과 오프셋 (도) 을 받아 새 rpy 를 출력한다."""
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("joint", help="joint 이름 (예: shoulder_lift)")
    parser.add_argument("offset_deg", type=float, help="오프셋 (도). 관절의 +방향으로 실물이 더 돌아 있으면 +")
    parser.add_argument("--file", default=DEFAULT_XACRO, help="읽을 xacro 경로 (기본: 레포의 so101.urdf.xacro)")
    args = parser.parse_args()

    # xacro 파일에서 해당 joint 의 <origin> 을 찾는다 (없으면 rpy = 0 0 0)
    root = ET.parse(args.file).getroot()
    joint = root.find(f".//joint[@name='{args.joint}']")
    if joint is None or joint.find("parent") is None:
        raise SystemExit(f"joint 를 찾지 못했다: {args.joint} (ros2_control 블록의 joint 가 아니라 링크를 잇는 joint 이름)")
    origin = joint.find("origin")
    rpy_old = tuple(float(v) for v in (origin.get("rpy") if origin is not None else "0 0 0").split())
    xyz = origin.get("xyz") if origin is not None else "0 0 0"

    # 핵심 한 줄: R_new = R_old * Rz(theta)
    theta = math.radians(args.offset_deg)
    r_new = matmul(rpy_to_matrix(*rpy_old), rot_z(theta))
    rpy_new = matrix_to_rpy(r_new)

    # 되풀이 검산: 새 rpy 로 행렬을 다시 만들면 R_new 와 같아야 한다
    r_check = rpy_to_matrix(*rpy_new)
    err = max(abs(r_check[i][j] - r_new[i][j]) for i in range(3) for j in range(3))

    print(f"joint {args.joint}: 오프셋 {args.offset_deg:+.2f}도 = {theta:+.5f} rad")
    print(f"  현재 rpy = {' '.join(f'{v:g}' for v in rpy_old)}")
    print(f"  새   rpy = {rpy_new[0]:.5f} {rpy_new[1]:.5f} {rpy_new[2]:.5f}   (검산 오차 {err:.1e})")
    print("\n# so101.urdf.xacro 의 해당 joint 에 옮겨 적을 줄 (xyz 는 그대로):")
    print(f'<origin xyz="{xyz}" rpy="{rpy_new[0]:.5f} {rpy_new[1]:.5f} {rpy_new[2]:.5f}"/>')


if __name__ == "__main__":
    main()
