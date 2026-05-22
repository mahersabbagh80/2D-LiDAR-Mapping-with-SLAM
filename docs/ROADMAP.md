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
- [x] RViz2 config created (`config/rviz/mapping.rviz`) — map, scan, TF displays pre-configured
- [ ] Hardware session: connect to the Jetson via NoMachine, then launch RViz2:
  ```bash
  source /opt/ros/humble/setup.bash
  rviz2 -d ~/mapping_ws/src/2D-LiDAR-Mapping-with-SLAM/config/rviz/mapping.rviz
  ```
- [ ] Drive the robot around the room and watch the map build
- [ ] Save the map (run on the Jetson while the mapping stack is still up):
  ```bash
  ros2 run nav2_map_server map_saver_cli -f ~/maps/room_map --ros-args -p map_subscribe_transient_local:=true
  ```

**Done when:** Driving a closed loop produces a recognisable map; `.pgm` and `.yaml` files saved successfully.

---

### Stretch Goal 1 — Tuning
- [ ] Run at least 3 mapping sessions and compare results
- [ ] Tune key slam_toolbox params (`resolution`, `minimum_travel_distance`, `minimum_travel_heading`)
- [ ] Save the best parameter set to `config/slam_toolbox_params.yaml`

**Done when:** Best parameter set documented; >= 90% of drivable floor area mapped in a single session.

---

### Stretch Goal 2 — Sensor Fusion with IMU

The JetRover bringup already runs `ekf_filter_node` (`robot_localization`) fusing `/imu` + `/odom_raw` into `/odom`. The pipeline is there — but the EKF covariances are almost certainly defaults, not tuned values.

**Goal:** Characterize and tune the EKF fusion pipeline to minimize localization drift.

- **What it is:** The EKF has covariance matrices for each sensor input (process noise Q, measurement noise R). Poorly tuned values cause the filter to over-trust the IMU, over-trust wheel odometry, or oscillate between them — all of which degrade map quality silently.
- **What to do:** Record a known trajectory, compare `/odom` output against ground truth, and iterate on the `robot_localization` YAML config to minimize drift. Measure localization error on a closed loop before and after tuning.
- **Why it matters:** This connects perception (the map), estimation (EKF output), and real-world error. The result is directly observable in map quality and the before/after comparison is a strong portfolio artifact.
- **When to attempt it:** After Milestone 5 — once baseline map quality is established, you have a clear reference to measure improvement against.
