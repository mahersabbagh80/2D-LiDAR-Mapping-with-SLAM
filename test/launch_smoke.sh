#!/usr/bin/env bash
# Run against the installed package with the ROS workspace already sourced.
set -eo pipefail
work_dir=$(mktemp -d)
launch_pid=""

stop_launch() {
  if [ -n "$launch_pid" ]; then
    # Noninteractive shells may make background jobs inherit ignored SIGINT.
    # Give each launch its own process group and stop only that group's children.
    kill -TERM -- "-$launch_pid" 2>/dev/null || true
    for attempt in {1..30}; do
      if ! kill -0 "$launch_pid" 2>/dev/null; then break; fi
      sleep 0.1
    done
    kill -KILL -- "-$launch_pid" 2>/dev/null || true
    wait "$launch_pid" || true
    launch_pid=""
  fi
}

cleanup() {
  result=$?
  stop_launch
  if [ "$result" -ne 0 ]; then
    cat "$work_dir"/*.log >&2 2>/dev/null || true
  fi
  rm -rf "$work_dir"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

# A representative message validates that conditional vendor/SLAM branches
# really are skipped and the installed YAML and C++ executable are usable.
setsid ros2 launch two_d_lidar_mapping_with_slam mapping.launch.xml \
  start_robot:=false start_slam:=false > "$work_dir/mapping.log" 2>&1 &
launch_pid=$!
timeout --signal=INT --kill-after=3s 10s ros2 topic echo /diagnostics \
  diagnostic_msgs/msg/DiagnosticArray --once > "$work_dir/waiting.log"
kill -0 "$launch_pid"
grep -q 'message: Waiting for scans' "$work_dir/waiting.log"
stop_launch
echo 'PASS: installed mapping XML launches diagnostics without hardware or SLAM'

# Check configuration precedence through the actual installed launch and node.
cat > "$work_dir/custom.yaml" <<'YAML'
scan_diagnostics:
  ros__parameters:
    scan_topic: /regression/yaml_scan
    publish_period_sec: 0.1
YAML
for mode in yaml override; do
  extra_args=()
  expected_topic=/regression/yaml_scan
  if [ "$mode" = override ]; then
    extra_args+=(scan_topic:=/regression/override_scan)
    expected_topic=/regression/override_scan
  fi
  setsid ros2 launch two_d_lidar_mapping_with_slam scan_diagnostics.launch.xml \
    params_file:="$work_dir/custom.yaml" "${extra_args[@]}" > "$work_dir/$mode.log" 2>&1 &
  launch_pid=$!
  timeout --signal=INT --kill-after=3s 10s ros2 topic echo /diagnostics \
    diagnostic_msgs/msg/DiagnosticArray --once > "$work_dir/$mode-message.log"
  kill -0 "$launch_pid"
  grep -q "hardware_id: scan_topic:$expected_topic" "$work_dir/$mode-message.log"
  stop_launch
  echo "PASS: $mode scan topic configuration"
done

setsid ros2 launch two_d_lidar_mapping_with_slam diagnostics_demo.launch.xml \
  > "$work_dir/demo.log" 2>&1 &
launch_pid=$!
if timeout --signal=INT --kill-after=3s 28s ros2 topic echo /demo/diagnostics \
  diagnostic_msgs/msg/DiagnosticArray > "$work_dir/diagnostics.log"; then
  echo 'Diagnostic observer exited before its observation window' >&2
  exit 1
else
  observer_status=$?
  test "$observer_status" -eq 124
fi
kill -0 "$launch_pid"
awk '
  BEGIN {
    expected[0] = "Scan stream healthy"
    expected[1] = "Too few finite in-range returns"
    expected[2] = "Empty ranges array"
    expected[3] = "Scan stream timed out"
    expected[4] = "Scan stream healthy"
    step = 0
  }
  /message: / {
    sub(/^ *message: /, "")
    if ($0 == expected[step]) step++
  }
  END {exit(step == 5 ? 0 : 1)}
' "$work_dir/diagnostics.log"
echo 'PASS: C++ demo produces ordered OK -> WARN -> ERROR -> STALE -> OK diagnostics'
