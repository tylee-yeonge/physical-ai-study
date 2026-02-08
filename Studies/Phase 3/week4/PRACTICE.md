# Week 4 실습: ICP (Iterative Closest Point) 구현

> 🎯 **목표**: SVD 기반 Point-to-Point ICP 구현 및 수렴 관찰
> 💻 **언어**: C++ (Eigen, OpenCV)
> ⏰ **예상 시간**: 8시간
> 💡 **참고**: VIO에서는 사용 안 하지만, LiDAR SLAM / AMR에서 핵심

---

## 📋 실습 개요

합성 Point Cloud를 만들고, SVD 기반 ICP로 정합하는 과정을 구현합니다. 반복마다 오차가 줄어드는 과정을 직접 관찰합니다.

---

## 🔧 환경 설정

```bash
# Eigen3 필요
pkg-config --modversion eigen3

# OpenCV는 시각화용 (선택)
```

---

## 프로젝트 구조

```
week4_icp/
├── CMakeLists.txt
└── src/
    └── main.cpp
```

---

## Step 1: SVD 기반 정합 + ICP 반복

### src/main.cpp

```cpp
#include <Eigen/Dense>
#include <Eigen/SVD>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>

/**
 * SVD 기반 Point-to-Point 정합
 *
 * 대응점이 주어졌을 때 R, t를 closed-form으로 구함
 * 핵심: H = Σ (p'_i)(q'_i)^T → SVD → R = V * U^T
 */
void computeAlignment(
    const std::vector<Eigen::Vector3d>& src,
    const std::vector<Eigen::Vector3d>& tgt,
    Eigen::Matrix3d& R,
    Eigen::Vector3d& t
) {
    int n = (int)src.size();

    // 1. 중심점 계산
    Eigen::Vector3d p_mean = Eigen::Vector3d::Zero();
    Eigen::Vector3d q_mean = Eigen::Vector3d::Zero();
    for (int i = 0; i < n; i++) {
        p_mean += src[i];
        q_mean += tgt[i];
    }
    p_mean /= n;
    q_mean /= n;

    // 2. Covariance 행렬 H
    Eigen::Matrix3d H = Eigen::Matrix3d::Zero();
    for (int i = 0; i < n; i++) {
        H += (src[i] - p_mean) * (tgt[i] - q_mean).transpose();
    }

    // 3. SVD 분해
    Eigen::JacobiSVD<Eigen::Matrix3d> svd(
        H, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Eigen::Matrix3d U = svd.matrixU();
    Eigen::Matrix3d V = svd.matrixV();

    // 4. R, t 계산
    R = V * U.transpose();

    // Reflection 보정
    if (R.determinant() < 0) {
        V.col(2) *= -1;
        R = V * U.transpose();
    }

    t = q_mean - R * p_mean;
}

/**
 * 최근접 이웃 찾기 (Naive, O(n*m))
 *
 * 실무에서는 K-D Tree를 사용하지만, 개념 이해를 위해 brute-force
 */
void findCorrespondences(
    const std::vector<Eigen::Vector3d>& src,
    const std::vector<Eigen::Vector3d>& tgt,
    std::vector<int>& correspondences
) {
    correspondences.resize(src.size());

    for (size_t i = 0; i < src.size(); i++) {
        double min_dist = 1e10;
        int min_idx = 0;

        for (size_t j = 0; j < tgt.size(); j++) {
            double dist = (src[i] - tgt[j]).squaredNorm();
            if (dist < min_dist) {
                min_dist = dist;
                min_idx = (int)j;
            }
        }
        correspondences[i] = min_idx;
    }
}

/**
 * 평균 오차 계산
 */
double computeError(
    const std::vector<Eigen::Vector3d>& src,
    const std::vector<Eigen::Vector3d>& tgt,
    const std::vector<int>& correspondences
) {
    double total = 0;
    for (size_t i = 0; i < src.size(); i++) {
        total += (src[i] - tgt[correspondences[i]]).squaredNorm();
    }
    return std::sqrt(total / src.size());
}

int main() {
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "Week 4: ICP 실습" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    // ═══════════════════════════════════
    // 1. 합성 데이터 생성
    // ═══════════════════════════════════

    std::default_random_engine gen(42);
    std::normal_distribution<double> noise(0.0, 0.01);

    // 타겟 Point Cloud (고정)
    std::vector<Eigen::Vector3d> target;
    for (int i = 0; i < 100; i++) {
        double x = (rand() % 100 - 50) / 10.0;
        double y = (rand() % 100 - 50) / 10.0;
        double z = (rand() % 100 - 50) / 10.0;
        target.push_back(Eigen::Vector3d(x, y, z));
    }

    // Ground truth 변환
    double angle = 15.0 * M_PI / 180.0;  // 15도 회전
    Eigen::Matrix3d R_gt;
    R_gt << cos(angle), -sin(angle), 0,
            sin(angle),  cos(angle), 0,
            0,           0,          1;
    Eigen::Vector3d t_gt(0.5, 0.3, -0.2);

    // 소스 = R * target + t + noise
    std::vector<Eigen::Vector3d> source;
    for (const auto& pt : target) {
        Eigen::Vector3d transformed = R_gt * pt + t_gt;
        transformed += Eigen::Vector3d(noise(gen), noise(gen), noise(gen));
        source.push_back(transformed);
    }

    std::cout << "데이터:" << std::endl;
    std::cout << "  타겟: " << target.size() << "개 점" << std::endl;
    std::cout << "  소스: " << source.size() << "개 점" << std::endl;
    std::cout << "  GT 회전: " << angle * 180.0 / M_PI << "도" << std::endl;
    std::cout << "  GT 이동: [" << t_gt.transpose() << "]\n" << std::endl;

    // ═══════════════════════════════════
    // 2. ICP 반복
    // ═══════════════════════════════════

    std::cout << "ICP 반복 시작...\n" << std::endl;

    std::vector<Eigen::Vector3d> current = source;
    Eigen::Matrix3d R_total = Eigen::Matrix3d::Identity();
    Eigen::Vector3d t_total = Eigen::Vector3d::Zero();

    int max_iter = 30;
    double prev_error = 1e10;

    for (int iter = 0; iter < max_iter; iter++) {
        // Step 1: 대응 찾기
        std::vector<int> correspondences;
        findCorrespondences(current, target, correspondences);

        // Step 2: 오차 계산
        double error = computeError(current, target, correspondences);

        std::cout << "  Iteration " << iter
                  << ": error = " << error;

        if (iter > 0) {
            double improvement = (prev_error - error) / prev_error * 100;
            std::cout << " (" << improvement << "% 개선)";
        }
        std::cout << std::endl;

        // Step 3: 수렴 체크
        if (std::abs(prev_error - error) < 1e-6) {
            std::cout << "\n  수렴! (iteration " << iter << ")" << std::endl;
            break;
        }
        prev_error = error;

        // Step 4: 대응점으로 R, t 계산
        std::vector<Eigen::Vector3d> matched_tgt;
        for (int idx : correspondences) {
            matched_tgt.push_back(target[idx]);
        }

        Eigen::Matrix3d R_step;
        Eigen::Vector3d t_step;
        computeAlignment(current, matched_tgt, R_step, t_step);

        // Step 5: 소스 변환
        for (auto& pt : current) {
            pt = R_step * pt + t_step;
        }

        // 누적 변환
        R_total = R_step * R_total;
        t_total = R_step * t_total + t_step;
    }

    // ═══════════════════════════════════
    // 3. 결과 비교
    // ═══════════════════════════════════

    // ICP가 추정한 R, t의 역변환 = GT의 근사
    Eigen::Matrix3d R_est_inv = R_total.transpose();
    Eigen::Vector3d t_est_inv = -R_total.transpose() * t_total;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "결과 비교" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    double rot_error = (R_gt - R_est_inv).norm();
    double trans_error = (t_gt - t_est_inv).norm();

    std::cout << "회전 오차 (Frobenius): " << rot_error << std::endl;
    std::cout << "이동 오차: " << trans_error << " m" << std::endl;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "SLAM에서의 의미:" << std::endl;
    std::cout << "  - LiDAR SLAM: 매 스캔마다 ICP로 포즈 추정" << std::endl;
    std::cout << "  - AMR: 2D LiDAR scan matching에 사용" << std::endl;
    std::cout << "  - VIO에서는 3D-2D (PnP)를 사용 (Week 3)" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

    return 0;
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(Week4_ICP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Executable
add_executable(icp_demo src/main.cpp)
target_link_libraries(icp_demo Eigen3::Eigen)
```

