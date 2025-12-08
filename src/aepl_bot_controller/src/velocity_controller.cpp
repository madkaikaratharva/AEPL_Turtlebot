#include <Eigen/Geometry>
#include "aepl_bot_controller/velocity_controller.hpp"

using std::placeholders::_1;

VelocityController::VelocityController(const char* name)
    : Node(name)
{
    // declare parameters
    declare_parameter("wheel_radius", 0.033); // in meters
    declare_parameter("wheel_separation", 0.17); // in meters

    // get runtime values of parameters
    wheel_radius_ = get_parameter("wheel_radius").as_double();
    wheel_separation_ = get_parameter("wheel_separation").as_double();

    // Log the runtime parameter values
    RCLCPP_INFO_STREAM(get_logger(), "Wheel Radius: " << wheel_radius_);
    RCLCPP_INFO_STREAM(get_logger(), "Wheel Separation: " << wheel_separation_);

    // create motor velocity publisher
    velocity_pub_ = create_publisher<std_msgs::msg::Float64MultiArray>("/simple_velocity_controller/commands", 10);

    // create subscription to /cmd_vel topic to get velocity commands
    velocity_sub_ = create_subscription<geometry_msgs::msg::TwistStamped>("/joy/cmd_vel", 10,
                                                                            std::bind(&VelocityController::velocityCallback, this, _1));

                                                
   // Define the differential speed matrix
   differential_matrix_ << wheel_radius_/2, wheel_radius_/2, wheel_radius_/wheel_separation_, -wheel_radius_/wheel_separation_;

   // print the differential speed matrix
   RCLCPP_INFO_STREAM(get_logger(), "Differential Speed Matrix \n" << differential_matrix_); 
   
}

void VelocityController::velocityCallback(const geometry_msgs::msg::TwistStamped& msg)
{   
    // create a 2d vector
    Eigen::Vector2d robot_speed(msg.twist.linear.x, msg.twist.angular.z);
    
    // motor speeds
    Eigen::Vector2d motor_speed = differential_matrix_.inverse() * robot_speed;

    // create float64 array message
    std_msgs::msg::Float64MultiArray motor_speed_msg;
    motor_speed_msg.data.push_back(motor_speed.coeff(1));   // Left wheel
    motor_speed_msg.data.push_back(motor_speed.coeff(0));   // Right wheel

    // publish the motor speed message
    velocity_pub_->publish(motor_speed_msg);

}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<VelocityController>("velocity_controller");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}