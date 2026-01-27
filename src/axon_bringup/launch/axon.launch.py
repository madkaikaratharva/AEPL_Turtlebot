import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration

from launch_ros.actions import Node

def generate_launch_description():

    # launch arguments
    use_sim_time_arg = DeclareLaunchArgument(
        name="use_sim_time",
        default_value="False"
    )
    use_sim_time = LaunchConfiguration("use_sim_time")

    # launch hardware interface
    hardware_interface_launch = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("axon_firmware"),
            "launch",
            "hardware_interface.launch.py"
        )
    )

    # axon controller
    axon_controller_launch = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("axon_controller"),
            "launch",
            "controller.launch.py"
        ),
        launch_arguments={
            "use_sim_time": "False"
        }.items()
    )

    # joystick teleop
    joystick_teleop_launch = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("axon_controller"),
            "launch",
            "joystick_teleop.launch.py"
        ),
        launch_arguments={
            "use_sim_time": "False"
        }.items()
    )

    # rplidar node
    rplidar_node = Node(
        package="rplidar_ros",
        executable="rplidar_node",
        name="rplidar_node",
        parameters=[
            os.path.join(get_package_share_directory("axon_firmware"), "config", "rplidar_a1_config.yaml"),
            {"use_sim_time": use_sim_time}
        ],
        output="screen"
    )

    # imu driver
    imu_driver_node = Node(
        package="axon_firmware",
        executable="mpu6050_driver.py",
        parameters=[
            {"use_sim_time": use_sim_time}
        ]
    )

    return LaunchDescription([
        use_sim_time_arg,
        hardware_interface_launch,
        axon_controller_launch,
        joystick_teleop_launch,
        rplidar_node
    ])
