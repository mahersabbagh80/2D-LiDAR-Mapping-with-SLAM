# Architecture

![Pipeline](images/pipeline.png)

> Note: the pipeline image is pending an update. The diagrams below reflect the accurate data flow.

---

## Pipeline

### Mapping — sensor data to map

```
controller   ──/odom_raw──►  odom_relay  ──/odom──►  slam_toolbox  ──/map──►  RViz2
                                  │                       ▲                    map_saver_cli
                                  └── TF: odom→base_fp ──┘
peripherals  ──/scan──────────────────────────────►  slam_toolbox
```

- `controller` (HiWonder) reads wheel encoders and publishes raw odometry on `/odom_raw`.
- `odom_relay` (this package) forwards `/odom_raw → /odom` and publishes the `odom → base_footprint` transform.
- `peripherals` (HiWonder, wraps `sllidar_ros2`) reads the RPLidar A1 and publishes `/scan`.
- `slam_toolbox` consumes `/scan`, `/odom`, and the TF tree to build the occupancy-grid map and publish it on `/map`.
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
| `peripherals` | LiDAR driver | HiWonder vendor package; pre-configured for the RPLidar A1 on this robot |
| `controller` | Motor driver + raw odometry | HiWonder vendor package; handles the Mecanum hardware and encoder readout |
| `odom_relay` | Odometry relay — forwards `/odom_raw` → `/odom` + TF | Included in this package; simpler than a full EKF for a wheel-odometry-only setup |
| `slam_toolbox` | SLAM — builds the map | Industry-standard ROS 2 SLAM library; async mode is safe for embedded hardware; supports map saving and later map reuse |

---

## Topics

| Topic | Message Type | Publisher | Subscriber(s) |
|-------|-------------|-----------|---------------|
| `/scan` | `sensor_msgs/LaserScan` | `peripherals` (sllidar_ros2) | `slam_toolbox` |
| `/odom_raw` | `nav_msgs/Odometry` | `controller` | `odom_relay` |
| `/odom` | `nav_msgs/Odometry` | `odom_relay` | `slam_toolbox` |
| `/cmd_vel` | `geometry_msgs/Twist` | `peripherals` teleop | `controller` |
| `/map` | `nav_msgs/OccupancyGrid` | `slam_toolbox` | RViz2, `map_saver_cli` |
| `/tf` | `tf2_msgs/TFMessage` | `odom_relay`, `robot_state_publisher`, `slam_toolbox` | all nodes |

- `/scan` — raw distance readings from the LiDAR, one array of ranges per full rotation.
- `/odom_raw` — wheel encoder estimate of robot displacement; noisy and drifts over time.
- `/odom` — relayed wheel odometry; what `slam_toolbox` uses to track robot motion between scans.
- `/cmd_vel` — velocity command: linear x/y and angular z for Mecanum drive.
- `/map` — 2D occupancy grid where each cell is free (0), occupied (100), or unknown (-1).
- `/tf` — the transform tree; every node reads this to know where things are in space.

---

## TF Tree

The TF tree tracks the position of every physical part of the robot relative to each other and to the map. Every frame is a named coordinate origin attached to something real.

```
map
 └── odom                      (slam_toolbox — corrects drift between map and odometry)
      └── base_footprint        (odom_relay — tracks wheel movement, ~30 Hz)
           └── base_link        (robot_state_publisher — rigid offset from footprint, from URDF)
                └── lidar_link  (robot_state_publisher — LiDAR mount position, from URDF)
```

- `map` — fixed reference frame for the whole room. Created by `slam_toolbox`.
- `odom` — the robot's starting position. The `map → odom` transform is updated continuously by `slam_toolbox` to correct accumulated odometry drift.
- `base_footprint` — the 2D floor-projected center of the robot, updated at ~30 Hz by `odom_relay` as the robot moves.
- `base_link` — the 3D center of the robot body; a static transform above `base_footprint` defined in the URDF.
- `lidar_link` — the LiDAR sensor mount; a static transform relative to `base_link` defined in the URDF.

When `slam_toolbox` receives a laser scan, it looks up the TF tree to find where the LiDAR was in the room at that exact timestamp — that is how it places each scan correctly in the map.

---

## odom_relay.py

`odom_relay` is a lightweight node included in this package. It subscribes to `/odom_raw` (published by `controller`), republishes the data on `/odom`, and broadcasts the `odom → base_footprint` transform — the two things `slam_toolbox` needs to track the robot's position.

It is launched by `mapping.launch.py` as the odometry layer for this project. slam_toolbox's scan matching compensates for the noise in raw wheel odometry, so a full EKF is not needed at this stage.
