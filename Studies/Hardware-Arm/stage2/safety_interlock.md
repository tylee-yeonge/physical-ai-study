# Hardware-Arm Stage 2 - 안전 인터록 (C++)


> 2027.04


---


## C++ 안전 인터록 노드


### Phase 7 week 4-6 의 사전 작업


본 Stage 2 의 마무리 = Phase 7 의 안전 노드의 토대.


```cpp
// safety_node.cpp (간략)
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "std_msgs/msg/bool.hpp"


class SafetyNode : public rclcpp::Node {
public:
    SafetyNode() : Node("safety_node") {
        // 위치 한계 (URDF 의 limit 그대로)
        position_limits_ = {
            {-1.57, 1.57}, {-1.57, 1.57}, {-1.57, 1.57},
            {-3.14, 3.14}, {-3.14, 3.14}, {-3.14, 3.14}
        };
        // 속도 한계 (rad/s)
        velocity_limits_ = std::vector<double>(6, 3.14);
        // 토크 한계 (Nm)
        torque_limits_ = std::vector<double>(6, 2.0); // XM430 80%


        cmd_sub_ = create_subscription<JointState>(
            "/inference/joint_command", 10,
            std::bind(&SafetyNode::on_cmd, this, _1));
        state_sub_ = create_subscription<JointState>(
            "/joint_states", 10,
            std::bind(&SafetyNode::on_state, this, _1));
        estop_sub_ = create_subscription<std_msgs::msg::Bool>(
            "/emergency_stop", 10,
            std::bind(&SafetyNode::on_estop, this, _1));


        safe_pub_ = create_publisher<JointState>("/joint_command", 10);
    }


    bool check_safety(const JointState& cmd) {
        // Position
        for (int i = 0; i < 6; i++) {
            if (cmd.position[i] < position_limits_[i].first ||
                cmd.position[i] > position_limits_[i].second) {
                RCLCPP_WARN(this->get_logger(), "Position violation");
                return false;
            }
        }
        // Velocity
        for (int i = 0; i < 6; i++) {
            double vel = (cmd.position[i] - current_position_[i]) / dt_;
            if (std::abs(vel) > velocity_limits_[i]) {
                RCLCPP_WARN(this->get_logger(), "Velocity violation");
                return false;
            }
        }
        // Torque (from current state)
        for (int i = 0; i < 6; i++) {
            if (std::abs(current_effort_[i]) > torque_limits_[i]) {
                RCLCPP_WARN(this->get_logger(), "Torque violation");
                return false;
            }
        }
        return true;
    }


    void on_cmd(JointState::SharedPtr cmd) {
        if (estop_active_) return;
        if (check_safety(*cmd)) {
            safe_pub_->publish(*cmd);
        }
    }
    // ...
};
```


---


## 충돌 감지


```cpp
bool collision_detected(JointState state) {
    for (int i = 0; i < 6; i++) {
        double delta = std::abs(state.effort[i] - prev_effort_[i]);
        if (delta > collision_threshold_[i]) {
            return true; // 1ms 토크 급증
        }
    }
    return false;
}
```


---


## e-stop


Hardware button (RPi GPIO) 또는 keyboard (test):
```python
# e-stop publisher
ros2 topic pub /emergency_stop std_msgs/Bool "data: true"
```


---


## 오버헤드 측정


```cpp
auto t0 = this->now();
bool safe = check_safety(*cmd);
auto t1 = this->now();
double overhead = (t1 - t0).seconds() * 1000; // ms
// 기대 ~ 1 ms
```


---


## 체크리스트
- [ ] C++ 안전 노드 빌드
- [ ] 위치/속도/토크 한계 동작
- [ ] e-stop 동작
- [ ] 충돌 감지 동작
- [ ] 오버헤드 ~ 1 ms 측정
