cmake_minimum_required(VERSION 3.10)
project(Phase2_Week7_PnP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(basic basic.cpp)
target_link_libraries(basic ${OpenCV_LIBS})

add_executable(quiz_easy quiz_easy.cpp)
target_link_libraries(quiz_easy ${OpenCV_LIBS})

add_executable(quiz_medium quiz_medium.cpp)
target_link_libraries(quiz_medium ${OpenCV_LIBS})

find_package(GTest QUIET)
if(GTest_FOUND)
    enable_testing()
    add_executable(test_basic test/test_basic.cpp)
    target_link_libraries(test_basic ${OpenCV_LIBS} GTest::GTest GTest::Main)
    add_test(NAME Week7Tests COMMAND test_basic)
endif()

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 2 Week 7: PnP & Visual Odometry")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
