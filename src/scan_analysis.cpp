#include "two_d_lidar_mapping_with_slam/scan_analysis.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace two_d_lidar_mapping_with_slam
{

ScanAnalysis analyze_scan(const sensor_msgs::msg::LaserScan & scan)
{
  ScanAnalysis result;
  result.total_count = scan.ranges.size();
  if (scan.ranges.empty()) {
    result.error = "Empty ranges array";
    return result;
  }
  if (scan.header.frame_id.empty()) {
    result.error = "Empty frame_id";
    return result;
  }
  if (!std::isfinite(scan.range_min) || !std::isfinite(scan.range_max) ||
    scan.range_min < 0.0F || scan.range_max <= scan.range_min)
  {
    result.error = "Invalid range limits";
    return result;
  }
  if (!std::isfinite(scan.angle_min) || !std::isfinite(scan.angle_max) ||
    !std::isfinite(scan.angle_increment) || scan.angle_increment == 0.0F)
  {
    result.error = "Invalid angular metadata";
    return result;
  }
  // Permit float rounding, but reject a missing/extra beam or wrong scan geometry.
  const double expected_end = static_cast<double>(scan.angle_min) +
    static_cast<double>(scan.angle_increment) * (scan.ranges.size() - 1);
  const double tolerance = std::max(1e-5, std::abs(scan.angle_increment) * 0.01);
  if (std::abs(expected_end - scan.angle_max) > tolerance) {
    result.error = "Angular span does not match ranges count";
    return result;
  }
  if (!scan.intensities.empty() && scan.intensities.size() != scan.ranges.size()) {
    result.error = "Intensities count does not match ranges count";
    return result;
  }
  result.structure_valid = true;
  double sum = 0.0;
  double minimum = std::numeric_limits<double>::infinity();
  double maximum = -std::numeric_limits<double>::infinity();
  for (const float range : scan.ranges) {
    if (std::isinf(range) && range > 0.0F) {
      ++result.no_return_count;
    } else if (std::isfinite(range) && range >= scan.range_min && range <= scan.range_max) {
      ++result.valid_count;
      sum += range;
      minimum = std::min(minimum, static_cast<double>(range));
      maximum = std::max(maximum, static_cast<double>(range));
    } else {
      ++result.invalid_count;
    }
  }
  result.valid_fraction = static_cast<double>(result.valid_count) / result.total_count;
  if (result.valid_count > 0) {
    result.minimum_range = minimum;
    result.maximum_range = maximum;
    result.mean_range = sum / result.valid_count;
  }
  return result;
}

}  // namespace two_d_lidar_mapping_with_slam
