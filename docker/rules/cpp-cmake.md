# C++ / CMake (Google C++ Style 기반)

## Google Style과 다른 점
- **들여쓰기: 4칸 스페이스** (Google 기본은 2칸이지만 4칸 사용)
- **중괄호: Allman 스타일** (여는 중괄호를 다음 줄에 배치)
  ```cpp
  // Good (Allman)
  void foo()
  {
      ...
  }

  // Bad (K&R)
  void foo() {
      ...
  }
  ```

## 주석 (Doxygen)
- 헤더 파일의 클래스, 함수에 Doxygen 형식 주석 필수
- 형식: @brief, @param, @return, @note
- 주석 언어: 한국어
- 예시:
  ```cpp
  /**
   * @brief 3D 월드 좌표를 2D 픽셀 좌표로 투영
   * @param P_world 월드 좌표계의 3D 점
   * @return 픽셀 좌표 (u, v). 카메라 뒤면 (-1, -1)
   */
  ```

## Google Style 따르는 항목
- 헤더 가드: #ifndef 방식 (PROJECT_PATH_FILE_H_)
- 네이밍: snake_case(변수/함수), CamelCase(클래스), kConstant(상수)
- 멤버 변수: trailing underscore (value_)
- const 참조 적극 사용 (불필요한 복사 방지)
- auto: 타입이 명확할 때만 (가독성 우선)
- include 순서: 관련 헤더 → C 시스템 → C++ 표준 → 서드파티 → 프로젝트
- 함수 길이는 짧게 유지 (한 가지 역할만)

## C++ 표준
- C++17 사용

## CMake
- cmake_minimum_required(VERSION 3.10) 이상
- target_link_libraries에서 PUBLIC/PRIVATE 구분
- find_package로 의존성 관리
- 컴파일 경고: -Wall -Wextra

## 빌드/테스트
- 빌드: mkdir -p build && cd build && cmake .. && make
- 테스트 프레임워크: Google Test
