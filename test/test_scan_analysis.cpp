#include <cmath>
#include <limits>

#include <gtest/gtest.h>

#include "two_d_lidar_mapping_with_slam/scan_analysis.hpp"

namespace
{
sensor_msgs::msg::LaserScan make_scan()
{
  sensor_msgs::msg::LaserScan scan;
  scan.header.frame_id = "lidar_frame";
  scan.angle_min = -1.0F;
  scan.angle_increment = 1.0F;
  scan.angle_max = 1.0F;
  scan.range_min = 0.1F;
  scan.range_max = 10.0F;
  scan.ranges = {1.0F, 2.0F, 3.0F};
  return scan;
}
using two_d_lidar_mapping_with_slam::analyze_scan;

TEST(ScanAnalysis, ComputesFiniteRangeStatistics)
{
  const auto result = analyze_scan(make_scan());
  ASSERT_TRUE(result.structure_valid);
  EXPECT_EQ(result.total_count, 3U);
  EXPECT_EQ(result.valid_count, 3U);
  EXPECT_EQ(result.invalid_count, 0U);
  EXPECT_EQ(result.no_return_count, 0U);
  EXPECT_DOUBLE_EQ(result.valid_fraction, 1.0);
  EXPECT_DOUBLE_EQ(result.minimum_range, 1.0);
  EXPECT_DOUBLE_EQ(result.maximum_range, 3.0);
  EXPECT_DOUBLE_EQ(result.mean_range, 2.0);
}

TEST(ScanAnalysis, IncludesBothRangeBoundaries)
{
  auto scan = make_scan();
  scan.ranges = {scan.range_min, scan.range_max, 2.0F};
  EXPECT_EQ(analyze_scan(scan).valid_count, 3U);
}

TEST(ScanAnalysis, SeparatesNoReturnsFromInvalidSamples)
{
  auto scan = make_scan();
  scan.ranges = {
    2.0F, std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN(),
    0.0F, 11.0F};
  scan.angle_max = 4.0F;
  const auto original = scan.ranges;
  const auto result = analyze_scan(scan);
  ASSERT_TRUE(result.structure_valid);
  EXPECT_EQ(result.valid_count, 1U);
  EXPECT_EQ(result.no_return_count, 1U);
  EXPECT_EQ(result.invalid_count, 4U);
  EXPECT_DOUBLE_EQ(result.valid_fraction, 1.0 / 6.0);
  EXPECT_DOUBLE_EQ(result.mean_range, 2.0);
  EXPECT_EQ(result.valid_count + result.no_return_count + result.invalid_count, result.total_count);
  // Input is observed without rewriting NaN, infinity, or out-of-range readings.
  EXPECT_EQ(scan.ranges.size(), original.size());
  EXPECT_TRUE(std::isnan(scan.ranges[3]));
  EXPECT_EQ(scan.ranges[5], 11.0F);
}

TEST(ScanAnalysis, HandlesNoFiniteReturns)
{
  auto scan = make_scan();
  scan.ranges.assign(3, std::numeric_limits<float>::infinity());
  const auto result = analyze_scan(scan);
  ASSERT_TRUE(result.structure_valid);
  EXPECT_EQ(result.no_return_count, 3U);
  EXPECT_EQ(result.valid_count, 0U);
  EXPECT_DOUBLE_EQ(result.valid_fraction, 0.0);
}

TEST(ScanAnalysis, RejectsEmptyRanges)
{
  auto scan = make_scan();
  scan.ranges.clear();
  EXPECT_EQ(analyze_scan(scan).error, "Empty ranges array");
  EXPECT_FALSE(analyze_scan(scan).structure_valid);
}

TEST(ScanAnalysis, RejectsMissingFrame)
{
  auto scan = make_scan();
  scan.header.frame_id.clear();
  EXPECT_EQ(analyze_scan(scan).error, "Empty frame_id");
}

TEST(ScanAnalysis, RejectsInvalidRangeLimits)
{
  auto scan = make_scan();
  scan.range_min = -1.0F;
  EXPECT_FALSE(analyze_scan(scan).structure_valid);
  scan.range_min = scan.range_max;
  EXPECT_FALSE(analyze_scan(scan).structure_valid);
  scan.range_min = 0.0F;
  scan.range_max = std::numeric_limits<float>::infinity();
  EXPECT_FALSE(analyze_scan(scan).structure_valid);
  scan.range_max = std::numeric_limits<float>::quiet_NaN();
  EXPECT_FALSE(analyze_scan(scan).structure_valid);
}

TEST(ScanAnalysis, RejectsInvalidAngularMetadata)
{
  auto scan = make_scan();
  scan.angle_increment = 0.0F;
  EXPECT_FALSE(analyze_scan(scan).structure_valid);
  scan.angle_increment = std::numeric_limits<float>::quiet_NaN();
  EXPECT_FALSE(analyze_scan(scan).structure_valid);
  scan = make_scan();
  scan.angle_max = std::numeric_limits<float>::infinity();
  EXPECT_FALSE(analyze_scan(scan).structure_valid);
}

TEST(ScanAnalysis, RejectsMismatchedAngularSpan)
{
  auto scan = make_scan();
  scan.angle_max = 2.0F;
  EXPECT_EQ(analyze_scan(scan).error, "Angular span does not match ranges count");
}

TEST(ScanAnalysis, AcceptsClockwiseScanAndSmallRoundingError)
{
  auto scan = make_scan();
  scan.angle_min = 1.0F;
  scan.angle_increment = -1.0F;
  scan.angle_max = -1.0F + 1e-6F;
  EXPECT_TRUE(analyze_scan(scan).structure_valid);
}

TEST(ScanAnalysis, ChecksOptionalIntensityLength)
{
  auto scan = make_scan();
  scan.intensities = {1.0F};
  EXPECT_FALSE(analyze_scan(scan).structure_valid);
  scan.intensities = {1.0F, 2.0F, 3.0F};
  EXPECT_TRUE(analyze_scan(scan).structure_valid);
}
}  // namespace
