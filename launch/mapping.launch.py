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

    # JetRover hardware packages — resolved through the ROS package index.
    controller_dir = get_package_share_directory('controller')
    peripherals_dir = get_package_share_directory('peripherals')

    # --------------------------------------------------------------------------
    # 1. JetRover controller
    #    Brings up motor drivers, wheel odometry, IMU filter, and EKF.
    #    The EKF publishes the odom -> base_footprint TF transform that
    #    slam_toolbox needs to track robot motion between scans.
    # --------------------------------------------------------------------------
    controller = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(controller_dir, 'launch', 'controller.launch.py')
        )
    )

    # --------------------------------------------------------------------------
    # 2. LiDAR
    #    Brings up the RPLidar A1 driver and laser_filters, publishing
    #    filtered scans to /scan — the topic slam_toolbox subscribes to.
    # --------------------------------------------------------------------------
    lidar = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(peripherals_dir, 'launch', 'lidar.launch.py')
        )
    )

    # --------------------------------------------------------------------------
    # 3. SLAM Toolbox (async mode)
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

    return LaunchDescription([
        controller,
        lidar,
        slam,
    ])
