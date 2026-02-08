cmake_minimum_required(VERSION 3.10)
project(Phase3_Week11_Ceres_Practice)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)
find_package(Ceres REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR} ${CERES_INCLUDE_DIRS})

add_executable(quiz_easy quiz_easy.cpp)
target_link_libraries(quiz_easy Eigen3::Eigen)

add_executable(quiz_medium quiz_medium.cpp)
target_link_libraries(quiz_medium Eigen3::Eigen)

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 3 Week 11: Ceres 실습")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
