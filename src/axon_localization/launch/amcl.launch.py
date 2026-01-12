import os
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution


def generate_launch_description():

    # Declare Launch Arguments
    map_name_arg = DeclareLaunchArgument(
        name="map_name",
        default_value="small_house"
    )
    map_name = LaunchConfiguration("map_name")

    use_sim_time_arg = DeclareLaunchArgument(
        name="use_sim_time",
        default_value="true"
    )
    use_sim_time = LaunchConfiguration("use_sim_time")

    amcl_config_arg = DeclareLaunchArgument(
        name="amcl_config",
        default_value=os.path.join(
            get_package_share_directory("axon_localization"),
            "config",
            "amcl.yaml"
        ),
        description="Full path to amcl yaml file to load"
    )
    amcl_config = LaunchConfiguration("amcl_config")

    # Map Path
    map_path = PathJoinSubstitution([
        get_package_share_directory("axon_mapping"), "maps", map_name, "map.yaml"
    ])
    
    # NAV2 MAP SERVER NODE. Publishes the map
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

    # NAV2 AMCL NODE
    nav2_amcl = Node(
        package="nav2_amcl",
        executable="amcl",
        name="amcl",
        output="screen",
        emulate_tty=True,
        parameters=[
            amcl_config,
            {"use_sim_time": use_sim_time}
        ]
    )

    # NAV2 LIFECYCLE MANAGER NODE
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
        amcl_config_arg,
        nav2_map_server,
        nav2_amcl,
        nav2_lifecycle_manager
    ])