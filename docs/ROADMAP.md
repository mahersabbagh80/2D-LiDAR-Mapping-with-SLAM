# Goals & Roadmap

---

## Goals & Success Criteria

| Goal | Done when |
|------|-----------|
| LiDAR and odometry data flowing | `/scan` and `/odom` publishing on the Jetson |
| TF tree correct | `map -> odom -> base_footprint -> base_link -> lidar_link -> lidar_frame` resolves without errors |
| SLAM running | `/map` visible in RViz2, updating as the robot moves |
| Map saved | `.pgm` and `.yaml` files loadable by `map_server` |

---

## Milestones

### Milestone 1 — Hardware Bringup
- [x] Confirm ROS 2 Humble is running on the Jetson
- [x] Stop the HiWonder auto-start service: `sudo systemctl stop start_app_node.service`
- [x] Launch `ros2 launch bringup bringup.launch.py` without errors
- [x] Confirm `/scan` publishes: `ros2 topic echo /scan`
- [x] Confirm `/odom` publishes: `ros2 topic echo /odom`
- [x] Confirm your LiDAR model (G4 or A1) via the robot config tool

**Done when:** `/scan` is live at ~10 Hz, `/odom` has a non-zero covariance matrix, bringup is clean.

---

### Milestone 2 — TF Tree
- [x] Load `jetrover_description` and check URDF: `check_urdf`
- [x] Visualise the TF tree: `ros2 run tf2_tools view_frames`
- [x] Confirm `base_link -> lidar_link -> lidar_frame` offset matches the physical LiDAR mounting position

**Done when:** Full chain `odom -> base_footprint -> base_link -> lidar_link -> lidar_frame` resolves with no transform errors.

---

### Milestone 3 — Odometry Validation
- [x] Confirm `/odom` publisher node: `ros2 topic info /odom --verbose`
- [x] Calibrate linear velocity (command 1 m, measure actual distance, tune scale)
- [x] Calibrate angular velocity (command 360°, measure actual rotation, tune scale — run 3× and average)
- [x] Verify the robot travels ~1 m straight and returns to within ~5 cm

**Done when:** Odometry drift over a short loop is acceptable before SLAM correction kicks in.

---

### Milestone 4 — Teleoperation
- [x] Drive the robot: `ros2 launch peripherals teleop_key_control.launch.py`
- [x] Confirm all four Mecanum drive directions work (including strafe)
- [x] Tune speed to a safe mapping speed (slow enough to avoid scan blur)

**Done when:** Robot moves correctly in all directions, `/cmd_vel` visible while driving.

---

### Milestone 5 — SLAM
- [x] Launch `slam_toolbox` in online async mode
- [x] RViz2 config updated (`config/rviz/mapping.rviz`) — Map, LaserScan, TF, and RobotModel displays pre-configured
- [x] `jetrover_description` vendored in host workspace (`src/vendor/jetrover_description/`) — URDF/xacro for Mecanum chassis + RPLidar A1
- [x] `mapping.launch.py` finalised: removed redundant `robot_state_publisher` and `joint_state_publisher` (vendor controller stack owns those); added `init_pose` to move arm to resting position on startup; added `joystick_control` for gamepad driving
- [x] Deployed to Jetson: pulled latest, `colcon build --symlink-install`, no build errors
- [x] Confirmed `odom → base_footprint` TF live and robot model renders in RViz2
- [x] Drove the robot around the apartment and built a complete map
- [x] Map saved to `maps/apartment.pgm` / `maps/apartment.yaml` and `maps/apartment.posegraph` (pose graph for resuming)
  ```bash
  ros2 run nav2_map_server map_saver_cli -f ~/jetson_ws/src/2D-LiDAR-Mapping-with-SLAM/maps/apartment
  ros2 service call /slam_toolbox/serialize_map slam_toolbox/srv/SerializePoseGraph "{filename: '/home/ubuntu/jetson_ws/src/2D-LiDAR-Mapping-with-SLAM/maps/apartment'}"
  ```
  Current checkout note: `maps/apartment.yaml` references `apartment.pgm`, but the tracked PGM file is spelled `apratment.pgm`. Rename/copy the PGM or update the YAML `image:` field before loading this checked-in map.

**Done when:** Driving a closed loop produces a recognisable map; robot model visible in RViz2; `.pgm`, `.yaml`, and `.posegraph` files saved and committed.