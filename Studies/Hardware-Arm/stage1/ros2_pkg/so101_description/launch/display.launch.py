"""팔 없이 URDF 만 띄워 보는 launch.

실제 팔에 연결하지 않는다. URDF 가 제대로 읽히는지, 관절이 어느 방향으로 도는지를
확인하는 용도다.

    ros2 launch so101_description display.launch.py            # 화면 없이 (기본)
    ros2 launch so101_description display.launch.py gui:=true  # 슬라이더 + RViz
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.conditions import UnlessCondition
from launch.substitutions import Command
from launch.substitutions import LaunchConfiguration
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description() -> LaunchDescription:
    """URDF 표시용 launch 구성을 만든다.

    Returns:
        robot_state_publisher, joint_state_publisher, (gui:=true 일 때) RViz 를
        담은 LaunchDescription
    """
    # gui:=true 면 관절 슬라이더 창과 RViz 를 띄운다.
    # 기본값이 false 인 이유: 이 컨테이너에는 화면 (DISPLAY) 이 없어 창을 띄울 수 없다
    gui = LaunchConfiguration("gui")
    gui_arg = DeclareLaunchArgument(
        "gui",
        default_value="false",
        description="true 면 관절 슬라이더 창과 RViz 를 띄운다 (화면이 있을 때만)",
    )

    # xacro 명령으로 .urdf.xacro 를 URDF 문자열로 편다
    xacro_file = PathJoinSubstitution(
        [FindPackageShare("so101_description"), "urdf", "so101.urdf.xacro"]
    )
    robot_description = {
        "robot_description": ParameterValue(
            Command(["xacro ", xacro_file]), value_type=str
        )
    }

    # URDF + /joint_states 로 각 링크의 위치 (TF) 를 계산해 내보낸다
    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[robot_description],
        output="both",
    )

    # 팔이 없으므로 /joint_states 를 대신 내보낸다 (화면 없음: 모든 관절 0)
    joint_state_publisher = Node(
        package="joint_state_publisher",
        executable="joint_state_publisher",
        condition=UnlessCondition(gui),
    )

    # 화면 있음: 슬라이더로 관절값을 바꿔 가며 /joint_states 를 내보낸다
    joint_state_publisher_gui = Node(
        package="joint_state_publisher_gui",
        executable="joint_state_publisher_gui",
        condition=IfCondition(gui),
    )

    # RViz — 처음 뜨면 Fixed Frame 을 base_link 로 바꾸고 RobotModel 을 추가한다
    rviz = Node(
        package="rviz2",
        executable="rviz2",
        condition=IfCondition(gui),
    )

    return LaunchDescription(
        [
            gui_arg,
            robot_state_publisher,
            joint_state_publisher,
            joint_state_publisher_gui,
            rviz,
        ]
    )
