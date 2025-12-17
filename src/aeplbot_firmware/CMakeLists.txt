cmake_minimum_required(VERSION 3.8)
project(aeplbot_firmware)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(std_msgs REQUIRED)
find_package(PkgConfig REQUIRED)

ament_python_install_package(${PROJECT_NAME})

pkg_check_modules(SERIAL libserial)

add_executable(serial_transmitter_test src/serial_transmitter_test.cpp)
ament_target_dependencies(serial_transmitter_test rclcpp std_msgs)
target_include_directories(serial_transmitter_test PUBLIC "${SERIAL_INCLUDE_DIRS}")
target_link_libraries(serial_transmitter_test "${SERIAL_LDFLAGS}")

add_executable(serial_receiver_test src/serial_receiver_test.cpp)
ament_target_dependencies(serial_receiver_test rclcpp std_msgs)
target_include_directories(serial_receiver_test PUBLIC "${SERIAL_INCLUDE_DIRS}")
target_link_libraries(serial_receiver_test "${SERIAL_LDFLAGS}")

install(TARGETS
  serial_transmitter_test
  serial_receiver_test
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  DESTINATION lib/${PROJECT_NAME}
)

install(PROGRAMS
  ${PROJECT_NAME}/mpu6050_driver.py
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
