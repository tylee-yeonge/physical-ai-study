# Week 4 실습: 안전 인터록 C++ 노드


```cpp
// safety_node.cpp
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"


class SafetyNode : public rclcpp::Node {
public:
    SafetyNode() : Node("safety_node") {
        cmd_sub_ = create_subscription<sensor_msgs::msg::JointState>(
            "/inference/joint_command", 10,
            std::bind(&SafetyNode::on_cmd, this, _1));
        state_sub_ = create_subscription<sensor_msgs::msg::JointState>(
            "/joint_states", 10,
            std::bind(&SafetyNode::on_state, this, _1));
        safe_pub_ = create_publisher<sensor_msgs::msg::JointState>(
            "/joint_command", 10);
    }


private:
    void on_cmd(JointState::SharedPtr cmd) {
        if (check_safety(cmd, current_)) {
            safe_pub_->publish(*cmd);
        } else {
            RCLCPP_WARN(this->get_logger(), "Safety violation");
        }
    }
    // ...
};
```


체크리스트:
- [ ] C++ 노드 빌드
- [ ] 3 한계 동작
- [ ] 오버헤드 ~ 1 ms 측정
