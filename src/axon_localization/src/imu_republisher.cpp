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

    // Support Variables
    unsigned int counter_;

    double bias_x_;
    double bias_y_;
    double bias_z_;

    double bias_theta_;

public:
    ImuRepublisher(const char* name)
        : Node(name)
        , counter_(0)
        , bias_x_(0.0)
        , bias_y_(0.0)
        , bias_z_(0.0)
        , bias_theta_(0.0)
    {

        // Initialize Publisher
        imu_pub_ = create_publisher<sensor_msgs::msg::Imu>("/imu/corrected", 10);

        // Initialize Subscriber
        imu_sub_ = create_subscription<sensor_msgs::msg::Imu>("/imu/out", 10, 
                                                                std::bind(&ImuRepublisher::imuCallback, this, _1));

        //RCLCPP_INFO_ONCE(get_logger(), "Replublishing Imu data");
    }

private:
    void imuCallback(const sensor_msgs::msg::Imu::SharedPtr msg)
    {   
       sensor_msgs::msg::Imu imu_message;       
       if(counter_ < 100)
       {
            // Log
            RCLCPP_INFO_ONCE(get_logger(), "Calibrating IMU Data. Keep Robot Stationary.");

            counter_++;

            // Accumulate the biases
            bias_x_ += (msg->linear_acceleration.x - bias_x_) / counter_;
            bias_y_ += (msg->linear_acceleration.y - bias_y_) / counter_;
            bias_z_ += (msg->linear_acceleration.z - bias_z_) / counter_;

            bias_theta_ += (msg->angular_velocity.z - bias_theta_) / counter_;
            return;
       }
       RCLCPP_INFO_ONCE(get_logger(), "Calibration Complete. Publishing Corrected Data!");
       RCLCPP_INFO_STREAM_ONCE(get_logger(), "bias_x: " << bias_x_ << " bias_z: " << bias_z_);

       imu_message = *msg;

       // Publish Corrected Readings
       imu_message.linear_acceleration.x -= bias_x_;
       imu_message.linear_acceleration.y -= bias_y_;
       imu_message.linear_acceleration.z -= (bias_z_ - 9.81);
       
       // Publish the corrected message
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