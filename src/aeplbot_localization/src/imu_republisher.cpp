#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"

using std::placeholders::_1;
using namespace std::chrono_literals;

class ImuRepublisher : public rclcpp::Node
{
private:
    // Imu Subscriber and Publisher
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;


public:
    ImuRepublisher(const char* name)
        : Node(name)
    {


        // Wait for 1s
        RCLCPP_INFO_ONCE(get_logger(), "Listening for 2 seconds");
        std::this_thread::sleep_for(2s);
        RCLCPP_INFO_ONCE(get_logger(), "Replublishing Imu data");

        // Initialize Publisher
        imu_pub_ = create_publisher<sensor_msgs::msg::Imu>("/imu_ekf", 10);

        // Initialize Subscriber
        imu_sub_ = create_subscription<sensor_msgs::msg::Imu>("/imu/out", 10, 
                                                                std::bind(&ImuRepublisher::imuCallback, this, _1));
    }   
private:
    void imuCallback(const sensor_msgs::msg::Imu::SharedPtr msg)
    {
       sensor_msgs::msg::Imu imu_message;
       imu_message = *msg;
       imu_message.header.frame_id = "base_footprint_ekf";
       imu_pub_->publish(imu_message); 
    }
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ImuRepublisher>("imu_republisher_node");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}