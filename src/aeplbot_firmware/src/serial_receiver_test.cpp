#include <chrono>
#include <libserial/SerialPort.h>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using std::placeholders::_1;
using namespace std::chrono_literals;

class SerialReceiver : public rclcpp::Node
{
private:
    // Publisher to Publish Serial Messages
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr serial_pub_;

    // Create a Timer Object
    rclcpp::TimerBase::SharedPtr timer_;

    // Create Serial Port
    LibSerial::SerialPort arduino_;

    // Port name
    std::string port_name_;
public:
    SerialReceiver()
        : Node("simple_serial_receiver")
    {
        // declare parameters
        declare_parameter("port", "/dev/ttyUSB0");
        port_name_ = get_parameter("port").as_string();

        // Open Serial Port and set Baud Rate
        arduino_.Open(port_name_);
        arduino_.SetBaudRate(LibSerial::BaudRate::BAUD_115200);

        // Initialize the Publisher
        serial_pub_ = create_publisher<std_msgs::msg::String>("/serial_receiver", 10);
        timer_ = create_wall_timer(0.01s, std::bind(&SerialReceiver::timerCallback, this));
        RCLCPP_INFO(get_logger(), "Publishing at 100 Hz");
    }

    ~SerialReceiver()
    {
        // Close the Serial Port
        arduino_.Close();
    }
private:
    void timerCallback()
    {   
        // Create the message
        std_msgs::msg::String msg;

        // Check if Node is running and data is available in arduino
        if(rclcpp::ok() && arduino_.IsDataAvailable())
        {   
            // Read the message sent by arudino and store in msd.data
            arduino_.ReadLine(msg.data);
        }

        serial_pub_->publish(msg);

    }
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SerialReceiver>();
    rclcpp::spin(node);
    rclcpp::shutdown();
}