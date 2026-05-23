# Architecture

![Pipeline](images/pipeline.png)

> Note: the pipeline image is pending an update. The diagrams below reflect the accurate data flow.

---

## Pipeline

### Mapping — sensor data to map

```
controller   ──/odom + TF: odom→base_fp──►  slam_toolbox  ──/map──►  RViz2
                                                  ▲                    map_saver_cli
peripherals  ──/scan──────────────────────────────┘
```

- `controller` (HiWonder) runs the full odometry stack internally: reads wheel encoders, filters the IMU, and fuses both through its built-in EKF. It publishes `/odom` and the `odom → base_footprint` transform.
- `peripherals` (HiWonder, wraps `sllidar_ros2`) reads the RPLidar A1 and publishes `/scan`.
- `slam_toolbox` consumes `/scan` and the TF tree to build the occupancy-grid map and publish it on `/map`.
- RViz2 displays the live map; `map_saver_cli` saves it to disk at the end of a session.

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
| `controller` | Motor driver + wheel/IMU odometry | HiWonder vendor package; ships with a pre-tuned EKF that fuses wheel encoders and the on-board IMU — no custom odometry code needed |
| `peripherals` | LiDAR driver + teleop | HiWonder vendor package; pre-configured for the RPLidar A1 and provides the keyboard teleop launch |
| `slam_toolbox` | SLAM — builds the map | Industry-standard ROS 2 SLAM library; async mode is safe for embedded hardware; supports map saving and later map reuse |

---

## Topics

| Topic | Message Type | Publisher | Subscriber(s) |
|-------|-------------|-----------|---------------|
| `/scan` | `sensor_msgs/LaserScan` | `peripherals` (sllidar_ros2) | `slam_toolbox` |
| `/odom` | `nav_msgs/Odometry` | `controller` (vendor EKF) | `slam_toolbox` |
| `/cmd_vel` | `geometry_msgs/Twist` | `peripherals` teleop | `controller` |
| `/map` | `nav_msgs/OccupancyGrid` | `slam_toolbox` | RViz2, `map_saver_cli` |
| `/tf` | `tf2_msgs/TFMessage` | `controller`, `slam_toolbox` | all nodes |

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
 └── odom                      (slam_toolbox — corrects drift between map and odometry)
      └── base_footprint        (controller vendor EKF — wheel + IMU fusion, ~30 Hz)
           └── base_link        (robot_state_publisher — rigid offset from footprint, from URDF)
                └── lidar_link  (robot_state_publisher — LiDAR mount position, from URDF)
```

- `map` — fixed reference frame for the whole room. Created by `slam_toolbox`.
- `odom` — the robot's starting position. The `map → odom` transform is updated continuously by `slam_toolbox` to correct accumulated odometry drift.
- `base_footprint` — the 2D floor-projected center of the robot, updated at ~30 Hz by the vendor EKF.
- `base_link` — the 3D center of the robot body; a static transform above `base_footprint` defined in the URDF.
- `lidar_link` — the LiDAR sensor mount; a static transform relative to `base_link` defined in the URDF.

When `slam_toolbox` receives a laser scan, it looks up the TF tree to find where the LiDAR was in the room at that exact timestamp — that is how it places each scan correctly in the map.
