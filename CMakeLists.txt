cmake_minimum_required(VERSION 3.16)
project(two_d_lidar_mapping_with_slam VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(diagnostic_msgs REQUIRED)

add_library(scan_analysis src/scan_analysis.cpp)
target_include_directories(scan_analysis PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(scan_analysis sensor_msgs)

add_library(scan_diagnostics src/scan_diagnostics_node.cpp)
target_link_libraries(scan_diagnostics scan_analysis)
ament_target_dependencies(scan_diagnostics rclcpp rcl_interfaces sensor_msgs diagnostic_msgs)

add_executable(scan_diagnostics_node src/scan_diagnostics_main.cpp)
target_link_libraries(scan_diagnostics_node scan_diagnostics)
ament_target_dependencies(scan_diagnostics_node rclcpp)

add_executable(scan_demo_node src/scan_demo_node.cpp)
ament_target_dependencies(scan_demo_node rclcpp sensor_msgs)

install(TARGETS scan_analysis scan_diagnostics
  EXPORT export_scan_diagnostics
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
install(TARGETS scan_diagnostics_node scan_demo_node
  DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY launch config scripts DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_test REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_scan_analysis test/test_scan_analysis.cpp)
  target_link_libraries(test_scan_analysis scan_analysis)
  ament_add_gtest(test_scan_diagnostics test/test_scan_diagnostics.cpp
    SKIP_LINKING_MAIN_LIBRARIES TIMEOUT 45)
  target_link_libraries(test_scan_diagnostics scan_diagnostics)
  ament_target_dependencies(test_scan_diagnostics rclcpp diagnostic_msgs sensor_msgs)
  ament_add_test(test_launch_smoke
    COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/test/launch_smoke.sh"
    TIMEOUT 90
    GENERATE_RESULT_FOR_RETURN_CODE_ZERO)
endif()

ament_export_targets(export_scan_diagnostics HAS_LIBRARY_TARGET)
ament_export_dependencies(rclcpp rcl_interfaces sensor_msgs diagnostic_msgs)
ament_package()
