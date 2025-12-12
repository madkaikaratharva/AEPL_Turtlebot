#include <chrono>
#include "cpp_test_nodes/simple_tf_kinematics.hpp"

using namespace std::chrono_literals;
using namespace std::placeholders;

SimpleTfKinematics::SimpleTfKinematics(const char* name)
    : Node(name)
    , x_increment_(0.05)
    , x_last_(0.0)
    , rotations_counter_(0)
{
    // Initialize Static Transform Broadcaster
    static_tf_broadcaster_ = std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);
    
    // Initialize dynamic transform
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    // Initialize TF2 Buffer and Listener
    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    // Initialize the  Static Transform Message
    static_transform_stamped_.header.stamp = get_clock()->now();    // get_clock()->now() provides ROS2 time of the current node
    static_transform_stamped_.header.frame_id = "wafflebot_base";
    static_transform_stamped_.child_frame_id = "wafflebot_top";

    // Translation between two frames
    static_transform_stamped_.transform.translation.x = 0.0;
    static_transform_stamped_.transform.translation.y = 0.0;
    static_transform_stamped_.transform.translation.z = 0.3;

    // Rotation between two frames in Quaternions
    static_transform_stamped_.transform.rotation.x = 0.0;
    static_transform_stamped_.transform.rotation.y = 0.0;
    static_transform_stamped_.transform.rotation.z = 0.0;
    static_transform_stamped_.transform.rotation.w = 1.0;

    // Initialize the Dynamic Transform Message
    dynamic_transform_stamped_.header.frame_id = "odom";
    dynamic_transform_stamped_.child_frame_id = "wafflebot_base";

    // Initialize initial orientation
    last_known_orientation_.setRPY(0, 0, 0);
    orientation_increment_.setRPY(0, 0, 0.05);

    // Publish the TransformStamped message
    static_tf_broadcaster_->sendTransform(static_transform_stamped_);

    RCLCPP_INFO_STREAM(get_logger(), "Publishing static transform between: " 
                << static_transform_stamped_.header.frame_id << " and " << static_transform_stamped_.child_frame_id);
    

    timer_ = create_wall_timer(0.1s, std::bind(&SimpleTfKinematics::timerCallback, this));

    // Initialize the service server
    get_transform_srv_ = create_service<aepl_bot_msgs::srv::GetTransform>("get_transform", 
                                                                            std::bind(&SimpleTfKinematics::getTransformCallback, this, _1, _2));
}

void SimpleTfKinematics::timerCallback()
{
    // Get the current time from ROS 2 library
    dynamic_transform_stamped_.header.stamp = get_clock()->now();

    // Translation between the frames
    dynamic_transform_stamped_.transform.translation.x = x_last_ + x_increment_;
    dynamic_transform_stamped_.transform.translation.y = 0.0;
    dynamic_transform_stamped_.transform.translation.z = 0.0;

    x_last_ = dynamic_transform_stamped_.transform.translation.x;

    // Rotation between two frames in Quaternions
    tf2::Quaternion q;
    q = last_known_orientation_ * orientation_increment_;
    q.normalize();

    dynamic_transform_stamped_.transform.rotation.x = q.x();
    dynamic_transform_stamped_.transform.rotation.y = q.y();
    dynamic_transform_stamped_.transform.rotation.z = q.z();
    dynamic_transform_stamped_.transform.rotation.w = q.w();

    last_known_orientation_ = q;

    tf_broadcaster_->sendTransform(dynamic_transform_stamped_);

    rotations_counter_++;
    if(rotations_counter_ >=100)
    {   
        // Invert the direction of rotation
        orientation_increment_ = orientation_increment_.inverse();
        rotations_counter_ = 0;
    }
}

bool SimpleTfKinematics::getTransformCallback(std::shared_ptr<aepl_bot_msgs::srv::GetTransform::Request> req,
                                              std::shared_ptr<aepl_bot_msgs::srv::GetTransform::Response> res)
{
    RCLCPP_INFO_STREAM(get_logger(), "Requested Transforms between: " << req->frame_id << " and " << req->child_frame_id);

    // Get the transforms between the frames
    // TransformStamped message
    geometry_msgs::msg::TransformStamped response_msg;

    // Look up transform. tf2::TimePointZero argument provides the latest available transform
    try{
        response_msg = tf_buffer_->lookupTransform(req->frame_id, req->child_frame_id, tf2::TimePointZero);
    }
    catch(tf2::TransformException& exception)
    {
        RCLCPP_ERROR_STREAM(get_logger(), "Error occurred: " << exception.what());
        res->success = false;
        return true;
    }

    res->success = true;
    res->transform = response_msg;
    return true;
}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node  = std::make_shared<SimpleTfKinematics>("simple_tf_kinematics");
    rclcpp::spin(node);
    rclcpp::shutdown();
}