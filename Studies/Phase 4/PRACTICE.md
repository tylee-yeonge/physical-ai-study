# Phase 4: VIO 실습 (C++)

> 🎯 **목표**: IMU Pre-integration 구현 및 Visual-Inertial 융합  
> 💻 **언어**: **C++** (Eigen, Sophus)  
> 🛠️ **환경**: Ubuntu (또는 Jetson)  
> ⏰ **예상 시간**: ~90시간 (3개월)

---

## 📋 실습 개요

Phase 4는 **VIO의 핵심인 IMU Pre-integration**을 구현합니다. 이는 VINS-Fusion, ORB-SLAM3 VI 모드의 핵심 알고리즘이며, **반드시 C++**로 구현해야 합니다.

### 언어: 왜 무조건 C++인가?

| 이유 | 설명 |
|------|------|
| **VINS 코드 직접 분석** | `imu_factor.cpp` 이해 필수 |
| **Lie 대수 라이브러리** | Sophus(C++ 전용) |
| **성능** | IMU 400Hz → 실시간 적분 필요 |
| **실무 적합성** | 모든 VIO 시스템이 C++ |

---

## 🔧 실습 1: IMU 기초 이해 (1주)

### IMU 데이터 구조

```cpp
struct IMUData {
    double timestamp;
    Eigen::Vector3d acc;   // 가속도 (m/s²)
    Eigen::Vector3d gyro;  // 각속도 (rad/s)
};
```

### IMU 적분 (Naive 방법)

**imu_integrator_naive.cpp**:
```cpp
#include <Eigen/Dense>
#include <vector>

class IMUIntegratorNaive {
public:
    IMUIntegratorNaive(const Eigen::Vector3d& gravity) 
        : gravity_(gravity) {}
    
    void integrate(const std::vector<IMUData>& imu_data,
                   double dt,
                   Eigen::Vector3d& p,
                   Eigen::Vector3d& v,
                   Eigen::Quaterniond& q) {
        
        p.setZero();
        v.setZero();
        q.setIdentity();
        
        for (size_t i = 1; i < imu_data.size(); i++) {
            const auto& imu = imu_data[i];
            
            // 회전 업데이트 (각속도)
            Eigen::Vector3d omega = imu.gyro;
            double angle = omega.norm() * dt;
            Eigen::Vector3d axis = omega.normalized();
            
            Eigen::Quaterniond dq(Eigen::AngleAxisd(angle, axis));
            q = q * dq;
            q.normalize();
            
            // 가속도 (body → world)
            Eigen::Vector3d acc_world = q * imu.acc + gravity_;
            
            // 속도/위치 업데이트
            v += acc_world * dt;
            p += v * dt + 0.5 * acc_world * dt * dt;
        }
    }
    
private:
    Eigen::Vector3d gravity_;
};
```

**문제점**:
- 오차가 빠르게 누적 (몇 초 안에 발산)
- 재적분 시 처음부터 다시 계산 (비효율)

→ **Pre-integration 필요**

---

## 🔧 실습 2: IMU Pre-integration 구현 (4-5주)

### Sophus 설치

```bash
git clone https://github.com/strasdat/Sophus.git
cd Sophus
mkdir build && cd build
cmake .. -DBUILD_SOPHUS_EXAMPLES=OFF
sudo make install
```

### Pre-integration 클래스

