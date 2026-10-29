# C++ migration validation

Validation date: 2026-10-29. Target: ROS 2 Humble on Ubuntu 22.04, x86-64, C++17, `RelWithDebInfo`.

The cloud host runs Debian; builds and runtime checks use the official `ros:humble-ros-base-jammy` container pinned to digest `sha256:d9bcf2e6f72df127fdee07048c2e87b1b4b9a59a74af0592f4489fc2844aa855`. The source checkout is mounted read-only. Build, install, log, and test outputs are stored outside the checkout. Test traffic uses localhost-only DDS on an isolated ROS domain.

## Verified checks

- CMake/ament build of both C++ executables, both libraries, and test executables.
- Installed `scan_diagnostics_node` and `scan_demo_node` discovered through `ros2 pkg executables`.
- Standalone and demo XML launch descriptions parsed through `ros2 launch --show-args`.
- Eleven scan-analysis tests: numerical statistics, inclusive range bounds, no-return/invalid separation, all-infinite input, empty arrays, frame presence, invalid range/angular metadata, angular count consistency, clockwise scans, float rounding, and optional intensity length.
- Three ROS/configuration tests: message-based health/fault/recovery transitions with paused ROS time, immutable startup parameters, and rejection of invalid configuration values.
- Automated shell smoke test: the installed mapping XML entry point produces a real waiting-for-scans diagnostic with hardware and SLAM disabled; the separate-process synthetic demo then produces healthy, degraded, malformed, timed-out, and recovered diagnostics in order.

The final suite passed: **14 C++ GoogleTest cases plus one launch/demo smoke check**, across three CTest entries, with no failures or skipped cases. `colcon test-result` reported **18 tests, 0 errors, 0 failures, 0 skipped** because its aggregate includes both CTest wrapper results and the underlying test cases. The smoke check took approximately 30 seconds, including its 28-second observation window; this duration is not a performance benchmark.

Result artifacts are generated under `build/two_d_lidar_mapping_with_slam/test_results/two_d_lidar_mapping_with_slam/`: `test_scan_analysis.gtest.xml`, `test_scan_diagnostics.gtest.xml`, and `test_launch_smoke.xml`. For the cloud validation, the build root is `/workspace/cloud-setup/lidar-cpp/build` on the host, mounted at `/cloud/build` in the container.

## Reproduce

In a fresh Humble workspace, after sourcing `/opt/ros/humble/setup.bash`:

```bash
colcon build --packages-select two_d_lidar_mapping_with_slam \
  --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON
source install/setup.bash
colcon test --packages-select two_d_lidar_mapping_with_slam \
  --return-code-on-test-failure --event-handlers console_direct+
colcon test-result --verbose
```

The shell smoke check launches the installed `mapping.launch.xml` with hardware and SLAM disabled and requires a real diagnostic message. It then launches the installed demo, observes its diagnostic topic for 28 seconds, and asserts the ordered fault/recovery sequence. Each launch owns a separate process group, which the test shuts down on success or failure.

The GitHub Actions workflow in `.github/workflows/ros2-ci.yml` uses the same pinned Humble image and runs the same build/tests. A hosted CI run is not claimed until the branch is pushed and GitHub executes it.

## Not validated here

- Full mapping launch on Jetson ARM64 with HiWonder `controller` and `peripherals` installed.
- Live RPLidar A1 data, vendor launch readiness, joystick/arm behavior, TF completeness, or DDS communication with the physical robot.
- SLAM scan-to-map behavior, RViz rendering, map saving, and pose-graph serialization for the migrated version.
- Any mapping-accuracy, processing-latency, CPU-use, or memory-use improvement.

The existing apartment map and screenshots are historical results of the original hardware integration. The cloud's earlier SLAM package installation was blocked by network policy; no unavailable vendor package has been replaced with a stub to claim a passing full-stack test.

## Jetson acceptance checklist

Use the real robot's vendor workspaces and a fresh C++ install prefix as described in [the migration guide](cpp_portfolio.md#migrating-the-existing-workspace). Confirm these items before treating the migrated hardware stack as deployed:

1. Build and run the C++ tests on ARM64. Check that `ros2 pkg prefix` selects the intended C++ overlay.
2. Start `mapping.launch.xml`; verify the same controller, arm initialization, joystick, LiDAR, and SLAM components as before, plus the diagnostics node.
3. Inspect `/scan`, `/odom`, `/tf`, and `/diagnostics`; set `expected_frame` only after checking the actual scan frame. Tune the timeout/finite-return threshold using measured scan behavior.
4. Verify the TF chain resolves and `/map` updates in RViz while driving under the existing operating procedure.
5. Save a new map and pose graph to a new output basename, preserving the original files. Record the commands and results in the logbook.

Pre-existing issue: `maps/apartment.yaml` names `apartment.pgm`, while the tracked PGM is spelled `apratment.pgm`. This migration does not alter those historical map assets. Resolve that mismatch before using them in a map-loading demonstration.
