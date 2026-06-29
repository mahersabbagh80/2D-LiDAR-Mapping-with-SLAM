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
├── two_d_lidar_mapping_with_slam/
│   └── __init__.py
├── launch/
│   └── mapping.launch.py        # controller + arm init + LiDAR + joystick + SLAM
├── maps/                        # saved maps (.pgm, .yaml, .posegraph)
├── scripts/
│   └── run_mapping.sh           # Jetson launch helper for a clean mapping session
└── docs/
    ├── ROADMAP.md               # goals, success criteria, and milestones
    ├── architecture.md
    ├── cross_machine_dds.md     # host ↔ Jetson ROS 2 discovery troubleshooting
    └── images/
```

---

## Dependencies

**On the Jetson (Humble):**

Launch-time dependencies are declared in `package.xml` and installed automatically by `rosdep` (see Getting Started). The key community package is `slam_toolbox`. HiWonder packages (`controller`, `peripherals`) are pre-installed on the Jetson and not managed by rosdep.

The map-saving command uses Nav2's map server CLI. Install it if it is not already available on the Jetson:

```zsh
sudo apt install ros-humble-nav2-map-server
```

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

# Clone the package into the workspace (if not already present)
cd ~/jetson_ws/src
git clone https://github.com/mahersabbagh80/2D-LiDAR-Mapping-with-SLAM.git

# Install dependencies
cd ~/jetson_ws
rosdep update
rosdep install --from-paths src --ignore-src -r -y

# Build
colcon build --symlink-install
source install/setup.zsh

# Launch the full mapping stack from a clean state
zsh ~/jetson_ws/src/2D-LiDAR-Mapping-with-SLAM/scripts/run_mapping.sh
```

`scripts/run_mapping.sh` stops the vendor auto-start service, kills leftover mapping nodes, sources the Jetson ROS workspaces, restarts the ROS 2 daemon with the FastDDS profile, sets the JetRover environment variables, and then launches `mapping.launch.py`.

Drive the robot with the gamepad. On the host, open RViz2 with the pre-configured layout. Use the source-tree path if running from a checkout:

```zsh
cd /path/to/2D-LiDAR-Mapping-with-SLAM
rviz2 -d config/rviz/mapping.rviz
```

If the package is installed in a colcon workspace on the host, the same config is also installed under the package share directory:

```zsh
rviz2 -d "$(ros2 pkg prefix two_d_lidar_mapping_with_slam)/share/two_d_lidar_mapping_with_slam/config/rviz/mapping.rviz"
```

When done, save the map from the Jetson while the mapping stack is still running. Pass a filename prefix, not just a directory:

```zsh
cd ~/jetson_ws/src/2D-LiDAR-Mapping-with-SLAM

# Standard map files (.pgm + .yaml) for Nav2
ros2 run nav2_map_server map_saver_cli -f maps/apartment

# Pose graph (.data + .posegraph) for resuming or inspecting the SLAM session
ros2 service call /slam_toolbox/serialize_map slam_toolbox/srv/SerializePoseGraph \
  "{filename: '/home/ubuntu/jetson_ws/src/2D-LiDAR-Mapping-with-SLAM/maps/apartment'}"
```

Keep the `.yaml` `image:` field and the `.pgm` filename synchronized when copying or renaming saved maps; Nav2 loads the image path relative to the YAML file.

### Mapping session checklist

Run these checks before driving a long loop:

```zsh
ros2 topic hz /scan
ros2 topic hz /odom
ros2 run tf2_ros tf2_echo map base_footprint
ros2 topic echo /map --once
```

- `/scan` confirms the RPLidar A1 driver and filter chain are publishing.
- `/odom` confirms the vendor controller stack is publishing odometry.
- `tf2_echo map base_footprint` confirms `slam_toolbox` has connected the `map → odom → base_footprint` chain.
- `/map` confirms RViz2 and `map_saver_cli` should be able to consume the live occupancy grid.

### Common pitfalls

- If RViz2 shows `Frame [map] does not exist`, verify `/scan`, `/odom`, and `map → base_footprint` in that order. SLAM cannot publish `map` until scans and odometry are both available through TF.
- If the robot model flickers in RViz2, check for duplicate `/joint_states` or `/robot_description` publishers. `mapping.launch.py` intentionally lets the vendor `controller` stack own robot description and joint state publication.
- If the host cannot see Jetson topics, use the FastDDS unicast setup in [`docs/cross_machine_dds.md`](docs/cross_machine_dds.md). Consumer WiFi routers often block DDS multicast between clients.
- If a saved map will not load, check that `maps/<name>.yaml` references an existing `maps/<name>.pgm`.

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
| `nav2_map_server` (`map_saver_cli`) | Community | Saves `/map` to `.yaml` + `.pgm` files on demand | [Docs](https://docs.nav2.org/configuration/packages/map_server/configuring-map-saver.html) |
| `controller` | Vendor (HiWonder, **closed source**) | Motor controller, wheel odometry + EKF, `odom → base_footprint` TF, robot description, joint states, arm init pose | Pre-installed on Jetson |
| `peripherals` | Vendor (HiWonder, **closed source**) | LiDAR launch, filtered `/scan`, joystick control node | Pre-installed on Jetson |
| `sllidar_ros2` (wrapped by `peripherals`) | Hardware vendor (SLAMTEC, open source) | RPLidar A1 hardware driver | [GitHub](https://github.com/Slamtec/sllidar_ros2) |
| `laser_filters` (wrapped by `peripherals`) | Official ROS | Filters raw LiDAR scan before SLAM | [GitHub](https://github.com/ros-perception/laser_filters) · [Docs](https://docs.ros.org/en/humble/p/laser_filters/) |
| `ros_robot_controller` (used by `controller`) | Vendor (HiWonder, **closed source**) | Serial protocol driver to the motor board and hardware controller | Pre-installed on Jetson |
