#!/bin/zsh
# Launch script for Room-Mapping-Explorer mapping stack on the Jetson.
# Do NOT set FASTRTPS_DEFAULT_PROFILES_FILE here — it silently breaks Python
# rclpy publisher data delivery on FastDDS 2.6.x (avoid_builtin_multicast bug).
# Cross-machine DDS discovery (WSL ↔ Jetson) is handled by ~/fastdds.xml which
# is loaded by the ros2 daemon started in ~/.zshrc, not by the launch nodes.
source /opt/ros/humble/local_setup.zsh
source /home/ubuntu/ros2_ws/install/local_setup.zsh
source /home/ubuntu/third_party_ros2/third_party_ws/install/local_setup.zsh
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
exec ros2 launch /home/ubuntu/my_projects/Room-Mapping-Explorer/launch/mapping.launch.py
