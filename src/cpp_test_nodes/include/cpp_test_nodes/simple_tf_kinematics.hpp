#pragma once
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/static_transform_broadcaster.h"
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "aepl_bot_msgs/srv/get_transform.hpp"

#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"
#include "tf2/LinearMath/Quaternion.h"

class SimpleTfKinematics : public rclcpp::Node
{
private:
    // Shared Pointer to Static Transform Broadcaster
    std::shared_ptr<tf2_ros::StaticTransformBroadcaster> static_tf_broadcaster_;

    // Shared Pointer to Dynamic Transform Broadcaster
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

    // Shared Pointer to service server
    rclcpp::Service<aepl_bot_msgs::srv::GetTransform>::SharedPtr get_transform_srv_;

    // Transform Messages
    geometry_msgs::msg::TransformStamped static_transform_stamped_;
    geometry_msgs::msg::TransformStamped dynamic_transform_stamped_;

    // Timer object to publish messages at a constant rate
    rclcpp::TimerBase::SharedPtr timer_;

    // Support variables
    double x_increment_;
    double x_last_;
    unsigned int rotations_counter_;

    // To store rotation, use quaternion variable
    tf2::Quaternion last_known_orientation_;
    tf2::Quaternion orientation_increment_;

    // Create the TF2_ROS Buffer and Listener shared ptrs
    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_{nullptr};


public:
    SimpleTfKinematics(const char* name);

private:
    void timerCallback();
    
    bool getTransformCallback(std::shared_ptr<aepl_bot_msgs::srv::GetTransform::Request> req,
                              std::shared_ptr<aepl_bot_msgs::srv::GetTransform::Response> res);

};