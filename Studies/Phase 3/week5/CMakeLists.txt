cmake_minimum_required(VERSION 3.10)
project(Phase3_Week5_MiniVO)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV 4 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(basic basic.cpp)
target_link_libraries(basic ${OpenCV_LIBS})

add_executable(my_basic my_basic.cpp)
target_link_libraries(my_basic ${OpenCV_LIBS})

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 3 Week 5: Mini VO 파이프라인")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
