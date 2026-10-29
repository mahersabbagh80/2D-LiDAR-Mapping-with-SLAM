#include <cstdio>
#include <exception>
#include <memory>

#include <rclcpp/rclcpp.hpp>

#include "two_d_lidar_mapping_with_slam/scan_diagnostics_node.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  int result = 0;
  try {
    rclcpp::spin(std::make_shared<two_d_lidar_mapping_with_slam::ScanDiagnosticsNode>());
  } catch (const std::exception & error) {
    std::fprintf(stderr, "scan_diagnostics_node: %s\n", error.what());
    result = 1;
  }
  rclcpp::shutdown();
  return result;
}
