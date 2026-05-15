# Project Logbook

This logbook is a chronological record of what I worked on, what I found, and what I will do next.
It complements the `README.md` (which explains what the project is and how to run it).

## How to use this logbook

- Add a new entry for each work session (daily or per meaningful chunk of work).
- Keep entries short and factual.
- Link to evidence where possible (RViz screenshots, TF tree images, `rosbag` names, terminal outputs).
- When you finish a milestone item, reference the exact command/config used so it is reproducible.

## Legend

- **Goal**: What I intended to achieve in this session.
- **Work done**: What I actually did (commands run, files changed, steps taken).
- **Results**: What worked/failed and what I observed (with metrics if available).
- **Evidence**: Screenshots, logs, bag files, links, or filenames.
- **Blockers**: Anything preventing progress.
- **Next**: The very next actions to take.

---

## 2026-05-01 — Project bootstrap

- **Goal**
  - Create repository scaffolding and define a clear plan/milestones.

- **Work done**
  - Created initial `README.md` documenting scope, requirements, and milestones.
  - Set up a TODO list for verifying JetRover ROS/LiDAR details.
  - Created `docs/LOGBOOK.md` so the README stays focused on “what/how”, while the logbook captures day-to-day progress.
  - Initialized git, created the initial commit, and connected/pushed the repository to GitHub.
  - Noted SSH vs HTTPS remote choice; attempted switching `origin` to SSH and diagnosed `Permission denied (publickey)` as a missing/unused key file in WSL (deferred; HTTPS is acceptable).
  - Shared how to invite collaborators so the tutor can access the repo.

- **Results**
  - Repository has a clear problem statement and success criteria.
  - Repo is on GitHub and ready for review/collaboration.

- **Evidence**
  - `README.md`
  - `docs/LOGBOOK.md`

- **Blockers**
  - None.

- **Next**
  - Verify JetRover ROS distro and LiDAR model from official docs / device outputs.

---

## 2026-05-02 — Architecture diagram + README corrections

- **Goal**
  - Refine project documentation and capture the end-to-end mapping pipeline at a high level.

- **Work done**
  - Updated `README.md` with details based on the official JetRover Orin Nano mapping tutorial.
  - Corrected the robot/software info: JetRover uses **ROS 2 Humble** (not ROS Noetic).
  - Added a system architecture/pipeline diagram and iterated on it to fix mistakes and wording:
    - `docs/images/pipeline.svg`
    - `docs/images/pipeline.drawio` (source)
  - Verified that GitHub showing an old embedded image can be a browser/CDN cache issue (hard refresh / private window resolves).

- **Results**
  - Documentation and diagram are aligned with the JetRover tutorial and reflect the ROS 2 Humble stack.
  - Diagram assets are stored both as an editable source (`.drawio`) and a rendered export (`.svg`).

- **Evidence**
  - `README.md`
  - `docs/images/pipeline.svg`
  - `docs/images/pipeline.drawio`

- **Blockers**
  - None.

- **Next**
  - Do more background research, validate and collect the papers, and tutorials used for this project.
  - Start validating the pipeline on the actual robot: confirm sensor topics/frames, SLAM package choice, and reproducible “bringup → mapping → save map” steps.

---

## 2026-05-04 — Planning phase: architecture doc and milestone enrichment

- **Goal**
  - Move from background research into the planning phase; produce a system schematic before writing any code.

