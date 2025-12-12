#include "cpp_test_nodes/turtlesim_kinematics.hpp"

using std::placeholders::_1;

TurtlesimKinematics::TurtlesimKinematics(const char* name)
    : Node(name)
{
    pose1_sub_ = create_subscription<turtlesim::msg::Pose>("/turtle1/pose", 10, 
                                                                std::bind(&TurtlesimKinematics::turtle1PoseCallback, this, _1));

    pose2_sub_ = create_subscription<turtlesim::msg::Pose>("/turtle2/pose", 10,
                                                                std::bind(&TurtlesimKinematics::turtle2PoseCallback, this, _1));
}

void TurtlesimKinematics::turtle1PoseCallback(const turtlesim::msg::Pose& pose)
{
    last_t1_pose_ = pose;
}

void TurtlesimKinematics::turtle2PoseCallback(const turtlesim::msg::Pose& pose)
{
    last_t2_pose_ = pose;
    float Tx = last_t2_pose_.x - last_t1_pose_.x;
    float Ty = last_t2_pose_.y - last_t1_pose_.y;

    // Relative orientation between two turtles
    float theta_rad = last_t2_pose_.theta - last_t1_pose_.theta;


    RCLCPP_INFO_STREAM(get_logger(), "Translation vector from turtl1 -> turtle2 \n" << "Tx: " << Tx << "Ty: " << Ty << "theta(rad) " << theta_rad);
}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TurtlesimKinematics>("turtlesim_kinematics");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}