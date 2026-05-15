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

    launch_dir = os.path.dirname(os.path.realpath(__file__))
    repo_config = os.path.join(launch_dir, '..', 'config')

    controller_dir = get_package_share_directory('controller')
    peripherals_dir = get_package_share_directory('peripherals')

    # --------------------------------------------------------------------------
    # 1. JetRover controller (hardware only — EKF disabled)
    #    enable_odom=false suppresses the vendor ekf_filter_node so we can
    #    start our own below with an odom-only config (no IMU input).
    #    The vendor IMU pipeline (/imu) is broken until Milestone 8.
    # --------------------------------------------------------------------------
    controller = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(controller_dir, 'launch', 'controller.launch.py')
        ),
        launch_arguments={'enable_odom': 'false'}.items(),
    )

    # --------------------------------------------------------------------------
    # 2. EKF — wheel odometry only (C++ node)
    #    Subscribes to /odom_raw, publishes /odom and the odom→base_footprint TF.
    #    Uses the C++ robot_localization ekf_node because Python rclpy nodes
    #    fail to deliver DDS data under avoid_builtin_multicast on FastDDS 2.6.x.
    #    IMU fusion deferred to Milestone 8.
    # --------------------------------------------------------------------------
    ekf = Node(
        package='robot_localization',
        executable='ekf_node',
        name='ekf_filter_node',
        output='screen',
        parameters=[os.path.join(repo_config, 'ekf_odom_only.yaml')],
        remappings=[('odometry/filtered', 'odom')],
    )

    # --------------------------------------------------------------------------
    # 3. LiDAR
    #    Brings up the RPLidar A1 driver and laser_filters, publishing
    #    filtered scans to /scan — the topic slam_toolbox subscribes to.
    # --------------------------------------------------------------------------
    lidar = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(peripherals_dir, 'launch', 'lidar.launch.py')
        )
    )

    # --------------------------------------------------------------------------
    # 4. SLAM Toolbox (async mode)
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
        ekf,
        lidar,
        slam,
    ])
