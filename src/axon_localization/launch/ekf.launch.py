import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

from launch_ros.actions import Node


def generate_launch_description():

    # Declare Launch Arguments
    use_sim_time_arg = DeclareLaunchArgument(
        name="use_sim_time",
        default_value="True"
    )
    use_sim_time = LaunchConfiguration("use_sim_time")

    #static_transform_publisher = Node(
    #    package="tf2_ros",
    #    executable="static_transform_publisher",
    #    arguments=["--x", "0", "--y", "0","--z", "0.103",
    #               "--qx", "1", "--qy", "0", "--qz", "0", "--qw", "0",
    #               "--frame-id", "base_footprint_ekf",
    #               "--child-frame-id", "imu_link_ekf"]
    #)

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

    imu_republisher_node = Node(
        package="axon_localization",
        executable="imu_republisher"
    )

    return LaunchDescription([
        use_sim_time_arg,
        ekf_filter_node,
        imu_republisher_node
    ])