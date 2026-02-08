cmake_minimum_required(VERSION 3.10)
project(Phase3_Week9_Schur)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

# Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Step 1: Schur Complement Demo (Eigen only)
add_executable(schur_demo schur_demo.cpp)

# Step 2: g2o option comparison (requires g2o)
find_package(G2O QUIET)
if(G2O_FOUND)
    include_directories(${G2O_INCLUDE_DIR})
    add_executable(compare_options compare_options.cpp)
    target_link_libraries(compare_options
        ${G2O_CORE_LIBRARY}
        ${G2O_STUFF_LIBRARY}
        ${G2O_TYPES_SBA}
        ${G2O_SOLVER_EIGEN}
    )
    message(STATUS "g2o found, compare_options will be built")
else()
    message(STATUS "g2o not found, skipping compare_options")
endif()
