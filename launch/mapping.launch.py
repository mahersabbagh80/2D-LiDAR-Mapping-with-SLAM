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

    pkg_share_dir = get_package_share_directory('two_d_lidar_mapping_with_slam')
    repo_config = os.path.join(pkg_share_dir, 'config')

    controller_dir = get_package_share_directory('controller')
    peripherals_dir = get_package_share_directory('peripherals')

    # --------------------------------------------------------------------------
    # 1. JetRover controller
    #    Runs with default enable_odom=true — the vendor EKF fuses wheel
    #    odometry and IMU, publishes /odom and the odom→base_footprint TF.
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
    #    map→odom transform.  Async mode is safe for embedded hardware.
    # --------------------------------------------------------------------------
    slam = Node(
        package='slam_toolbox',
        executable='async_slam_toolbox_node',
        name='slam_toolbox',
        output='screen',
        parameters=[os.path.join(repo_config, 'slam_toolbox_params.yaml')],
    )

    return LaunchDescription([
        controller,
        lidar,
        slam,
    ])
