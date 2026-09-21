"""팔을 ROS 2 에 붙이는 launch (드라이버 + 컨트롤러).

    # 실제 팔 — 띄우면 토크가 켜지고, 끄면 (Ctrl+C) 토크가 풀려 팔이 떨어진다.
    # 띄우기 전과 끄기 전에 팔을 휴식 자세에 둔다 (ros2_driver_setup.md 4절)
    ros2 launch so101_description bringup.launch.py

    # 팔 없이 launch / 컨트롤러 설정만 시험 (시리얼 포트를 열지 않는다)
    ros2 launch so101_description bringup.launch.py use_mock_hardware:=true

실제 팔로 띄울 때는 LeRobot 프로세스가 꺼져 있어야 한다 (같은 시리얼 포트를 쓴다).
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import Command
from launch.substitutions import LaunchConfiguration
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description() -> LaunchDescription:
    """팔 구동용 launch 구성을 만든다.

    Returns:
        robot_state_publisher, ros2_control_node, 컨트롤러 spawner,
        (gui:=true 일 때) RViz 를 담은 LaunchDescription
    """
    # use_mock_hardware:=true 면 실제 드라이버 대신 가짜 하드웨어를 쓴다
    use_mock_hardware = LaunchConfiguration("use_mock_hardware")
    use_mock_hardware_arg = DeclareLaunchArgument(
        "use_mock_hardware",
        default_value="false",
        description="true 면 팔에 연결하지 않고 가짜 하드웨어로 띄운다",
    )

    # gui:=true 면 RViz 를 띄운다. 이 컨테이너에는 화면이 없어 기본값은 false 다
    gui = LaunchConfiguration("gui")
    gui_arg = DeclareLaunchArgument(
        "gui",
        default_value="false",
        description="true 면 RViz 를 띄운다 (화면이 있을 때만)",
    )

    # xacro 에 use_mock_hardware 값을 넘겨 <ros2_control> 의 플러그인을 고른다
    xacro_file = PathJoinSubstitution(
        [FindPackageShare("so101_description"), "urdf", "so101.urdf.xacro"]
    )
    robot_description = {
        "robot_description": ParameterValue(
            Command(
                ["xacro ", xacro_file, " use_mock_hardware:=", use_mock_hardware]
            ),
            value_type=str,
        )
    }

    controllers_file = PathJoinSubstitution(
        [FindPackageShare("so101_description"), "config", "so101_controllers.yaml"]
    )

    # URDF 를 /robot_description 토픽으로 내보내고, /joint_states 로 TF 를 계산한다
    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[robot_description],
        output="both",
    )

    # ros2_control 본체. /robot_description 에서 <ros2_control> 블록을 읽어
    # 하드웨어 플러그인 (feetech 드라이버 또는 가짜 하드웨어) 을 불러온다
    ros2_control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[controllers_file],
        remappings=[("~/robot_description", "/robot_description")],
        output="both",
    )

    # yaml 에 정의한 컨트롤러 두 개를 불러와 켠다
    controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "position_controller"],
        output="both",
    )

    # RViz — 처음 뜨면 Fixed Frame 을 base_link 로 바꾸고 RobotModel 을 추가한다
    rviz = Node(
        package="rviz2",
        executable="rviz2",
        condition=IfCondition(gui),
    )

    return LaunchDescription(
        [
            use_mock_hardware_arg,
            gui_arg,
            robot_state_publisher,
            ros2_control_node,
            controller_spawner,
            rviz,
        ]
    )
