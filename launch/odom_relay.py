#!/usr/bin/env python3
"""
Relay /odom_raw → /odom + odom→base_footprint TF.
Replaces robot_localization EKF for single-sensor (wheel odometry only) setups.
"""
import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from geometry_msgs.msg import TransformStamped
from tf2_ros import TransformBroadcaster


class OdomRelay(Node):
    def __init__(self):
        super().__init__('odom_relay')
        self._tf_br = TransformBroadcaster(self)
        self._odom_pub = self.create_publisher(Odometry, 'odom', 10)
        self.create_subscription(Odometry, 'odom_raw', self._cb, 10)
        self.get_logger().info('odom_relay started — forwarding odom_raw → odom + TF')

    def _cb(self, msg: Odometry):
        self._odom_pub.publish(msg)

        t = TransformStamped()
        t.header = msg.header
        t.child_frame_id = msg.child_frame_id
        t.transform.translation.x = msg.pose.pose.position.x
        t.transform.translation.y = msg.pose.pose.position.y
        t.transform.translation.z = msg.pose.pose.position.z
        t.transform.rotation = msg.pose.pose.orientation
        self._tf_br.sendTransform(t)


def main():
    rclpy.init()
    rclpy.spin(OdomRelay())
    rclpy.shutdown()


if __name__ == '__main__':
    main()
