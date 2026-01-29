import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration

def generate_launch_description():

    # launch arguments
    use_ekf_arg = DeclareLaunchArgument(
        name="use_ekf",
        default_value="True"
    )
    use_ekf = LaunchConfiguration("use_ekf")

    # Axon Hardware Launch
    axon_launch = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("axon_bringup"),
            "launch",
            "axon.launch.py"
        ),
        launch_arguments={
            "use_sim_time": "False",
            "use_ekf": use_ekf
        }.items()
    )

    # Launch SLAM
    slam_launch = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("axon_mapping"),
            "launch",
            "slam.launch.py"
        ),
        launch_arguments={
            "use_sim_time": "False"
        }.items()
    )

    return LaunchDescription([
        use_ekf_arg,
        axon_launch,
        slam_launch
    ])