# Architecture & Planning

## System Architecture

The Room-Mapping Explorer pipeline follows a standard ROS 2 SLAM pattern:

```
[YDLiDAR G4]           →  /scan    →  [slam_toolbox]  →  /map  →  [RViz2]
                                                                 [map_saver_cli]
[jetrover_bringup]      →  /odom    →  (nav2 / debugging)
[jetrover_bringup]      →  /tf      →  [slam_toolbox]              (odom→base_link)
[robot_state_publisher] →  /tf      →  [slam_toolbox]              (base_link→laser)
[teleop_twist_keyboard] →  /cmd_vel →  [jetrover_bringup]
```

The high-level pipeline diagram is at [`docs/images/pipeline.svg`](images/pipeline.svg).
The authoritative runtime node graph (generated from the live system) will be added here after Milestone 5.

---

## Node / Topic Interface Table

This table defines the interface contracts between nodes. Each row is a verifiable claim — validate with `ros2 topic info <topic>` and `ros2 topic hz <topic>` once the system is running.

| Topic | Message Type | Publisher | Subscriber(s) | Notes |
|-------|-------------|-----------|---------------|-------|
| `/scan` | `sensor_msgs/LaserScan` | `ydlidar_ros2_driver` | `slam_toolbox` | ~10 Hz (to be confirmed — config-dependent); confirm QoS — mismatch causes silent failure |
| `/odom` | `nav_msgs/Odometry` | `jetrover_bringup` | `slam_toolbox` | Mecanum chassis; wheel encoder dead-reckoning. Computed from 4 independent wheel velocities — more susceptible to wheel slip than differential drive; covariance matrix must not be all zeros |
| `/tf` | `tf2_msgs/TFMessage` | `jetrover_bringup`, `robot_state_publisher`, `slam_toolbox` | All nodes | Full chain: `map → odom → base_link → laser` |
| `/map` | `nav_msgs/OccupancyGrid` | `slam_toolbox` | RViz2, `map_saver_cli` | QoS: transient local (required for `map_saver_cli`) |
| `/cmd_vel` | `geometry_msgs/Twist` | `teleop_twist_keyboard` | `jetrover_bringup` | Velocity commands to motor controller; does NOT feed slam_toolbox |
| `/imu/data` | `sensor_msgs/Imu` | `jetrover_bringup` | — | Not consumed in MVP; stretch goal for Milestone 8 sensor fusion |

### TF Tree

```
map
 └── odom                 (published by slam_toolbox)
      └── base_link        (published by jetrover_bringup via wheel odometry)
           └── laser       (published by robot_state_publisher from URDF)
```

> **Note:** The laser frame name above (`laser`) follows the convention used in the README and is the assumed default. The actual frame name must be verified against the JetRover URDF before Milestone 2.

**Who publishes what:**
- `odom → base_link`: `jetrover_bringup` (wheel encoder odometry)
- `base_link → laser`: `robot_state_publisher` (static, from URDF)
- `map → odom`: `slam_toolbox` (updated each scan match)

**Failure mode:** If `base_link → laser` has the wrong rotation (e.g., LiDAR mounted at an angle not reflected in the URDF), scan matching will be systematically wrong and the map will distort. Validate in Milestone 2 before proceeding.

### QoS Risk

slam_toolbox subscribes to `/scan` with specific QoS settings. If the YDLiDAR driver publishes with mismatched QoS (e.g., `RELIABLE` vs `BEST_EFFORT`), the subscription silently fails — no error, no data. Verify with:

```bash
ros2 topic info /scan --verbose
```

---

## Open Questions (resolve before Milestone 5)

- [ ] What QoS profile does `ydlidar_ros2_driver` use for `/scan`? (RELIABLE or BEST_EFFORT?)
- [ ] Does `jetrover_bringup` publish `/odom` with a populated covariance matrix?
- [ ] What is the exact `base_link → laser` offset in the JetRover URDF?
- [ ] Is `online_async` mode the default in the HiWonder slam_toolbox launch, or does it need to be set explicitly?
- [ ] What `frame_id` does `ydlidar_ros2_driver` stamp on `/scan` messages? Must match the URDF laser frame name. Verify with: `ros2 topic echo /scan --once | grep frame_id`
