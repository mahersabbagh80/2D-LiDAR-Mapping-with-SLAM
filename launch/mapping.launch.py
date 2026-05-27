import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():

    # --------------------------------------------------------------------------
    # Paths
    # --------------------------------------------------------------------------

    pkg_share_dir = get_package_share_directory('two_d_lidar_mapping_with_slam')
    repo_config = os.path.join(pkg_share_dir, 'config')

    controller_dir = get_package_share_directory('controller')
    peripherals_dir = get_package_share_directory('peripherals')

    urdf = os.path.join(
        get_package_share_directory('jetrover_description'), 'urdf', 'jetrover.xacro'
    )

    # --------------------------------------------------------------------------
    # Launch arguments
    #   machine_type — selects the chassis variant in the URDF
    #   lidar_type   — selects the LiDAR model in the URDF
    # --------------------------------------------------------------------------
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

    # xacro reads MACHINE_TYPE and LIDAR_TYPE from environment variables,
    # so we prefix the command with `env VAR=value` to pass them in.
    robot_description = ParameterValue(
        Command([
            'env MACHINE_TYPE=', LaunchConfiguration('machine_type'),
            ' LIDAR_TYPE=', LaunchConfiguration('lidar_type'),
            ' xacro ', urdf,
        ]),
        value_type=str,
    )

    # --------------------------------------------------------------------------
    # 1. Robot description
    #    Publishes the URDF on /robot_description (Transient Local — late-joining
    #    subscribers like RViz still receive it) and broadcasts the fixed TF
    #    transforms between every link in the robot (e.g. base_link → lidar_link).
    #    joint_state_publisher fills in the arm joint angles with zeros so that
    #    robot_state_publisher doesn't warn about missing joint states.
    # --------------------------------------------------------------------------
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

    # --------------------------------------------------------------------------
    # 2. JetRover controller
    #    Runs with default enable_odom=true — the vendor EKF fuses wheel
    #    odometry and IMU, publishes /odom and the odom→base_footprint TF.
    # --------------------------------------------------------------------------
    controller = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(controller_dir, 'launch', 'controller.launch.py')
        )
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
        declare_machine_type,
        declare_lidar_type,
        robot_state_pub,
        joint_state_pub,
        controller,
        lidar,
        slam,
    ])
