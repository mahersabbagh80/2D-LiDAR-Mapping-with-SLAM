import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():

    # --------------------------------------------------------------------------
    # Paths
    # --------------------------------------------------------------------------

    # This file lives at <repo>/launch/mapping.launch.py.
    # The repo is NOT a ROS package, so we resolve the config path relative to
    # this file's location rather than via get_package_share_directory.
    launch_dir = os.path.dirname(os.path.realpath(__file__))
    slam_params = os.path.join(launch_dir, '..', 'config', 'slam_toolbox_params.yaml')

    # Locate the JetRover bringup launch file through the ROS package index.
    bringup_dir = get_package_share_directory('bringup')
    bringup_launch = os.path.join(bringup_dir, 'launch', 'bringup.launch.py')

    # --------------------------------------------------------------------------
    # 1. JetRover bringup
    #    Brings up hardware drivers: motor controller, LiDAR, IMU, TF tree.
    # --------------------------------------------------------------------------
    bringup = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(bringup_launch)
    )

    # --------------------------------------------------------------------------
    # 2. SLAM Toolbox (async mode)
    #    Subscribes to /scan and /tf, publishes the occupancy grid map and the
    #    map -> odom transform.  Async mode means scan processing does not block
    #    the main loop — safe for embedded hardware like the Jetson.
    # --------------------------------------------------------------------------
    slam = Node(
        package='slam_toolbox',
        executable='async_slam_toolbox_node',
        name='slam_toolbox',
        output='screen',
        parameters=[slam_params],
    )

    # --------------------------------------------------------------------------
    # 3. RViz2
    #    Launched without a config file for now — you will configure the display
    #    panels manually during Milestone 5 and save the config afterward.
    # --------------------------------------------------------------------------
    rviz = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
    )

    return LaunchDescription([
        bringup,
        slam,
        rviz,
    ])
