cmake_minimum_required(VERSION 3.10)
project(Phase3_Week4_3D3D_Motion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})

add_executable(basic basic.cpp)
target_link_libraries(basic Eigen3::Eigen)

add_executable(quiz_easy quiz_easy.cpp)
target_link_libraries(quiz_easy Eigen3::Eigen)

add_executable(quiz_medium quiz_medium.cpp)
target_link_libraries(quiz_medium Eigen3::Eigen)

find_package(GTest QUIET)
if(GTest_FOUND)
    enable_testing()
    add_executable(test_basic test/test_basic.cpp)
    target_link_libraries(test_basic Eigen3::Eigen GTest::GTest GTest::Main)
    add_test(NAME Week4Tests COMMAND test_basic)
endif()

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 3 Week 4: 3D-3D Motion (ICP)")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