**include/imu_preintegration.hpp**:
```cpp
#ifndef IMU_PREINTEGRATION_HPP
#define IMU_PREINTEGRATION_HPP

#include <Eigen/Dense>
#include <sophus/so3.hpp>
#include <vector>

class IMUPreintegration {
public:
    IMUPreintegration(const Eigen::Vector3d& ba, 
                      const Eigen::Vector3d& bg,
                      const Eigen::Vector3d& gravity);
    
    // IMU 데이터 추가 및 적분
    void addIMU(const Eigen::Vector3d& acc, 
                const Eigen::Vector3d& gyro,
                double dt);
    
    // Pre-integrated 값 반환
    const Eigen::Vector3d& getDeltaP() const { return delta_p_; }
    const Eigen::Vector3d& getDeltaV() const { return delta_v_; }
    const Sophus::SO3d& getDeltaR() const { return delta_R_; }
    
    // Jacobian (bias 업데이트 시 사용)
    const Eigen::Matrix3d& getJacobP_ba() const { return J_p_ba_; }
    const Eigen::Matrix3d& getJacobP_bg() const { return J_p_bg_; }
    const Eigen::Matrix3d& getJacobV_ba() const { return J_v_ba_; }
    const Eigen::Matrix3d& getJacobV_bg() const { return J_v_bg_; }
    const Eigen::Matrix3d& getJacobR_bg() const { return J_R_bg_; }
    
    // Covariance
    const Eigen::Matrix<double, 9, 9>& getCovariance() const { return cov_; }
    
    // Bias 업데이트
    void repropagate(const Eigen::Vector3d& new_ba,
                     const Eigen::Vector3d& new_bg);
    
private:
    // Bias
    Eigen::Vector3d ba_, bg_;
    
    // Gravity
    Eigen::Vector3d gravity_;
    
    // Pre-integrated measurements
    Sophus::SO3d delta_R_;
    Eigen::Vector3d delta_v_;
    Eigen::Vector3d delta_p_;
    
    // Jacobians
    Eigen::Matrix3d J_R_bg_;
    Eigen::Matrix3d J_v_ba_, J_v_bg_;
    Eigen::Matrix3d J_p_ba_, J_p_bg_;
    
    // Covariance
    Eigen::Matrix<double, 9, 9> cov_;
    
    // Noise parameters
    double n_a_, n_g_;      // 측정 노이즈
    double n_ba_, n_bg_;    // Bias random walk
    
    // 버퍼 (repropagate용)
    struct IMUMeasurement {
        Eigen::Vector3d acc;
        Eigen::Vector3d gyro;
        double dt;
    };
    std::vector<IMUMeasurement> imu_buffer_;
};

#endif
```

### 구현 (핵심 부분)

