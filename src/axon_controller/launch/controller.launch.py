from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

from launch_ros.actions import Node

def generate_launch_description():

    # declare launch argument
    use_sim_time_arg = DeclareLaunchArgument(
        name="use_sim_time",
        default_value="False"
    )
    use_sim_time = LaunchConfiguration("use_sim_time")

    # controller manager nodes
    # joint_state_broadcaster
    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "joint_state_broadcaster",
            "--controller-manager",
            "/controller_manager"
        ]
    )

    # axon_controller
    axon_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "axon_controller",
            "--controller-manager",
            "/controller_manager"
        ]
    )

    return LaunchDescription([
        use_sim_time_arg,
        joint_state_broadcaster_spawner,
        axon_controller_spawner
    ])
