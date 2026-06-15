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
- [x] Inspect the JetRover URDF with `jetrover_description` when needed: `check_urdf` (optional separate repo for offline inspection)
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
- [x] Drive the robot with the gamepad through the vendor joystick launch included by `mapping.launch.py`
- [x] Confirm all four Mecanum drive directions work (including strafe)
- [x] Tune speed to a safe mapping speed (slow enough to avoid scan blur)

**Done when:** Robot moves correctly in all directions, `controller/cmd_vel` visible while driving.

---

### Milestone 5 — SLAM
- [x] Launch `slam_toolbox` in online async mode
- [x] RViz2 config updated (`config/rviz/mapping.rviz`) — Map, LaserScan, TF, and RobotModel displays pre-configured
- [x] Runtime robot model verified from the vendor `controller` stack — `/robot_description`, `robot_state_publisher`, and link TF are not launched directly by this package
- [x] `jetrover_description` kept optional for host-side URDF/xacro inspection only; it is not a runtime dependency of this package
- [x] `mapping.launch.py` finalised: removed redundant `robot_state_publisher` and `joint_state_publisher` (vendor controller stack owns those); added `init_pose` to move arm to resting position on startup; added `joystick_control` for gamepad driving
- [x] Deployed to Jetson: pulled latest, `colcon build --symlink-install`, no build errors
- [x] Confirmed `odom → base_footprint` TF live and robot model renders in RViz2
- [x] Drove the robot around the apartment and built a complete map
- [x] Map-save workflow documented for `maps/apartment.pgm` / `maps/apartment.yaml`; pose graph serialization command captured for future resume workflows
  ```bash
  ros2 run nav2_map_server map_saver_cli -f /home/ubuntu/my_projects/mapping_ws/src/2D-LiDAR-Mapping-with-SLAM/maps/apartment
  ros2 service call /slam_toolbox/serialize_map slam_toolbox/srv/SerializePoseGraph "{filename: '/home/ubuntu/my_projects/mapping_ws/src/2D-LiDAR-Mapping-with-SLAM/maps/apartment'}"
  ```

**Done when:** Driving a closed loop produces a recognisable map; robot model visible in RViz2; `.pgm` and `.yaml` files saved with matching filenames, and pose graph serialization validated when resuming is needed.