- **Work done**
  - Created `docs/architecture.md` containing:
    - ASCII pipeline diagram showing all nodes, topics, and data flow directions
    - Node/topic interface table (`/scan`, `/odom`, `/tf`, `/map`, `/cmd_vel`, `/imu/data`) with message types, publishers, subscribers, and notes
    - TF tree breakdown with publisher attribution and failure mode documentation
    - QoS mismatch risk note with verification command
    - Open questions to resolve before Milestone 5
  - Enriched `README.md` milestones: added `Depends on:` and `Done when:` blocks to all 8 milestones so task checklists and completion criteria live in one place
  - Removed the vague "TODO: Verify and improve the milestones" placeholder
  - Confirmed robot chassis is **Mecanum** (not Ackermann); noted odometry is computed from 4 independent wheel velocities and is more susceptible to wheel slip — covariance matrix must not be all zeros
  - Identified 5 open questions requiring hardware verification before Milestone 5 (QoS profile, covariance matrix, URDF laser offset, slam_toolbox mode default, `/scan` frame_id match)
  - Added Write/Edit/Bash tools to the `robotics-ai-mentor` agent definition (`~/.claude/agents/robotics-ai-mentor.md`) so Tony can edit files directly in future sessions

- **Results**
  - Project now has a genuine planning schematic, not just a vague strategy doc
  - Each milestone is self-contained: task checklist + done criteria + dependencies
  - Interface contracts are documented and verifiable with `ros2 topic info` / `ros2 topic hz`

- **Evidence**
  - `docs/architecture.md`
  - `README.md`

- **Blockers**
  - None — all open questions are deferred to hardware bringup (Milestone 1)

- **Next**
  - Start a new session with Tony (robotics-ai-mentor) — he now has file write access
  - Begin Milestone 1: environment setup on the Jetson Orin Nano
  - Resolve open questions from `docs/architecture.md` against the live system

---

## 2026-05-06 — Milestone 1: Hardware Bringup

- **Goal**
  - Complete Milestone 1: confirm all hardware is functioning and sensor data is flowing on the Jetson.

- **Context**
  - Robot: HiWonder JetRover (Jetson Orin Nano), Mecanum chassis
  - LiDAR: confirmed SLAMTEC RPLidar A1 (README corrected from G4)
  - Host: Ubuntu 22.04 (WSL2 on Windows 11), ROS 2 Humble
  - Jetson IP: 192.168.2.138, accessed via SSH

- **Work done**
  - Stopped HiWonder auto-start service: `sudo systemctl stop start_app_node.service`
  - Discovered bringup package is named `bringup`, not `jetrover_bringup` — found via `ls $(ros2 pkg prefix bringup)/share/bringup/launch/`
  - Launched bringup: `ros2 launch bringup bringup.launch.py`
  - Confirmed `/scan` publishing with real LiDAR data: `ros2 topic echo /scan --once`
  - Confirmed `/odom` publishing
  - Confirmed IMU calibration complete (bias = [0.004, 0.093, 0.005])
  - Fixed WSL2 networking: enabled mirrored networking mode via `C:\Users\maher\.wslconfig` — PC now on 192.168.2.102 (same subnet as Jetson)
  - Updated README: corrected LiDAR model from YDLiDAR G4 to SLAMTEC RPLidar A1, updated driver dependency accordingly

- **Results**
  - Bringup clean — all 22 nodes started, no errors
  - `/scan` live at 10 Hz, max range 12 m, frame_id: `lidar_frame`
  - `/odom` publishing
  - IMU: first message received, gyro calibration complete
  - LiDAR health status: OK

- **Evidence**
  - `ros2 topic echo /scan --once` — ranges ~0.25 m (wall detected), intensities 47.0
  - `ros2 node list` on Jetson — all nodes visible locally

- **Blockers / Issues**
  - Cross-machine ROS 2 discovery (PC → Jetson) not working: FastDDS unicast XML approach attempted but breaks local DDS when FASTRTPS_DEFAULT_PROFILES_FILE is set. Deferred to next session — FastDDS Discovery Server approach not yet tried.
  - All Milestone 1 verification done directly on the Jetson over SSH as a workaround.

- **Next**
  - Milestone 2: TF tree verification
    - Check URDF with `check_urdf`
    - Visualise TF tree: `ros2 run tf2_tools view_frames`
    - Confirm `odom -> base_link -> lidar_frame` chain resolves
  - Fix cross-machine discovery using FastDDS Discovery Server approach