**src/imu_preintegration.cpp**:
```cpp
#include "../include/imu_preintegration.hpp"

IMUPreintegration::IMUPreintegration(
    const Eigen::Vector3d& ba,
    const Eigen::Vector3d& bg,
    const Eigen::Vector3d& gravity)
    : ba_(ba), bg_(bg), gravity_(gravity),
      delta_R_(Sophus::SO3d()), delta_v_(Eigen::Vector3d::Zero()),
      delta_p_(Eigen::Vector3d::Zero()) {
    
    // Jacobian 초기화
    J_R_bg_.setZero();
    J_v_ba_.setZero();
    J_v_bg_.setZero();
    J_p_ba_.setZero();
    J_p_bg_.setZero();
    
    // Covariance 초기화
    cov_.setZero();
    
    // Noise (EuRoC 기준 예시)
    n_a_ = 2e-3;    // m/s²/√Hz
    n_g_ = 1.6e-4;  // rad/s/√Hz
    n_ba_ = 3e-3;   // m/s³/√Hz
    n_bg_ = 1.9e-5; // rad/s²/√Hz
}

void IMUPreintegration::addIMU(const Eigen::Vector3d& acc,
                                const Eigen::Vector3d& gyro,
                                double dt) {
    
    // 버퍼에 저장 (repropagate용)
    imu_buffer_.push_back({acc, gyro, dt});
    
    // Bias 보정
    Eigen::Vector3d acc_unbias = acc - ba_;
    Eigen::Vector3d gyro_unbias = gyro - bg_;
    
    // 1. Rotation update
    Sophus::SO3d dR = Sophus::SO3d::exp(gyro_unbias * dt);
    Eigen::Matrix3d R_prev = delta_R_.matrix();
    
    delta_R_ = delta_R_ * dR;
    
    // 2. Velocity update
    delta_v_ += delta_R_.matrix() * acc_unbias * dt;
    
    // 3. Position update
    delta_p_ += delta_v_ * dt + 0.5 * delta_R_.matrix() * acc_unbias * dt * dt;
    
    // 4. Jacobian update (right perturbation)
    // J_R_bg update
    Eigen::Matrix3d Jr_inv = Sophus::SO3d::JacobianRInv(gyro_unbias * dt);
    J_R_bg_ = dR.inverse().matrix() * J_R_bg_ - Jr_inv * dt;
    
    // J_v_ba, J_v_bg update
    J_v_ba_ -= R_prev * dt;
    J_v_bg_ -= R_prev * Sophus::SO3d::hat(acc_unbias) * J_R_bg_ * dt;
    
    // J_p_ba, J_p_bg update
    J_p_ba_ += J_v_ba_ * dt - 0.5 * R_prev * dt * dt;
    J_p_bg_ += J_v_bg_ * dt - 0.5 * R_prev * Sophus::SO3d::hat(acc_unbias) * J_R_bg_ * dt * dt;
    
    // 5. Covariance update (F, G matrix)
    Eigen::Matrix<double, 9, 9> F = Eigen::Matrix<double, 9, 9>::Identity();
    Eigen::Matrix<double, 9, 6> G;
    G.setZero();
    
    // F 행렬 (상태 전이)
    F.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity() * dt;  // p <- v
    F.block<3, 3>(3, 6) = -R_prev * Sophus::SO3d::hat(acc_unbias) * dt;  // v <- R
    F.block<3, 3>(6, 6) = dR.inverse().matrix();  // R <- R
    
    // G 행렬 (노이즈)
    G.block<3, 3>(3, 0) = -R_prev * dt;  // v <- acc noise
    G.block<3, 3>(6, 3) = -Jr_inv * dt;  // R <- gyro noise
    
    // Noise covariance
    Eigen::Matrix<double, 6, 6> Q;
    Q.setZero();
    Q.block<3, 3>(0, 0) = n_a_ * n_a_ / dt * Eigen::Matrix3d::Identity();
    Q.block<3, 3>(3, 3) = n_g_ * n_g_ / dt * Eigen::Matrix3d::Identity();
    
    cov_ = F * cov_ * F.transpose() + G * Q * G.transpose();
}

void IMUPreintegration::repropagate(const Eigen::Vector3d& new_ba,
                                     const Eigen::Vector3d& new_bg) {
    
    // Bias 변화량
    Eigen::Vector3d dba = new_ba - ba_;
    Eigen::Vector3d dbg = new_bg - bg_;
    
    // First-order correction (VINS 방식)
    delta_p_ += J_p_ba_ * dba + J_p_bg_ * dbg;
    delta_v_ += J_v_ba_ * dba + J_v_bg_ * dbg;
    delta_R_ = delta_R_ * Sophus::SO3d::exp(J_R_bg_ * dbg);
    
    // Bias 업데이트
    ba_ = new_ba;
    bg_ = new_bg;
    
    // (선택) 정확한 재적분: imu_buffer_ 사용해서 처음부터 다시 적분
}
```

### 테스트

**test_preintegration.cpp**:
```cpp
int main() {
    // EuRoC 데이터셋 로드
    std::vector<IMUData> imu_data = loadEuRoCIMU("/path/to/euroc");
    
    Eigen::Vector3d ba(0.1, -0.05, 0.02);  // 초기 bias
    Eigen::Vector3d bg(0.01, 0.005, -0.008);
    Eigen::Vector3d gravity(0, 0, -9.81);
    
    IMUPreintegration preint(ba, bg, gravity);
    
    // 두 키프레임 사이 IMU 적분
    double t_start = imu_data[0].timestamp;
    double t_end = t_start + 0.1;  // 0.1초
    
    for (const auto& imu : imu_data) {
        if (imu.timestamp < t_start) continue;
        if (imu.timestamp > t_end) break;
        
        double dt = 0.005;  // 200Hz IMU
        preint.addIMU(imu.acc, imu.gyro, dt);
    }
    
    // 결과 출력
    std::cout << "Delta P:\n" << preint.getDeltaP() << std::endl;
    std::cout << "Delta V:\n" << preint.getDeltaV() << std::endl;
    std::cout << "Delta R:\n" << preint.getDeltaR().matrix() << std::endl;
    
    // Bias 업데이트 테스트
    Eigen::Vector3d new_ba(0.12, -0.04, 0.025);
    Eigen::Vector3d new_bg(0.011, 0.006, -0.007);
    
    preint.repropagate(new_ba, new_bg);
    
    std::cout << "\nAfter bias update:" << std::endl;
    std::cout << "Delta P:\n" << preint.getDeltaP() << std::endl;
    
    return 0;
}
```

