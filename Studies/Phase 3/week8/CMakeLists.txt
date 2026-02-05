cmake_minimum_required(VERSION 3.10)
project(Phase3_Week8_BA_Ceres)

set(CMAKE_CXX_STANDARD 14)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Ceres
find_package(Ceres REQUIRED)
include_directories(${CERES_INCLUDE_DIRS})

# Executable
add_executable(ceres_ba_demo
    simple_ceres_ba.cpp
)

target_link_libraries(ceres_ba_demo
    ${CERES_LIBRARIES}
)
