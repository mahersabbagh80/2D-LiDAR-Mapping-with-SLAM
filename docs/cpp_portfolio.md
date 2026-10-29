# C++ implementation and presentation guide

## What this project demonstrates

The project integrates a JetRover's LiDAR, wheel odometry, vendor controller, and SLAM Toolbox into a teleoperated mapping system. The repository now adds a C++17 ROS 2 component that makes the LiDAR input observable through standard diagnostic messages.

The engineering contribution is scan-health analysis, ROS integration, reproducible launch/configuration, and testing. SLAM Toolbox supplies scan matching, pose-graph optimization, and occupancy-grid mapping. HiWonder supplies the hardware interfaces and controller. The existing apartment screenshots demonstrate the earlier mapping integration; they are not evidence that the new C++ node has run on the robot.

This is a C++ ROS package with XML launch orchestration. It is not a Python-free ROS system: ROS tooling and vendor-owned launch files still use Python internally.

## What changed

| Previously | Now | Reason |
|---|---|---|
| `setup.py`, `setup.cfg`, empty Python module, package resource marker | `CMakeLists.txt`, `ament_cmake`, C++17 sources and headers | Build and install actual C++ executables and libraries |
| `launch/mapping.launch.py` | `launch/mapping.launch.xml` | Use a supported declarative launch format for orchestration |
| No repository-owned sensor-health node | `scan_diagnostics_node` | Expose scan quality and freshness without altering mapping inputs |
| Robot needed for a live demonstration | `scan_demo_node` and `diagnostics_demo.launch.xml` | Demonstrate fault detection and recovery using synthetic data |
| No automated test suite | C++ unit tests, ROS integration tests, Humble CI workflow | Check numerical edge cases and actual message delivery |

The original Python implementation remains in Git history. The migration is developed on `feature/cpp-scan-diagnostics`; no duplicate Python implementation is required in the installed package. Vendor Python launch files remain external dependencies.

## Follow one scan through the code

1. The LiDAR driver publishes `sensor_msgs/msg/LaserScan` on `/scan`.
2. `ScanDiagnosticsNode` subscribes using `rclcpp::SensorDataQoS`. Its callback receives a shared pointer to an immutable scan.
3. `analyze_scan()` validates the scan structure, then walks through the range array once. It computes counts and statistics without changing the message.
4. The callback saves the analysis, frame ID, and monotonic receipt time. It does not retain a history of scans.
5. A wall timer publishes `diagnostic_msgs/msg/DiagnosticArray` on `/diagnostics`, including the current health state and named measurements.
6. SLAM Toolbox continues subscribing to the original scan stream independently.

Start reading at these files:

| File | What to explain |
|---|---|
| `include/two_d_lidar_mapping_with_slam/scan_analysis.hpp` | The result structure and analysis interface |
| `src/scan_analysis.cpp` | Validation, classification, and a single-pass statistics calculation |
| `src/scan_diagnostics_node.cpp` | ROS subscriptions, QoS, parameters, timer, and diagnostic states |
| `src/scan_diagnostics_main.cpp` | Node lifecycle and nonzero exit on startup failure |
| `src/scan_demo_node.cpp` | Deterministic synthetic input for the live demonstration |
| `launch/mapping.launch.xml` | Hardware integration, conditional launch groups, and unchanged SLAM configuration |
| `test/` | Independent expected results and real ROS message-based assertions |

## What the analysis checks

The scan must contain ranges and a nonempty frame ID. Range bounds must be finite, nonnegative at the lower bound, and strictly increasing. Angular fields must be finite, the increment must be nonzero, and the angular endpoint must agree with the sample count within a small rounding tolerance. Negative angular increments are supported. If intensities are present, their count must match the ranges.

For a structurally valid scan, every range belongs to one category:

| Category | Rule | Meaning |
|---|---|---|
| Valid | Finite and within the inclusive range limits | Usable distance measurement |
| No return | Positive infinity | Typically no detected obstacle within range |
| Invalid | NaN, negative infinity, or finite outside the limits | Unusable measurement |

`valid_fraction = valid_count / total_count`. Minimum, maximum, and mean distances use only valid samples. They are omitted from diagnostics when there are no valid samples. A low valid fraction can occur in open space; the warning does not prove a broken sensor.

The algorithm takes O(n) time for n ranges and O(1) extra storage. No speedup over Python is claimed, because there was no equivalent Python processing implementation to benchmark.

## Diagnostic behavior

| State | Condition | Example |
|---|---|---|
| `STALE` (3) | No scan received yet, or receipt age exceeds the timeout | Disconnected or stopped input stream |
| `ERROR` (2) | Latest scan is structurally malformed, or violates configured frame expectation | Empty ranges array |
| `WARN` (1) | Valid fraction is below the configured threshold | Mostly no-return measurements |
| `OK` (0) | Recent, structurally valid scan meets the threshold | Healthy synthetic scan |

Freshness takes precedence when a previously malformed stream stops. A new valid scan restores `OK`; no manual reset is needed.

The timeout uses `std::chrono::steady_clock`. ROS time may pause during simulation or change during playback, so it is unsuitable for detecting whether this process has recently received data. The diagnostic header still uses ROS time. This monitor does not compare sensor timestamps, detect old data being republished, or check TF availability.

The default callback group is mutually exclusive: scan and timer callbacks cannot update/read the stored analysis concurrently, even if the node is put in a multithreaded executor. There is no custom thread or mutex to maintain. This is not a hard real-time implementation or a motor safety system.

## Configuration

Edit a copy of `config/scan_diagnostics.yaml` and pass it with `params_file:=/absolute/path/to/file.yaml` to the standalone launch. The YAML `scan_topic` is used unless a nonempty `scan_topic:=...` launch argument explicitly overrides it. These parameters are validated at startup and read-only afterward; restart the node to apply changes.

