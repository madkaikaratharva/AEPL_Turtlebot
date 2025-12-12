#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "aepl_bot_msgs/srv/add_two_ints.hpp"

using namespace std::chrono_literals;
using std::placeholders::_1;

class SimpleServiceClient : public rclcpp::Node
{
private:
    // Shared Function to Client Class
    rclcpp::Client<aepl_bot_msgs::srv::AddTwoInts>::SharedPtr client_;
public:
    SimpleServiceClient(int a, int b)
        : Node("simple_service_client")
    {
        // Initialize the Client
        client_ = create_client<aepl_bot_msgs::srv::AddTwoInts>("add_two_ints");

        // Create the Request Message
        auto request = std::make_shared<aepl_bot_msgs::srv::AddTwoInts::Request>();
        request->a = a;
        request->b = b;

        // Verify if the Server is running
        while(!client_->wait_for_service(1s))
        {
            // Verify if ROS 2 interface is running
            if(!rclcpp::ok())
            {   
                RCLCPP_ERROR(get_logger(), "Interrupted while waiting for service");
            }

            RCLCPP_INFO(get_logger(), "Service not available");
        }

        // Send request to service server
        auto result = client_->async_send_request(request, std::bind(&SimpleServiceClient::responseCallback, this, _1));
    }
private:
    void responseCallback(rclcpp::Client<aepl_bot_msgs::srv::AddTwoInts>::SharedFuture future)
    {
        if(future.valid())
        {
            RCLCPP_INFO_STREAM(get_logger(), "service response recieved: " << future.get()->sum);
        }
        else
        {
            RCLCPP_ERROR(get_logger(), "Service failure");
        }
    }
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    
    if(argc !=3)
    {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Wrong number of arguments! Use only two arguments (int)");
        return 1;
    }
    auto node = std::make_shared<SimpleServiceClient>(atoi(argv[1]), atoi(argv[2]));
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}