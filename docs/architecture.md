# Architecture

![Pipeline](images/pipeline.png)

---

## Pipeline

### Mapping — sensor data to map

```
jetrover_description  ──/robot_description + TF: base_link→lidar_link──►  RViz2 (3D model)
controller            ──/odom + TF: odom→base_fp──►  slam_toolbox  ──/map──►  RViz2
                                                            ▲                   map_saver_cli
peripherals           ──/scan──────────────────────────────┘
```

- `jetrover_description` provides the robot URDF. `robot_state_publisher` reads it and publishes the `/robot_description` topic (so RViz2 can render the 3D model) and the static TF transforms between links (e.g. `base_link → lidar_link`).
- `controller` (HiWonder) runs the full odometry stack internally: reads wheel encoders, filters the IMU, and fuses both through its built-in EKF. It publishes `/odom` and the `odom → base_footprint` transform.
- `peripherals` (HiWonder, wraps `sllidar_ros2`) reads the RPLidar A1 and publishes `/scan`.
- `slam_toolbox` consumes `/scan` and the TF tree to build the occupancy-grid map and publish it on `/map`.
- RViz2 displays the live map and the 3D robot model; `map_saver_cli` saves the map to disk at the end of a session.

### Teleoperation — keyboard to wheels

```
teleop  ──/cmd_vel──►  controller  ──►  Mecanum wheels
```

- `peripherals` provides the teleop launch, which reads keyboard input and publishes velocity commands on `/cmd_vel`.
- `controller` receives `/cmd_vel` and drives the four Mecanum wheels accordingly.

---

## Why each package is in the pipeline

| Package | Role | Why this package |
|---------|------|-----------------|
| `jetrover_description` | Robot URDF + 3D meshes | Sibling repo; provides the physical description of the JetRover so `robot_state_publisher` can broadcast the TF link tree and RViz2 can render the 3D model |
| `controller` | Motor driver + wheel/IMU odometry | HiWonder vendor package; ships with a pre-tuned EKF that fuses wheel encoders and the on-board IMU — no custom odometry code needed |
| `peripherals` | LiDAR driver + teleop | HiWonder vendor package; pre-configured for the RPLidar A1 and provides the keyboard teleop launch |
| `slam_toolbox` | SLAM — builds the map | Industry-standard ROS 2 SLAM library; async mode is safe for embedded hardware; supports map saving and later map reuse |

---

## Topics

| Topic | Message Type | Publisher | Subscriber(s) |
|-------|-------------|-----------|---------------|
| `/robot_description` | `std_msgs/String` | `robot_state_publisher` | RViz2 (3D model) |
| `/scan` | `sensor_msgs/LaserScan` | `peripherals` (sllidar_ros2) | `slam_toolbox` |
| `/odom` | `nav_msgs/Odometry` | `controller` (vendor EKF) | `slam_toolbox` |
| `/cmd_vel` | `geometry_msgs/Twist` | `peripherals` teleop | `controller` |
| `/map` | `nav_msgs/OccupancyGrid` | `slam_toolbox` | RViz2, `map_saver_cli` |
| `/tf` | `tf2_msgs/TFMessage` | `controller`, `robot_state_publisher`, `slam_toolbox` | all nodes |

- `/robot_description` — the full URDF as a string, published once with Transient Local QoS so late-joining subscribers (like RViz2 on the dev machine) still receive it.
- `/scan` — raw distance readings from the LiDAR, one array of ranges per full rotation.
- `/odom` — wheel + IMU fused odometry from the vendor EKF; what `slam_toolbox` uses to track robot motion between scans.
- `/cmd_vel` — velocity command: linear x/y and angular z for Mecanum drive.
- `/map` — 2D occupancy grid where each cell is free (0), occupied (100), or unknown (-1).
- `/tf` — the transform tree; every node reads this to know where things are in space.

---

## TF Tree

The TF tree tracks the position of every physical part of the robot relative to each other and to the map. Every frame is a named coordinate origin attached to something real.

```
map
 └── odom                          (slam_toolbox — corrects drift between map and odometry)
      └── base_footprint            (ekf_filter_node — wheel odometry fusion, ~30 Hz)
           └── base_link            (robot_state_publisher — rigid offset from footprint, from URDF)
                ├── lidar_frame     (robot_state_publisher — LiDAR mount position, from URDF)
                ├── front_left_wheel   (robot_state_publisher — joint state from joint_state_publisher)
                ├── front_right_wheel  (robot_state_publisher — joint state from joint_state_publisher)
                ├── rear_left_wheel    (robot_state_publisher — joint state from joint_state_publisher)
                └── rear_right_wheel   (robot_state_publisher — joint state from joint_state_publisher)
```

- `map` — fixed reference frame for the whole room. Published by `slam_toolbox`.
- `odom` — the robot's starting position. The `map → odom` transform is updated continuously by `slam_toolbox` to correct accumulated odometry drift.
- `base_footprint` — the 2D floor-projected center of the robot, updated at ~30 Hz by `ekf_filter_node` (`robot_localization` package) fusing `/odom_raw` from the wheel encoders.
- `base_link` — the 3D center of the robot body; a static transform above `base_footprint` defined in the URDF.
- `lidar_frame` — the LiDAR sensor mount; a static transform relative to `base_link` defined in the URDF. This is the frame that SLAM uses to place each scan in the map.
- `*_wheel` frames — the four Mecanum wheel positions; static transforms from `base_link` defined in the URDF. `robot_state_publisher` needs joint angles from `/joint_states` (provided by `joint_state_publisher`) to publish these — without them the transforms are unpublished and warnings appear in the logs.

When `slam_toolbox` receives a laser scan, it looks up the TF tree to find where `lidar_frame` was in the room at that exact timestamp — that is how it places each scan correctly in the map.
