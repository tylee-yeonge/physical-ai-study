cmake_minimum_required(VERSION 3.10)
project(Phase3_Week4_ICP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

add_executable(basic basic.cpp)
target_link_libraries(basic Eigen3::Eigen)

add_executable(my_basic my_basic.cpp)
target_link_libraries(my_basic Eigen3::Eigen)

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 3 Week 4: 3D-3D ICP")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
