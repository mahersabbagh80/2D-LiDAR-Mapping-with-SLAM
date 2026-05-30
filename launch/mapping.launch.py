import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():

    # ── Package paths ──────────────────────────────────────────────────────────
    # get_package_share_directory returns the installed path of a ROS package —
    # where its config/, launch/, urdf/ folders live after `colcon build`.
    pkg = get_package_share_directory('two_d_lidar_mapping_with_slam')
    controller_pkg = get_package_share_directory('controller')

    urdf = os.path.join(
        get_package_share_directory('jetrover_description'), 'urdf', 'jetrover.xacro'
    )

    # ── Launch arguments ───────────────────────────────────────────────────────
    # Arguments are inputs you can pass on the command line when launching.
    # Example: ros2 launch ... mapping.launch.py machine_type:=JetRover_Tank
    declare_machine_type = DeclareLaunchArgument(
        'machine_type',
        default_value='JetRover_Mecanum',
        description='Chassis variant: JetRover_Mecanum | JetRover_Tank | JetRover_Acker',
    )
    declare_lidar_type = DeclareLaunchArgument(
        'lidar_type',
        default_value='A1',
        description='LiDAR model: A1 | A2 | S2L | LD14P | G4',
    )

    # xacro is a preprocessor for URDF files — it expands macros and substitutes
    # variables. MACHINE_TYPE and LIDAR_TYPE select the correct robot geometry.
    robot_description = ParameterValue(
        Command([
            'env MACHINE_TYPE=', LaunchConfiguration('machine_type'),
            ' LIDAR_TYPE=', LaunchConfiguration('lidar_type'),
            ' xacro ', urdf,
        ]),
        value_type=str,
    )

    # ── 1. Robot description ───────────────────────────────────────────────────
    # robot_state_publisher reads the URDF and broadcasts static TF transforms
    # between every link (e.g. base_link → lidar_frame). It also publishes the
    # full URDF on /robot_description so RViz can render the 3D model.
    # joint_state_publisher fills in wheel joint angles with zeros so that
    # robot_state_publisher does not warn about missing joint states.
    robot_state_pub = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{'robot_description': robot_description}],
    )

    joint_state_pub = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        parameters=[{'robot_description': robot_description}],
    )

    # ── 2. Hardware drivers (Layer 2 — vendor black box) ───────────────────────
    # ros_robot_controller speaks the proprietary serial protocol to the motor
    # board. It handles motor commands and reads raw sensor data over USB serial.
    ros_robot_controller = Node(
        package='ros_robot_controller',
        executable='ros_robot_controller',
        output='screen',
        parameters=[{'imu_frame': 'imu_link'}],
    )

    # odom_publisher reads wheel encoder counts from ros_robot_controller and
    # converts them to odometry. Publishes /odom_raw.
    # calibrate_params.yaml contains the wheel dimensions for this specific robot.
    odom_publisher = Node(
        package='controller',
        executable='odom_publisher',
        name='odom_publisher',
        output='screen',
        parameters=[
            os.path.join(controller_pkg, 'config', 'calibrate_params.yaml'),
            {
                'base_frame_id': 'base_footprint',
                'odom_frame_id': 'odom',
                'pub_odom_topic': True,
            },
        ],
    )

    # ── 3. EKF (Extended Kalman Filter) ───────────────────────────────────────
    # Fuses /odom_raw into a smooth pose estimate.
    # publish_tf: true in ekf.yaml is what broadcasts the odom→base_footprint
    # transform — the missing link that was breaking the TF chain.
    ekf = Node(
        package='robot_localization',
        executable='ekf_node',
        name='ekf_filter_node',
        output='screen',
        parameters=[os.path.join(pkg, 'config', 'ekf.yaml')],
    )

    # ── 4. LiDAR driver ───────────────────────────────────────────────────────
    # Drives the RPLidar A1 over serial and publishes raw scans.
    # Remapped to /scan_raw so the filter chain can sit between it and SLAM.
    sllidar = Node(
        package='sllidar_ros2',
        executable='sllidar_node',
        name='sllidar_node',
        output='screen',
        parameters=[{
            'channel_type': 'serial',
            'serial_port': '/dev/lidar',
            'serial_baudrate': 115200,
            'frame_id': 'lidar_frame',
            'inverted': False,
            'angle_compensate': True,
            'scan_mode': 'Sensitivity',
        }],
        remappings=[('scan', 'scan_raw')],  # publish to /scan_raw, not /scan
    )

    # ── 5. Laser filter ────────────────────────────────────────────────────────
    # Reads /scan_raw, applies angular and range filters from lidar_filters.yaml,
    # and publishes the clean result to /scan — the topic SLAM subscribes to.
    laser_filter = Node(
        package='laser_filters',
        executable='scan_to_scan_filter_chain',
        name='scan_to_scan_filter_chain',
        output='screen',
        parameters=[os.path.join(pkg, 'config', 'lidar_filters.yaml')],
        remappings=[
            ('scan', 'scan_raw'),       # read from /scan_raw
            ('scan_filtered', 'scan'),  # publish clean scan to /scan
        ],
    )

    # ── 6. SLAM Toolbox ────────────────────────────────────────────────────────
    # Subscribes to /scan and the TF tree. Builds the occupancy grid map and
    # publishes it on /map. Also broadcasts the map→odom TF transform.
    slam = Node(
        package='slam_toolbox',
        executable='async_slam_toolbox_node',
        name='slam_toolbox',
        output='screen',
        parameters=[os.path.join(pkg, 'config', 'slam_toolbox_params.yaml')],
    )

    return LaunchDescription([
        declare_machine_type,
        declare_lidar_type,
        robot_state_pub,
        joint_state_pub,
        ros_robot_controller,
        odom_publisher,
        ekf,
        sllidar,
        laser_filter,
        slam,
    ])
