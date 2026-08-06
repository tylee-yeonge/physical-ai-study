"""
실습 2/3/4 가 공유하는 action 표현 변환

두 방향을 한 파일에 둔다.

- `to_maniskill_action`: OpenVLA 표현 -> ManiSkill 입력 (정변환). week0 실습 6 과 같은 규칙
- `to_openvla_actions`: expert 궤적 -> OpenVLA 표현 라벨 (역변환). 실습 2

두 방향을 각각 다른 파일에 적으면, 한쪽만 고쳤을 때 **round-trip 은 통과하는데 데이터는
틀린 상태**가 만들어진다. 왕복 검사는 두 함수가 서로의 역이라는 것만 보이지, 그 쌍이
바깥 규약과 맞는지는 보이지 않기 때문이다. 그래서 한 파일에 두고 양쪽에서 부른다.

규칙의 근거는 week0 `action_contract.md` 의 계약 표다.
"""
import numpy as np


POS_LIMIT = 0.1  # pd_ee_delta_pose 의 위치 한계 (m). action 1.0 = 0.1 m (계약 표 1번)
ROT_SCALE = -0.1  # 회전 스케일 (rad). 부호가 반전돼 있다 (계약 표 4번)


def to_maniskill_action(raw_action: np.ndarray) -> np.ndarray:
    """OpenVLA 표현의 action(7,) 을 ManiSkill pd_ee_delta_pose action(7,) 으로 바꾼다.

    OpenVLA 는 물리량(m, rad)을 돌려주고 ManiSkill 은 전 차원 `[-1, 1]` 정규화값을
    받는다. 이 함수를 거치지 않으면 위치 명령이 의도의 1/10 로 줄고 회전이 반대로 돈다.

    Args:
        raw_action: (7,) 배열. `[dx,dy,dz]`(m), `[drx,dry,drz]`(rad), gripper `[0,1]`

    Returns:
        ManiSkill action (7,) float32. 전 차원 `[-1, 1]` 정규화값
    """
    pos = raw_action[:3] / POS_LIMIT  # 미터 -> 정규화 (±0.1 m 가 ±1)
    rot = raw_action[3:6] / ROT_SCALE  # 라디안 -> 정규화, rot_lower 곱셈 때문에 부호 반전
    rot_norm = np.linalg.norm(rot)  # 회전은 축별이 아니라 3벡터 노름으로 제한된다
    if rot_norm > 1.0:  # 노름이 1 을 넘으면 방향을 유지한 채 축소
        rot = rot / rot_norm
    grip = 2.0 * raw_action[6] - 1.0  # [0,1](0=닫힘) -> [-1,1](-1=닫힘)
    action = np.concatenate([np.clip(pos, -1, 1), rot, [np.clip(grip, -1, 1)]])
    return action.astype(np.float32)  # env.step 이 받는 dtype 으로 맞춘다


def to_openvla_actions(expert_actions: np.ndarray) -> np.ndarray:
    """expert 가 낸 action 을 OpenVLA 표현의 학습 라벨로 바꾼다 (역변환).

    라벨은 expert 가 **명령한** 델타를 물리 단위로 되돌린 값이다. 실제로 이동한 양
    (연속 TCP pose 의 차분)이 아니다. PD 컨트롤러는 한 스텝에 명령의 약 42% 만 따라가므로,
    실제 이동량을 라벨로 쓰면 그 값이 다시 명령이 되어 궤적이 재현되지 않는다
    (`outputs/roundtrip_check.md`). 원격조작 데이터셋이 사람의 **명령**을 기록하는 것과도
    같은 규약이다.

    정규화하지 않은 원시 물리 단위(미터·라디안)로 만든다. 정규화는 week2 의 학습
    파이프라인이 데이터셋 통계로 수행하므로, 여기서 미리 맞추면 이중 정규화가 된다.

    회전 3차원의 의미는 **intrinsic XYZ 오일러각**이다 -- 컨트롤러가 이 값을
    `euler_angles_to_matrix(delta, "XYZ")` 로 해석하기 때문이다 (`pd_ee_pose.py:242`).

    Args:
        expert_actions: (T, 7) 배열. expert 가 낸 ManiSkill 정규화 action.
            `[-1, 1]` 범위의 위치 3 + 회전 3 + gripper 1

    Returns:
        (T-1, 7) float32. `[dx,dy,dz]`(m) + `[drx,dry,drz]`(rad) + gripper `[0,1]`
    """
    # step t 의 라벨은 t -> t+1 을 만든 명령이다. 마지막 스텝의 명령은 대응하는 다음
    # 프레임이 없으므로 버린다. 그래서 라벨 개수가 관측보다 하나 적다
    commands = expert_actions[:-1]

    delta_position = commands[:, 0:3] * POS_LIMIT  # 정규화값 -> 미터 (계약 표 1번의 역)
    delta_rotation = commands[:, 3:6] * ROT_SCALE  # 정규화값 -> 라디안 (계약 표 4번의 역)
    gripper = (commands[:, 6:7] + 1.0) / 2.0  # [-1,1](-1=닫힘) -> [0,1](0=닫힘)

    labels = np.concatenate([delta_position, delta_rotation, gripper], axis=1)
    return labels.astype(np.float32)
