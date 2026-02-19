cmake_minimum_required(VERSION 3.10)
project(Phase3_Week7_BA_g2o)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# g2o
find_package(g2o REQUIRED)

# basic / my_basic (g2o 필요)
add_executable(basic basic.cpp)
target_link_libraries(basic
    Eigen3::Eigen
    g2o::core
    g2o::stuff
    g2o::types_sba
    g2o::solver_eigen
)

add_executable(my_basic my_basic.cpp)
target_link_libraries(my_basic
    Eigen3::Eigen
    g2o::core
    g2o::stuff
    g2o::types_sba
    g2o::solver_eigen
)

# Quiz (g2o 불필요 — 개념 문제)
add_executable(quiz_easy quiz_easy.cpp)
add_executable(quiz_medium quiz_medium.cpp)

# Solution 빌드
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/quiz_solutions/easy_sol.cpp)
    add_executable(easy_sol quiz_solutions/easy_sol.cpp)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/quiz_solutions/medium_sol.cpp)
    add_executable(medium_sol quiz_solutions/medium_sol.cpp)
endif()

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 3 Week 7: Bundle Adjustment (g2o)")
message(STATUS "⚠️  빌드 환경: Ubuntu + g2o 필요")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
