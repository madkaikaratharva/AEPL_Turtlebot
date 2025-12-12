import os
from pathlib import Path
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, SetEnvironmentVariable, IncludeLaunchDescription
from launch.substitutions import Command, LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.actions import Node


def generate_launch_description():

    # Env variables
    ros_distro = os.environ["ROS_DISTRO"]
    is_ignition = "True" if ros_distro == "humble" else "False"

    # Argument for providing path to URDF file. Will contain /path/to/urdf
    model_arg = DeclareLaunchArgument(
        name="model",
        default_value=os.path.join(
            get_package_share_directory("wafflebot_description"), "urdf", "wafflebot.urdf.xacro"
        ),
        description="Path to your URDF"
    )

    # Read the URDF provided by 'xacro' command line instruction. Robot state publisher node needs URDF 
    robot_description = ParameterValue(
        Command(["xacro ", LaunchConfiguration("model"), " is_ignition:=", is_ignition]),
        value_type=str
    )

    # Robot State Publisher Node
    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description" : robot_description}]
    )

    # Set Gazebo Resource Path. Need to set a path to robot's description (URDF)
    gazebo_resource_path = SetEnvironmentVariable(
        name="GZ_SIM_RESOURCE_PATH",
        value=[
            str(Path(get_package_share_directory("wafflebot_description")).parent.resolve())
            ]
    )

    # Launch Gazebo using gazebo's launch file
    gazebo = IncludeLaunchDescription(PythonLaunchDescriptionSource([
        os.path.join(
            get_package_share_directory("ros_gz_sim"), "launch"), "/gz_sim.launch.py"]),
        launch_arguments=[
            ("gz_args", [" -v 4", " -r", " empty.sdf"])
        ]
    )

    # Spawn the robot in the world
    gz_spawner_node = Node(
        package="ros_gz_sim",
        executable="create",
        output="screen",
        arguments=["-topic", "robot_description", "-name", "wafflebot"]
    )

    gz_ros2_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "/imu@sensor_msgs/msg/Imu[gz.msgs.IMU"
        ],
        remappings=[
            ("imu", "/imu/out")
        ]
    )


    return LaunchDescription([
        model_arg,
        robot_state_publisher,
        gazebo_resource_path,
        gazebo,
        gz_spawner_node,
        gz_ros2_bridge
    ])