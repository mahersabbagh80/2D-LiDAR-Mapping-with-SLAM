#ifndef TWO_D_LIDAR_MAPPING_WITH_SLAM__SCAN_DIAGNOSTICS_NODE_HPP_
#define TWO_D_LIDAR_MAPPING_WITH_SLAM__SCAN_DIAGNOSTICS_NODE_HPP_

#include <chrono>
#include <string>

#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

#include "two_d_lidar_mapping_with_slam/scan_analysis.hpp"

namespace two_d_lidar_mapping_with_slam
{

class ScanDiagnosticsNode : public rclcpp::Node
{
public:
  explicit ScanDiagnosticsNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

private:
  void receive_scan(sensor_msgs::msg::LaserScan::ConstSharedPtr scan);
  void publish_diagnostics();

  std::string scan_topic_;
  std::string expected_frame_;
  double scan_timeout_sec_;
  double min_valid_fraction_;
  bool received_scan_{false};
  std::string last_frame_;
  ScanAnalysis last_analysis_;
  std::chrono::steady_clock::time_point last_received_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
  rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace two_d_lidar_mapping_with_slam

#endif  // TWO_D_LIDAR_MAPPING_WITH_SLAM__SCAN_DIAGNOSTICS_NODE_HPP_
