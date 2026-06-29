---
trigger: always_on
---

# Workspace Rules — 2D LiDAR Mapping with SLAM

1. Treat this repository as a focused robotics learning project for building and validating a real-hardware 2D LiDAR SLAM mapping pipeline.
   - Keep the scope narrow and aligned with the repository documentation.
   - Avoid introducing unnecessary features, abstractions, dependencies, or architecture beyond the current project goals.

2. Before proposing or making non-trivial changes, inspect the relevant repository context first:
   - `README.md`
   - `docs/architecture.md`
   - `docs/LOGBOOK.md`
   - the relevant files under `launch/`, `config/`, and `scripts/`

3. Use the repository documentation as the project-specific source of truth.
   - If code, configuration, and documentation appear inconsistent, point that out explicitly.
   - Do not silently assume which one is correct.

4. Preserve the existing project structure and conventions unless there is a clear technical reason to change them.
   - Do not casually rename files, ROS topics, TF frames, parameters, launch entry points, or folders.
   - If a structural change is advisable, explain the reason before making it.

5. Be especially careful with robotics integration details:
   - ROS 2 launch behavior
   - topics, parameters, and node wiring
   - TF frame relationships
   - timestamps and frame IDs
   - LiDAR, odometry, SLAM, and RViz configuration
   - behavior differences between desktop development and execution on the JetRover / Jetson

6. Prefer incremental, verifiable development.
   - Make small, targeted changes rather than large broad rewrites.
   - When debugging, isolate one likely cause at a time.
   - Avoid changing multiple robotics subsystems simultaneously unless explicitly requested.

7. When modifying launch files, YAML configs, or runtime scripts:
   - Explain what runtime behavior the change affects.
   - State important assumptions.
   - Mention any parameters, topics, or frames that may need verification after the change.

8. When troubleshooting, reason from evidence first:
   - terminal output
   - ROS 2 logs
   - topic and node inspection
   - TF inspection
   - parameter values
   - current repository files
   Avoid speculative fixes that are not grounded in the available evidence.

9. When relevant, provide concrete validation steps after a change:
   - build or source commands
   - launch commands
   - ROS 2 inspection commands
   - TF/topic checks
   - RViz or map-output verification
   - robot-side test steps where applicable

10. Keep documentation synchronized with meaningful implementation changes.
   - Suggest or update documentation when setup, architecture, workflow, milestones, or project status changes.
   - Do not edit documentation for trivial internal changes that do not affect understanding or usage.

11. The robot's files live on the Jetson (ubuntu@192.168.2.138), not on the local machine.
   - Never attempt to read robot-side files (ROS 2 workspace, launch files, configs under `/home/ubuntu/ros2_ws/`) using local file tools — they do not exist locally.
   - When inspecting files on the Jetson, provide `ssh` or `cat` commands for Maher to run and paste back.

---

## Reference: ROS 2 Cross-Machine Network Setup

ROS 2 topics between the Jetson (192.168.2.138) and host PC (192.168.2.102) rely on the FastDDS unicast setup documented in `docs/cross_machine_dds.md`.

Key constraints to preserve:

- Both machines need `FASTRTPS_DEFAULT_PROFILES_FILE=~/fastdds.xml` when ROS 2 processes start.
- The FastDDS profile should explicitly list unicast locators for participant ports 7410-7608 on the peer machine.
- Do **not** set `<avoid_builtin_multicast>true</avoid_builtin_multicast>` on the Jetson profile; this setup previously caused ROS 2 discovery to appear healthy while local publisher data failed to reach subscribers.
- IPs are hardcoded. If either machine gets a new DHCP lease, update the peer address in both `fastdds.xml` files.
