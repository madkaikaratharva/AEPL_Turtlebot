import os
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution


def generate_launch_description():


    # Declare Launch Arguments
    use_sim_time_arg = DeclareLaunchArgument(
        name="use_sim_time",
        default_value="true"
    )
    use_sim_time = LaunchConfiguration("use_sim_time")

    slam_config_arg = DeclareLaunchArgument(
        name="slam_config",
        default_value=os.path.join(
            get_package_share_directory("axon_mapping"), "config", "slam_toolbox.yaml"
        ),
        description="Full path to slam yaml file to load"
    )
    slam_config = LaunchConfiguration("slam_config")
    
    # MAP SAVER NODE CONFIG
    nav2_map_saver_node = Node(
        package="nav2_map_server",
        executable="map_saver_server",
        name="map_saver_server",
        output="screen",
        parameters=[
            {"save_map_timeout": 5.0},
            {"use_sim_time": use_sim_time},
            {"free_thresh_default", "0.196"},
            {"occupied_thresh_default", "0.65"},
        ]
    )

    # SLAM TOOLBOX NODE
    slam_toolbox_node = Node(
        package="slam_toolbox",
        executable="sync_slam_toolbox_node",
        name="slam_toolbox",
        output="screen",
        parameters=[
            slam_config,
            {"use_sim_time": use_sim_time},
        ],
    )

    return LaunchDescription([
        use_sim_time_arg,
        slam_config_arg,
        slam_toolbox_node
    ])
