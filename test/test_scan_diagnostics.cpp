#include <chrono>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include "two_d_lidar_mapping_with_slam/scan_diagnostics_node.hpp"

namespace
{
using Status = diagnostic_msgs::msg::DiagnosticStatus;
using Scan = sensor_msgs::msg::LaserScan;
using two_d_lidar_mapping_with_slam::ScanDiagnosticsNode;

class ScanDiagnosticsTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    rclcpp::NodeOptions options;
    options.parameter_overrides({
      {"scan_topic", "/test/scan"}, {"expected_frame", "lidar_frame"},
      {"scan_timeout_sec", 0.4}, {"publish_period_sec", 0.05},
      {"min_valid_fraction", 0.75}, {"use_sim_time", true}});
    options.arguments({"--ros-args", "-r", "/diagnostics:=/test/diagnostics"});
    monitor_ = std::make_shared<ScanDiagnosticsNode>(options);
    driver_ = std::make_shared<rclcpp::Node>("diagnostics_test_driver");
    subscription_ = driver_->create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
      "/test/diagnostics", 10,
      [this](diagnostic_msgs::msg::DiagnosticArray::ConstSharedPtr message) {
        if (!message->status.empty()) {
          latest_ = message->status.front();
        }
      });
    publisher_ = driver_->create_publisher<Scan>("/test/scan", rclcpp::SensorDataQoS());
    executor_.add_node(monitor_);
    executor_.add_node(driver_);
  }

  void TearDown() override
  {
    executor_.remove_node(monitor_);
    executor_.remove_node(driver_);
  }

  bool wait_for_status(uint8_t level, const std::string & text, const Scan * scan = nullptr)
  {
    latest_.reset();
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(4);
    while (std::chrono::steady_clock::now() < deadline) {
      if (scan) {
        publisher_->publish(*scan);
      }
      executor_.spin_some();
      if (latest_ && latest_->level == level && latest_->message == text) {
        return true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return false;
  }

  std::string value(const std::string & key) const
  {
    for (const auto & item : latest_->values) {
      if (item.key == key) {
        return item.value;
      }
    }
    return "";
  }

  rclcpp::executors::SingleThreadedExecutor executor_;
  std::shared_ptr<ScanDiagnosticsNode> monitor_;
  rclcpp::Node::SharedPtr driver_;
  rclcpp::Publisher<Scan>::SharedPtr publisher_;
  rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr subscription_;
  std::optional<Status> latest_;
};

TEST_F(ScanDiagnosticsTest, ReportsFaultsAndRecoversWithPausedRosTime)
{
  // No /clock is published. Timeout detection must still work using steady time.
  ASSERT_TRUE(wait_for_status(Status::STALE, "Waiting for scans"));
  Scan scan;
  scan.header.frame_id = "lidar_frame";
  scan.angle_min = -1.0F;
  scan.angle_increment = 1.0F;
  scan.angle_max = 1.0F;
  scan.range_min = 0.1F;
  scan.range_max = 10.0F;
  scan.ranges = {1.0F, 2.0F, 3.0F};
  ASSERT_TRUE(wait_for_status(Status::OK, "Scan stream healthy", &scan));
  EXPECT_EQ(value("valid_count"), "3");
  EXPECT_EQ(value("mean_range_m"), "2.000000");

  scan.ranges = {1.0F, std::numeric_limits<float>::infinity(), -1.0F};
  ASSERT_TRUE(wait_for_status(Status::WARN, "Too few finite in-range returns", &scan));
  EXPECT_EQ(value("no_return_count"), "1");
  EXPECT_EQ(value("invalid_count"), "1");

  scan.ranges.clear();
  ASSERT_TRUE(wait_for_status(Status::ERROR, "Empty ranges array", &scan));
  scan.ranges = {1.0F, 2.0F, 3.0F};
  scan.header.frame_id = "wrong_frame";
  ASSERT_TRUE(wait_for_status(Status::ERROR, "Unexpected scan frame", &scan));
  scan.header.frame_id = "lidar_frame";
  ASSERT_TRUE(wait_for_status(Status::OK, "Scan stream healthy", &scan));
  ASSERT_TRUE(wait_for_status(Status::STALE, "Scan stream timed out"));
  ASSERT_TRUE(wait_for_status(Status::OK, "Scan stream healthy", &scan));
}

TEST_F(ScanDiagnosticsTest, StartupConfigurationCannotBeSilentlyChanged)
{
  const auto result = monitor_->set_parameter(rclcpp::Parameter("scan_timeout_sec", 2.0));
  EXPECT_FALSE(result.successful);
  EXPECT_DOUBLE_EQ(monitor_->get_parameter("scan_timeout_sec").as_double(), 0.4);
}

TEST(ScanDiagnosticsConfiguration, RejectsInvalidParameters)
{
  const std::vector<rclcpp::Parameter> invalid = {
    {"scan_timeout_sec", 0.0}, {"scan_timeout_sec", -1.0},
    {"publish_period_sec", 0.0}, {"publish_period_sec", 0.0001},
    {"publish_period_sec", 3600.1},
    {"publish_period_sec", std::numeric_limits<double>::infinity()},
    {"min_valid_fraction", -0.1}, {"min_valid_fraction", 1.1},
    {"min_valid_fraction", std::numeric_limits<double>::quiet_NaN()},
    {"scan_topic", ""}};
  for (const auto & parameter : invalid) {
    rclcpp::NodeOptions options;
    options.parameter_overrides({parameter});
    EXPECT_THROW(std::make_shared<ScanDiagnosticsNode>(options), std::invalid_argument);
  }
}
}  // namespace

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  rclcpp::init(argc, argv);
  const int result = RUN_ALL_TESTS();
  rclcpp::shutdown();
  return result;
}
