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

## 2026-05-22 — Milestone 5: repo restructure, RViz2 setup, DDS regression debug

- **Goal**
  - Complete Milestone 5 mapping session: get RViz2 showing a live map, drive the robot, save the map.

- **Context**
  - Robot: HiWonder JetRover (Jetson Orin Nano), IP: 192.168.2.138
  - Host: WSL2 Ubuntu 22.04, IP: 192.168.2.102
  - Repo was restructured into a standard ROS 2 colcon package (`two_d_lidar_mapping_with_slam`) since the last session

- **Work done**
  - Created `config/rviz/mapping.rviz` — RViz2 config with Map (`/map`, transient-local), LaserScan (`/scan`), and TF displays. Fixed frame: `map`. No need to manually add displays each session.
  - Updated `setup.py` to install `config/rviz/*.rviz` via colcon.
  - Fixed `scripts/run_mapping.sh` — was missing `source ~/my_projects/mapping_ws/install/local_setup.zsh`. Without this line the `two_d_lidar_mapping_with_slam` package is not found and the launch silently fails.
  - Fixed `~/.zshrc` (WSL) — daemon restart logic changed from "start if not running" to "restart if running without FastDDS profile". Prevents stale daemon after PC reboot from breaking cross-machine DDS. The new logic checks `/proc/<pid>/environ` for the env var before deciding whether to restart.
  - Diagnosed why EKF receives 0 measurements despite `/odom_raw` publishing at 46 Hz:
    - `ros2 topic info /odom_raw --verbose` → publisher QoS: RELIABLE; EKF subscriber QoS: BEST_EFFORT
    - Initially suspected QoS incompatibility — but per DDS spec, RELIABLE publisher + BEST_EFFORT subscriber IS compatible. True root cause not yet identified.
    - EKF debug log (`/tmp/ekf_debug.txt`): "0 measurements in queue. Filter not yet initialized." — confirms EKF sees no data.
    - Consequence: no `odom → base_footprint` TF → slam_toolbox drops all scans ("Message Filter: lidar_frame does not exist") → no `map` frame → RViz2 shows "Frame [map] does not exist".

- **Results**
  - Cross-machine DDS: fully working ✓ (all Jetson topics visible from WSL after `.zshrc` fix)
  - Mapping stack starts cleanly via `run_mapping.sh` ✓
  - RViz2 opens with pre-configured displays ✓
  - **Blocker:** EKF not receiving `/odom_raw` → TF chain broken → no map

- **Evidence**
  - `ros2 topic hz /odom_raw` from WSL: ~46 Hz ✓
  - `ros2 run tf2_ros tf2_echo odom base_footprint` on both WSL and Jetson: "frame does not exist"
  - EKF debug: `/tmp/ekf_debug.txt` → "0 measurements in queue"
  - RViz2 screenshot: Global Status Error — "Frame [map] does not exist"

- **Blockers / Issues**
  - EKF (`robot_localization`) not receiving `/odom_raw` despite matching topic names and compatible QoS. Next step: check if there is a RMW mismatch between `odom_publisher` and `ekf_filter_node` (Jetson environment has `CYCLONEDDS_URI` set which may affect RMW selection), or check if a namespace/remapping issue is causing the subscription to point to the wrong topic.

- **Next**
  - SSH to Jetson, check `$RMW_IMPLEMENTATION` inside the running `ekf_node` process
  - Run `ros2 topic echo /odom_raw` on the Jetson (with daemon restarted) to confirm data is visible locally
  - If RMW mismatch: set `RMW_IMPLEMENTATION=rmw_fastrtps_cpp` explicitly in `run_mapping.sh`
  - If topic visible but EKF still empty: try replacing EKF with `odom_relay.py` and test if Python node has same issue (will confirm if problem is EKF-specific or DDS-local)
  - Once TF chain is fixed: open RViz2, drive robot, save map

---

## 2026-05-25 — Migrated from ROS 2 Humble to ROS 2 Jazzy

- **Goal**
  - Update the entire project to target ROS 2 Jazzy (Ubuntu 24.04) after uninstalling ROS 2 Humble.

- **Work done**
  - `scripts/run_mapping.sh`: updated `source /opt/ros/humble/local_setup.zsh` → `/opt/ros/jazzy/local_setup.zsh`
  - `README.md`: updated Software table (Ubuntu 24.04, ROS 2 Jazzy), all `ros-humble-*` apt commands → `ros-jazzy-*`, sourcing example, and docs link → jazzy
  - `docs/ROADMAP.md`: updated milestone checklist text and RViz2 launch command to reference Jazzy
  - `config/slam_toolbox_params.yaml`: updated comment header `# ROS 2 Humble` → `# ROS 2 Jazzy`

