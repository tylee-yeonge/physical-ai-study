# Ceres Solver 곡선 피팅 예제


이 예제는 Ceres Solver를 사용하여 `y = a·e^(b·x)` 형태의 지수 함수 곡선을 피팅합니다.


## 빌드 및 실행


### 1. Ceres Solver 설치 확인


```bash
# macOS
brew install ceres-solver


# Ubuntu
sudo apt-get install libceres-dev
```


### 2. 빌드


```bash
mkdir build && cd build
cmake ..
make
```


### 3. 실행


```bash
./curve_fitting
```


## 예상 출력


```
초기값: a = 1, b = 0.1


iter cost cost_change |gradient| |step| tr_ratio tr_radius ls_iter iter_time total_time
   0 9.842456e+02 0.00e+00 1.23e+03 0.00e+00 0.00e+00 1.00e+04 0 1.23e-04 2.45e-04
   ...


Ceres Solver Report: Iterations: 11, Initial cost: 9.842e+02, Final cost: 1.235e-02


최적화 결과:
  a = 2.498 (실제: 2.5)
  b = 0.301 (실제: 0.3)
```


## 주요 개념


- **AutoDiffCostFunction**: 자동 미분을 사용하여 Jacobian 계산
- **Residual**: 측정값 - 예측값
- **Solver Options**: `DENSE_QR` 선형 솔버, 100회 최대 반복
- **Levenberg-Marquardt**: 기본 trust region 전략
