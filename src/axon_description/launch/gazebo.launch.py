import os
from os import pathsep
from pathlib import Path
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution, PythonExpression
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue

def generate_launch_description():
    
    # description pkg directory
    axon_description_dir = get_package_share_directory("axon_description")

    # ROS Distro
    ros_distro = os.environ["ROS_DISTRO"]
    is_ignition = "True" if ros_distro == "humble" else "False"

    # launch arguments
    model_arg = DeclareLaunchArgument(
        name="model",
        default_value=os.path.join(axon_description_dir, "urdf", "axon.urdf.xacro"),
        description="Absolute path to robot's urdf"
    )
    model = LaunchConfiguration("model")

    world_name_arg = DeclareLaunchArgument(
        name="world_name",
        default_value="empty"
    )
    world_name = LaunchConfiguration("world_name")


    # path to gazebo world
    world_path = PathJoinSubstitution([
        axon_description_dir,
        "worlds",
        PythonExpression(expression=["'", world_name, "'", " + '.world'"])
    ])

    # gazebo resource path
    model_path = str(Path(axon_description_dir).parent.resolve())
    model_path += pathsep + os.path.join(axon_description_dir, 'models')
    gazebo_resource_path = SetEnvironmentVariable(
        "GZ_SIM_RESOURCE_PATH",
        model_path
    )

    # robot description
    robot_description = ParameterValue(
        Command([
            "xacro ",
            model,
            " is_ignition:=",
            is_ignition,
            " is_sim:=",
            "true"
        ]),
        value_type=str
    )

    # robot_state_publisher
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[
            {"robot_description": robot_description,
             "use_sim_time": True}
        ]
    )

    # gazebo
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            os.path.join(get_package_share_directory("ros_gz_sim"), "launch"),
            "/gz_sim.launch.py"
        ]),
        launch_arguments={
            "gz_args": PythonExpression(["'", world_path, " -v 4 -r'"])
        }.items()
    )

    # gazebo robot spawner
    gz_spawner = Node(
        package="ros_gz_sim",
        executable="create",
        output="screen",
        arguments=[
            "-topic", "robot_description",
            "-name", "axon"
        ]
    )

    # gazebo bridge
    gz_ros2_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock",
            "/imu@sensor_msgs/msg/Imu[gz.msgs.IMU",
            "/scan@sensor_msgs/msg/LaserScan[gz.msgs.LaserScan"
        ],
        remappings=[
            ('/imu', '/imu/out')
        ]
    )

    return LaunchDescription([
        model_arg,
        world_name_arg,
        gazebo_resource_path,
        robot_state_publisher_node,
        gazebo,
        gz_spawner,
        gz_ros2_bridge
    ])