/**
    Joy Topic Publisher
    @author Kenta Suzuki
*/

#include <cnoid/Joystick>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include <iostream>
#include <memory>
#include <string>

using namespace std::chrono_literals;

class Choreonoid2JoyNode : public rclcpp::Node
{
public:
    Choreonoid2JoyNode()
        : Node("choreonoid_joy2")
    {
        // 1. Declare parameters with default values
        this->declare_parameter<std::string>("device_name", "/dev/input/js0");
        this->declare_parameter<std::string>("topic_name", "joy");

        // 2. Retrieve parameter values
        std::string device_path = this->get_parameter("device_name").as_string();
        std::string topic_name = this->get_parameter("topic_name").as_string();

        // Initialize the Choreonoid Joystick
        joystick_ = std::make_unique<cnoid::Joystick>(device_path.c_str());

        // Create publisher using the topic name from parameters
        publisher_ = this->create_publisher<sensor_msgs::msg::Joy>(topic_name, 10);

        // Set up signal connections (callbacks for state changes)
        joystick_->sigButton().connect([this](int, bool) { state_changed_ = true; });
        joystick_->sigAxis().connect([this](int, double) { state_changed_ = true; });

        // Set up the main loop timer (60Hz / approx 16.6ms)
        timer_ = this->create_wall_timer(16.6ms, std::bind(&Choreonoid2JoyNode::update, this));

        RCLCPP_INFO(
            this->get_logger(), "Node initialized with device: %s, topic: %s", device_path.c_str(), topic_name.c_str());
    }

private:
    void update()
    {
        // Check if the joystick device is ready
        if(!joystick_->isReady()) {
            if(!joystick_->makeReady()) {
                // Throttled warning to avoid flooding the console (once every 1000ms)
                RCLCPP_WARN_THROTTLE(
                    this->get_logger(), *this->get_clock(), 1000, "Joystick is not ready. Retrying...");
                return;
            }
        }

        // Print readiness message once after initial connection
        if(is_before_initial_reading_) {
            RCLCPP_INFO(this->get_logger(), "Joystick \"%s\" is ready.", joystick_->device().c_str());
            is_before_initial_reading_ = false;
        }

        // Poll the hardware state
        joystick_->readCurrentState();

        // Only publish if a button or axis state has changed
        if(state_changed_) {
            auto joy_msg = std::make_unique<sensor_msgs::msg::Joy>();

            // Note: joy.header.seq is deprecated/removed in ROS 2
            joy_msg->header.stamp = this->now();
            joy_msg->header.frame_id = "joy";

            // Copy axis data
            int num_axes = joystick_->numAxes();
            joy_msg->axes.resize(num_axes);
            for(int i = 0; i < num_axes; ++i) {
                joy_msg->axes[i] = static_cast<float>(joystick_->getPosition(i));
            }

            // Copy button data
            int num_buttons = joystick_->numButtons();
            joy_msg->buttons.resize(num_buttons);
            for(int i = 0; i < num_buttons; ++i) {
                joy_msg->buttons[i] = joystick_->getButtonState(i) ? 1 : 0;
            }

            // Publish the message and reset the change flag
            publisher_->publish(std::move(joy_msg));
            state_changed_ = false;
        }
    }

    std::unique_ptr<cnoid::Joystick> joystick_;
    rclcpp::Publisher<sensor_msgs::msg::Joy>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    bool state_changed_ = false;
    bool is_before_initial_reading_ = true;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Choreonoid2JoyNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}