- **Results**
  - All project files consistently reference ROS 2 Jazzy.
  - No changes needed to `package.xml`, `setup.py`, `setup.cfg`, or `launch/mapping.launch.py` — they are distro-agnostic.

- **Blockers / Issues**
  - Verify that `slam_toolbox` and `robot_localization` Jazzy packages are available: `apt-cache show ros-jazzy-slam-toolbox`
  - Verify HiWonder vendor packages (`controller`, `peripherals`) have been rebuilt or are available for Jazzy on the Jetson.

- **Next**
  - Rebuild the colcon workspace: `colcon build --symlink-install`
  - Run `source /opt/ros/jazzy/setup.zsh` and confirm `ros2` CLI works
  - Re-test the full mapping stack on the Jetson with Jazzy

---

## 2026-05-26 — Reverted host to Ubuntu 22.04 / ROS 2 Humble

- **Goal**
  - Revert the host machine back to Ubuntu 22.04 and ROS 2 Humble after the short-lived Jazzy migration.

- **Context**
  - Robot: HiWonder JetRover (Jetson Orin Nano), ROS 2 Humble (unchanged)
  - Host: reverted from Ubuntu 24.04 / ROS 2 Jazzy → Ubuntu 22.04 / ROS 2 Humble
  - Both machines now on the same Ubuntu and ROS 2 distro

- **Work done**
  - Reverted host OS to Ubuntu 22.04 LTS.
  - Updated `README.md`: Software table host entry updated (24.04 / Jazzy → 22.04 / Humble); host dependency apt commands updated (`ros-jazzy-*` → `ros-humble-*`).
  - Updated `docs/ROADMAP.md`: RViz2 host launch command updated (`/opt/ros/jazzy` → `/opt/ros/humble`).

- **Results**
  - All project files consistently reference ROS 2 Humble across both Jetson and host.
  - No changes needed to `package.xml`, `setup.py`, `launch/mapping.launch.py`, or `config/` — they are distro-agnostic.

- **Blockers / Issues**
  - None.

- **Next**
  - Resume debugging the EKF `/odom_raw` subscription issue (carried over from 2026-05-22 session).

---

## 2026-05-28 — DDS fixed on native Ubuntu; TF chain broken; Layer 1 rewrite planned

- **Goal**
  - Get a map appearing in RViz2 on the host PC with the mapping stack running on the Jetson.

- **Context**
  - Host machine changed: now native Ubuntu 22.04 (no longer WSL2), IP: 192.168.2.102
  - Jetson: Ubuntu 22.04, ROS 2 Humble, IP: 192.168.2.138
  - Starting fresh: vendor auto-start disabled, new clean workspace `~/jetson_ws` created on Jetson

- **Work done**
  - Disabled vendor auto-start service permanently: `sudo systemctl stop start_app_node.service && sudo systemctl disable start_app_node.service`
  - Created `~/jetson_ws/src/` on Jetson; cloned `2D-LiDAR-Mapping-with-SLAM` and `jetrover_description` repos; built with `colcon build --symlink-install`
  - Fixed cross-machine DDS (router blocks multicast between WiFi clients):
    - Created `~/fastdds.xml` on both machines with **100 explicit unicast port locators** (ports 7410–7608, covering participant IDs 0–99) pointing at the other machine
    - Set `export FASTRTPS_DEFAULT_PROFILES_FILE=~/fastdds.xml` permanently in Jetson `~/.bashrc` and host `~/.zshrc`
    - Documented root causes and solution in `docs/cross_machine_dds.md`
  - Fixed zsh/bash incompatibility: `source install/setup.bash` hangs in zsh because colcon uses `BASH_SOURCE`. Fix: always use bash terminals when sourcing Jetson workspaces.
  - Launched `ros2 launch two_d_lidar_mapping_with_slam mapping.launch.py` — LiDAR health OK, SLAM toolbox started, but map never appeared in RViz2.
  - Diagnosed "Frame [map] does not exist" and "No map received" in RViz2:
    - `ros2 run tf2_tools view_frames` on PC → `base_footprint` is root, **`odom` frame absent**
    - `ros2 topic hz /odom_raw` → healthy at 48 Hz ✓
    - `ros2 topic hz /imu` → **0 Hz — no publisher**
    - `ros2 topic hz /imu/data_raw` → **not published** — `imu_calib` hardware driver silent
    - Root cause: vendor EKF requires both `/odom_raw` and `/imu`; with `/imu` missing it never initializes and never publishes `odom → base_footprint` TF → slam_toolbox has no odometry → no `map` frame
  - Investigated vendor source code (local copy at `~/Downloads/JetRover vendor data/.../ROS2/src`):
    - `controller/launch/controller.launch.py`: `namespace/` in `ekf.yaml` is a template placeholder replaced at launch time via `nav2_common.launch.ReplaceString` — not a bug, by design
    - `peripherals/launch/lidar.launch.py`: launches `sllidar_node` + `scan_to_scan_filter_chain`
    - `peripherals/launch/include/sllidar_a1.launch.py`: sllidar_node params — `/dev/lidar`, 115200 baud, `Sensitivity` scan mode
    - `peripherals/config/lidar_filters_config_a1.yaml`: two filters — angular bounds (±1.6 rad) + range filter (0.2–12 m)
    - `controller/launch/odom_publisher.launch.py`: launches `ros_robot_controller` (Layer 2) + `odom_publisher` (Layer 2)
    - `controller/config/ekf.yaml`: confirmed identical to Jetson version — not modified by user
  - **Decision**: rewrite Layer 1 (our own launch file + configs) using vendor code as reference only, keeping only the hardware drivers (`ros_robot_controller`, `odom_publisher`) as black-box vendor dependencies. IMU deferred to later as stretch goal.

