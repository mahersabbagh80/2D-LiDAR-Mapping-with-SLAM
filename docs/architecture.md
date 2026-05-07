# Architecture

## High Level Architecture

![Pipeline](images/pipeline.png)

### Inputs

- **LiDAR** — publishes `/scan` (`LaserScan`); distance readings from one full 360-degree sweep.
- **Wheel odometry** — publishes `/odom` (`Odometry`); how far the robot has moved based on wheel rotation.
- **Teleop** — publishes `/cmd_vel` (`Twist`); keyboard commands translated into velocity instructions.
- **IMU ✦** — publishes `/imu` (`Imu`); measures orientation and angular velocity. ✦ stretch goal, optional.

### Perception

- **LiDAR driver** — vendor package (off-the-shelf); reads the physical sensor and produces `/scan`.
- **TF tree** — coordinate frame chain `map → odom → base_link → lidar_link → lidar_frame`; tells every node where each part of the robot is in space.
- **IMU filter ✦** — `imu_filter_madgwick`; fuses IMU data to improve orientation estimates. ✦ stretch goal, optional.

### SLAM

- **SLAM node** — `slam_toolbox` (or `cartographer_ros` ✦ as stretch); consumes `/scan` and `/odom`, builds a map, and localises the robot inside it simultaneously.

### Planning / Control

- **map_saver** — off-the-shelf, configured to save the map on demand.
- **RViz2** — displays the live map and robot position during exploration.
- **Motor driver** — vendor package (off-the-shelf); receives `/cmd_vel` and drives the Mecanum wheels.

### Outputs

- **Saved map** — written to `room.pgm` + `room.yaml`; success criterion: coverage >= 90%.
- **Live map** — published on `/map` (`OccupancyGrid`); streamed to RViz2 while the robot moves.
- **Robot motion** — Mecanum drive executed by the motor driver in response to `/cmd_vel`.


---

## Low Level Architecture (ROS 2)

```
[SLAMTEC RPLidar A1]  ->  /scan    ->  [slam_toolbox]  ->  /map  ->  [RViz2]
                                                                  [map_saver_cli]
[bringup]             ->  /odom    ->  [slam_toolbox]
[bringup]             ->  /tf      ->  [slam_toolbox]   (odom -> base_link)
[robot_state_pub]     ->  /tf      ->  [slam_toolbox]   (base_link -> lidar_link -> lidar_frame)
[teleop_keyboard]     ->  /cmd_vel ->  [bringup]
```

In ROS 2, software is organized as nodes. A node is just a program that does one job. Nodes talk to each other by publishing and subscribing to topics — a topic is like a named channel that carries a specific type of message.
In the Diagram above, anything in between square brackets [] is designated as a node. each node has subscribers are publishers. publishers publish Topics and Subscribers listens to these Topics.

### Topics

| Topic | Message Type | Who Publishes | Who Reads |
|-------|-------------|---------------|-----------|
| `/scan` | `sensor_msgs/LaserScan` | `ydlidar_ros2_driver` | `slam_toolbox` |
| `/odom` | `nav_msgs/Odometry` | `jetrover_bringup` | `slam_toolbox` |
| `/tf` | `tf2_msgs/TFMessage` | `jetrover_bringup`, `robot_state_publisher`, `slam_toolbox` | all nodes |
| `/map` | `nav_msgs/OccupancyGrid` | `slam_toolbox` | RViz2, `map_saver_cli` |
| `/cmd_vel` | `geometry_msgs/Twist` | `teleop_twist_keyboard` | `jetrover_bringup` |

- `/scan` carries the raw distance readings from the LiDAR, one full rotation at a time.
- `/odom` carries the robot's estimated position based on wheel movement since it started.
- `/tf` carries coordinate frame relationships — it tells every node where each physical part of the robot is relative to everything else.
- `/map` carries the occupancy grid — a 2D grid where each cell is marked free, occupied, or unknown.
- `/cmd_vel` carries a velocity command: how fast to move forward and how fast to turn.

### TF Tree

The TF tree is ROS 2's way of tracking the positions of physical parts of the robot relative to each other. Every frame is a named coordinate origin attached to something real.

```
map
 └── odom                    (published by slam_toolbox)
      └── base_footprint     (published by bringup — tracks wheel movement)
           └── base_link     (static — 2D floor projection of base_link, from URDF)
                └── lidar_link
                     └── lidar_frame  (static — LiDAR scan origin, from URDF)
```

- `map` is the fixed reference frame for the whole room. slam_toolbox creates it.
- `odom` is the robot's starting position. The relationship between `map` and `odom` is updated by slam_toolbox as it corrects for drift.
- `base_footprint` is the dynamic frame tracking wheel movement, published at ~30 Hz by bringup.
- `base_link` is the center of the robot body. It is a static transform directly above `base_footprint` (the 2D floor projection convention).
- `lidar_link` and `lidar_frame` represent the LiDAR sensor mount and its scan origin. Their positions relative to `base_link` are defined in the URDF and do not change.

When slam_toolbox receives a laser scan, it uses the TF tree to know exactly where in the room that scan came from, which is how it builds an accurate map.
