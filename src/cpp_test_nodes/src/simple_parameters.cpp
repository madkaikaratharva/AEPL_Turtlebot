#include <string>
#include "rclcpp/rclcpp.hpp"


class SimpleParameter : public rclcpp::Node
{
private:

public:
    SimpleParameter()
        : Node("simple_parameter_node")
    {
        // declare node parameters
        // declare_parameter<type>("paramter_name", default_value)
        declare_parameter<int>("simple_int_param", 20);
        declare_parameter<std::string>("simple_string_param", "Wafflebot");

        // 
    }
};