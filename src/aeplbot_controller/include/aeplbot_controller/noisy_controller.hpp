#pragma once
#include "rclcpp/rclcpp.hpp"

#include "sensor_msgs/msg/joint_state.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"

#include "tf2_ros/transform_broadcaster.h"


class NoisyController : public rclcpp::Node
{
private:
    // wheel radius and separation
    double wheel_radius_;
    double wheel_separation_;

    // Internal state
    double right_wheel_prev_pos_;
    double left_wheel_prev_pos_;
    rclcpp::Time prev_time_;

    // State variables
    double x_;
    double y_;
    double theta_;

    // subscription to get joint states
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_sub_;

    // publisher to publish odometry
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;

    // TF2 Brodadcaster
    std::unique_ptr<tf2_ros::TransformBroadcaster> transform_broadcaster_;

    // Odometry Message
    nav_msgs::msg::Odometry odom_msg_;

    // TransformStamped Message
    geometry_msgs::msg::TransformStamped transform_stamped_;
public:
    NoisyController(const char* name);

private:

    // JointState callback
    void jointCallback(const sensor_msgs::msg::JointState &msg);

};