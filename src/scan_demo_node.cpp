#include <chrono>
#include <cmath>
#include <cstdio>
#include <exception>
#include <limits>
#include <memory>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

namespace
{
// Synthetic diagnostic demo, not a robot simulator or mapping dataset.
class ScanDemoNode : public rclcpp::Node
{
public:
  ScanDemoNode()
  : Node("scan_demo"), started_(std::chrono::steady_clock::now())
  {
    publisher_ = create_publisher<sensor_msgs::msg::LaserScan>("/demo/scan", rclcpp::SensorDataQoS());
    timer_ = create_wall_timer(std::chrono::milliseconds(100), [this]() {publish_scan();});
    RCLCPP_INFO(get_logger(), "Synthetic scans only: each phase lasts 5 seconds");
  }

private:
  void publish_scan()
  {
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::steady_clock::now() - started_).count();
    const int phase = static_cast<int>((elapsed / 5) % 5);
    if (phase != last_phase_) {
      const char * names[] = {"healthy", "degraded", "malformed", "no scans", "recovered"};
      RCLCPP_INFO(get_logger(), "Demo phase: %s", names[phase]);
      last_phase_ = phase;
    }
    if (phase == 3) {
      return;
    }
    sensor_msgs::msg::LaserScan scan;
    scan.header.stamp = now();
    scan.header.frame_id = "demo_lidar";
    scan.angle_min = -1.5707963F;
    scan.angle_increment = 0.017453293F;
    scan.angle_max = scan.angle_min + 180 * scan.angle_increment;
    scan.range_min = 0.1F;
    scan.range_max = 12.0F;
    scan.scan_time = 0.1F;
    scan.ranges.assign(181, 2.0F);
    if (phase == 1) {
      // No returns can be normal in open space; the threshold is a warning,
      // not proof of sensor failure. Keep distinct from NaN/out-of-range samples.
      for (std::size_t i = 0; i < 170; ++i) {
        scan.ranges[i] = std::numeric_limits<float>::infinity();
      }
    } else if (phase == 2) {
      scan.ranges.clear();
    }
    publisher_->publish(scan);
  }

  std::chrono::steady_clock::time_point started_;
  int last_phase_{-1};
  rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};
}  // namespace

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  int result = 0;
  try {
    rclcpp::spin(std::make_shared<ScanDemoNode>());
  } catch (const std::exception & error) {
    std::fprintf(stderr, "scan_demo_node: %s\n", error.what());
    result = 1;
  }
  rclcpp::shutdown();
  return result;
}
