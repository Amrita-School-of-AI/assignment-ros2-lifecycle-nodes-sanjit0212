#include <chrono>
#include <memory>
#include <random>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "std_msgs/msg/float64.hpp"

using namespace std::chrono_literals;
using rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface;

class LifecycleSensor : public rclcpp_lifecycle::LifecycleNode
{
public:
  LifecycleSensor()
  : rclcpp_lifecycle::LifecycleNode("lifecycle_sensor")
  {
    gen_.seed(rd_());
    dist_ = std::uniform_real_distribution<double>(0.0, 100.0);
  }

  LifecycleNodeInterface::CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    publisher_ = this->create_publisher<std_msgs::msg::Float64>("/sensor_data", 10);
    RCLCPP_INFO(get_logger(), "Sensor configured");
    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  LifecycleNodeInterface::CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
  {
    publisher_->on_activate();
    timer_ = this->create_wall_timer(
      500ms, std::bind(&LifecycleSensor::timer_callback, this));
    RCLCPP_INFO(get_logger(), "Sensor activated");
    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  LifecycleNodeInterface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override
  {
    publisher_->on_deactivate();
    timer_.reset(); // Stop the timer so it doesn't publish
    RCLCPP_INFO(get_logger(), "Sensor deactivated");
    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  LifecycleNodeInterface::CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override
  {
    timer_.reset();
    publisher_.reset();
    RCLCPP_INFO(get_logger(), "Sensor cleaned up");
    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  LifecycleNodeInterface::CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override
  {
    timer_.reset();
    publisher_.reset();
    RCLCPP_INFO(get_logger(), "Sensor shutting down");
    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

private:
  void timer_callback()
  {
    if (publisher_->is_activated()) {
      auto message = std_msgs::msg::Float64();
      message.data = dist_(gen_);
      RCLCPP_INFO(get_logger(), "Publishing sensor data: %.2f", message.data);
      publisher_->publish(message);
    }
  }

  std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::Float64>> publisher_;
  std::shared_ptr<rclcpp::TimerBase> timer_;

  std::random_device rd_;
  std::mt19937 gen_;
  std::uniform_real_distribution<double> dist_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<LifecycleSensor>();
  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}
