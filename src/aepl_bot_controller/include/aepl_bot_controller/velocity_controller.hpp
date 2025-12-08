#pragma once
#include <Eigen/Core>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"


class VelocityController : public rclcpp::Node
{
private:
    // wheel radius and seperation
    double wheel_radius_;
    double wheel_separation_;

    // Initialize the differential matrix using Eigen. Eigen::Matrix2d (2x2) matrix
    Eigen::Matrix2d differential_matrix_;

    // subscription object. subscribe to /cmd_vel topic
    rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr velocity_sub_;

    //publihser object. published motor command to simple velocity controller
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr velocity_pub_;

public:
    // declare the constructor
    VelocityController(const char* name);

private:
    // declare velocity callback function
    void velocityCallback(const geometry_msgs::msg::TwistStamped& msg);
};
