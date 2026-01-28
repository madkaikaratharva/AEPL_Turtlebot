import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

from launch_ros.actions import Node

def generate_launch_description():

    # launch arguments
    use_sim_time_arg = DeclareLaunchArgument(
        name="use_sim_time",
        default_value="False"
    )
    use_sim_time = LaunchConfiguration("use_sim_time")

    # EKF filter node
    ekf_filter_node = Node(
        package="robot_localization",
        executable="ekf_node",
        name="ekf_filter_node",
        output="screen",
        parameters=[
            os.path.join(get_package_share_directory("axon_localization"), "config", "ekf.yaml"),
            {"use_sim_time": use_sim_time}
        ]
    )

    # IMU Republisher node
    imu_republisher_node = Node(
        package="axon_localization",
        executable="imu_republisher",
        output="screen",
        parameters=[
            {"use_sim_time": use_sim_time}
        ]
    )

    return LaunchDescription([
        use_sim_time_arg,
        ekf_filter_node,
        imu_republisher_node
    ])