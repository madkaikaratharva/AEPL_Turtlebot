#include <chrono>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class SimplePublisher : public rclcpp::Node
{
private:
    // support variables
    unsigned int counter_;

    // shared pointer to the publisher object
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;

    // shared pointer to the timer object
    rclcpp::TimerBase::SharedPtr timer_;

public:
    // create class constructor
    SimplePublisher()
        : Node("simple_publisher")
        , counter_(0)
    {
        // create a publisher object
        pub_ = create_publisher<std_msgs::msg::String>("/chatter", 10);

        // create a timer object (creates a loop that executes a certain function periodically)
        timer_ = create_wall_timer(1s, std::bind(&SimplePublisher::timerCallback, this));

        RCLCPP_INFO(get_logger(), "publishing at 1 Hz");
    }

private:
    void timerCallback()
    {
        // create a message variable
        std_msgs::msg::String message;
        message.data = "Hello ROS2 : " + std::to_string(counter_);
        counter_++;

        // publish the message
        pub_->publish(message);
    }
};

int main(int argc, char* argv[])
{
    // Initialize ROS2 interface
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SimplePublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}