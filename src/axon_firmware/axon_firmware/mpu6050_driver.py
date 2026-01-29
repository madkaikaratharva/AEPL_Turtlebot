#!/usr/bin/env python3
import smbus
import math

import rclpy
import rclpy.time
from rclpy.node import Node
from rclpy.qos import qos_profile_sensor_data

from sensor_msgs.msg import Imu


# MPU6050 Registers
PWR_MGMT_1   = 0x6B
SMPLRT_DIV   = 0x19
CONFIG       = 0x1A
GYRO_CONFIG  = 0x1B
ACCEL_CONFIG = 0x1C
INT_ENABLE   = 0x38

ACCEL_XOUT_H = 0x3B
ACCEL_YOUT_H = 0x3D
ACCEL_ZOUT_H = 0x3F

GYRO_XOUT_H  = 0x43
GYRO_YOUT_H  = 0x45
GYRO_ZOUT_H  = 0x47

DEVICE_ADDRESS = 0x68

# Datasheets Constants
ACCEL_SENS_2G = 16384.0     # LSB / g
GYRO_SENS_2000DPS = 16.4    # LSB / deg/s
G = 9.80665

class MPU6050_Driver(Node):

    def __init__(self):
        super().__init__("mpu6050_driver")
        
        # I2C Interafce
        self.is_connected_ = False
        self.init_i2c()

        # ROS 2 Interface
        # IMU Publisher
        self.imu_pub_ = self.create_publisher(Imu, "/imu/out", qos_profile=qos_profile_sensor_data)

        # IMU Message
        self.imu_msg_ = Imu()
        self.imu_msg_.header.frame_id = "imu_link"

        # Publishing Loop (100 Hz)
        self.frequency_ = 0.01
        self.timer_ = self.create_timer(self.frequency_, self.timerCallback)

    def timerCallback(self):
        try:
            if not self.is_connected_:
                self.init_i2c()
            
            # Read Accelerometer raw value
            acc_x = self.read_raw_data(ACCEL_XOUT_H)
            acc_y = self.read_raw_data(ACCEL_YOUT_H)
            acc_z = self.read_raw_data(ACCEL_ZOUT_H)
            
            # Read Gyroscope raw value
            gyro_x = self.read_raw_data(GYRO_XOUT_H)
            gyro_y = self.read_raw_data(GYRO_YOUT_H)
            gyro_z = self.read_raw_data(GYRO_ZOUT_H)
            
            # Full scale range +/- 250 degree/C as per sensitivity scale factor     
            self.imu_msg_.linear_acceleration.x = (acc_x / ACCEL_SENS_2G) * G
            self.imu_msg_.linear_acceleration.y = (acc_y / ACCEL_SENS_2G) * G
            self.imu_msg_.linear_acceleration.z = (acc_z / ACCEL_SENS_2G) * G

            self.imu_msg_.angular_velocity.x = (gyro_x / GYRO_SENS_2000DPS) * (math.pi / 180.0)
            self.imu_msg_.angular_velocity.y = (gyro_y / GYRO_SENS_2000DPS) * (math.pi / 180.0)
            self.imu_msg_.angular_velocity.z = (gyro_z / GYRO_SENS_2000DPS) * (math.pi / 180.0)

            # Publish IMU Message
            self.imu_msg_.header.stamp = self.get_clock().now().to_msg()
            self.imu_msg_.orientation_covariance[0] = -1.0
            self.imu_pub_.publish(self.imu_msg_)
        except OSError:
            self.is_connected_ = False

    def init_i2c(self):
        try:
            # Initialize I2C Serial Bus
            self.bus_ = smbus.SMBus(1)

            # Wake up device
            self.bus_.write_byte_data(DEVICE_ADDRESS, PWR_MGMT_1, 0x01)

            # Set sample rate = 125 Hz
            self.bus_.write_byte_data(DEVICE_ADDRESS, SMPLRT_DIV, 7)

            # Low-pass filter
            self.bus_.write_byte_data(DEVICE_ADDRESS, CONFIG, 0x03)

            # Gyro Config
            self.bus_.write_byte_data(DEVICE_ADDRESS, GYRO_CONFIG, 0x18)

            # Accel Config
            self.bus_.write_byte_data(DEVICE_ADDRESS, ACCEL_CONFIG, 0x00)

            self.bus_.write_byte_data(DEVICE_ADDRESS, INT_ENABLE, 1)

            self.is_connected_ = True
            self.get_logger().info("MPU6050 connected")
        except OSError as e:
            self.is_connected_ = False
            self.get_logger().error(f"I2C init failed: {e}")
        
    def read_raw_data(self, addr):
        #Accelero and Gyro value are 16-bit
        high = self.bus_.read_byte_data(DEVICE_ADDRESS, addr)
        low = self.bus_.read_byte_data(DEVICE_ADDRESS, addr+1)
        
        #concatenate higher and lower value
        value = ((high << 8) | low)
            
        #to get signed value from mpu6050
        if(value > 32768):
            value = value - 65536
        return value


def main():
    rclpy.init()
    mpu6050_driver = MPU6050_Driver()
    rclpy.spin(mpu6050_driver)
    mpu6050_driver.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()