| Parameter | Default | Purpose |
|---|---|---|
| `scan_topic` | `/scan` | Input LaserScan topic; also exposed as a launch argument |
| `expected_frame` | empty string | Optional exact frame expectation; a frame must always be nonempty |
| `scan_timeout_sec` | `1.0` | Maximum age since receipt; finite and greater than zero |
| `publish_period_sec` | `1.0` | Wall-timer period; finite and in [0.001, 3600] s |
| `min_valid_fraction` | `0.1` | Warning threshold in [0, 1] |

`scan_age_sec` is time since the callback received the latest scan, not sensor-to-computer latency. Timeout reporting occurs on the next timer tick, subject to executor scheduling. Tune the timeout and period to the actual sensor rate; they are not safety guarantees.

## A short live demonstration

Use two terminals with the same ROS environment and sourced workspace:

```bash
# Terminal 1
ros2 launch two_d_lidar_mapping_with_slam diagnostics_demo.launch.xml

# Terminal 2
ros2 topic echo /demo/diagnostics
```

Explain the sequence while it runs:

| Approximate elapsed time | Input | Expected observation |
|---|---|---|
| 0–5 s | All 181 ranges are 2 m | `OK`, valid fraction 1, mean range 2 m |
| 5–10 s | 170 of 181 ranges are positive infinity | `WARN`, 11 valid and 170 no returns |
| 10–15 s | Empty range array | `ERROR`, “Empty ranges array” |
| 15–20 s | Publisher stops sending | After 1 s plus a timer tick: `STALE` |
| 20–25 s | Healthy scans resume | Recovery to `OK` |

The sequence repeats. Startup may briefly report `STALE` before DDS discovery completes. Stop with Ctrl+C. Synthetic scans are isolated on `/demo/scan`; they are not fed into the real mapping stack. The demo has no robot motion, TF simulation, or mapping result.

To exercise the mapping XML entry point without robot or SLAM dependencies:

```bash
ros2 launch two_d_lidar_mapping_with_slam mapping.launch.xml \
  start_robot:=false start_slam:=false
```

It should report `STALE` on `/diagnostics` until a real or recorded `/scan` stream is available. This mode validates launch/configuration and the diagnostic executable, not full mapping.

## Migrating the existing workspace

An existing `ament_python` install can leave old `.launch.py` files behind. In the actual ROS workspace root, use a fresh build/install prefix for the C++ version:

```bash
source /opt/ros/humble/setup.bash
# On the Jetson, source the installed vendor workspaces as usual.
colcon --log-base log_cpp build --build-base build_cpp --install-base install_cpp \
  --packages-select two_d_lidar_mapping_with_slam \
  --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON
source install_cpp/setup.bash
colcon --log-base log_cpp test --build-base build_cpp --install-base install_cpp \
  --packages-select two_d_lidar_mapping_with_slam --return-code-on-test-failure
colcon test-result --test-result-base build_cpp --verbose
ros2 pkg prefix two_d_lidar_mapping_with_slam
```

The reported prefix should point into `install_cpp`, and the new entry point is `mapping.launch.xml`. Use a fresh terminal rather than sourcing an old Python overlay first. For zsh use the corresponding `setup.zsh` files.

`scripts/run_mapping.sh` retains the existing Jetson-specific vendor paths. It accepts `MAPPING_INSTALL_PREFIX` for the project's install location. On the robot, set that variable to the absolute `install_cpp` path before invoking the script. Review the actual robot paths first: the README's example `~/jetson_ws` and the existing run script's `/home/ubuntu/my_projects/mapping_ws` are different workspace layouts. Do not run this robot-control script on a development/cloud machine.

## Presenting the work in an interview

A concise explanation, after you have reviewed and practiced the implementation:

> “This project integrates a JetRover's LiDAR and wheel odometry with ROS 2 Humble and SLAM Toolbox for indoor mapping. I extended the package with a C++17 scan-health monitor, migrated its build to CMake, and expressed launch orchestration in XML. The monitor checks scan structure, measurement quality, and receipt timeouts, then publishes standard ROS diagnostics. I used unit tests and ROS integration tests to check edge cases and recovery, and added a hardware-free fault demonstration. SLAM Toolbox provides the mapping algorithm; the vendor provides the robot drivers.”

Useful questions to rehearse:

- **Why keep XML launch?** ROS 2 supports it directly. Starting processes does not need a custom C++ supervisor; C++ is used for message processing.
- **Why best-effort QoS?** LiDAR drivers commonly use sensor-data QoS. A reliable-only subscriber may fail to match a best-effort publisher. The observer prioritizes current sensor data over retransmitting every historical scan.
- **Why distinguish infinity and NaN?** Positive infinity can mean a legitimate no-return observation; NaN is unusable. Treating both as evidence of hardware failure would be misleading.
- **Why a separate analysis library?** Numerical behavior can be tested without starting ROS, while the node handles messaging, timing, and configuration.
- **Why not filter bad scans before SLAM?** This change adds observability while preserving the proven input path. Introducing filtering would need separate mapping-quality evaluation on recorded and live sensor data.
- **What did you validate?** Refer to [the validation record](validation.md). Separate automated diagnostic checks from the historical apartment map and pending Jetson deployment.
- **What remains limited?** There is no new SLAM algorithm, no demonstrated localization improvement, no timestamp/TF validation, and no hard real-time or safety guarantee.

A suitable project title is **“ROS 2 Indoor Mapping with C++ LiDAR Diagnostics.”** A factual application bullet is: “Extended a ROS 2 Humble mapping integration with C++17 LiDAR health diagnostics, CMake/XML deployment, and automated unit and ROS integration tests.” Add measured performance or robot-validation claims only after collecting that evidence.
