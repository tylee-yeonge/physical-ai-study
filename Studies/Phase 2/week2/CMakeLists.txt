cmake_minimum_required(VERSION 3.10)
project(Phase2_Week2_CameraCalibration)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 컴파일 옵션
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

# OpenCV 찾기 (버전 4 이상 필요)
find_package(OpenCV 4 REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "Found OpenCV ${OpenCV_VERSION}")
    include_directories(${OpenCV_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "OpenCV not found!")
endif()

# Eigen3 찾기 (선택적)
find_package(Eigen3 3.3 QUIET)
if(Eigen3_FOUND)
    message(STATUS "Found Eigen ${EIGEN3_VERSION_STRING}")
    include_directories(${Eigen3_INCLUDE_DIRS})
endif()

# 기본 데모 실행 파일
add_executable(basic
    basic.cpp
)
target_link_libraries(basic ${OpenCV_LIBS})

add_executable(my_basic my_basic.cpp)
target_link_libraries(my_basic ${OpenCV_LIBS})

# 기초 퀴즈
add_executable(quiz_easy
    quiz_easy.cpp
)
target_link_libraries(quiz_easy ${OpenCV_LIBS})

# 중급 퀴즈
add_executable(quiz_medium
    quiz_medium.cpp
)
target_link_libraries(quiz_medium ${OpenCV_LIBS})

# Google Test (선택적)
find_package(GTest QUIET)
if(GTest_FOUND)
    message(STATUS "Found GTest - Building tests")
    enable_testing()
    
    add_executable(test_basic
        test/test_basic.cpp
        basic.cpp
    )
    target_compile_definitions(test_basic PRIVATE BASIC_LIB_ONLY)
    target_link_libraries(test_basic
        ${OpenCV_LIBS}
        GTest::GTest
        GTest::Main
    )
    
    add_test(NAME BasicTests COMMAND test_basic)

    # 내 풀이 테스트
    add_executable(test_my_basic
        test/test_basic.cpp
        my_basic.cpp
    )
    target_compile_definitions(test_my_basic PRIVATE MY_BASIC_LIB_ONLY)
    target_link_libraries(test_my_basic
        ${OpenCV_LIBS}
        GTest::GTest
        GTest::Main
    )
    add_test(NAME MyBasicTests COMMAND test_my_basic)
else()
    message(STATUS "GTest not found - Skipping tests")
endif()

# 설치 규칙
install(TARGETS basic quiz_easy quiz_medium
    RUNTIME DESTINATION bin
)

# 정보 출력
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Phase 2 Week 2: Camera Calibration")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
if(Eigen3_FOUND)
    message(STATUS "Eigen3 version: ${EIGEN3_VERSION_STRING}")
endif()
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