---

## 2026-05-14 — Cross-machine ROS 2 discovery fixed; RViz now runs on WSL2

- **Goal**
  - Fix cross-machine DDS discovery so RViz can run natively on the WSL2 machine instead of via NoMachine remote desktop.

- **Context**
  - Robot: HiWonder JetRover (Jetson Orin Nano), IP: 192.168.2.138 (wlan0)
  - Host: WSL2 Ubuntu 22.04, IP: 192.168.2.102 (eth3, mirrored networking mode)
  - Both on the same 192.168.2.0/24 subnet
  - ROS 2 Humble, FastDDS 2.6.11 on both machines

- **Work done**
  - Diagnosed root cause via systematic elimination:
    - Confirmed Layer 3 reachability: ping works both directions, no packet loss
    - Confirmed ROS_DOMAIN_ID matches (both 0) and DDS implementation matches (both FastDDS)
    - Confirmed `ros2 topic list` was failing due to daemon not running (XML-RPC timeout) — fixed with `ros2 daemon start`
    - Confirmed DDS cross-machine discovery broken: Jetson publishing `/test_topic` but WSL2 saw nothing
    - Confirmed UDP blocked at network layer using netcat: Jetson UDP packets not reaching WSL2
  - **Fix 1 — FastDDS unicast peer config** (bypasses multicast which WiFi routers block):
    - Created `~/fastdds.xml` on WSL2 with Jetson (192.168.2.138) as explicit unicast peer
    - Created `~/fastdds.xml` on Jetson with WSL2 (192.168.2.102) as explicit unicast peer
    - Added `export FASTRTPS_DEFAULT_PROFILES_FILE=~/fastdds.xml` to `~/.zshrc` on both machines
  - **Fix 2 — Windows Firewall rule** (allows inbound UDP from LAN to WSL2):
    - Ran in PowerShell (Admin): `New-NetFirewallRule -DisplayName "ROS2 DDS - Allow UDP from LAN" -Direction Inbound -Protocol UDP -RemoteAddress 192.168.2.0/24 -Action Allow`
  - Added auto-start of `ros2 daemon` to WSL2 `~/.zshrc` so daemon starts silently on terminal open
  - Verified that both fixes are independently necessary (tested each in isolation)

- **Results**
  - Full Jetson topic list visible from WSL2: `/map`, `/scan`, `/odom`, `/tf`, `/slam_toolbox/*`, `/imu`, `/cmd_vel`, and all robot hardware topics
  - RViz can now run on the WSL2 machine and connect to live robot data
  - Cross-machine discovery survives Jetson reboots (confirmed)

- **Evidence**
  - `ros2 topic list` on WSL2 returned 38 topics including `/map` and `/slam_toolbox/scan_visualization`
  - netcat UDP test confirmed packets flowing after firewall rule applied

- **Blockers / Issues**
  - IPs in `fastdds.xml` are hardcoded — if DHCP reassigns either machine's IP, both files must be updated. Mitigation: configure DHCP reservations in router.

- **Next**
  - Launch RViz2 on WSL2 and visualize the SLAM map in real time
  - Continue Milestone 5: SLAM mapping session with full visualization on host machine

---

## 2026-05-15 — Milestone 5: EKF diagnosis and odom-only fix

- **Goal**
  - Unblock slam_toolbox: get the `odom → base_footprint` TF publishing so SLAM can process scans and emit the `map` frame.

- **Context**
  - Robot: HiWonder JetRover (Jetson Orin Nano), IP: 192.168.2.138
  - Host: WSL2 Ubuntu 22.04, IP: 192.168.2.102
  - ROS 2 Humble, cross-machine DDS working from last session
  - All diagnostics run from WSL2 desktop (cross-machine visibility)

