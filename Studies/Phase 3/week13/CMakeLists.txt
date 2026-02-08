cmake_minimum_required(VERSION 3.10)
project(Phase3_Week13_Scale_Recovery)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})

add_executable(quiz_easy quiz_easy.cpp)
target_link_libraries(quiz_easy Eigen3::Eigen)

add_executable(quiz_medium quiz_medium.cpp)
target_link_libraries(quiz_medium Eigen3::Eigen)

# 실습 시뮬레이션 (PRACTICE.md의 코드)
# scale_recovery_sim.cpp를 작성한 경우 아래 주석 해제
# add_executable(scale_recovery_sim scale_recovery_sim.cpp)
# target_link_libraries(scale_recovery_sim Eigen3::Eigen)

# Solution 빌드
add_executable(easy_sol quiz_solutions/easy_sol.cpp)

add_executable(medium_sol quiz_solutions/medium_sol.cpp)
target_link_libraries(medium_sol Eigen3::Eigen)

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 3 Week 13: 스케일 복구 방법")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
