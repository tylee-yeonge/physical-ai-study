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
        // 토크 한계 — STS3215 는 present current / present load 로 부하 감시
        // (정격 대비 여유를 두고 실측으로 캘리브레이션; 값은 v2.5 실기 데이터 기준 재산정)
        torque_limits_ = std::vector<double>(6, 2.0);


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
        // e-stop 이 걸려 있는 동안은 상위 명령을 전부 버린다
        if (estop_active_) return;
        if (check_safety(*cmd)) {
            safe_pub_->publish(*cmd);
        }
    }


    void on_estop(std_msgs::msg::Bool::SharedPtr msg) {
        estop_active_ = msg->data;
        if (!estop_active_) return;
        // 토크는 끄지 않는다 — 풀리면 팔이 자중으로 낙하한다.
        // 현재 위치를 목표로 한 번 써서 그 자리에 세운다 (이후 명령은 on_cmd 가 버린다)
        JointState hold;
        hold.name = joint_names_;
        hold.position = current_position_;
        safe_pub_->publish(hold);
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


**동작 정의: 토크를 유지한 채 현 위치 정지.** `/emergency_stop` 이 true 면 인터록이 ① 상위 명령을 전부 버리고 ② 현재 위치를 목표로 한 번 써서 팔을 그 자리에 세운다 (위 `on_estop`). Stage 1 의 소프트웨어 정지 (키 또는 ROS2 서비스 호출) 를 C++ 인터록이 인수한 형태다.


- **토크 OFF 는 정지 동작으로 쓰지 않는다** — 토크가 풀리면 팔이 자중으로 떨어진다. DC 차단도 결과가 같아서 정지 수단이 아니고, 물리 전원 차단 스위치는 두지 않는다 ([../BOM.md](../BOM.md)).
- **명령을 버리기만 해서는 멈추지 않는다** — 위치 제어는 마지막으로 받은 목표를 계속 따라가므로, 이동 중에 e-stop 이 걸리면 팔이 그 목표까지 간다. 그래서 현재 위치를 목표로 덮어쓴다.
- **최후 수단은 USB 분리** — 인터록 노드나 PC 가 죽어 토픽이 통하지 않을 때 쓴다. USB 를 뽑으면 통신이 끊기고 팔로워가 현재 위치에서 멈춘다 ([조립 가이드](../spike/week1/so-arm101-assembly-guide.md) §7).


입력은 Hardware button (RPi GPIO) 또는 keyboard (test) 다. 버튼은 같은 토픽을 발행하는 입력일 뿐 전원을 끊지 않는다:
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
- [ ] e-stop 동작 — 이동 중에 걸어도 토크를 유지한 채 그 자리에 정지 (낙하 없음), 걸려 있는 동안 명령 차단
- [ ] 충돌 감지 동작
- [ ] 오버헤드 ~ 1 ms 측정
