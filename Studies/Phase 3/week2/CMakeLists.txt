cmake_minimum_required(VERSION 3.10)
project(Phase3_Week2_2D2D_Motion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV 4 REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
)

add_executable(basic basic.cpp)
target_link_libraries(basic ${OpenCV_LIBS} Eigen3::Eigen)

add_executable(quiz_easy quiz_easy.cpp)
target_link_libraries(quiz_easy ${OpenCV_LIBS} Eigen3::Eigen)

add_executable(quiz_medium quiz_medium.cpp)
target_link_libraries(quiz_medium ${OpenCV_LIBS} Eigen3::Eigen)

find_package(GTest QUIET)
if(GTest_FOUND)
    enable_testing()
    add_executable(test_basic test/test_basic.cpp)
    target_link_libraries(test_basic ${OpenCV_LIBS} Eigen3::Eigen GTest::GTest GTest::Main)
    add_test(NAME Week2Tests COMMAND test_basic)
endif()

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 3 Week 2: 2D-2D Motion (Essential)")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
