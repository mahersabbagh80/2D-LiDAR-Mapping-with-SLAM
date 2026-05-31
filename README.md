# 2D-LiDAR-Mapping-with-SLAM

Teleoperated 2-D occupancy-grid mapping with a JetRover, ROS 2, LiDAR, and SLAM — visualised live in RViz2.

---

## Project Overview

Drive the HiWonder JetRover around a room while `slam_toolbox` builds a 2-D occupancy-grid map from LiDAR scans and wheel odometry. Save the map to disk when done.

This is the first project in a progressive JetRover series. The goal is to get a working SLAM pipeline on real hardware — nothing more.

---

## Problem Statement

The robot enters an unknown room with no prior map. Using its LiDAR and wheel odometry, it must build a 2-D map in real time that can be saved and reused for future navigation.

---

## Hardware

| Component | Details |
|-----------|---------|
| Robot | HiWonder JetRover (Orin Nano version), Mecanum chassis |
| Compute | NVIDIA Jetson Orin Nano |
| LiDAR | SLAMTEC RPLidar A1 |

---

## Software

| Machine | Ubuntu | ROS 2 |
|---------|--------|-------|
| Jetson Orin Nano | 22.04 LTS | Humble |
| Host (dev machine) | 22.04 LTS | Humble |

The robot stack runs entirely on the Jetson. The host connects over DDS for visualization (RViz2) and teleop only.

---

## Architecture

![Data Pipeline](docs/images/pipeline.png)

See [`docs/architecture.md`](docs/architecture.md) for a full description of the pipeline.

---

## Directory Structure

```
2D-LiDAR-Mapping-with-SLAM/
├── config/
│   ├── slam_toolbox_params.yaml
│   └── rviz/
│       └── mapping.rviz
├── two_d_lidar_mapping_with_slam/
│   └── __init__.py
├── launch/
│   └── mapping.launch.py        # robot description + controller + EKF + LiDAR + SLAM
├── maps/                        # saved maps (gitignored)
└── docs/
    ├── ROADMAP.md               # goals, success criteria, and milestones
    ├── architecture.md
    └── images/
```

---

## Dependencies

**Sibling ROS 2 package — must be in the same workspace:**

| Package | Repo | Role |
|---------|------|------|
| `jetrover_description` | [jetrover_description](https://github.com/mahersabbagh80/jetrover_description) | URDF + meshes for the JetRover; provides `robot_state_publisher` with the robot model |

**On the Jetson (Humble):**

All dependencies are declared in `package.xml` and installed automatically by `rosdep` (see Getting Started). The packages installed are: `slam_toolbox`, `robot_localization`, `laser_filters`, `sllidar_ros2`, `robot_state_publisher`, `joint_state_publisher`. HiWonder packages (`ros_robot_controller`, `controller`) are pre-installed on the Jetson and not managed by rosdep.

**On the host (Humble):**
```zsh
sudo apt install ros-humble-rviz2              # for viewing the map
sudo apt install ros-humble-tf2-tools          # optional, for host-side TF debugging
```

---

## Development Workflow

All development runs on the Jetson. The recommended approach is to edit code locally and push to git, then pull and build on the robot.

**Access the robot:**
```zsh
ssh ubuntu@192.168.2.138   # password: ubuntu
```

For visualization, run RViz2 natively on your host machine — it connects to the Jetson over DDS.

> Note: `192.168.2.138` is a DHCP-assigned IP — update this if it changes.

---

## Getting Started

```zsh
# SSH into the Jetson
ssh ubuntu@192.168.2.138

# Stop the auto-start service
sudo systemctl stop start_app_node.service

# Clone both packages into the workspace (if not already present)
cd ~/jetrover_ws/src
git clone https://github.com/mahersabbagh80/2D-LiDAR-Mapping-with-SLAM.git
git clone https://github.com/mahersabbagh80/jetrover_description.git

# Install dependencies
cd ~/jetrover_ws
rosdep update
rosdep install --from-paths src --ignore-src -r -y

# Build
colcon build --symlink-install
source install/setup.zsh

# Launch a mapping session
ros2 launch two_d_lidar_mapping_with_slam mapping.launch.py

# In a second terminal — drive the robot
source /opt/ros/humble/setup.zsh && ros2 launch peripherals teleop_key_control.launch.py

# When done, save the map
ros2 run nav2_map_server map_saver_cli -f "room_map" --ros-args -p map_subscribe_transient_local:=true
```

---

## Limitations

- 2-D only — no vertical structure
- Wheel odometry drifts on smooth floors
- Requires manual teleoperation (no autonomous exploration)

---

## References

- [ROS 2 Humble Docs](https://docs.ros.org/en/humble/)
- [HiWonder JetRover Docs](https://docs.hiwonder.com/projects/JetRover/en/jetson-orin-nano/)
- [HiWonder Course for Mapping & Navigation](https://docs.hiwonder.com/projects/JetRover/en/jetson-orin-nano/docs/4.Mapping_Navigation_Course.html#mapping)
- [Goals & Roadmap](docs/ROADMAP.md)

**Packages used in this project:**

| Package | Type | Role | Docs / Source |
|---------|------|------|---------------|
| `slam_toolbox` | Community | SLAM — builds the map, publishes `map → odom` TF | [GitHub](https://github.com/SteveMacenski/slam_toolbox) |
| `robot_localization` (ekf_filter_node) | Community | Fuses wheel odometry, publishes `odom → base_footprint` TF | [GitHub](https://github.com/cra-ros-pkg/robot_localization) · [Docs](https://docs.ros.org/en/humble/p/robot_localization/) |
| `laser_filters` (scan_to_scan_filter_chain) | Official ROS | Filters raw LiDAR scan before SLAM | [GitHub](https://github.com/ros-perception/laser_filters) · [Docs](https://docs.ros.org/en/humble/p/laser_filters/) |
| `sllidar_ros2` (sllidar_node) | Hardware vendor (SLAMTEC, open source) | RPLidar A1 hardware driver | [GitHub](https://github.com/Slamtec/sllidar_ros2) |
| `robot_state_publisher` | Official ROS | Publishes static TF from URDF (`base_link → lidar_frame`) | [GitHub](https://github.com/ros/robot_state_publisher) · [Docs](https://docs.ros.org/en/humble/p/robot_state_publisher/) |
| `joint_state_publisher` | Official ROS | Publishes default joint states so robot_state_publisher can complete the TF tree | [GitHub](https://github.com/ros/joint_state_publisher) · [Docs](https://docs.ros.org/en/humble/p/joint_state_publisher/) |