- **Work done**
  - Traced the EKF failure root cause through the full IMU pipeline:
    - Confirmed `/odom_raw` healthy at ~48 Hz ✓
    - Confirmed `imu_calib` running and publishing `/imu_corrected` ✓
    - Confirmed `imu_filter` subscribed to `/imu_corrected` ✓
    - Confirmed `/imu` has 0 Hz — `imu_filter_madgwick` receives input but publishes nothing
    - Confirmed EKF not publishing `odometry/filtered` or any TF — `odom` frame does not exist
  - Read all relevant vendor launch files from the Jetson to understand the full pipeline: `controller.launch.py`, `odom_publisher.launch.py`, `imu_filter.launch.py`, `lidar.launch.py`, `ekf.yaml`
  - Found HiWonder's own `slam.launch.py` at `~/ros2_ws/src/slam/launch/slam.launch.py` — uses `robot.launch.py` as hardware base and delays SLAM start by 5 seconds
  - Discovered `controller.launch.py` exposes `enable_odom` launch argument that conditionally starts the vendor EKF (`IfCondition(enable_odom)`)
  - **Attempted** `SetRemap(src='/imu', dst='/imu_disabled')` inside `GroupAction` — did not work because `controller.launch.py` uses `OpaqueFunction`, and remaps do not propagate into `OpaqueFunction` launch contexts
  - **Solution applied**: pass `enable_odom=false` to suppress the vendor EKF entirely; start our own `ekf_filter_node` directly in `mapping.launch.py` with a wheel-odometry-only config
  - Created `config/ekf_odom_only.yaml`: EKF fuses `/odom_raw` only (vx, vy, vyaw), no IMU input
  - Rewrote `launch/mapping.launch.py`: 4 explicit components — controller (hardware only), our EKF, lidar, slam_toolbox

- **Results**
  - Config and launch file ready; **not yet tested on Jetson** — session ended before deployment
  - `/odom_raw` confirmed: `frame_id: odom`, `child_frame_id: base_footprint` — matches `ekf_odom_only.yaml` exactly
  - Root cause of the `/imu` silence: `imu_filter_madgwick` appears to stall despite `use_mag: false` being set in the vendor config — exact cause not pinpointed (possible QoS mismatch on output side or gravity-alignment stall)

- **Evidence**
  - `config/ekf_odom_only.yaml` — new file
  - `launch/mapping.launch.py` — rewritten

- **Blockers / Issues**
  - IMU chain root cause not fully explained: `use_mag: false` is already set in vendor `imu_filter.launch.py`, ruling out magnetometer stall as the cause. Deferred to Milestone 8.

- **Next**
  - Pull changes to Jetson and run: `ros2 launch launch/mapping.launch.py`
  - Verify `odom → base_footprint` TF now publishes: `ros2 run tf2_ros tf2_echo odom base_footprint`
  - If EKF is publishing: verify `map` frame appears in TF tree, add Map + LaserScan displays in RViz2, drive the robot, save the map

---

## 2026-05-15 (afternoon) — Milestone 5: mapping stack fully working

- **Goal**
  - Deploy the EKF + odom-relay config from the morning session, verify `odom → base_footprint` TF, and get slam_toolbox building a map.

- **Context**
  - Robot: HiWonder JetRover (Jetson Orin Nano), IP: 192.168.2.138
  - Host: WSL2 Ubuntu 22.04, IP: 192.168.2.102
  - SSH access to Jetson now available: `ssh jetrover`
  - Mapping launched via `tmux new-session -d -s mapping 'zsh /tmp/run_mapping.sh'`

