# 🗺️ Room-Mapping Explorer

> Autonomous 2-D occupancy-grid mapping with a JetRover, ROS, LiDAR, and SLAM —
> visualised live in RViz.

---

## Table of Contents

- [🗺️ Room-Mapping Explorer](#️-room-mapping-explorer)
  - [Table of Contents](#table-of-contents)
  - [Project Overview](#project-overview)
  - [Problem Statement](#problem-statement)
  - [Goals \& Success Criteria](#goals--success-criteria)
  - [System Architecture](#system-architecture)
  - [Hardware Requirements](#hardware-requirements)
  - [Software Requirements](#software-requirements)
  - [ROS Packages \& Dependencies](#ros-packages--dependencies)
  - [Key Concepts Covered](#key-concepts-covered)
  - [Project Milestones](#project-milestones)
    - [TODOs](#todos)
    - [Milestone 1 — Environment Setup](#milestone-1--environment-setup)
    - [Milestone 2 — TF Tree \& URDF](#milestone-2--tf-tree--urdf)
    - [Milestone 3 — Teleoperation](#milestone-3--teleoperation)
    - [Milestone 4 — SLAM Integration (gmapping)](#milestone-4--slam-integration-gmapping)
    - [Milestone 5 — Parameter Tuning \& Evaluation](#milestone-5--parameter-tuning--evaluation)
    - [Milestone 6 — Stretch: Cartographer](#milestone-6--stretch-cartographer)
    - [Milestone 7 — Stretch: IMU Fusion](#milestone-7--stretch-imu-fusion)
  - [Directory Structure](#directory-structure)
  - [Getting Started](#getting-started)
  - [Known Limitations \& Future Extensions](#known-limitations--future-extensions)
  - [References](#references)

---

## Project Overview

Room-Mapping Explorer is a ROS-based autonomous mapping project built on the
**HiWonder JetRover** platform (NVIDIA Jetson + LiDAR + Mecanum chassis).
The robot is teleoperated or driven autonomously around an indoor environment
while a SLAM algorithm builds a 2-D occupancy-grid map in real time.
The resulting map is saved to disk and can be re-used for subsequent
autonomous navigation tasks.

This is the **first project** in a progressive JetRover robotics series and
is deliberately scoped to solidify core ROS fundamentals before tackling
more complex subsystems.

---

## Problem Statement

When a mobile robot enters an unknown indoor environment it has no prior
knowledge of the space — walls, doorways, furniture, or free floor area.
Without a map, higher-level behaviours such as autonomous navigation,
delivery, or search-and-rescue are impossible.

**The challenge:** Given a mobile platform equipped with a 2-D LiDAR and
wheel odometry, construct an accurate, real-time occupancy-grid map of an
indoor room or apartment using SLAM, and visualise the map as it is built.

Constraints:
- The environment is unknown and unstructured (typical home/office).
- The robot must handle wheel-slip and sensor noise gracefully.
- The pipeline must run on the onboard Jetson (edge compute, no cloud).
- The map must be exportable in a format compatible with the ROS
  `map_server` / `nav2_map_server` for future navigation projects.

---

## Goals & Success Criteria

| #   | Goal                 | Success Criterion                                                       |
| --- | -------------------- | ----------------------------------------------------------------------- |
| 1   | Real-time SLAM       | Occupancy grid updates at ≥ 1 Hz in RViz with no noticeable lag         |
| 2   | Accurate map         | Walls and obstacles visible as solid cells; free space clearly distinct |
| 3   | Full room coverage   | ≥ 90 % of the drivable floor area mapped in a single session            |
| 4   | Map persistence      | Saved `.pgm` / `.yaml` pair loadable by `map_server`                    |
| 5   | TF tree correct      | `odom → base_link → laser` transform chain resolves without errors      |
| 6   | IMU fusion (stretch) | Fusing IMU data reduces odometry drift on hard-floor surfaces           |

---

## System Architecture

![System Architecture Pipeline](docs/images/pipeline.svg)

> ✦ Stretch goal — IMU fusion is optional and not required for the MVP.

---

## Hardware Requirements

| Component      | Details                                                        |
| -------------- | -------------------------------------------------------------- |
| Robot platform | HiWonder JetRover (Mecanum or Ackermann chassis)               |
| Compute        | NVIDIA Jetson Nano / Orin Nano (onboard)                       |
| LiDAR          | JetRover built-in 2-D LiDAR (e.g. YDLiDAR or RPLIDAR)          |
| IMU            | Onboard IMU (for odometry fusion — stretch goal)               |
| Depth camera   | Intel RealSense D435 (available on platform; not used in MVP)  |
| Host PC        | Ubuntu 20.04 / 22.04 machine for RViz visualisation (optional) |
| Network        | Wi-Fi or Ethernet for ROS master / SSH access                  |

---

## Software Requirements

| Software | Version        | Notes                                  |
| -------- | -------------- | -------------------------------------- |
| Ubuntu   | 20.04 LTS      | On Jetson                              |
| ROS      | Noetic (ROS 1) | JetRover ships with ROS Noetic support |
| Python   | 3.8+           | ROS Noetic default                     |
| C++      | 14 / 17        | For any custom nodes                   |
| RViz     | Noetic         | Visualisation                          |
| Git      | 2.x            | Version control                        |

- [ ] TODO: Verify JetRover ROS distro/support details for this row.
- [ ] TODO: Verify the Type of Lidar used in my Version of the JetRover.

> **Note:** If migrating to ROS 2, replace gmapping with `slam_toolbox` and
> `nav2_map_server`; the concepts remain identical.

---

## ROS Packages & Dependencies

```bash
# Core SLAM — choose one
sudo apt install ros-noetic-gmapping          # particle-filter SLAM (simpler)
sudo apt install ros-noetic-cartographer-ros  # graph-based SLAM (more accurate)

# Map saving & serving
sudo apt install ros-noetic-map-server

# Teleoperation
sudo apt install ros-noetic-teleop-twist-keyboard

# TF utilities
sudo apt install ros-noetic-tf2-tools ros-noetic-tf2-ros

# IMU filter (stretch goal)
sudo apt install ros-noetic-imu-filter-madgwick

# Visualisation
sudo apt install ros-noetic-rviz
```

Vendor / platform packages (provided by HiWonder):
- `jetrover_bringup` — launches motor drivers, LiDAR driver, URDF
- `jetrover_description` — URDF / XACRO robot model
- `jetrover_teleop` — joystick / keyboard bindings

---

## Key Concepts Covered

- **ROS Topics & Nodes** — publisher/subscriber pattern for sensor data
- **TF2 Transform Tree** — `map → odom → base_link → laser` chain
- **Occupancy Grid** (`nav_msgs/OccupancyGrid`) — probabilistic map representation
- **SLAM** — Simultaneous Localisation and Mapping (gmapping or Cartographer)
- **Odometry** (`nav_msgs/Odometry`) — dead-reckoning from wheel encoders
- **Launch Files** — composing multi-node systems declaratively
- **RViz Configuration** — custom `.rviz` files for repeatable visualisation
- **map_saver** — persisting maps to `.pgm` / `.yaml` for re-use

---

## Project Milestones

### TODOs
- [ ] TODO: Verify and improve the milestones for what makes the most sense.

### Milestone 1 — Environment Setup
- [ ] Flash Jetson with Ubuntu 20.04 + ROS Noetic (if not already done)
- [ ] Clone this repository onto the Jetson
- [ ] Verify JetRover bringup launches without errors
- [ ] Confirm `/scan` topic publishes LiDAR data
- [ ] Confirm `/odom` topic publishes odometry data

### Milestone 2 — TF Tree & URDF
- [ ] Load `jetrover_description` and verify URDF with `check_urdf`
- [ ] Visualise TF tree using `rosrun tf2_tools view_frames.py`
- [ ] Confirm `base_link → laser` static transform is correct

### Milestone 3 — Teleoperation
- [ ] Drive robot with `teleop_twist_keyboard`
- [ ] Confirm `/cmd_vel` commands move the physical robot
- [ ] Tune linear/angular speed limits in config

### Milestone 4 — SLAM Integration (gmapping)
- [ ] Launch `slam_gmapping` with tuned parameters
- [ ] Open RViz and add `/map` and `/scan` displays
- [ ] Drive robot around room and observe map building in real time
- [ ] Save map with `rosrun map_server map_saver -f ~/maps/room_map`

### Milestone 5 — Parameter Tuning & Evaluation
- [ ] Compare maps from multiple runs; identify drift or artefacts
- [ ] Tune key gmapping params (`particles`, `linearUpdate`, `angularUpdate`)
- [ ] Document best-performing parameter set in `config/gmapping_params.yaml`

### Milestone 6 — Stretch: Cartographer
- [ ] Replace gmapping with `cartographer_ros`
- [ ] Write Cartographer `.lua` configuration file
- [ ] Compare map quality vs gmapping; document findings

### Milestone 7 — Stretch: IMU Fusion
- [ ] Launch `imu_filter_madgwick` to fuse IMU + odometry
- [ ] Verify improved localisation on slippery (hard) floor surfaces

---

## Directory Structure

```
room-mapping-explorer/
├── README.md
├── CMakeLists.txt
├── package.xml
│
├── config/
│   ├── gmapping_params.yaml       # Tuned gmapping parameters
│   ├── cartographer_config.lua    # Cartographer config (stretch)
│   └── rviz/
│       └── mapping.rviz           # Saved RViz layout
│
├── launch/
│   ├── mapping.launch             # Main entry point: bringup + SLAM + RViz
│   ├── gmapping.launch            # SLAM only (gmapping)
│   ├── cartographer.launch        # SLAM only (cartographer, stretch)
│   └── teleop.launch              # Keyboard teleoperation
│
├── maps/
│   └── .gitkeep                   # Saved maps go here (gitignored)
│
├── nodes/
│   └── (custom Python/C++ nodes if needed)
│
├── urdf/
│   └── (symlink or copy of jetrover URDF for reference)
│
└── docs/
    ├── architecture.md
    ├── parameter_tuning.md
    └── images/
        └── (screenshots of maps, RViz, TF tree)
```

---

## Getting Started

```bash
# 1. SSH into the Jetson
ssh jetson@<JETROVER_IP> #TODO

# 2. Clone the repo
cd ~/catkin_ws/src
git clone https://github.com/<your-username>/room-mapping-explorer.git #TODO

# 3. Install dependencies
cd ~/catkin_ws
rosdep install --from-paths src --ignore-src -r -y

# 4. Build
catkin_make
source devel/setup.bash

# 5. Launch mapping session
roslaunch room-mapping-explorer mapping.launch

# 6. In a new terminal — drive the robot
roslaunch room-mapping-explorer teleop.launch

# 7. When done, save the map
rosrun map_server map_saver -f ~/catkin_ws/src/room-mapping-explorer/maps/room_map
```

---

## Known Limitations & Future Extensions

**Current limitations (MVP scope):**
- 2-D mapping only — no vertical structure captured
- Relies on wheel odometry which drifts on smooth floors
- Manual teleoperation required; no autonomous exploration
- Single-session only — no map merging across sessions

**Planned extensions (future projects):**
- **Autonomous Navigation** — use the saved map with `move_base` / `nav2` for
  point-to-point navigation (Project #2 in the series)
- **Frontier Exploration** — autonomous coverage using `explore_lite`
- **3-D Mapping** — integrate the RealSense D435 with `rtabmap_ros`
- **Multi-floor** — elevator / staircase traversal and map stitching

---

## References

- [ROS Noetic Documentation](https://wiki.ros.org/noetic)
- [gmapping Wiki](https://wiki.ros.org/gmapping)
- [Google Cartographer ROS](https://google-cartographer-ros.readthedocs.io/)
- [nav_msgs/OccupancyGrid](https://docs.ros.org/en/noetic/api/nav_msgs/html/msg/OccupancyGrid.html)
- [TF2 ROS Tutorial](https://wiki.ros.org/tf2/Tutorials)
- [HiWonder JetRover Product Page](https://www.hiwonder.com/products/jetrover)
- Thrun, S., Burgard, W., Fox, D. — *Probabilistic Robotics* (MIT Press, 2005)