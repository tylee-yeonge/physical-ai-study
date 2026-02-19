cmake_minimum_required(VERSION 3.10)
project(Phase3_Week10_g2o_Advanced)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# g2o
find_package(g2o REQUIRED)

# g2o 링크 라이브러리
set(G2O_LIBS
    Eigen3::Eigen
    g2o::core
    g2o::stuff
    g2o::types_sba
    g2o::solver_eigen
    g2o::solver_dense
)

# basic / my_basic (g2o 필요)
add_executable(basic basic.cpp)
target_link_libraries(basic ${G2O_LIBS})

add_executable(my_basic my_basic.cpp)
target_link_libraries(my_basic ${G2O_LIBS})

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 3 Week 10: g2o 심화")
message(STATUS "⚠️  빌드 환경: Ubuntu + g2o 필요")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
