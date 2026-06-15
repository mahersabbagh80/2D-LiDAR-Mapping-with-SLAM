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

The robot stack runs entirely on the Jetson. The host connects over DDS for visualization (RViz2) only.

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
├── launch/
│   └── mapping.launch.py        # controller + arm init + LiDAR + joystick + SLAM
├── maps/                        # saved map outputs (.pgm, .yaml; optional pose graph)
├── scripts/
│   └── run_mapping.sh           # Jetson runbook: DDS env, cleanup, workspace sourcing
├── two_d_lidar_mapping_with_slam/
│   └── __init__.py
└── docs/
    ├── ROADMAP.md               # goals, success criteria, and milestones
    ├── architecture.md
    ├── cross_machine_dds.md     # host-to-Jetson discovery setup
    └── images/
```

---

## Dependencies

**On the Jetson (Humble):**

This package directly launches `slam_toolbox` plus the HiWonder `controller` and `peripherals` packages. `controller` and `peripherals` are pre-installed on the Jetson and are runtime dependencies of the robot image, not packages managed by this repository.

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

# Clone the package into the workspace used by scripts/run_mapping.sh
mkdir -p ~/my_projects/mapping_ws/src
cd ~/my_projects/mapping_ws/src
git clone https://github.com/mahersabbagh80/2D-LiDAR-Mapping-with-SLAM.git

# Install dependencies
cd ~/my_projects/mapping_ws
rosdep update
rosdep install --from-paths src --ignore-src -r -y

# Build
colcon build --symlink-install
source install/setup.zsh

# Launch the full mapping stack with the Jetson runbook
zsh "$(ros2 pkg prefix two_d_lidar_mapping_with_slam)/share/two_d_lidar_mapping_with_slam/scripts/run_mapping.sh"
```

`scripts/run_mapping.sh` stops the HiWonder auto-start service, kills stale ROS nodes from prior sessions, exports the FastDDS unicast profile, sources the vendor and mapping workspaces, restarts the ROS 2 daemon, sets the JetRover environment (`LIDAR_TYPE=A1`, `MACHINE_TYPE=JetRover_Mecanum`, `ROS_DOMAIN_ID=0`), and then runs `mapping.launch.py`.

Drive the robot with the gamepad. On the host, make sure the FastDDS profile from [`docs/cross_machine_dds.md`](docs/cross_machine_dds.md) is active, then open RViz2 with the pre-configured layout:

```zsh
# If this package is built and sourced on the host:
rviz2 -d "$(ros2 pkg prefix two_d_lidar_mapping_with_slam)/share/two_d_lidar_mapping_with_slam/config/rviz/mapping.rviz"

# Or, from a checkout of this repository:
rviz2 -d config/rviz/mapping.rviz
```

RViz uses `map` as its fixed frame, subscribes to `/robot_description` with Transient Local durability so the robot model appears for late-joining RViz sessions, and displays `/map`, `/map_updates`, and `/scan`. It can take up to about 15 seconds after startup for DDS endpoint matching to complete over WiFi.

When done, save the map (run on the Jetson while the stack is still up):

```zsh
# Standard map files (.pgm + .yaml) for nav2
ros2 run nav2_map_server map_saver_cli -f /home/ubuntu/my_projects/mapping_ws/src/2D-LiDAR-Mapping-with-SLAM/maps/apartment

# Pose graph (.data + .posegraph) — lets you resume mapping in a future session
ros2 service call /slam_toolbox/serialize_map slam_toolbox/srv/SerializePoseGraph \
  "{filename: '/home/ubuntu/my_projects/mapping_ws/src/2D-LiDAR-Mapping-with-SLAM/maps/apartment'}"
```

`map_saver_cli -f` expects a basename, not a directory. If you copy or rename committed map assets, keep the `.yaml` `image:` value and the `.pgm` filename in sync or `map_server` will fail to load the map.

---

## Limitations

- 2-D only — no vertical structure
- Wheel odometry drifts on smooth floors, which may result in some inaccurate mappings
- Requires manual teleoperation (no autonomous exploration)

---

## Results

The robot successfully mapped a full apartment in a single session. Rooms, corridors, doorways, and furniture outlines are clearly resolved at 5 cm/cell resolution.

| Robot model with live LiDAR scan | Side view of the apartment |
|---|---|
| ![Robot model and live LiDAR scan in RViz2](docs/images/rviz_map_closeup.png) | ![Side view of the apartment](docs/images/rviz_robot_sideview.png) |

![Completed apartment map](docs/images/rviz_map_complete.png)
*Completed apartment map — full floor layout resolved in a single teleoperated session*

### TF Tree

The live TF tree captured during a mapping session with the command: ′ros2 run tf2_tools view_frames′
Matches the expected `map → odom → base_footprint → base_link → lidar_link` chain described in the architecture.

![TF Tree](docs/images/tf_tree.png)

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
| `controller` | Vendor (HiWonder, **closed source**) | Wheel odometry and EKF, `/odom`, `odom → base_footprint`, `/robot_description`, `robot_state_publisher`, servo joint states, arm init pose | Pre-installed on Jetson |
| `peripherals` | Vendor (HiWonder, **closed source**) | RPLidar A1 launch, scan filtering, and joystick control | Pre-installed on Jetson |
| `ros_robot_controller` | Vendor (HiWonder, **closed source**) | Hardware interface used by the vendor controller/peripherals stack | Pre-installed on Jetson |

This repository declares only `slam_toolbox`, `controller`, and `peripherals` in `package.xml`. Lower-level LiDAR, filter, and hardware-interface nodes are brought in by the vendor launch files rather than launched directly here.
