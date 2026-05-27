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
```zsh
sudo apt install ros-humble-slam-toolbox
sudo apt install ros-humble-nav2-map-server    # map_saver_cli
sudo apt install ros-humble-tf2-tools          # tf2_echo, view_frames
sudo apt install ros-humble-robot-state-publisher
sudo apt install ros-humble-joint-state-publisher
```

**On the host (Humble):**
```zsh
sudo apt install ros-humble-rviz2
sudo apt install ros-humble-tf2-tools          # optional, for host-side TF debugging
```

HiWonder packages (pre-installed on the Jetson):
- `controller` — motor drivers, hardware abstraction layer
- `peripherals` — LiDAR driver and teleop launch files

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
- [slam_toolbox](https://github.com/SteveMacenski/slam_toolbox)
- [HiWonder JetRover Docs](https://docs.hiwonder.com/projects/JetRover/en/jetson-orin-nano/)
- [HiWonder Course for Mapping & Navigation](https://docs.hiwonder.com/projects/JetRover/en/jetson-orin-nano/docs/4.Mapping_Navigation_Course.html#mapping)
- [HiWonder JetRover GitHub](https://github.com/Hiwonder/JetRover)
- [Goals & Roadmap](docs/ROADMAP.md)
