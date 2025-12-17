import os
from pathlib import Path
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.substitutions import Command, LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():

    # Env variables
    ros_distro = os.environ["ROS_DISTRO"]
    is_ignition = "True" if ros_distro == "humble" else "False"

    # Argument for providing path to URDF file. Contains default path to aeplbot.urdf.xacro
    aeplbot_description = get_package_share_directory("aeplbot_description")

    model_arg = DeclareLaunchArgument(
        name="model", 
        default_value=os.path.join(
            aeplbot_description, "urdf", "aeplbot.urdf.xacro"
        ),
        description="/Absolute/Path/To/urdf/file"
    )

    # Set Gazebo Resource Path. Include the path to aeplbot_description package
    gazebo_resource_path = SetEnvironmentVariable(
        name="GZ_SIM_RESOURCE_PATH",
        value=[
            str(Path(aeplbot_description).parent.resolve())
            ]
        )
    
    # Read the URDF provided by 'xacro' command line instruction. Robot state publisher node needs URDF
    robot_description = ParameterValue(Command([
            "xacro ",
            LaunchConfiguration("model"),
            " is_ignition:=",
            is_ignition
        ]),
        value_type=str
    )

    # Robot State Publisher Node
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description": robot_description,
                     "use_sim_time": True}]
    )

    # Launch Gazebo using gazebo's launch file
    gazebo = IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(
                    get_package_share_directory("ros_gz_sim"), "launch"), "/gz_sim.launch.py"]),
                launch_arguments=[
                    ("gz_args", [" -v 4", " -r", " empty.sdf"])
                ]
            )

    # Robot Spawner Node to spawn aeplbot in Gazebo
    gz_spawner_node = Node(
        package="ros_gz_sim",
        executable="create",
        output="screen",
        arguments=["-topic", "robot_description", "-name", "aeplbot"],
    )

    # Gazebo-ROS2 Bridge Node to bridge /clock and /imu topics between ROS2 and Gazebo
    gz_ros2_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock",
            "/imu@sensor_msgs/msg/Imu[gz.msgs.IMU"
        ],
        remappings=[
            ('/imu', '/imu/out'),
        ]
    )

    return LaunchDescription([
        model_arg,
        gazebo_resource_path,
        robot_state_publisher_node,
        gazebo,
        gz_spawner_node,
        gz_ros2_bridge
    ])