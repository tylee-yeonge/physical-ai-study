cmake_minimum_required(VERSION 3.10)
project(Phase3_Week9_Schur)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

add_executable(basic basic.cpp)
target_link_libraries(basic Eigen3::Eigen)

add_executable(my_basic my_basic.cpp)
target_link_libraries(my_basic Eigen3::Eigen)

# Quizzes
add_executable(quiz_easy quiz_easy.cpp)
add_executable(quiz_medium quiz_medium.cpp)

# Quiz solutions
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/quiz_solutions/easy_sol.cpp)
    add_executable(easy_sol quiz_solutions/easy_sol.cpp)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/quiz_solutions/medium_sol.cpp)
    add_executable(medium_sol quiz_solutions/medium_sol.cpp)
endif()

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 3 Week 9: Schur Complement")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
