"""
실습 1/4 가 공유하는 scripted expert

week0 실습 5(`../week0/harness_check.py`)가 상한 대조에 쓴 정책과 같은 규칙이다.
큐브·목표 좌표를 시뮬레이터에서 직접 읽고(카메라를 보지 않는다) above -> descend ->
close -> lift -> hold 5단계로 움직인다.

이 파일을 따로 둔 이유: 실습 1 의 궤적 덤프와 실습 4 의 본 수집이 **같은 정책**을 써야
한다. 두 곳에 각각 적으면 한쪽만 고치는 순간 라벨과 데이터가 어긋나고, 그 어긋남은
파일이 정상적으로 쌓이기 때문에 눈에 띄지 않는다.
"""
import numpy as np


POS_LIMIT = 0.1  # pd_ee_delta_pose 의 위치 한계 (m). action 1.0 = 0.1 m (week0 계약 표 1번)
MAX_STEP_M = 0.03  # 한 step 에 요청할 최대 이동량 (m). 크게 잡으면 PD 추종이 흔들린다
APPROACH_HEIGHT = 0.05  # 큐브 위 어느 높이에서 하강을 시작할지 (m)
CLOSE_STEPS = 8  # 그리퍼가 실제로 닫히기까지 기다리는 step 수
ABOVE_TOL = 0.008  # above 단계를 마칠 거리 기준 (m)
DESCEND_TOL = 0.006  # descend 단계를 마칠 거리 기준 (m)
GOAL_TOL = 0.02  # 큐브가 목표에 이만큼 붙으면 정지 단계로 (m). goal_thresh(0.025) 보다 보수적


def to_vec(pose_field) -> np.ndarray:
    """배치 텐서로 오는 좌표를 (3,) numpy 벡터로 바꾼다.

    Args:
        pose_field: `pose.p` 같은 (1, 3) 형태의 텐서

    Returns:
        (3,) float numpy 배열
    """
    return np.asarray(pose_field.cpu())[0]  # 텐서 -> CPU -> numpy, 배치 차원 제거


class ScriptedExpert:
    """PickCube-v1 을 푸는 상태 기반 scripted 정책.

    이 정책의 action 은 현재 상태만으로 정해지지 않는다 -- 지금 어느 단계인지(`phase`)와
    그리퍼를 몇 step 째 닫고 있는지(`close_count`)를 기억해야 한다. 그래서 한 줄 함수가
    아니라 클래스이고, episode 를 새로 시작할 때마다 `reset()` 으로 단계를 되돌려야 한다.
    """

    def __init__(self, env):
        """정책을 환경에 연결한다.

        Args:
            env: `pd_ee_delta_pose` 로 생성된 ManiSkill 환경
        """
        self.base = env.unwrapped  # 큐브·목표 좌표는 wrapper 를 벗겨야 보인다
        self.reset()  # 단계 상태 초기화

    def reset(self) -> None:
        """단계 상태를 처음으로 되돌린다. `env.reset()` 직후마다 함께 호출한다."""
        self.phase = "above"  # 현재 단계 (above -> descend -> close -> lift -> hold)
        self.close_count = 0  # 그리퍼 닫기 명령을 몇 step 유지했는지

    def act(self) -> np.ndarray:
        """현재 sim 상태를 읽어 이번 step 의 action 을 만든다.

        Returns:
            (7,) float32 action. EEF 위치 델타 3 + 회전 델타 3 + gripper 1,
            전 차원 `[-1, 1]` 정규화값
        """
        tcp = to_vec(self.base.agent.tcp.pose.p)  # 그리퍼 끝(TCP) 현재 위치
        cube = to_vec(self.base.cube.pose.p)  # 큐브 위치 (특권 정보 -- 카메라를 안 본다)
        goal = to_vec(self.base.goal_site.pose.p)  # 목표 지점 위치
        grip = 1.0  # 기본은 열림 (+1 = 열림, week0 계약 표 5번)

        if self.phase == "above":  # 1) 큐브 위쪽으로 이동
            target = cube + np.array([0.0, 0.0, APPROACH_HEIGHT])
            if np.linalg.norm(target - tcp) < ABOVE_TOL:  # 충분히 도달하면 다음 단계
                self.phase = "descend"
        elif self.phase == "descend":  # 2) 큐브 중심 높이까지 하강
            target = cube
            if np.linalg.norm(target - tcp) < DESCEND_TOL:
                self.phase = "close"
        elif self.phase == "close":  # 3) 제자리에서 그리퍼 닫기
            target = tcp  # 이동 없음 (델타 0)
            grip = -1.0  # -1 = 닫힘
            self.close_count += 1
            if self.close_count >= CLOSE_STEPS:  # 실제로 닫힐 시간을 준 뒤 다음 단계
                self.phase = "lift"
        elif self.phase == "lift":  # 4) 큐브를 든 채 목표 지점으로 이동
            target = goal
            grip = -1.0
            if np.linalg.norm(goal - cube) < GOAL_TOL:
                self.phase = "hold"
        else:  # 5) 정지 -- success 판정이 is_robot_static 도 요구한다
            target = tcp
            grip = -1.0

        delta = np.clip(target - tcp, -MAX_STEP_M, MAX_STEP_M)  # step 당 이동량 제한
        action = np.concatenate(
            [
                delta / POS_LIMIT,  # 미터 -> [-1, 1] 정규화 (week0 계약 표 1번)
                np.zeros(3),  # 회전 델타 없음 (초기 자세가 이미 그리퍼를 아래로 향한다)
                [grip],  # gripper 명령
            ]
        )
        return action.astype(np.float32)
