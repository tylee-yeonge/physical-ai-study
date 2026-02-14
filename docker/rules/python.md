# Python (Google Style + Black Formatter)

## 포맷터
- Black formatter 사용 (line-length: 88)
- Black과 충돌하는 수동 포맷팅 하지 않기

## 주석 (Docstring)
- 모든 함수/클래스에 Google style docstring 필수
- 형식: Args, Returns, Raises 섹션
- 주석 언어: 한국어
- 예시:
  ```python
  def project_point(point_3d, K):
      """3D 월드 좌표를 2D 픽셀 좌표로 투영한다.

      Args:
          point_3d: 월드 좌표계의 3D 점 [X, Y, Z]
          K: 3x3 카메라 내부 파라미터 행렬

      Returns:
          픽셀 좌표 [u, v]
      """
  ```

## Google Python Style 따르는 항목
- 네이밍: snake_case(변수/함수), CamelCase(클래스), UPPER_CASE(상수)
- type hints 적극 사용
- **import 문은 항상 파일 맨 상단에 위치** (함수/클래스 내부에서 import 금지)
- import 순서: 표준 라이브러리 → 서드파티 → 로컬 (각 그룹 사이 빈 줄)
- `from` import 시 쉼표로 나열하지 않고 한 줄에 하나씩 작성
  ```python
  # Good - 파일 상단에 모든 import
  import os
  import sys
  from typing import Optional
  from typing import List
  from typing import Dict

  import rclpy
  from rclpy.node import Node

  from .utils import helper_function


  def process_data():
      return os.getcwd()

  # Bad - 함수 내부에서 import
  def process_data():
      import os  # 금지!
      return os.getcwd()
  ```

## 기타
- Python 3.8+ 기준
- f-string 사용 (.format() 대신)
