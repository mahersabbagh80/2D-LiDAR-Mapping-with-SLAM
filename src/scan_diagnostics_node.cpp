#include "two_d_lidar_mapping_with_slam/scan_diagnostics_node.hpp"

#include <cmath>
#include <functional>
#include <stdexcept>
#include <utility>

#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <diagnostic_msgs/msg/key_value.hpp>

namespace two_d_lidar_mapping_with_slam
{
namespace
{
template<typename T>
T declare_read_only(rclcpp::Node & node, const std::string & name, const T & value)
{
  rcl_interfaces::msg::ParameterDescriptor descriptor;
  descriptor.read_only = true;
  return node.declare_parameter<T>(name, value, descriptor);
}

void add_value(
  diagnostic_msgs::msg::DiagnosticStatus & status,
  const std::string & key, const std::string & value)
{
  diagnostic_msgs::msg::KeyValue item;
  item.key = key;
  item.value = value;
  status.values.push_back(std::move(item));
}
}  // namespace

ScanDiagnosticsNode::ScanDiagnosticsNode(const rclcpp::NodeOptions & options)
: Node("scan_diagnostics", options)
{
  scan_topic_ = declare_read_only<std::string>(*this, "scan_topic", "/scan");
  expected_frame_ = declare_read_only<std::string>(*this, "expected_frame", "");
  scan_timeout_sec_ = declare_read_only<double>(*this, "scan_timeout_sec", 1.0);
  min_valid_fraction_ = declare_read_only<double>(*this, "min_valid_fraction", 0.1);
  const auto period = declare_read_only<double>(*this, "publish_period_sec", 1.0);
  if (scan_topic_.empty() || !std::isfinite(scan_timeout_sec_) || scan_timeout_sec_ <= 0.0 ||
    !std::isfinite(period) || period < 0.001 || period > 3600.0 ||
    !std::isfinite(min_valid_fraction_) ||
    min_valid_fraction_ < 0.0 || min_valid_fraction_ > 1.0)
  {
    throw std::invalid_argument(
            "scan_topic must be nonempty; scan_timeout_sec > 0; publish_period_sec in [0.001, 3600]; "
            "min_valid_fraction in [0, 1]; numeric parameters must be finite");
  }
  publisher_ = create_publisher<diagnostic_msgs::msg::DiagnosticArray>("/diagnostics", 10);
  subscription_ = create_subscription<sensor_msgs::msg::LaserScan>(
    scan_topic_, rclcpp::SensorDataQoS(),
    std::bind(&ScanDiagnosticsNode::receive_scan, this, std::placeholders::_1));
  timer_ = create_wall_timer(
    std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(period)),
    std::bind(&ScanDiagnosticsNode::publish_diagnostics, this));
  RCLCPP_INFO(get_logger(), "Monitoring %s; scans are observed without modification", scan_topic_.c_str());
}

void ScanDiagnosticsNode::receive_scan(sensor_msgs::msg::LaserScan::ConstSharedPtr scan)
{
  last_analysis_ = analyze_scan(*scan);
  last_frame_ = scan->header.frame_id;
  last_received_ = std::chrono::steady_clock::now();
  received_scan_ = true;
}

void ScanDiagnosticsNode::publish_diagnostics()
{
  using Status = diagnostic_msgs::msg::DiagnosticStatus;
  Status status;
  status.name = std::string(get_fully_qualified_name()) + ": scan";
  status.hardware_id = std::string("scan_topic:") + subscription_->get_topic_name();
  add_value(status, "scan_topic", subscription_->get_topic_name());
  if (!received_scan_) {
    status.level = Status::STALE;
    status.message = "Waiting for scans";
  } else {
    // Receipt age uses a monotonic clock: ROS time jumps or paused simulation time
    // must not hide an interrupted sensor stream. This is not header timestamp latency.
    const auto age = std::chrono::duration<double>(
      std::chrono::steady_clock::now() - last_received_).count();
    add_value(status, "scan_age_sec", std::to_string(age));
    add_value(status, "frame_id", last_frame_);
    add_value(status, "total_count", std::to_string(last_analysis_.total_count));
    if (age > scan_timeout_sec_) {
      status.level = Status::STALE;
      status.message = "Scan stream timed out";
    } else if (!last_analysis_.structure_valid) {
      status.level = Status::ERROR;
      status.message = last_analysis_.error;
    } else if (!expected_frame_.empty() && last_frame_ != expected_frame_) {
      status.level = Status::ERROR;
      status.message = "Unexpected scan frame";
    } else if (last_analysis_.valid_fraction < min_valid_fraction_) {
      status.level = Status::WARN;
      status.message = "Too few finite in-range returns";
    } else {
      status.level = Status::OK;
      status.message = "Scan stream healthy";
    }
    if (last_analysis_.structure_valid) {
      add_value(status, "valid_count", std::to_string(last_analysis_.valid_count));
      add_value(status, "no_return_count", std::to_string(last_analysis_.no_return_count));
      add_value(status, "invalid_count", std::to_string(last_analysis_.invalid_count));
      add_value(status, "valid_fraction", std::to_string(last_analysis_.valid_fraction));
      if (last_analysis_.valid_count > 0) {
        add_value(status, "minimum_range_m", std::to_string(last_analysis_.minimum_range));
        add_value(status, "maximum_range_m", std::to_string(last_analysis_.maximum_range));
        add_value(status, "mean_range_m", std::to_string(last_analysis_.mean_range));
      }
    }
  }
  diagnostic_msgs::msg::DiagnosticArray message;
  message.header.stamp = now();
  message.status.push_back(std::move(status));
  publisher_->publish(message);
}

}  // namespace two_d_lidar_mapping_with_slam
