cmake_minimum_required(VERSION 3.10)
project(Phase3_Week12_Scale_Ambiguity)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})

add_executable(basic basic.cpp)
target_link_libraries(basic Eigen3::Eigen)

add_executable(my_basic my_basic.cpp)
target_link_libraries(my_basic Eigen3::Eigen)

add_executable(quiz_easy quiz_easy.cpp)
target_link_libraries(quiz_easy Eigen3::Eigen)

add_executable(quiz_medium quiz_medium.cpp)
target_link_libraries(quiz_medium Eigen3::Eigen)

add_executable(scale_drift_sim scale_drift_sim.cpp)
target_link_libraries(scale_drift_sim Eigen3::Eigen)

add_executable(easy_sol quiz_solutions/easy_sol.cpp)

add_executable(medium_sol quiz_solutions/medium_sol.cpp)
target_link_libraries(medium_sol Eigen3::Eigen)

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 3 Week 12: Monocular 스케일 모호성")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
