#!/bin/zsh
# Launch script for 2D-LiDAR-Mapping-with-SLAM stack on the Jetson.
# Set FastDDS unicast config so all launched nodes (ekf, slam_toolbox, etc.)
# proactively contact the host machine as a unicast peer.
# The Jetson's ~/fastdds.xml no longer has avoid_builtin_multicast, so it is
# safe to set for both C++ and Python nodes on FastDDS 2.6.x.
export FASTRTPS_DEFAULT_PROFILES_FILE=~/fastdds.xml
# Stop vendor auto-start service and any leftover ROS launch/nodes — but NOT
# the ros2 daemon (matching "ros2-daemon" is excluded so we keep the daemon alive).
sudo systemctl stop start_app_node.service 2>/dev/null || true
sleep 1
pkill -f "ros2 launch" 2>/dev/null || true
pkill -f "slam_toolbox\|ekf_node\|sllidar\|robot_state_publisher\|joint_state_publisher\|odom_publisher\|scan_to_scan\|ros_robot_controller\|servo_controller\|apply_calib\|imu_filter" 2>/dev/null || true
sleep 3

source /opt/ros/humble/local_setup.zsh
source /home/ubuntu/ros2_ws/install/local_setup.zsh
source /home/ubuntu/third_party_ros2/third_party_ws/install/local_setup.zsh
source /home/ubuntu/my_projects/mapping_ws/install/local_setup.zsh

# Restart the daemon so it picks up the FASTRTPS config and re-discovers peers
ros2 daemon stop 2>/dev/null || true
sleep 1
ros2 daemon start
export ROS_DOMAIN_ID=0
export LIDAR_TYPE=A1
export MACHINE_TYPE=JetRover_Mecanum
export HOST=/
export MASTER=/
export need_compile=False
export VERSION='|V1.1.1|'
export DEPTH_CAMERA_TYPE=Dabai
export ASR_LANGUAGE=English
export MIC_TYPE=WonderEchoPro
export LD_PRELOAD=/home/ubuntu/third_party_ros2/Open3D/build/lib/Release/libOpen3D.so
exec ros2 launch two_d_lidar_mapping_with_slam mapping.launch.py
