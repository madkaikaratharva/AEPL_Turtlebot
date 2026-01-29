import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node

def generate_launch_description():

    # launch arguments
    map_name_arg = DeclareLaunchArgument(
        name="map_name",
        default_value="office_test"
    )
    map_name = LaunchConfiguration("map_name")

    use_sim_time_arg = DeclareLaunchArgument(
        name="use_sim_time",
        default_value="False"
    )
    use_sim_time = LaunchConfiguration("use_sim_time")

    # map path
    map_path = PathJoinSubstitution([
        get_package_share_directory("axon_mapping"),
        "maps",
        map_name,
        "office_test.yaml"
    ])

    # Nav2 Map Server
    nav2_map_server = Node(
        package="nav2_map_server",
        executable="map_server",
        name="map_server",
        output="screen",
        parameters=[
            {"yaml_filename": map_path},
            {"use_sim_time": use_sim_time}
        ]
    )

    # Nav2 AMCL
    nav2_amcl = Node(
        package="nav2_amcl",
        executable="amcl",
        name="amcl",
        output="screen",
        emulate_tty=True,
        parameters=[
            os.path.join(get_package_share_directory("axon_localization"), "config", "amcl.yaml"),
            {"use_sim_time": use_sim_time}
        ]
    )

    # Lifecycle manager
    nav2_lifecycle_manager = Node(
        package="nav2_lifecycle_manager",
        executable="lifecycle_manager",
        name="lifecycle_manager_localization",
        output="screen",
        parameters=[
            {"node_names": ["map_server", "amcl"]},
            {"use_sim_time": use_sim_time},
            {"autostart": True}
        ]
    )

    return LaunchDescription([
        map_name_arg,
        use_sim_time_arg,
        nav2_map_server,
        nav2_amcl,
        nav2_lifecycle_manager
    ])