- **Results**
  - DDS cross-machine communication: **fully working** ✓ (all Jetson topics visible on PC)
  - Clean workspace builds and launches correctly ✓
  - `/odom_raw` healthy ✓, LiDAR healthy ✓
  - **Blocker**: `odom → base_footprint` TF not published → no map in RViz2

- **Evidence**
  - `docs/cross_machine_dds.md` — full DDS solution documentation
  - `~/fastdds.xml` deployed on both machines
  - TF tree PDF: `base_footprint` is root, no `odom` parent

- **Blockers / Issues**
  - IMU hardware driver (`imu_calib`) not publishing `/imu/data_raw` — root cause unknown (hardware or driver issue). Deferred to stretch goal.
  - Vendor EKF requires IMU to initialize — cannot be used without IMU.

- **Next**
  - Create `config/lidar_filters.yaml` in our package (from vendor A1 config)
  - Create `config/ekf.yaml` in our package (odometry only, no IMU, correct frame names)
  - Rewrite `launch/mapping.launch.py` to launch all nodes directly without vendor launch includes
  - Build, deploy to Jetson, verify `odom → base_footprint` TF, verify map appears in RViz2

---

## 2026-05-29 — Layer 1 rewrite: config files, launch file, and documentation

- **Goal**
  - Write the three files needed to fix the broken TF chain: `config/lidar_filters.yaml`, `config/ekf.yaml`, and a rewritten `launch/mapping.launch.py` that launches all nodes directly without vendor launch includes.

- **Context**
  - Continuing from 2026-05-28 decision to rewrite Layer 1
  - All work done on the host (PC); build and test deferred to next session

- **Work done**
  - Created `config/lidar_filters.yaml` — angular bounds filter (±1.6 rad) and range filter (0.2–12 m), values taken from vendor's `lidar_filters_config_a1.yaml` unchanged
  - Created `config/ekf.yaml` — odometry-only EKF fusing `/odom_raw` (x, y, yaw), `publish_tf: true` to broadcast `odom → base_footprint` TF, no IMU input
  - Rewrote `launch/mapping.launch.py` — directly launches all 8 nodes with no vendor `IncludeLaunchDescription`; replaces `controller.launch.py` with `ros_robot_controller` + `odom_publisher` + our `ekf_filter_node`; replaces `peripherals/lidar.launch.py` with `sllidar_node` + `scan_to_scan_filter_chain`
  - Updated `package.xml` — added `robot_localization`, `laser_filters`, `sllidar_ros2`, `ros_robot_controller`; removed `peripherals` (no longer a dependency)
  - Updated `README.md` — updated dependencies section (rosdep handles apt installs), added packages table with type and source links, removed stale vendor package list
  - Added `README.md` to `jetrover_description` repo — documents URDF variants, how other packages consume the xacro file, standalone viewing, and dependencies

- **Results**
  - All three Layer 1 files written and committed
  - `package.xml` is now the single source of truth for dependencies
  - Both repos committed: `jetrover_description` (README) and `2D-LiDAR-Mapping-with-SLAM` (7 files changed)
  - **Not yet tested** — build and deployment to Jetson deferred to next session

- **Evidence**
  - `config/lidar_filters.yaml` — new file
  - `config/ekf.yaml` — new file
  - `launch/mapping.launch.py` — rewritten (63% change)
  - `package.xml` — updated
  - `README.md` — updated