---

## 🔧 실습 3: VIO Factor Graph (3-4주)

### Ceres로 VIO 구현

**imu_factor.hpp**:
```cpp
#include <ceres/ceres.h>
#include <Eigen/Dense>
#include "../include/imu_preintegration.hpp"

struct IMUFactor {
    IMUFactor(const IMUPreintegration& preint,
             const Eigen::Vector3d& gravity)
        : preint_(preint), gravity_(gravity) {}
    
    template <typename T>
    bool operator()(const T* const p_i,      // 위치 i
                    const T* const q_i,      // 회전 i (쿼터니언)
                    const T* const v_i,      // 속도 i
                    const T* const ba_i,     // bias_a i
                    const T* const bg_i,     // bias_g i
                    const T* const p_j,      // 위치 j
                    const T* const q_j,      // 회전 j
                    const T* const v_j,      // 속도 j
                    const T* const ba_j,     // bias_a j
                    const T* const bg_j,     // bias_g j
                    T* residuals) const {
        
        // Convert to Eigen (제대로된 구현은 매우 복잡함)
        // VINS imu_factor.cpp 참고
        
        // Residual: [r_p, r_v, r_R, r_ba, r_bg]
        // ...
        
        return true;
    }
    
    static ceres::CostFunction* Create(
        const IMUPreintegration& preint,
        const Eigen::Vector3d& gravity) {
        
        return new ceres::AutoDiffCostFunction<IMUFactor, 15, 3, 4, 3, 3, 3, 3, 4, 3, 3, 3>(
            new IMUFactor(preint, gravity));
    }
    
private:
    const IMUPreintegration& preint_;
    Eigen::Vector3d gravity_;
};
```

**VIO 최적화 실행**:
```cpp
ceres::Problem problem;

// 파라미터 블록
std::vector<std::vector<double>> positions(num_frames, std::vector<double>(3));
std::vector<std::vector<double>> rotations(num_frames, std::vector<double>(4));
std::vector<std::vector<double>> velocities(num_frames, std::vector<double>(3));
std::vector<double> ba(3), bg(3);

// IMU Factor 추가
for (int i = 0; i < num_frames - 1; i++) {
    ceres::CostFunction* imu_cost = 
        IMUFactor::Create(preint_[i], gravity);
    
    problem.AddResidualBlock(
        imu_cost, nullptr,
        positions[i].data(), rotations[i].data(), velocities[i].data(),
        ba.data(), bg.data(),
        positions[i+1].data(), rotations[i+1].data(), velocities[i+1].data(),
        ba.data(), bg.data()
    );
}

// Visual Factor 추가 (Phase 3의 RePro jectionError)
// ...

// Solve
ceres::Solver::Options options;
options.linear_solver_type = ceres::SPARSE_SCHUR;
options.max_num_iterations = 50;

ceres::Solver::Summary summary;
ceres::Solve(options, &problem, &summary);
```

---

## ✅ 체크리스트

### IMU Pre-integration
- [ ] Naive 적분 구현 및 발산 확인
- [ ] Pre-integration 클래스 구현
- [ ] Jacobian 계산 구현
- [ ] Repropagate 구현
- [ ] EuRoC 데이터로 테스트

### VIO Factor
- [ ] Ceres로 IMU Factor 구현
- [ ] Visual + IMU 결합 최적화
- [ ] Bias 추정 정확도 확인

---

## 📚 참고 자료

- [VINS-Fusion GitHub](https://github.com/HKUST-Aerial-Robotics/VINS-Fusion) - `imu_factor.cpp` 필독
- [Sophus Tutorial](https://github.com/strasdat/Sophus/blob/master/sophus/so3.hpp)
- Forster et al., "IMU Preintegration on Manifold" (논문)

---

**다음 단계**: Phase 7 - Detection/Depth (딥러닝 - Python)
