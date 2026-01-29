#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "rclcpp/qos.hpp"

using std::placeholders::_1;

static const unsigned int SAMPLES = 200;
constexpr double ACCL_VAR = 0.07 * 0.07;
constexpr double GYRO_VAR = 0.05 * 0.05;


class ImuRepublisher : public rclcpp::Node
{
private:
    // Support Variables
    unsigned int sample_count_;

    double accel_x_bias_;
    double accel_y_bias_;
    double accel_z_bias_;

    double gyro_x_bias_;
    double gyro_y_bias_;
    double gyro_z_bias_;

    bool is_calibrated_;

    // Publishers and Subscribers
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;    

public:

    ImuRepublisher()
        : Node("imu_republisher_node")
        , sample_count_(0)
        , accel_x_bias_(0.0)
        , accel_y_bias_(0.0)
        , accel_z_bias_(0.0)
        , gyro_x_bias_(0.0)
        , gyro_y_bias_(0.0)
        , gyro_z_bias_(0.0)
        , is_calibrated_(false)
    {
        // Initialize Publisher and Subscribers
        imu_pub_ = create_publisher<sensor_msgs::msg::Imu>("/imu/ekf", rclcpp::SensorDataQoS());
        imu_sub_ = create_subscription<sensor_msgs::msg::Imu>("/imu/out", rclcpp::SensorDataQoS(),
                                            std::bind(&ImuRepublisher::imuCallback, this, _1));
    }

private:
    void imuCallback(const sensor_msgs::msg::Imu::SharedPtr msg)
    {   
        if(!is_calibrated_)
        {
            RCLCPP_INFO_ONCE(get_logger(), "Calibrating IMU. Do not move ...");

            accel_x_bias_ += msg->linear_acceleration.x;
            accel_y_bias_ += msg->linear_acceleration.y;
            accel_z_bias_ += msg->linear_acceleration.z;


            gyro_x_bias_ += msg->angular_velocity.x;
            gyro_y_bias_ += msg->angular_velocity.y;
            gyro_z_bias_ += msg->angular_velocity.z;

            sample_count_++;

            if(sample_count_ == SAMPLES)
            {
                accel_x_bias_ = accel_x_bias_ / sample_count_;
                accel_y_bias_ = accel_y_bias_ / sample_count_;
                accel_z_bias_ = (accel_z_bias_ / sample_count_) - 9.80665;

                gyro_x_bias_ = gyro_x_bias_ / sample_count_;
                gyro_y_bias_ = gyro_y_bias_ / sample_count_;
                gyro_z_bias_ = gyro_z_bias_ / sample_count_;

                is_calibrated_ = true;

                RCLCPP_INFO(get_logger(), "IMU Calibration complete!");
            }
        }

        if(is_calibrated_)
        {
            sensor_msgs::msg::Imu imu_msg = *msg;
            
            imu_msg.header.frame_id = "base_footprint";
            
            imu_msg.linear_acceleration.x -= accel_x_bias_;
            imu_msg.linear_acceleration.y -= accel_y_bias_;
            imu_msg.linear_acceleration.z -= accel_z_bias_;

            imu_msg.angular_velocity.x -= gyro_x_bias_;
            imu_msg.angular_velocity.y -= gyro_y_bias_;
            imu_msg.angular_velocity.z -= gyro_z_bias_;

            imu_msg.linear_acceleration_covariance = {
                ACCL_VAR, 0.0, 0.0,
                0.0, ACCL_VAR, 0.0,
                0.0, 0.0, ACCL_VAR
            };

            imu_msg.angular_velocity_covariance = {
                GYRO_VAR, 0.0, 0.0,
                0.0, GYRO_VAR, 0.0,
                0.0, 0.0, GYRO_VAR
            };

            imu_pub_->publish(imu_msg);
            
        }

    }
};

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);
  std::shared_ptr<ImuRepublisher> node = std::make_shared<ImuRepublisher>();
  rclcpp::spin(node);
  rclcpp::shutdown();
}