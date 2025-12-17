#include <libserial/SerialPort.h>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using std::placeholders::_1;

class SerialTransmitter : public rclcpp::Node
{
private:
    // Subscription to Listen to Serial Transmission Topic
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr serial_trans_sub_;

    // Serial port for arduino
    LibSerial::SerialPort arduino_;

    // Support Variables
    std::string port_name_;
public:
    SerialTransmitter()
        : Node("serial_transmitter")
    {   
        // declare parameter
        declare_parameter<std::string>("port", "/dev/ttyUSB0");
        port_name_ =get_parameter("port").as_string();

        // Open the serial port
        arduino_.Open(port_name_);
        
        // declare baud rate
        arduino_.SetBaudRate(LibSerial::BaudRate::BAUD_115200);


        // Initialize the subscription
        serial_trans_sub_ = create_subscription<std_msgs::msg::String>("/serial_transmitter", 10, 
                                                    std::bind(&SerialTransmitter::msgCallback, this, _1));
    }

    ~SerialTransmitter()
    {
        arduino_.Close();
    }

private:
    // Callback function
    void msgCallback(const std_msgs::msg::String& msg)
    {
        arduino_.Write(msg.data + "\n");
    }

};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SerialTransmitter>();
    rclcpp::spin(node);
    rclcpp::shutdown();

}