#!/usr/bin/env python3
"""소프트웨어 정지 노드 — 토크를 유지한 채 현 위치에 멈추고, 명령 흐름을 끊는다.

ros2_driver_setup.md §6.4 의 명세를 그대로 구현한다. bringup.launch.py 가 같이 띄운다.

    # 정지: 현재 위치를 목표로 한 번 보낸 뒤 position_controller 를 비활성화한다
    ros2 service call /soft_stop/stop std_srvs/srv/Trigger

    # 해제: position_controller 를 다시 활성화한다. 그 뒤 첫 명령은 §4.2 규칙대로
    ros2 service call /soft_stop/release std_srvs/srv/Trigger

왜 명령만 끊으면 안 되는가: 위치 제어는 마지막 목표까지 계속 간다. 이동 중에 컨트롤러만
끄면 서보는 마지막 목표를 향해 계속 움직인다. 그래서 현재 위치를 목표로 덮어쓴 다음에 끊는다.
정지 중에는 position_controller 가 비활성이라 /position_controller/commands 로 오는 명령이
하드웨어에 전달되지 않는다. joint_state_broadcaster 는 그대로 돌아 /joint_states 는 살아 있다.
"""

import time
from typing import Dict
from typing import List
from typing import Optional

import rclpy
from controller_manager_msgs.srv import SwitchController
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.executors import MultiThreadedExecutor
from rclpy.node import Node
from sensor_msgs.msg import JointState
from std_msgs.msg import Float64MultiArray
from std_srvs.srv import Trigger

# config/so101_controllers.yaml 의 joints 순서 = 명령 배열의 순서
COMMAND_ORDER: List[str] = [
    "shoulder_pan",
    "shoulder_lift",
    "elbow_flex",
    "wrist_flex",
    "wrist_roll",
    "gripper",
]
CONTROLLER = "position_controller"  # 끊고 다시 켤 컨트롤러 이름 (yaml 과 같아야 한다)
APPLY_WAIT_S = 0.05  # 유지 명령을 보낸 뒤 컨트롤러가 한 주기 (10 ms) 적용할 시간을 준다
SWITCH_TIMEOUT_S = 2.0  # controller_manager 서비스 응답 대기 상한


class SoftStop(Node):
    """/joint_states 를 보고 있다가 stop · release 서비스로 컨트롤러를 끄고 켠다."""

    def __init__(self) -> None:
        super().__init__("soft_stop")
        # 서비스 콜백 안에서 다른 서비스를 호출하므로, 콜백이 서로를 막지 않는 그룹을 쓴다
        self._group = ReentrantCallbackGroup()
        self._latest: Optional[Dict[str, float]] = None  # 가장 최근 /joint_states (이름 -> rad)
        self.create_subscription(
            JointState, "/joint_states", self._on_joint_states, 10, callback_group=self._group
        )
        self._pub = self.create_publisher(Float64MultiArray, f"/{CONTROLLER}/commands", 10)
        self._switch = self.create_client(
            SwitchController, "/controller_manager/switch_controller", callback_group=self._group
        )
        self.create_service(Trigger, "~/stop", self._on_stop, callback_group=self._group)
        self.create_service(Trigger, "~/release", self._on_release, callback_group=self._group)
        self.get_logger().info("soft_stop ready: ~/stop, ~/release (std_srvs/Trigger)")

    def _on_joint_states(self, msg: JointState) -> None:
        """관절 이름으로 위치를 저장한다 (/joint_states 는 알파벳 순서라 이름으로 찾는다)."""
        self._latest = dict(zip(msg.name, msg.position))

    def _switch_controller(self, activate: List[str], deactivate: List[str]) -> SwitchController.Response:
        """controller_manager 에 컨트롤러 켜기 / 끄기를 요청하고 응답을 돌려준다.

        Args:
            activate: 활성화할 컨트롤러 이름 목록
            deactivate: 비활성화할 컨트롤러 이름 목록

        Returns:
            SwitchController 응답. 서비스가 없으면 ok=False 인 응답
        """
        if not self._switch.wait_for_service(timeout_sec=SWITCH_TIMEOUT_S):
            self.get_logger().error("/controller_manager/switch_controller 가 없다 (bringup 이 떠 있는가?)")
            return SwitchController.Response(ok=False)
        request = SwitchController.Request()
        request.activate_controllers = activate
        request.deactivate_controllers = deactivate
        request.strictness = SwitchController.Request.STRICT  # 하나라도 안 되면 전체 실패
        request.timeout.sec = int(SWITCH_TIMEOUT_S)
        return self._switch.call(request)

    def _on_stop(self, request: Trigger.Request, response: Trigger.Response) -> Trigger.Response:
        """현재 위치를 목표로 1회 보내고 position_controller 를 끈다."""
        del request  # Trigger 요청은 내용이 없다
        if self._latest is None:
            response.success = False
            response.message = "/joint_states 를 아직 받지 못했다 — 아무것도 보내지 않음"
            return response
        missing = [name for name in COMMAND_ORDER if name not in self._latest]
        if missing:
            response.success = False
            response.message = f"/joint_states 에 없는 관절: {missing} — 아무것도 보내지 않음"
            return response

        # ① 현재 위치를 yaml 순서로 늘어놓아 유지 명령으로 보낸다
        hold = [self._latest[name] for name in COMMAND_ORDER]
        self._pub.publish(Float64MultiArray(data=hold))
        time.sleep(APPLY_WAIT_S)  # 컨트롤러가 이 목표를 하드웨어에 한 번 쓸 시간

        # ② 컨트롤러를 끈다 — 이후 /position_controller/commands 는 하드웨어에 가지 않는다
        result = self._switch_controller(activate=[], deactivate=[CONTROLLER])
        values = ", ".join(f"{name}={value:.4f}" for name, value in zip(COMMAND_ORDER, hold))
        if not result.ok:
            response.success = False
            response.message = f"유지 명령은 보냈으나 {CONTROLLER} 비활성화 실패 (이미 정지 중?) — {values}"
            self.get_logger().error(response.message)
            return response
        response.success = True
        response.message = f"정지: {CONTROLLER} 비활성, 유지 목표 {values}"  # ③ 쓴 값을 응답에 남긴다
        self.get_logger().warn(response.message)
        return response

    def _on_release(self, request: Trigger.Request, response: Trigger.Response) -> Trigger.Response:
        """position_controller 를 다시 켠다. 켠 뒤 첫 명령은 현재 위치 + 한 관절만 조금 (§4.2)."""
        del request
        result = self._switch_controller(activate=[CONTROLLER], deactivate=[])
        if not result.ok:
            response.success = False
            response.message = f"{CONTROLLER} 활성화 실패 (이미 켜져 있나?)"
            self.get_logger().error(response.message)
            return response
        response.success = True
        response.message = f"해제: {CONTROLLER} 활성. 첫 명령은 현재 위치 + 한 관절만 조금 (§4.2)"
        self.get_logger().info(response.message)
        return response


def main() -> None:
    """노드를 멀티스레드 실행기로 돌린다 (서비스 콜백 안의 서비스 호출이 막히지 않게)."""
    rclpy.init()
    node = SoftStop()
    executor = MultiThreadedExecutor(num_threads=4)
    executor.add_node(node)
    try:
        executor.spin()
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()
