cmake_minimum_required(VERSION 3.10)
project(Phase3_Week7_BA_g2o)

set(CMAKE_CXX_STANDARD 14)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# g2o
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(G2O REQUIRED)
include_directories(${G2O_INCLUDE_DIRS})

# Executable
add_executable(ba_demo
    simple_ba_demo.cpp
)

target_link_libraries(ba_demo
    g2o::core
    g2o::stuff
    g2o::types_sba
    g2o::solver_eigen
)
