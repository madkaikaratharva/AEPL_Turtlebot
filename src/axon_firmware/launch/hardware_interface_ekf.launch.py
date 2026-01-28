import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.substitutions import Command

from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue

def generate_launch_description():

    # axon description pkg directory
    axon_description_pkg = get_package_share_directory("axon_description")

    # robot description
    robot_description = ParameterValue(
        Command([
            "xacro ",
            os.path.join(axon_description_pkg, "urdf", "axon_hardware.urdf.xacro"),
            " is_sim:=false"
        ]),
        value_type=str
    )

    # robot state publisher
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[
            {"robot_description": robot_description,
             "use_sim_time": False}
        ]
    )

    # controller manager node
    controller_manager_node  = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[
            os.path.join(get_package_share_directory("axon_controller"), "config", "axon_controllers_ekf.yaml"),
            {"robot_description": robot_description,
             "use_sim_time": False}
        ]
    )

    return LaunchDescription([
        robot_state_publisher_node,
        controller_manager_node
    ])