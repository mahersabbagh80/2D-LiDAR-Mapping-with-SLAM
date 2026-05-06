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

## 2026-05-07 — Milestone 1: Hardware Bringup

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

