from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, OpaqueFunction
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    
    # Declare Launch Arguments
    use_sim_time_arg = DeclareLaunchArgument(
        "use_sim_time",
        default_value="True",
    )
    use_sim_time = LaunchConfiguration("use_sim_time")

    # Controller manager nodes
    # Joint State Broadcaster
    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "joint_state_broadcaster",
            "--controller-manager",
            "/controller_manager"
        ]
    )

    # Differential Drive Controller
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