- **Work done**
  - Deployed morning changes to Jetson; repeatedly launched and diagnosed why no `odom` frame appeared.
  - Discovered that **Python `odom_relay` receives 0 messages from odom_publisher** despite DDS showing them as "matched" — same for C++ EKF (0 measurements in debug log).
  - Used `strace -e trace=sendto` on `odom_publisher` (pid 30193): confirmed it sends **only DDS metatraffic (ports 7410–7416)** and **zero user-data packets** to any subscriber port — the Python `publish()` call silently does nothing at the network layer.
  - Root cause identified: **`avoid_builtin_multicast: true` in `~/fastdds.xml` breaks Python rclpy publisher data delivery on FastDDS 2.6.x.** With this flag set, FastDDS picks the LAN interface (`192.168.2.138`) as the data transport locator for all participants; Jetson-to-Jetson data never actually transmits. C++ nodes (robot_state_publisher, sllidar) are unaffected.
  - **Fix 1** — updated `mapping.launch.py` to use the C++ `robot_localization` `ekf_node` instead of the Python `odom_relay.py` (same job: subscribe to `/odom_raw`, publish `/odom` + `odom→base_footprint` TF; C++ DDS unaffected by the bug).
  - **Fix 2** — removed `FASTRTPS_DEFAULT_PROFILES_FILE` export from `run_mapping.sh`; launched nodes now use default FastDDS multicast for local discovery, which works correctly. Cross-machine WSL discovery is handled by the `ros2 daemon` (started via `~/.zshrc` which still sources `~/fastdds.xml`).
  - Updated `~/fastdds.xml` on Jetson to remove `<avoid_builtin_multicast>true</avoid_builtin_multicast>` (kept `initialPeersList` for WSL contact).
  - Added `scripts/run_mapping.sh` to the repository (previously only existed as `/tmp/run_mapping.sh` on the Jetson).

- **Results**
  - `odom → base_footprint` TF: **live** ✓
  - `map → odom → base_footprint → base_link → lidar_frame` full TF chain: **complete** ✓
  - slam_toolbox: registered sensor, processing scans, `/map` topic publishing ✓
  - LiDAR mounted 180° — handled correctly by URDF, no configuration change needed.

- **Evidence**
  - `ros2 run tf2_ros tf2_echo map lidar_frame` → Translation [0.090, 0.000, 0.157], RPY [0, 0, 180°]
  - `ros2 topic echo /map --once` → `frame_id: map`, `resolution: 0.05`
  - slam_toolbox log: `Registering sensor: [Custom Described Lidar]`
  - strace: zero `sendto()` to subscriber ports while `FASTRTPS_DEFAULT_PROFILES_FILE` was set

- **Blockers / Issues**
  - Cross-machine topic visibility (WSL ↔ Jetson) not re-verified after removing `FASTRTPS_DEFAULT_PROFILES_FILE` from launch nodes. The daemon still has the config so `/map` and `/tf` should cross, but this needs confirmation.
  - LiDAR 180° mount: scan data will appear rotated in RViz2 — verify it matches the room geometry when driving.

- **Next**
  - Confirm `/map` and `/tf` are visible from WSL: `ros2 topic list` on WSL should show `/map`
  - Open RViz2 on WSL, add Map and LaserScan displays, confirm real-time map update
  - Drive the robot (publish `/cmd_vel`) to build a complete room map
  - Save the map: `ros2 run nav2_map_server map_saver_cli -f maps/room_map --ros-args -p map_subscribe_transient_local:=true`
  - Commit map output to `maps/`

---

## Template — copy/paste for new entries

## YYYY-MM-DD — <short title>

- **Goal**
  - ...

- **Context**
  - Robot / environment: ...
  - Software state: ROS distro, branch/commit, machine (Jetson/host): ...

- **Work done**
  - Commands run:
    - `...`
  - Config changes:
    - `...` (file + what changed)
  - Notes:
    - ...

- **Results**
  - Expected:
    - ...
  - Observed:
    - ...

- **Evidence**
  - Screenshots: `docs/images/...`
  - TF frames output: `...`
  - Bag files: `...`
  - Map outputs: `maps/...`

- **Blockers / Issues**
  - ...

- **Next**
  - ...

