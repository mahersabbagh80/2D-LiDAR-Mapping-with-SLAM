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

| Software | Version |
|----------|---------|
| Ubuntu | 22.04 LTS (on Jetson) |
| ROS 2 | Humble |
| slam_toolbox | Humble |
| robot_localization | Humble |
| RViz2 | Humble |

> The Orin Nano version ships with ROS 2 Humble natively — no Docker required.

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
│   ├── __init__.py
│   └── odom_relay.py            # /odom_raw → /odom + TF relay node
├── launch/
│   └── mapping.launch.py        # controller + EKF + LiDAR + SLAM
├── maps/                        # saved maps (gitignored)
└── docs/
    ├── ROADMAP.md               # goals, success criteria, and milestones
    ├── architecture.md
    └── images/
```

---

## Dependencies

```bash
# Community packages (declared in package.xml)
sudo apt install ros-humble-slam-toolbox
sudo apt install ros-humble-robot-localization

# Tools used manually (not launched by the package)
sudo apt install ros-humble-nav2-map-server   # map_saver_cli
sudo apt install ros-humble-rviz2
sudo apt install ros-humble-tf2-tools         # tf2_echo, view_frames
```

HiWonder packages (pre-installed on the Jetson):
- `controller` — motor drivers, hardware abstraction layer
- `peripherals` — LiDAR driver and teleop launch files

---

## Development Workflow

All development runs on the Jetson. The recommended approach is to edit code locally and push to git, then pull and build on the robot.

**Access the robot:**
```bash
ssh ubuntu@192.168.2.138   # password: ubuntu
```

For visualization (RViz2), connect to the Jetson desktop via **NoMachine** at `192.168.2.138`.

> Note: `192.168.2.138` is a DHCP-assigned IP — update this if it changes.

---

## Getting Started

```bash
# SSH into the Jetson
ssh ubuntu@192.168.2.138

# Stop the auto-start service
sudo systemctl stop start_app_node.service

# Clone the repo (if not already present)
cd ~/mapping_ws/src
git clone https://github.com/mahersabbagh80/2D-LiDAR-Mapping-with-SLAM.git

# Install dependencies
cd ~/mapping_ws
rosdep update
rosdep install --from-paths src --ignore-src -r -y

# Build
colcon build --symlink-install
source install/setup.bash

# Launch a mapping session
ros2 launch two_d_lidar_mapping_with_slam mapping.launch.py

# In a second terminal — drive the robot
source /opt/ros/humble/setup.bash && ros2 launch peripherals teleop_key_control.launch.py

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
- [slam_toolbox](https://github.com/SteveMacenski/slam_toolbox)
- [HiWonder JetRover Docs](https://docs.hiwonder.com/projects/JetRover/en/jetson-orin-nano/)
- [Goals & Roadmap](docs/ROADMAP.md)
