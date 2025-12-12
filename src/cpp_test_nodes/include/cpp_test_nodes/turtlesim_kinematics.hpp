#pragma once
#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"

class TurtlesimKinematics : public rclcpp::Node
{
private:
    turtlesim::msg::Pose last_t1_pose_;
    turtlesim::msg::Pose last_t2_pose_;

    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose1_sub_;
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose2_sub_;

public:
    TurtlesimKinematics(const char* name);

private:
    void turtle1PoseCallback(const turtlesim::msg::Pose& pose);
    void turtle2PoseCallback(const turtlesim::msg::Pose& pose);

};