---

## 빌드 및 실행

```bash
cd week4_icp
mkdir build && cd build
cmake ..
make
./icp_demo
```

### 예상 출력

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Week 4: ICP 실습
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

데이터:
  타겟: 100개 점
  소스: 100개 점
  GT 회전: 15도
  GT 이동: [ 0.5  0.3 -0.2]

ICP 반복 시작...

  Iteration 0: error = 1.234
  Iteration 1: error = 0.345 (72% 개선)
  Iteration 2: error = 0.087 (75% 개선)
  Iteration 3: error = 0.019 (78% 개선)
  Iteration 4: error = 0.012 (37% 개선)

  수렴! (iteration 5)

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
결과 비교
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
회전 오차 (Frobenius): 0.0031
이동 오차: 0.0018 m

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
SLAM에서의 의미:
  - LiDAR SLAM: 매 스캔마다 ICP로 포즈 추정
  - AMR: 2D LiDAR scan matching에 사용
  - VIO에서는 3D-2D (PnP)를 사용 (Week 3)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## ✅ 체크리스트

- [ ] SVD 기반 정합 구현 및 확인
- [ ] ICP 반복 수렴 관찰 (보통 5-10회)
- [ ] 회전/이동 오차가 0에 가까운지 확인
- [ ] "ICP와 PnP의 차이"를 설명할 수 있음
- [ ] "LiDAR SLAM에서 ICP의 역할"을 설명할 수 있음

---

## 💡 핵심 포인트

1. **ICP = 반복적 정합**: 대응 찾기 → SVD → 변환 → 반복
2. **VIO에서는 안 씀**: 카메라는 2D 이미지이므로 PnP (3D-2D) 사용
3. **AMR에서는 핵심**: 2D LiDAR scan matching = 2D 버전 ICP

---

**다음**: [Week 5 - Mini VO 프로젝트](../week5/README.md)