- **Blockers / Issues**
  - None at this stage — all files written, ready to build.

- **Next**
  - SSH to Jetson, pull both repos
  - Run `rosdep install --from-paths src --ignore-src -r -y` to install `robot_localization` and `laser_filters`
  - Build: `colcon build --symlink-install`
  - Launch: `ros2 launch two_d_lidar_mapping_with_slam mapping.launch.py`
  - Verify `odom → base_footprint` TF: `ros2 run tf2_ros tf2_echo odom base_footprint`
  - Verify map appears in RViz2

---

## 2026-06-01 — Milestone 5 complete: full apartment mapped and saved

- **Goal**
  - Fix robot model rendering in RViz2, resolve arm flickering, integrate joystick control, complete a full mapping session, and close out project documentation.

- **Context**
  - Robot: HiWonder JetRover (Jetson Orin Nano), IP: 192.168.2.138
  - Host: native Ubuntu 22.04, ROS 2 Humble
  - Stack was previously reverted to vendor launch includes (`controller.launch.py`, `peripherals`) after the Layer 1 rewrite approach proved unnecessary once the vendor EKF was confirmed working

- **Work done**
  - **RViz2 robot model**: discovered model was not rendering because RViz2 was launched bare (`rviz2`) with no config. Fixed by loading `config/rviz/mapping.rviz` via File → Open Config.
  - **Arm flickering fix**: diagnosed two `joint_state_publisher` nodes and two `robot_state_publisher` nodes running simultaneously (confirmed via `ros2 topic info /joint_states --verbose` → Publisher count: 2). Root cause: `mapping.launch.py` was launching its own `robot_state_publisher` and `joint_state_publisher`, conflicting with the vendor controller stack which already owns those nodes. Removed both from `mapping.launch.py`.
  - **Arm init pose**: added `init_pose.launch.py` (from `controller` package) to `mapping.launch.py`. Node waits for `/controller_manager/init_finish` service and sends resting servo positions from `controller/config/init_pose.yaml` (id2: 750, id3: 0, id4: 375). Arm now moves to resting position automatically on startup.
  - **Joystick control**: added `joystick_control.launch.py` (from `peripherals` package) to `mapping.launch.py`. Reads gamepad input from `ros_robot_controller/joy`, publishes to `controller/cmd_vel`.
  - **Full mapping session**: drove the robot around the full apartment using the gamepad. Complete map built in a single session.
  - **Map saved**:
    - `maps/apartment.pgm` + `maps/apartment.yaml` via `nav2_map_server map_saver_cli`
    - `maps/apartment.posegraph` via `ros2 service call /slam_toolbox/serialize_map` (enables resuming mapping in future sessions)
  - **slam_toolbox params**: added `map_file_name` and `map_start_at_dock: true` so the pose graph is loaded on restart
  - **package.xml**: removed stale `robot_state_publisher`, `joint_state_publisher`, `jetrover_description` dependencies
  - **Documentation overhaul**:
    - `README.md`: added Results section with 3 RViz2 screenshots and TF tree; fixed stale dependencies, getting-started commands, and packages table
    - `docs/architecture.md`: updated pipeline to reflect vendor controller owning robot description; updated teleoperation section (keyboard → gamepad/joystick_control); corrected TF tree frame names from live `.gv` output; removed `joint_state_publisher` references
    - `docs/images/pipeline.dot` + `pipeline.png`: renamed "Teleop" → "Joystick Controller"; routed through `peripherals`; updated topic label to `controller/cmd_vel`; regenerated PNG
    - `docs/ROADMAP.md`: marked all Milestone 5 items complete; removed Stretch Goal 1
    - Added `docs/images/rviz_robot_closeup.png`, `rviz_map_building.png`, `rviz_map_complete.png`, `tf_tree.png`, `tf_tree.pdf`

- **Results**
  - Robot model renders correctly in RViz2 with no flickering ✓
  - Arm moves to resting position on startup ✓
  - Gamepad driving works from launch ✓
  - Full apartment map built and saved ✓
  - All five milestones complete ✓

- **Evidence**
  - `maps/apartment.pgm`, `maps/apartment.yaml`, `maps/apartment.posegraph`
  - `docs/images/rviz_map_complete.png` — completed apartment map
  - `docs/images/tf_tree.png` — live TF tree confirming `map → odom → base_footprint → base_link → lidar_frame` chain

- **Blockers / Issues**
  - None.

- **Next**
  - Future project: autonomous navigation using the saved map (Nav2 stack).

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

