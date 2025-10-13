choreonoid_add_simple_controller(
    JoyTopicSubscriberController JoyTopicSubscriberController.cpp)

ament_target_dependencies(
    JoyTopicSubscriberController rclcpp sensor_msgs)