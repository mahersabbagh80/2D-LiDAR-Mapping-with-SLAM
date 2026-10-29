#ifndef TWO_D_LIDAR_MAPPING_WITH_SLAM__SCAN_ANALYSIS_HPP_
#define TWO_D_LIDAR_MAPPING_WITH_SLAM__SCAN_ANALYSIS_HPP_

#include <cstddef>
#include <string>

#include <sensor_msgs/msg/laser_scan.hpp>

namespace two_d_lidar_mapping_with_slam
{

struct ScanAnalysis
{
  bool structure_valid{false};
  std::string error;
  std::size_t total_count{0};
  std::size_t valid_count{0};
  std::size_t no_return_count{0};
  std::size_t invalid_count{0};
  double valid_fraction{0.0};
  // Range statistics are meaningful only when valid_count > 0.
  double minimum_range{0.0};
  double maximum_range{0.0};
  double mean_range{0.0};
};

// O(n) time, O(1) extra space; never modifies or republishes the input.
// Positive infinity represents no return, not an invalid numeric sample.
ScanAnalysis analyze_scan(const sensor_msgs::msg::LaserScan & scan);

}  // namespace two_d_lidar_mapping_with_slam

#endif  // TWO_D_LIDAR_MAPPING_WITH_SLAM__SCAN_ANALYSIS_HPP_
