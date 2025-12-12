#include "rclcpp/rclcpp.hpp"
#include "aepl_bot_msgs/srv/add_two_ints.hpp"

using namespace std::placeholders;

class SimpleServiceServer : public rclcpp::Node
{
private:
    // Create a pointer to service server object
    rclcpp::Service<aepl_bot_msgs::srv::AddTwoInts>::SharedPtr service_;
public:
    SimpleServiceServer()
        : Node("simple_service_server")
    {   
        // Initialize the Service
        service_ = create_service<aepl_bot_msgs::srv::AddTwoInts>("add_two_ints", std::bind(&SimpleServiceServer::serviceCallback, this, _1, _2));

        // Log 
        RCLCPP_INFO(get_logger(), "Service add_two_ints_ready");
    }
private:
    void serviceCallback(std::shared_ptr<aepl_bot_msgs::srv::AddTwoInts::Request> req,
                         std::shared_ptr<aepl_bot_msgs::srv::AddTwoInts::Response> res)
    {   
        RCLCPP_INFO_STREAM(get_logger(), "New request received to add :" << req->a << " and " << req->b);

        // Sum
        res->sum = req->a + req->b;
        RCLCPP_INFO_STREAM(get_logger(), "Response sum: " << res->sum);
    }
};

int main(int argc,char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SimpleServiceServer>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}