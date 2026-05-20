import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/maher/My_Projects/2D-LiDAR-Mapping-with-SLAM/install/two_d_lidar_mapping_with_slam'
